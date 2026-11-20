#ifndef PARKING_LOT_H
#define PARKING_LOT_H

#include "vehicle.h"
#include "parking_slot.h"
#include "vehicle_types.h"

#include <vector>
#include <memory>   // for std::unique_ptr
#include <string>

// Structure to store parking history records
// Tracks which vehicles have exited and how much they paid
struct ParkingRecord {
    std::string registrationNumber;
    std::string vehicleType;
    int slotID;
    std::chrono::system_clock::time_point entryTime;
    std::chrono::system_clock::time_point exitTime;
    double durationHours;
    double feePaid;
};

// ============================================================
// ParkingLot class - the central manager of the entire system
// Demonstrates:
//   - RUNTIME POLYMORPHISM via Vehicle* / unique_ptr<Vehicle>
//   - Encapsulation of all parking data
//   - Association with ParkingSlot and Vehicle classes
// ============================================================
class ParkingLot {
private:
    std::string lotName;

    // ALL PARKED VEHICLES stored as base-class smart pointers
    // THIS IS WHERE RUNTIME POLYMORPHISM HAPPENS:
    //   - We store unique_ptr<Vehicle> (base class)
    //   - The actual objects are Car, Bike, or Truck (derived classes)
    //   - When we call vehicle->calculateFee(), the CORRECT derived
    //     version runs automatically based on the actual object type!
    std::vector<std::unique_ptr<Vehicle>> parkedVehicles;

    // Parking slots in the lot
    std::vector<ParkingSlot> parkingSlots;

    // History of all vehicles that have exited
    std::vector<ParkingRecord> parkingHistory;

    // Running total of all fees collected
    double totalRevenue;

    // ===== Private Helper Functions =====

    // Find a slot suitable for a given vehicle type, return its index
    // Returns -1 if no suitable slot available
    int findAvailableSlotIndex(const std::string& vehicleTypeStr) const;

    // Check if a registration number is already in use (parked)
    bool isDuplicateRegistration(const std::string& regNum) const;

    // Create a new Vehicle object of the correct derived type
    // Returns a unique_ptr<Vehicle> pointing to the new Car/Bike/Truck
    std::unique_ptr<Vehicle> createVehicleObject(
        const std::string& vehicleTypeStr,
        const std::string& regNum,
        int slotID
    ) const;

    // Overload: create vehicle object with a specific entry time (for persistence restore)
    std::unique_ptr<Vehicle> createVehicleObject(
        const std::string& vehicleTypeStr,
        const std::string& regNum,
        int slotID,
        const std::chrono::system_clock::time_point& entryTime
    ) const;

    // Force-occupy a specific slot ID (used during persistence restore)
    bool occupySlotByID(int slotID);

public:
    // Constructor: initializes parking lot with given name and default slots
    ParkingLot(const std::string& name);

    // Destructor
    ~ParkingLot();

    // ===== Core Operations =====

    // Register and park a vehicle
    // Returns true on success, false on failure
    // Displays appropriate messages for errors
    bool registerVehicle(const std::string& vehicleTypeStr,
                         const std::string& registrationNumber);

    // Release a vehicle (exit parking), calculate and collect fee
    // Returns true on success, false if vehicle not found
    bool releaseVehicle(const std::string& registrationNumber);

    // Search for a parked vehicle by registration number
    // Returns pointer to Vehicle (nullptr if not found)
    Vehicle* searchVehicle(const std::string& registrationNumber) const;

    // ===== Status Reports =====

    // Display status of every parking slot
    void displayAllSlots() const;

    // Display all currently parked vehicles with details
    void displayParkedVehicles() const;

    // Display total revenue collected so far
    void displayTotalRevenue() const;

    // Display complete parking history (all exited vehicles)
    void displayParkingHistory() const;

    // Display a quick summary dashboard
    void displaySummary() const;

    // ===== Getter Methods =====
    int getTotalSlots() const;
    int getAvailableSlotsCount() const;
    int getOccupiedSlotsCount() const;
    int getParkedVehiclesCount() const;
    double getTotalRevenue() const;
    std::string getLotName() const;

    // ===== API / Enumeration Accessors (used by HTTP server) =====

    // Get parked vehicle by index (0..count-1). Returns nullptr if out of range.
    Vehicle* getParkedVehicleAt(int index) const;

    // Get parking slot by index (0..count-1). Returns nullptr if out of range.
    const ParkingSlot* getSlotAt(int index) const;

    // Count of parking history records (exited vehicles)
    int getHistoryCount() const;

    // Get history record by index (0..count-1). Returns false if out of range.
    bool getHistoryAt(int index, ParkingRecord& outRecord) const;

    // Counts of currently parked vehicles by type
    int getParkedCountByType(const std::string& typeName) const;

    // ===== Persistence (save/load to file on disk) =====

    // Save all current state (parked vehicles + history + revenue) to a JSON-like text file
    // Returns true on success
    bool saveStateToFile(const std::string& filePath) const;

    // Load state from a file previously written by saveStateToFile.
    // Existing parked vehicles and history are REPLACED.
    // Returns true on success.
    bool loadStateFromFile(const std::string& filePath);

    // Directly restore a parked vehicle (bypasses normal slot search).
    // Used during loadStateFromFile.
    bool restoreParkedVehicle(const std::string& vehicleTypeStr,
                              const std::string& regNum,
                              int slotID,
                              const std::chrono::system_clock::time_point& entry);

    // Append a history record directly (for loadStateFromFile).
    void addHistoryRecord(const ParkingRecord& record);

    // Directly set total revenue (used during loadStateFromFile)
    void setTotalRevenue(double value);
};

#endif // PARKING_LOT_H
