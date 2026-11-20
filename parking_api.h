#ifndef PARKING_API_H
#define PARKING_API_H

#include "parking_lot.h"
#include "vehicle.h"

#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>

// ============================================================
// API extraction helpers
// Per project requirements, BOTH helpers accept EXACTLY 3 parameters.
// ============================================================

// extractEpoch - 3 parameters
//   Param 1 (in):  the parking record or vehicle time_point to read
//   Param 2 (in):  field name (string) - informative, can be used for error logging
//   Param 3 (out): reference to a long long that receives the epoch milliseconds value
// Returns true on success, false otherwise.
inline bool extractEpoch(const std::chrono::system_clock::time_point& timePoint,
                         const std::string& fieldName,
                         long long& outEpochMs) {
    (void)fieldName; // parameter reserved for diagnostics / logging
    try {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      timePoint.time_since_epoch()).count();
        outEpochMs = static_cast<long long>(ms);
        return true;
    } catch (...) {
        outEpochMs = 0;
        return false;
    }
}

// extractIntField - 3 parameters
//   Param 1 (in):  the integer value from any object (slot ID, counts, etc.)
//   Param 2 (in):  field name (string) - informative, for diagnostics
//   Param 3 (out): reference to an int that receives the value
// Returns true on success, false otherwise.
inline bool extractIntField(int value,
                            const std::string& fieldName,
                            int& outValue) {
    (void)fieldName; // parameter reserved for diagnostics / logging
    outValue = value;
    return true;
}

// ============================================================
// JSON string helpers
// Builds JSON strings manually to avoid external JSON library dependency.
// ============================================================

// Escape a string for embedding into a JSON string literal
inline std::string jsonEscape(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '"':  o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b";  break;
            case '\f': o << "\\f";  break;
            case '\n': o << "\\n";  break;
            case '\r': o << "\\r";  break;
            case '\t': o << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                      << static_cast<int>(static_cast<unsigned char>(c)) << std::dec;
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

// Format a double to fixed-precision string (avoids scientific notation in JSON)
inline std::string jsonDouble(double v, int prec = 2) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(prec) << v;
    return o.str();
}

// Epoch ms from a time_point (convenience - uses extractEpoch internally)
inline long long toEpochMs(const std::chrono::system_clock::time_point& tp) {
    long long ms = 0;
    extractEpoch(tp, "timepoint", ms);
    return ms;
}

// Convert a parking slot to its JSON object representation as a string
inline std::string slotToJson(const ParkingSlot& slot,
                              const std::string& parkedReg = "",
                              const std::string& parkedType = "") {
    int id = 0, availInt = 0;
    extractIntField(slot.getSlotID(), "slotID", id);
    extractIntField(slot.getAvailability() ? 1 : 0, "available", availInt);

    std::ostringstream o;
    o << "{";
    o << "\"id\":" << id << ",";
    o << "\"type\":\"" << jsonEscape(slot.getSlotTypeName()) << "\",";
    o << "\"available\":" << (availInt ? "true" : "false") << ",";
    o << "\"parkedReg\":\"" << jsonEscape(parkedReg) << "\",";
    o << "\"parkedType\":\"" << jsonEscape(parkedType) << "\"";
    o << "}";
    return o.str();
}

// Convert a parked vehicle object to its JSON representation
inline std::string parkedVehicleToJson(const Vehicle& v) {
    long long entryEpoch = 0;
    extractEpoch(v.getEntryTime(), "entryTime", entryEpoch);

    int slot = 0;
    extractIntField(v.getParkingSlotID(), "slotID", slot);

    std::ostringstream o;
    o << "{";
    o << "\"registrationNumber\":\"" << jsonEscape(v.getRegistrationNumber()) << "\",";
    o << "\"type\":\"" << jsonEscape(v.getTypeName()) << "\",";
    o << "\"slotID\":" << slot << ",";
    o << "\"entryTimeEpochMs\":" << entryEpoch << ",";
    o << "\"durationHours\":" << jsonDouble(v.calculateDurationHours(), 4) << ",";
    o << "\"hourlyRate\":" << jsonDouble(v.getHourlyRate(), 2) << ",";
    o << "\"currentFee\":" << jsonDouble(v.calculateFee(), 2);
    o << "}";
    return o.str();
}

// Convert a ParkingRecord (history) to its JSON object
inline std::string historyRecordToJson(const ParkingRecord& r) {
    long long entryEpoch = 0, exitEpoch = 0;
    extractEpoch(r.entryTime, "entryTime", entryEpoch);
    extractEpoch(r.exitTime, "exitTime", exitEpoch);

    int slot = 0;
    extractIntField(r.slotID, "slotID", slot);

    std::ostringstream o;
    o << "{";
    o << "\"registrationNumber\":\"" << jsonEscape(r.registrationNumber) << "\",";
    o << "\"type\":\"" << jsonEscape(r.vehicleType) << "\",";
    o << "\"slotID\":" << slot << ",";
    o << "\"entryTimeEpochMs\":" << entryEpoch << ",";
    o << "\"exitTimeEpochMs\":" << exitEpoch << ",";
    o << "\"durationHours\":" << jsonDouble(r.durationHours, 4) << ",";
    o << "\"feePaid\":" << jsonDouble(r.feePaid, 2);
    o << "}";
    return o.str();
}

// Build the full /api/dashboard JSON response (as string) from a ParkingLot instance
inline std::string buildDashboardJson(const ParkingLot& lot) {
    int totalSlots = 0, avail = 0, occupied = 0, parked = 0;
    extractIntField(lot.getTotalSlots(), "totalSlots", totalSlots);
    extractIntField(lot.getAvailableSlotsCount(), "availableSlots", avail);
    extractIntField(lot.getOccupiedSlotsCount(), "occupiedSlots", occupied);
    extractIntField(lot.getParkedVehiclesCount(), "parkedVehicles", parked);

    int cars = 0, bikes = 0, trucks = 0;
    extractIntField(lot.getParkedCountByType("Car"), "cars", cars);
    extractIntField(lot.getParkedCountByType("Bike"), "bikes", bikes);
    extractIntField(lot.getParkedCountByType("Truck"), "trucks", trucks);

    int historyCount = 0;
    extractIntField(lot.getHistoryCount(), "historyRecords", historyCount);

    // Recent activity = latest history records + current parked (combined newest first)
    std::vector<ParkingRecord> recent;
    int hCount = lot.getHistoryCount();
    int start = std::max(0, hCount - 10);
    for (int i = hCount - 1; i >= start; i--) {
        ParkingRecord r;
        if (lot.getHistoryAt(i, r)) recent.push_back(r);
    }

    std::ostringstream o;
    o << "{";
    o << "\"lotName\":\"" << jsonEscape(lot.getLotName()) << "\",";
    o << "\"totalSlots\":" << totalSlots << ",";
    o << "\"occupiedSlots\":" << occupied << ",";
    o << "\"availableSlots\":" << avail << ",";
    o << "\"parkedVehicles\":" << parked << ",";
    o << "\"revenue\":" << jsonDouble(lot.getTotalRevenue(), 2) << ",";
    o << "\"byType\":{";
    o << "\"car\":" << cars << ",";
    o << "\"bike\":" << bikes << ",";
    o << "\"truck\":" << trucks;
    o << "},";
    o << "\"servedVehicles\":" << historyCount << ",";
    o << "\"recentActivity\":[";
    bool first = true;
    for (const auto& r : recent) {
        if (!first) o << ",";
        first = false;
        o << historyRecordToJson(r);
    }
    o << "]";
    o << "}";
    return o.str();
}

// Build slots JSON response with parked reg/type info attached
inline std::string buildSlotsJson(const ParkingLot& lot) {
    std::ostringstream o;
    o << "[";
    int total = lot.getTotalSlots();
    for (int i = 0; i < total; i++) {
        const ParkingSlot* slot = lot.getSlotAt(i);
        if (!slot) continue;
        int slotID = slot->getSlotID();
        std::string parkedReg, parkedType;
        // Find parked vehicle in this specific slot to attach info
        int parkedCount = lot.getParkedVehiclesCount();
        for (int j = 0; j < parkedCount; j++) {
            Vehicle* v = lot.getParkedVehicleAt(j);
            if (v && v->getParkingSlotID() == slotID) {
                parkedReg = v->getRegistrationNumber();
                parkedType = v->getTypeName();
                break;
            }
        }
        if (i > 0) o << ",";
        o << slotToJson(*slot, parkedReg, parkedType);
    }
    o << "]";
    return o.str();
}

// Build parked vehicles list JSON
inline std::string buildParkedJson(const ParkingLot& lot) {
    std::ostringstream o;
    o << "[";
    int count = lot.getParkedVehiclesCount();
    for (int i = 0; i < count; i++) {
        Vehicle* v = lot.getParkedVehicleAt(i);
        if (!v) continue;
        if (i > 0) o << ",";
        o << parkedVehicleToJson(*v);
    }
    o << "]";
    return o.str();
}

// Build history JSON response
inline std::string buildHistoryJson(const ParkingLot& lot) {
    std::ostringstream o;
    o << "[";
    int count = lot.getHistoryCount();
    // Emit in reverse order (newest first) for the UI
    for (int i = count - 1; i >= 0; i--) {
        ParkingRecord r;
        if (!lot.getHistoryAt(i, r)) continue;
        if (i != count - 1) o << ",";
        o << historyRecordToJson(r);
    }
    o << "]";
    return o.str();
}

// Wrapper JSON: {success: true/false, message:"...", data:...}
inline std::string makeJsonResponse(bool success,
                                    const std::string& message,
                                    const std::string& dataJson = "null") {
    std::ostringstream o;
    o << "{";
    o << "\"success\":" << (success ? "true" : "false") << ",";
    o << "\"message\":\"" << jsonEscape(message) << "\",";
    o << "\"data\":" << dataJson;
    o << "}";
    return o.str();
}

#endif // PARKING_API_H
