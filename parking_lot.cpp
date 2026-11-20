#include "parking_lot.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>

// ============================================================
// ParkingLot class implementation
// Core manager class demonstrating runtime polymorphism
// ============================================================

// Constructor: set up parking lot name, revenue, and create parking slots
ParkingLot::ParkingLot(const std::string& name)
    : lotName(name), totalRevenue(0.0) {

    // Initialize 15 parking slots with a realistic mix:
    //   5 Bike slots, 5 Car slots, 3 Truck slots, 2 Universal slots
    int slotCounter = 1;

    // Bike slots: 1-5
    for (int i = 0; i < 5; i++) {
        parkingSlots.emplace_back(slotCounter++, SlotType::BIKE_SLOT);
    }

    // Car slots: 6-10
    for (int i = 0; i < 5; i++) {
        parkingSlots.emplace_back(slotCounter++, SlotType::CAR_SLOT);
    }

    // Truck slots: 11-13
    for (int i = 0; i < 3; i++) {
        parkingSlots.emplace_back(slotCounter++, SlotType::TRUCK_SLOT);
    }

    // Universal slots: 14-15
    for (int i = 0; i < 2; i++) {
        parkingSlots.emplace_back(slotCounter++, SlotType::UNIVERSAL);
    }
}

ParkingLot::~ParkingLot() {
    // unique_ptr handles cleanup of Vehicle objects automatically
    // No manual delete needed - smart pointers do the work!
}

// ============================================================
// Private Helper: Find an available slot suitable for this vehicle
// Returns slot INDEX in vector, or -1 if none available
// ============================================================
int ParkingLot::findAvailableSlotIndex(const std::string& vehicleTypeStr) const {
    for (size_t i = 0; i < parkingSlots.size(); i++) {
        const ParkingSlot& slot = parkingSlots[i];
        // Slot must be both available AND compatible with vehicle type
        if (slot.getAvailability() && slot.canFitVehicle(vehicleTypeStr)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ============================================================
// Private Helper: Check if a registration number already exists
// Prevents duplicate vehicle entries
// ============================================================
bool ParkingLot::isDuplicateRegistration(const std::string& regNum) const {
    for (const auto& vehicle : parkedVehicles) {
        if (vehicle->getRegistrationNumber() == regNum) {
            return true;
        }
    }
    return false;
}

// ============================================================
// Private Helper: FACTORY METHOD pattern
// Creates the correct derived object (Car/Bike/Truck) based on type string
// Returns as unique_ptr<Vehicle> - this is where polymorphism is set up!
// ============================================================
std::unique_ptr<Vehicle> ParkingLot::createVehicleObject(
    const std::string& vehicleTypeStr,
    const std::string& regNum,
    int slotID
) const {
    // Depending on type, instantiate the correct DERIVED class
    // but return it stored in a BASE CLASS pointer (unique_ptr<Vehicle>)
    // This is the setup for runtime polymorphism!

    if (vehicleTypeStr == "Car") {
        return std::make_unique<Car>(regNum, slotID);
    } else if (vehicleTypeStr == "Bike") {
        return std::make_unique<Bike>(regNum, slotID);
    } else if (vehicleTypeStr == "Truck") {
        return std::make_unique<Truck>(regNum, slotID);
    }

    // Unknown vehicle type - return nullptr
    return nullptr;
}

// Overload: create vehicle object but with explicit entry time (persistence restore)
std::unique_ptr<Vehicle> ParkingLot::createVehicleObject(
    const std::string& vehicleTypeStr,
    const std::string& regNum,
    int slotID,
    const std::chrono::system_clock::time_point& entryTime
) const {
    if (vehicleTypeStr == "Car") {
        return std::make_unique<Car>(regNum, slotID, entryTime);
    } else if (vehicleTypeStr == "Bike") {
        return std::make_unique<Bike>(regNum, slotID, entryTime);
    } else if (vehicleTypeStr == "Truck") {
        return std::make_unique<Truck>(regNum, slotID, entryTime);
    }
    return nullptr;
}

// Mark a specific slot (by numeric ID) as occupied regardless of other logic.
// Returns true if slot was found and updated.
bool ParkingLot::occupySlotByID(int slotID) {
    for (auto& slot : parkingSlots) {
        if (slot.getSlotID() == slotID) {
            slot.occupy();
            return true;
        }
    }
    return false;
}

// ============================================================
// Register a vehicle entry into the parking lot
// Steps: validate input -> find slot -> create vehicle object -> store it
// ============================================================
bool ParkingLot::registerVehicle(const std::string& vehicleTypeStr,
                                 const std::string& registrationNumber) {

    // Input validation 1: check vehicle type is valid
    if (vehicleTypeStr != "Car" &&
        vehicleTypeStr != "Bike" &&
        vehicleTypeStr != "Truck") {
        std::cout << "  [ERROR] Invalid vehicle type: '" << vehicleTypeStr
                  << "'." << std::endl;
        std::cout << "          Valid types: Car, Bike, Truck" << std::endl;
        return false;
    }

    // Input validation 2: empty registration number
    if (registrationNumber.empty()) {
        std::cout << "  [ERROR] Registration number cannot be empty." << std::endl;
        return false;
    }

    // Input validation 3: duplicate check
    if (isDuplicateRegistration(registrationNumber)) {
        std::cout << "  [ERROR] Vehicle with registration number '"
                  << registrationNumber << "' is ALREADY parked!" << std::endl;
        return false;
    }

    // Find a suitable parking slot
    int slotIndex = findAvailableSlotIndex(vehicleTypeStr);
    if (slotIndex == -1) {
        std::cout << "  [SORRY] No suitable parking slots available for "
                  << vehicleTypeStr << "." << std::endl;
        std::cout << "          Please try again later or try a different lot."
                  << std::endl;
        return false;
    }

    // Get the slot and mark it as occupied
    ParkingSlot& slot = parkingSlots[slotIndex];
    slot.occupy();
    int slotID = slot.getSlotID();

    // ======================================
    // CREATE DERIVED OBJECT VIA BASE POINTER
    // This is where the polymorphic object is created!
    // ======================================
    std::unique_ptr<Vehicle> newVehicle =
        createVehicleObject(vehicleTypeStr, registrationNumber, slotID);

    if (!newVehicle) {
        // This shouldn't happen since we validated type earlier, but be safe
        slot.vacate(); // undo slot occupation
        return false;
    }

    // Store in the vector of base-class pointers
    // Note: std::move transfers ownership of the unique_ptr into the vector
    parkedVehicles.push_back(std::move(newVehicle));

    // Success message
    std::cout << "  [SUCCESS] " << vehicleTypeStr
              << " registered successfully!" << std::endl;
    std::cout << "          Reg. No: " << registrationNumber << std::endl;
    std::cout << "          Allocated Slot: #" << slotID
              << " (" << slot.getSlotTypeName() << ")" << std::endl;

    return true;
}

// ============================================================
// Release a vehicle (exit parking), calculate fee, collect payment
// ============================================================
bool ParkingLot::releaseVehicle(const std::string& registrationNumber) {

    if (registrationNumber.empty()) {
        std::cout << "  [ERROR] Registration number cannot be empty." << std::endl;
        return false;
    }

    // Find the vehicle in our parked vehicles list
    // Use manual loop to get the index (we need to erase from vector)
    int foundIndex = -1;
    for (size_t i = 0; i < parkedVehicles.size(); i++) {
        if (parkedVehicles[i]->getRegistrationNumber() == registrationNumber) {
            foundIndex = static_cast<int>(i);
            break;
        }
    }

    if (foundIndex == -1) {
        std::cout << "  [NOT FOUND] No vehicle with registration number '"
                  << registrationNumber << "' is currently parked." << std::endl;
        return false;
    }

    // Get a reference to the vehicle object (through base class pointer!)
    Vehicle* vehicle = parkedVehicles[foundIndex].get();

    // ======================================
    //   RUNTIME POLYMORPHISM IN ACTION!
    // ======================================
    // vehicle is a Vehicle* (base class pointer)
    // But when we call ->calculateFee(), the CORRECT derived class
    // version runs (Car::calculateFee, Bike::calculateFee, OR Truck::calculateFee)
    // depending on what the ACTUAL object type is!
    // This decision happens at RUNTIME, not compile time - that's polymorphism!
    double fee = vehicle->calculateFee();
    std::string vehicleType = vehicle->getTypeName();
    int slotID = vehicle->getParkingSlotID();
    auto entryTime = vehicle->getEntryTime();
    double duration = vehicle->calculateDurationHours();
    auto exitTime = std::chrono::system_clock::now();

    // Free up the parking slot
    for (auto& slot : parkingSlots) {
        if (slot.getSlotID() == slotID) {
            slot.vacate();
            break;
        }
    }

    // Record this exit in parking history
    ParkingRecord record;
    record.registrationNumber = registrationNumber;
    record.vehicleType = vehicleType;
    record.slotID = slotID;
    record.entryTime = entryTime;
    record.exitTime = exitTime;
    record.durationHours = duration;
    record.feePaid = fee;
    parkingHistory.push_back(record);

    // Add to total revenue
    totalRevenue += fee;

    // Print exit receipt
    std::cout << "  ===============================" << std::endl;
    std::cout << "           EXIT RECEIPT          " << std::endl;
    std::cout << "  ===============================" << std::endl;

    std::time_t entryT = std::chrono::system_clock::to_time_t(entryTime);
    std::time_t exitT = std::chrono::system_clock::to_time_t(exitTime);
    char entryBuf[100], exitBuf[100];
    std::strftime(entryBuf, sizeof(entryBuf), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&entryT));
    std::strftime(exitBuf, sizeof(exitBuf), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&exitT));

    std::cout << "  Vehicle Type:      " << vehicleType << std::endl;
    std::cout << "  Registration No:   " << registrationNumber << std::endl;
    std::cout << "  Slot ID:           #" << slotID << std::endl;
    std::cout << "  Entry Time:        " << entryBuf << std::endl;
    std::cout << "  Exit Time:         " << exitBuf << std::endl;
    std::cout << "  Duration:          " << std::fixed << std::setprecision(2)
              << duration << " hours" << std::endl;
    std::cout << "  Parking Fee:       Rs. " << std::fixed
              << std::setprecision(2) << fee << std::endl;
    std::cout << "  ===============================" << std::endl;
    std::cout << "  [THANK YOU] Please drive safely!" << std::endl;

    // Remove the vehicle from parked list (unique_ptr handles delete automatically)
    parkedVehicles.erase(parkedVehicles.begin() + foundIndex);

    return true;
}

// ============================================================
// Search for a parked vehicle by registration number
// ============================================================
Vehicle* ParkingLot::searchVehicle(const std::string& registrationNumber) const {
    if (registrationNumber.empty()) {
        std::cout << "  [ERROR] Registration number cannot be empty." << std::endl;
        return nullptr;
    }

    for (const auto& vehicle : parkedVehicles) {
        if (vehicle->getRegistrationNumber() == registrationNumber) {
            // Return raw pointer; caller must NOT delete this!
            return vehicle.get();
        }
    }
    return nullptr;
}

// ============================================================
// Display all parking slots and their statuses
// ============================================================
void ParkingLot::displayAllSlots() const {
    std::cout << "\n  ===== PARKING SLOT STATUS (" << lotName << ") ====="
              << std::endl;
    for (const auto& slot : parkingSlots) {
        slot.displaySlotInfo();
    }
    std::cout << "  ================================================="
              << std::endl;
    std::cout << "  Available: " << getAvailableSlotsCount() << " / "
              << getTotalSlots() << " slots" << std::endl;
}

// ============================================================
// Display all currently parked vehicles
// This demonstrates POLYMORPHISM: we loop through Vehicle* pointers,
// and call getTypeName()/calculateFee() which resolve to the correct
// derived class implementations AT RUNTIME!
// ============================================================
void ParkingLot::displayParkedVehicles() const {
    std::cout << "\n  ===== CURRENTLY PARKED VEHICLES ("
              << lotName << ") =====" << std::endl;

    if (parkedVehicles.empty()) {
        std::cout << "  (No vehicles currently parked)" << std::endl;
        return;
    }

    std::cout << "  Total vehicles parked: " << parkedVehicles.size()
              << std::endl;
    std::cout << "  ------------------------------------------------" << std::endl;

    int counter = 1;
    for (const auto& vehicle : parkedVehicles) {
        std::cout << "  Vehicle #" << counter << ":" << std::endl;

        // POLYMORPHIC CALLS: vehicle is unique_ptr<Vehicle> (base)
        // But these methods dispatch to the actual derived class!
        vehicle->displayDetails();
        std::cout << "  Hourly Rate:       Rs. " << std::fixed
                  << std::setprecision(2)
                  << vehicle->getHourlyRate() << " (Polymorphic rate)"
                  << std::endl;
        std::cout << std::endl;
        counter++;
    }

    std::cout << "  ================================================="
              << std::endl;
}

// ============================================================
// Display total revenue collected
// ============================================================
void ParkingLot::displayTotalRevenue() const {
    std::cout << "\n  ===== REVENUE SUMMARY (" << lotName << ") =====" << std::endl;
    std::cout << "  Total vehicles served (exited): "
              << parkingHistory.size() << std::endl;
    std::cout << "  Total Revenue Collected: Rs. "
              << std::fixed << std::setprecision(2) << totalRevenue
              << std::endl;
    std::cout << "  =================================================" << std::endl;
}

// ============================================================
// Display complete parking history (all vehicles that exited)
// ============================================================
void ParkingLot::displayParkingHistory() const {
    std::cout << "\n  ===== PARKING HISTORY (" << lotName << ") =====" << std::endl;

    if (parkingHistory.empty()) {
        std::cout << "  (No history yet - no vehicles have exited)" << std::endl;
        std::cout << "  ================================================="
                  << std::endl;
        return;
    }

    std::cout << "  Total historical records: " << parkingHistory.size()
              << std::endl;
    std::cout << "  ------------------------------------------------" << std::endl;

    int counter = 1;
    for (const auto& rec : parkingHistory) {
        std::time_t entryT = std::chrono::system_clock::to_time_t(rec.entryTime);
        std::time_t exitT = std::chrono::system_clock::to_time_t(rec.exitTime);
        char entryBuf[100], exitBuf[100];
        std::strftime(entryBuf, sizeof(entryBuf), "%Y-%m-%d %H:%M:%S",
                      std::localtime(&entryT));
        std::strftime(exitBuf, sizeof(exitBuf), "%Y-%m-%d %H:%M:%S",
                      std::localtime(&exitT));

        std::cout << "  Record #" << counter << ":" << std::endl;
        std::cout << "    Type:       " << rec.vehicleType << std::endl;
        std::cout << "    Reg. No:    " << rec.registrationNumber << std::endl;
        std::cout << "    Slot:       #" << rec.slotID << std::endl;
        std::cout << "    Entry:      " << entryBuf << std::endl;
        std::cout << "    Exit:       " << exitBuf << std::endl;
        std::cout << "    Duration:   " << std::fixed << std::setprecision(2)
                  << rec.durationHours << " hrs" << std::endl;
        std::cout << "    Fee Paid:   Rs. " << std::fixed << std::setprecision(2)
                  << rec.feePaid << std::endl;
        std::cout << std::endl;
        counter++;
    }

    std::cout << "  Total revenue from history: Rs. "
              << std::fixed << std::setprecision(2) << totalRevenue << std::endl;
    std::cout << "  =================================================" << std::endl;
}

// ============================================================
// Quick summary dashboard
// ============================================================
void ParkingLot::displaySummary() const {
    std::cout << "\n  =================================================" << std::endl;
    std::cout << "        " << lotName << " - SYSTEM SUMMARY" << std::endl;
    std::cout << "  =================================================" << std::endl;
    std::cout << "  Total Slots:        " << getTotalSlots() << std::endl;
    std::cout << "  Occupied Slots:     " << getOccupiedSlotsCount() << std::endl;
    std::cout << "  Available Slots:    " << getAvailableSlotsCount() << std::endl;
    std::cout << "  Parked Vehicles:    " << getParkedVehiclesCount() << std::endl;
    std::cout << "  Served (Exited):    " << parkingHistory.size() << std::endl;
    std::cout << "  Total Revenue:      Rs. " << std::fixed
              << std::setprecision(2) << totalRevenue << std::endl;
    std::cout << "  =================================================" << std::endl;
}

// ===== Simple Getter Methods =====
int ParkingLot::getTotalSlots() const { return static_cast<int>(parkingSlots.size()); }

int ParkingLot::getAvailableSlotsCount() const {
    int count = 0;
    for (const auto& slot : parkingSlots) {
        if (slot.getAvailability()) count++;
    }
    return count;
}

int ParkingLot::getOccupiedSlotsCount() const {
    return getTotalSlots() - getAvailableSlotsCount();
}

int ParkingLot::getParkedVehiclesCount() const {
    return static_cast<int>(parkedVehicles.size());
}

double ParkingLot::getTotalRevenue() const { return totalRevenue; }

std::string ParkingLot::getLotName() const { return lotName; }


// ============================================================
// API / Enumeration accessors (used by HTTP server)
// ============================================================

Vehicle* ParkingLot::getParkedVehicleAt(int index) const {
    if (index < 0 || index >= static_cast<int>(parkedVehicles.size())) return nullptr;
    return parkedVehicles[index].get();
}

const ParkingSlot* ParkingLot::getSlotAt(int index) const {
    if (index < 0 || index >= static_cast<int>(parkingSlots.size())) return nullptr;
    return &parkingSlots[index];
}

int ParkingLot::getHistoryCount() const {
    return static_cast<int>(parkingHistory.size());
}

bool ParkingLot::getHistoryAt(int index, ParkingRecord& outRecord) const {
    if (index < 0 || index >= static_cast<int>(parkingHistory.size())) return false;
    outRecord = parkingHistory[index];
    return true;
}

int ParkingLot::getParkedCountByType(const std::string& typeName) const {
    int count = 0;
    for (const auto& v : parkedVehicles) {
        if (v->getTypeName() == typeName) count++;
    }
    return count;
}

void ParkingLot::addHistoryRecord(const ParkingRecord& record) {
    parkingHistory.push_back(record);
}

void ParkingLot::setTotalRevenue(double value) {
    totalRevenue = value;
}

// Directly restore a parked vehicle (used during persistence restore).
// Bypasses the normal "find first available" slot search; uses the exact slot ID.
bool ParkingLot::restoreParkedVehicle(const std::string& vehicleTypeStr,
                                      const std::string& regNum,
                                      int slotID,
                                      const std::chrono::system_clock::time_point& entry) {
    if (vehicleTypeStr != "Car" && vehicleTypeStr != "Bike" && vehicleTypeStr != "Truck") {
        return false;
    }
    if (regNum.empty()) return false;
    if (isDuplicateRegistration(regNum)) return false;

    // Make sure the slot exists and mark it occupied
    if (!occupySlotByID(slotID)) return false;

    auto veh = createVehicleObject(vehicleTypeStr, regNum, slotID, entry);
    if (!veh) {
        // Rollback slot occupancy on failure
        for (auto& s : parkingSlots) {
            if (s.getSlotID() == slotID) { s.vacate(); break; }
        }
        return false;
    }
    parkedVehicles.push_back(std::move(veh));
    return true;
}


// ============================================================
// Persistence - save / load state to disk (simple line-delimited format)
// Format is intentionally simple and parser-friendly, no external JSON lib needed.
//   SECTION:HEADER
//   field1|field2|field3|...
// ============================================================

#include <fstream>
#include <sstream>

// Helper: convert time_point to epoch milliseconds stored in a uint64_t string
static std::string timePointToEpochStr(const std::chrono::system_clock::time_point& tp) {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  tp.time_since_epoch()).count();
    return std::to_string(ms);
}

// Helper: parse epoch milliseconds string back to time_point
static std::chrono::system_clock::time_point epochStrToTimePoint(const std::string& s) {
    long long ms = 0;
    try { ms = std::stoll(s); } catch (...) { ms = 0; }
    return std::chrono::system_clock::time_point(
               std::chrono::duration_cast<std::chrono::system_clock::duration>(
                   std::chrono::milliseconds(ms)));
}

// Escape pipe characters in a single field for the simple save format
static std::string escapeField(std::string s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '|') out += "\\p";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

// Unescape a single field read from disk
static std::string unescapeField(std::string s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[i + 1];
            if (n == '\\') { out += '\\'; i++; }
            else if (n == 'p') { out += '|'; i++; }
            else if (n == 'n') { out += '\n'; i++; }
            else out += s[i];
        } else {
            out += s[i];
        }
    }
    return out;
}

// Split a string by the '|' delimiter respecting backslash escapes
static std::vector<std::string> splitLineFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '|') { fields.push_back(unescapeField(cur)); cur.clear(); }
        else if (c == '\\' && i + 1 < line.size()) {
            cur.push_back(c); cur.push_back(line[++i]);
        } else {
            cur.push_back(c);
        }
    }
    fields.push_back(unescapeField(cur));
    return fields;
}

// Save everything to disk
bool ParkingLot::saveStateToFile(const std::string& filePath) const {
    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) return false;

    ofs << "#SMARTPARK_STATE_V1\n";
    ofs << "LOT_NAME|" << escapeField(lotName) << "\n";
    ofs << "REVENUE|" << std::fixed << std::setprecision(6) << totalRevenue << "\n";

    ofs << "[PARKED_VEHICLES]\n";
    for (const auto& v : parkedVehicles) {
        ofs << "V|" << escapeField(v->getTypeName())
            << "|" << escapeField(v->getRegistrationNumber())
            << "|" << v->getParkingSlotID()
            << "|" << timePointToEpochStr(v->getEntryTime()) << "\n";
    }

    ofs << "[HISTORY]\n";
    for (const auto& r : parkingHistory) {
        ofs << "H|" << escapeField(r.vehicleType)
            << "|" << escapeField(r.registrationNumber)
            << "|" << r.slotID
            << "|" << timePointToEpochStr(r.entryTime)
            << "|" << timePointToEpochStr(r.exitTime)
            << "|" << std::fixed << std::setprecision(6) << r.durationHours
            << "|" << std::fixed << std::setprecision(6) << r.feePaid << "\n";
    }
    return true;
}

// Load everything back from disk
bool ParkingLot::loadStateFromFile(const std::string& filePath) {
    std::ifstream ifs(filePath);
    if (!ifs.is_open()) return false;

    // Clear in-memory state first
    parkedVehicles.clear();
    parkingHistory.clear();
    totalRevenue = 0.0;
    // Reset all slots to available
    for (auto& s : parkingSlots) s.vacate();

    std::string line;
    enum class Section { NONE, PARKED, HISTORY };
    Section section = Section::NONE;
    bool headerOK = false;

    while (std::getline(ifs, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') {
            if (line == "#SMARTPARK_STATE_V1") headerOK = true;
            continue;
        }
        if (line == "[PARKED_VEHICLES]") { section = Section::PARKED; continue; }
        if (line == "[HISTORY]") { section = Section::HISTORY; continue; }

        if (section == Section::NONE) {
            auto fields = splitLineFields(line);
            if (fields.empty()) continue;
            if (fields[0] == "LOT_NAME" && fields.size() >= 2) lotName = fields[1];
            else if (fields[0] == "REVENUE" && fields.size() >= 2) {
                try { totalRevenue = std::stod(fields[1]); } catch (...) { totalRevenue = 0.0; }
            }
        } else if (section == Section::PARKED) {
            auto f = splitLineFields(line);
            if (f.size() >= 5 && f[0] == "V") {
                std::string type = f[1];
                std::string reg  = f[2];
                int slotID = 0;
                try { slotID = std::stoi(f[3]); } catch (...) { continue; }
                auto entry = epochStrToTimePoint(f[4]);
                restoreParkedVehicle(type, reg, slotID, entry);
            }
        } else if (section == Section::HISTORY) {
            auto f = splitLineFields(line);
            if (f.size() >= 8 && f[0] == "H") {
                ParkingRecord r;
                r.vehicleType = f[1];
                r.registrationNumber = f[2];
                try { r.slotID = std::stoi(f[3]); } catch (...) { r.slotID = 0; }
                r.entryTime = epochStrToTimePoint(f[4]);
                r.exitTime  = epochStrToTimePoint(f[5]);
                try { r.durationHours = std::stod(f[6]); } catch (...) { r.durationHours = 0; }
                try { r.feePaid = std::stod(f[7]); } catch (...) { r.feePaid = 0; }
                parkingHistory.push_back(r);
            }
        }
    }
    return headerOK;
}

