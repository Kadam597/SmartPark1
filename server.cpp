// ============================================================
// SmartPark HTTP Backend Server (cpp-httplib)
//
// This is the SINGLE SOURCE OF TRUTH for ALL business logic:
//   - Registration  -> calls ParkingLot::registerVehicle()
//   - Exit/fee calc -> calls ParkingLot::releaseVehicle() (polymorphic!)
//   - All duration + fee logic lives in Vehicle::calculateDurationHours()
//     and the OVERRIDDEN Vehicle::calculateFee() methods (runtime polymorphism)
//
// Frontend is a pure client: it fetches JSON, displays it, and sends user
// input back. No parking calculations are duplicated in JavaScript.
// ============================================================

#define CPPHTTPLIB_IMPLEMENTATION
#include "httplib.h"

#include "parking_lot.h"
#include "parking_api.h"

#include <iostream>
#include <string>
#include <mutex>
#include <filesystem>
#include <thread>
#include <chrono>
#include <cstdlib>

// Protects the ParkingLot instance from concurrent HTTP requests modifying it
std::mutex lotMutex;

// Single persistence file path (placed next to the executable / project folder)
const std::string STATE_FILE = "smartpark_state.dat";
const std::string WEB_ROOT = "web";

// ============================================================
// Tiny hand-written JSON field extractor (flat objects only).
// Avoids std::regex which is unreliable across compilers for this use case.
// Handles string values, bare numbers, booleans, null.
// ============================================================
#include <cctype>
static std::string extractJsonField(const std::string& json, const std::string& key) {
    size_t pos = 0;
    const size_t N = json.size();

    while (pos < N) {
        // Skip whitespace
        while (pos < N && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;
        if (pos >= N) break;

        // Expect start of key: opening quote
        if (json[pos] != '"') { pos++; continue; }
        pos++; // skip opening "

        // Read the key name
        std::string curKey;
        while (pos < N && json[pos] != '"') {
            if (json[pos] == '\\' && pos + 1 < N) {
                curKey.push_back(json[pos + 1]);
                pos += 2;
            } else {
                curKey.push_back(json[pos]);
                pos++;
            }
        }
        if (pos >= N) break;
        pos++; // skip closing "

        // Skip whitespace
        while (pos < N && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;
        if (pos >= N || json[pos] != ':') continue; // not a key:value pair?
        pos++; // skip :

        // Skip whitespace
        while (pos < N && std::isspace(static_cast<unsigned char>(json[pos]))) pos++;
        if (pos >= N) break;

        if (curKey == key) {
            // FOUND. Now read value:
            if (json[pos] == '"') {
                // String value
                pos++;
                std::string val;
                while (pos < N && json[pos] != '"') {
                    if (json[pos] == '\\' && pos + 1 < N) {
                        char c = json[pos + 1];
                        switch (c) {
                            case 'n': val.push_back('\n'); break;
                            case 't': val.push_back('\t'); break;
                            case 'r': val.push_back('\r'); break;
                            case '\"': val.push_back('\"'); break;
                            case '\\': val.push_back('\\'); break;
                            default: val.push_back(c);
                        }
                        pos += 2;
                    } else {
                        val.push_back(json[pos]);
                        pos++;
                    }
                }
                return val;
            }
            // Number / boolean / null
            size_t vStart = pos;
            while (pos < N && json[pos] != ',' && json[pos] != '}' &&
                   !std::isspace(static_cast<unsigned char>(json[pos]))) {
                pos++;
            }
            return json.substr(vStart, pos - vStart);
        } else {
            // Skip this unknown value (string or bare)
            if (json[pos] == '"') {
                pos++;
                while (pos < N && json[pos] != '"') {
                    if (json[pos] == '\\' && pos + 1 < N) pos += 2;
                    else pos++;
                }
                if (pos < N) pos++;
            } else {
                while (pos < N && json[pos] != ',' && json[pos] != '}') pos++;
            }
        }
        // Skip comma if present
        if (pos < N && json[pos] == ',') pos++;
    }
    return "";
}

// Save the parking lot safely to disk, under the mutex.
void persistState(const ParkingLot& lot) {
    lot.saveStateToFile(STATE_FILE);
}

// Load state from disk at startup (returns false if file doesn't exist yet)
bool restoreState(ParkingLot& lot) {
    std::error_code ec;
    if (!std::filesystem::exists(STATE_FILE, ec)) return false;
    return lot.loadStateFromFile(STATE_FILE);
}

// ============================================================
// main()
// Sets up HTTP routes, loads any saved state, and begins listening on port 8080.
// ============================================================
int main() {
    ParkingLot lot("SmartPark Main Campus Lot");

    // Try to restore previous session from disk
    if (restoreState(lot)) {
        std::cout << "[SmartPark] Restored saved state from " << STATE_FILE << std::endl;
        std::cout << "[SmartPark] Parked: " << lot.getParkedVehiclesCount()
                  << "  Revenue: Rs. " << lot.getTotalRevenue() << std::endl;
    } else {
        std::cout << "[SmartPark] No save file found. Starting with empty lot." << std::endl;
    }

    httplib::Server server;

    // CORS headers - allow the frontend to be opened via file:// OR different port
    auto setCORS = [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Cache-Control", "no-store");
    };

    server.set_pre_routing_handler([setCORS](const httplib::Request& req, httplib::Response& res) {
        setCORS(req, res);
        if (req.method == "OPTIONS") {
            res.status = httplib::StatusCode::OK_200;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // --------------------------------------------------------
    // GET /api/health - simple heartbeat
    // --------------------------------------------------------
    server.Get("/api/health", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(lotMutex);
        std::string data = "{\"status\":\"ok\",\"lotName\":\"" + jsonEscape(lot.getLotName()) + "\"}";
        res.set_content(makeJsonResponse(true, "Server running", data), "application/json");
    });

    // --------------------------------------------------------
    // GET /api/dashboard - statistics for dashboard cards
    // --------------------------------------------------------
    server.Get("/api/dashboard", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(lotMutex);
        std::string dashboard = buildDashboardJson(lot);
        res.set_content(makeJsonResponse(true, "Dashboard summary", dashboard), "application/json");
    });

    // --------------------------------------------------------
    // GET /api/slots - parking slot grid with occupancy info
    // --------------------------------------------------------
    server.Get("/api/slots", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(lotMutex);
        std::string slots = buildSlotsJson(lot);
        res.set_content(makeJsonResponse(true, "Slot list", slots), "application/json");
    });

    // --------------------------------------------------------
    // GET /api/vehicles - currently parked vehicles list
    // --------------------------------------------------------
    server.Get("/api/vehicles", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(lotMutex);
        std::string list = buildParkedJson(lot);
        res.set_content(makeJsonResponse(true, "Parked vehicles", list), "application/json");
    });

    // --------------------------------------------------------
    // POST /api/vehicles/register - park a new vehicle
    // Body JSON: {"type":"Car","registrationNumber":"MH-12-AB-1234"}
    // --------------------------------------------------------
    server.Post("/api/vehicles/register", [&](const httplib::Request& req, httplib::Response& res) {
        std::string type = extractJsonField(req.body, "type");
        std::string reg  = extractJsonField(req.body, "registrationNumber");

        // Backend validation - frontend validation is NOT trusted
        if (type != "Car" && type != "Bike" && type != "Truck") {
            res.status = httplib::StatusCode::BadRequest_400;
            res.set_content(makeJsonResponse(false, "Invalid vehicle type. Valid: Car, Bike, Truck"), "application/json");
            return;
        }
        if (reg.empty()) {
            res.status = httplib::StatusCode::BadRequest_400;
            res.set_content(makeJsonResponse(false, "Registration number cannot be empty"), "application/json");
            return;
        }

        int slotID = 0;
        std::string message;
        bool ok = false;
        {
            std::lock_guard<std::mutex> lock(lotMutex);
            // Call the EXACT SAME core logic as the console app!
            ok = lot.registerVehicle(type, reg);
            if (ok) {
                // Find the slot that was assigned (it's stored on the vehicle itself)
                Vehicle* v = lot.searchVehicle(reg);
                if (v) {
                    extractIntField(v->getParkingSlotID(), "slotID", slotID);
                }
                message = type + " registered successfully";
                // Persist to disk after successful operation
                persistState(lot);
            } else {
                // Distinguish two common errors
                if (lot.searchVehicle(reg) != nullptr) {
                    message = "Duplicate registration - vehicle already parked";
                } else {
                    message = "No suitable slot available for " + type;
                }
            }
        }

        if (!ok) {
            res.status = httplib::StatusCode::Conflict_409;
            if (message.find("No suitable") == 0) {
                res.status = httplib::StatusCode::ServiceUnavailable_503;
            }
            res.set_content(makeJsonResponse(false, message), "application/json");
            return;
        }

        // Build response with details + slot allocated + current fee
        std::ostringstream data;
        Vehicle* fresh = nullptr;
        {
            std::lock_guard<std::mutex> lock(lotMutex);
            fresh = lot.searchVehicle(reg);
        }
        if (fresh) {
            data << parkedVehicleToJson(*fresh);
        } else {
            data << "{\"slotID\":" << slotID << ",\"registrationNumber\":\"" << jsonEscape(reg) << "\"}";
        }
        res.status = httplib::StatusCode::Created_201;
        res.set_content(makeJsonResponse(true, message, data.str()), "application/json");
    });

    // --------------------------------------------------------
    // GET /api/vehicles/{number} - look up one parked vehicle
    // --------------------------------------------------------
    server.Get(R"(/api/vehicles/(.+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string reg = req.matches[1].str();
        Vehicle* v = nullptr;
        {
            std::lock_guard<std::mutex> lock(lotMutex);
            v = lot.searchVehicle(reg);
        }
        if (!v) {
            res.status = httplib::StatusCode::NotFound_404;
            res.set_content(makeJsonResponse(false, "Vehicle not found in currently parked list"), "application/json");
            return;
        }
        std::lock_guard<std::mutex> lock(lotMutex);
        res.set_content(makeJsonResponse(true, "Vehicle found", parkedVehicleToJson(*v)), "application/json");
    });

    // --------------------------------------------------------
    // POST /api/vehicles/exit - release a vehicle
    // Body JSON: {"registrationNumber":"MH-12-AB-1234"}
    //
    // The fee calculation call here is the RUNTIME POLYMORPHISM entry point.
    // releaseVehicle() internally calls vehicle->calculateFee() on a
    // Vehicle* pointer; the correct Car/Bike/Truck overridden version
    // is automatically dispatched at runtime.
    // --------------------------------------------------------
    server.Post("/api/vehicles/exit", [&](const httplib::Request& req, httplib::Response& res) {
        std::string reg = extractJsonField(req.body, "registrationNumber");
        if (reg.empty()) {
            res.status = httplib::StatusCode::BadRequest_400;
            res.set_content(makeJsonResponse(false, "Registration number required"), "application/json");
            return;
        }

        // Collect data BEFORE releasing (to have entry time / type info)
        ParkingRecord exitRecord;
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(lotMutex);
            Vehicle* v = lot.searchVehicle(reg);
            if (!v) {
                res.status = httplib::StatusCode::NotFound_404;
                res.set_content(makeJsonResponse(false, "Vehicle with that registration is not parked"), "application/json");
                return;
            }
            exitRecord.registrationNumber = reg;
            exitRecord.vehicleType = v->getTypeName();
            int sid = 0;
            extractIntField(v->getParkingSlotID(), "slotID", sid);
            exitRecord.slotID = sid;
            exitRecord.entryTime = v->getEntryTime();
            exitRecord.durationHours = v->calculateDurationHours();
            // THIS IS THE POLYMORPHIC FEE CALCULATION:
            exitRecord.feePaid = v->calculateFee();
            found = true;
        }

        bool ok = false;
        if (found) {
            std::lock_guard<std::mutex> lock(lotMutex);
            ok = lot.releaseVehicle(reg);  // will re-calculate internally too
            if (ok) {
                exitRecord.exitTime = std::chrono::system_clock::now();
                // Ensure our exitRecord matches what ParkingLot.history stored:
                // re-read fee and duration from the history record to be accurate.
                int hc = lot.getHistoryCount();
                for (int i = hc - 1; i >= 0; i--) {
                    ParkingRecord stored;
                    if (lot.getHistoryAt(i, stored) && stored.registrationNumber == reg) {
                        exitRecord = stored;
                        break;
                    }
                }
                persistState(lot);
            }
        }

        if (!ok) {
            res.status = httplib::StatusCode::InternalServerError_500;
            res.set_content(makeJsonResponse(false, "Failed to release vehicle"), "application/json");
            return;
        }

        std::ostringstream data;
        long long e1 = 0, e2 = 0;
        extractEpoch(exitRecord.entryTime, "entry", e1);
        extractEpoch(exitRecord.exitTime, "exit", e2);
        data << "{";
        data << "\"registrationNumber\":\"" << jsonEscape(exitRecord.registrationNumber) << "\",";
        data << "\"type\":\"" << jsonEscape(exitRecord.vehicleType) << "\",";
        data << "\"slotID\":" << exitRecord.slotID << ",";
        data << "\"entryTimeEpochMs\":" << e1 << ",";
        data << "\"exitTimeEpochMs\":" << e2 << ",";
        data << "\"durationHours\":" << jsonDouble(exitRecord.durationHours, 4) << ",";
        data << "\"feePaid\":" << jsonDouble(exitRecord.feePaid, 2) << ",";
        data << "\"revenue\":" << jsonDouble(lot.getTotalRevenue(), 2);
        data << "}";

        res.set_content(makeJsonResponse(true, "Vehicle exited successfully", data.str()), "application/json");
    });

    // --------------------------------------------------------
    // GET /api/history - parking history (exited vehicles)
    // --------------------------------------------------------
    server.Get("/api/history", [&](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(lotMutex);
        std::string h = buildHistoryJson(lot);
        // Also include parked vehicles as "active" records so history page can show both
        std::ostringstream data;
        data << "{";
        data << "\"completed\":" << h << ",";
        data << "\"active\":" << buildParkedJson(lot);
        data << "}";
        res.set_content(makeJsonResponse(true, "Parking history", data.str()), "application/json");
    });

    // --------------------------------------------------------
    // Static file serving: serve the web/ directory on "/"
    // Automatically detects "web", "Web", or current directory
    // --------------------------------------------------------
    std::string webDir = "web";
    std::error_code ec;
    if (!std::filesystem::exists(webDir, ec) || !std::filesystem::is_directory(webDir, ec)) {
        if (std::filesystem::exists("Web", ec) && std::filesystem::is_directory("Web", ec)) {
            webDir = "Web";
        } else if (std::filesystem::exists("index.html", ec)) {
            webDir = ".";
        }
    }

    if (std::filesystem::exists(webDir, ec) && std::filesystem::is_directory(webDir, ec)) {
        server.set_mount_point("/", webDir.c_str());
        std::cout << "[SmartPark] Serving static frontend from: " << webDir << std::endl;
    } else {
        std::cout << "[SmartPark] WARNING: '" << webDir << "' directory not found for static serving." << std::endl;
        std::cout << "[SmartPark] You can still open web/index.html directly in your browser." << std::endl;
    }

    // Root fallback: redirect "/" to index.html if mounted
    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_redirect("/index.html");
    });

    // --------------------------------------------------------
    // Start the server (bind to HOST and PORT from environment if available)
    // --------------------------------------------------------
    const char* hostEnv = std::getenv("HOST");
    std::string HOST = hostEnv ? hostEnv : "0.0.0.0";

    const char* portEnv = std::getenv("PORT");
    int PORT = 8080;
    if (portEnv) {
        try {
            PORT = std::stoi(portEnv);
        } catch (...) {
            PORT = 8080;
        }
    }

    std::cout << "=================================================" << std::endl;
    std::cout << "  SmartPark Backend Server - starting..." << std::endl;
    std::cout << "  C++ Core Logic: ParkingLot + Vehicle (polymorphic)" << std::endl;
    std::cout << "  Listening on: http://" << HOST << ":" << PORT << std::endl;
    std::cout << "  Dashboard URL: http://" << HOST << ":" << PORT << "/" << std::endl;
    std::cout << "  API health :   http://" << HOST << ":" << PORT << "/api/health" << std::endl;
    std::cout << "  Press Ctrl+C to stop." << std::endl;
    std::cout << "=================================================" << std::endl;

    // Auto-save every 30 seconds in case of abnormal shutdown
    std::thread autosave([&]() {
        while (server.is_running()) {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            if (!server.is_running()) break;
            std::lock_guard<std::mutex> lock(lotMutex);
            lot.saveStateToFile(STATE_FILE);
        }
    });

    server.listen(HOST.c_str(), PORT);

    // Final save + cleanup on shutdown
    {
        std::lock_guard<std::mutex> lock(lotMutex);
        lot.saveStateToFile(STATE_FILE);
    }
    if (autosave.joinable()) autosave.join();

    std::cout << "[SmartPark] Server stopped. State saved to " << STATE_FILE << std::endl;
    return 0;
}
