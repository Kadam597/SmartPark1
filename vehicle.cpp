#include "vehicle.h"
#include <iomanip>
#include <ctime>

// ============================================================
// Vehicle class implementation (Base class for polymorphism)
// ============================================================

// Constructor: stores registration number, slot ID, and current time as entry time
Vehicle::Vehicle(const std::string& regNum, int slotID)
    : registrationNumber(regNum), parkingSlotID(slotID) {
    entryTime = std::chrono::system_clock::now();
}

// Overloaded constructor: uses a supplied entry time (used when restoring from save)
Vehicle::Vehicle(const std::string& regNum, int slotID,
                 const std::chrono::system_clock::time_point& entry)
    : registrationNumber(regNum), entryTime(entry), parkingSlotID(slotID) {
    // Use the provided timestamp instead of "now" - preserves historical parking durations
}

// Virtual destructor: ensures proper cleanup of derived class objects
// when deleted through a base class pointer (critical for polymorphism)
Vehicle::~Vehicle() {
    // Destructor is virtual so derived destructors get called correctly
    // Empty for now - no dynamic allocation to clean up in base class
}

// Getter: returns vehicle registration number (encapsulation)
std::string Vehicle::getRegistrationNumber() const {
    return registrationNumber;
}

// Getter: returns entry timestamp
std::chrono::system_clock::time_point Vehicle::getEntryTime() const {
    return entryTime;
}

// Getter: returns assigned parking slot ID
int Vehicle::getParkingSlotID() const {
    return parkingSlotID;
}

// Helper: calculate how long (in hours) the vehicle has been parked
// Uses std::chrono for accurate time calculations
double Vehicle::calculateDurationHours() const {
    auto currentTime = std::chrono::system_clock::now();
    auto duration = currentTime - entryTime;

    // Convert duration to hours as a floating-point value (e.g., 1.5 hours)
    double hours = std::chrono::duration<double, std::ratio<3600>>(duration).count();

    // Minimum charge: round up to at least 0.01 hours (~36 seconds)
    // to avoid zero fees for very short stays
    if (hours < 0.01) hours = 0.01;

    return hours;
}

// Display all vehicle information in a formatted way
void Vehicle::displayDetails() const {
    std::time_t entryTimeT = std::chrono::system_clock::to_time_t(entryTime);
    char timeBuffer[100];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S",
                  std::localtime(&entryTimeT));

    std::cout << "  Type:              " << getTypeName() << std::endl;
    std::cout << "  Registration No:   " << registrationNumber << std::endl;
    std::cout << "  Parking Slot:      #" << parkingSlotID << std::endl;
    std::cout << "  Entry Time:        " << timeBuffer << std::endl;
    std::cout << "  Duration (hrs):    " << std::fixed << std::setprecision(2)
              << calculateDurationHours() << std::endl;
    std::cout << "  Current Fee (Rs):  " << std::fixed << std::setprecision(2)
              << calculateFee() << std::endl;
}
