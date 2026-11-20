#include "vehicle_types.h"

// ============================================================
// CAR class implementation
// ============================================================

// Constructor passes parameters to base Vehicle constructor
Car::Car(const std::string& regNum, int slotID) : Vehicle(regNum, slotID) {
    // Additional Car-specific initialization can go here
}

// Overloaded constructor: forwards explicit entry time to Vehicle base
Car::Car(const std::string& regNum, int slotID,
         const std::chrono::system_clock::time_point& entry)
    : Vehicle(regNum, slotID, entry) {
    // Preserves historical entry time (used when loading state from disk)
}

// Destructor - override of virtual base destructor
Car::~Car() {
    // Clean up any Car-specific resources here
}

// RUNTIME POLYMORPHISM IN ACTION:
// This function overrides Vehicle::calculateFee() with Car-specific logic
// When called through a Vehicle* pointer, THIS version runs (not the base)
double Car::calculateFee() const {
    double hours = calculateDurationHours();
    // Car rate: Rs. 40.00 per hour
    return hours * HOURLY_RATE;
}

std::string Car::getTypeName() const {
    return "Car";
}

double Car::getHourlyRate() const {
    return HOURLY_RATE;
}


// ============================================================
// BIKE class implementation
// ============================================================

Bike::Bike(const std::string& regNum, int slotID) : Vehicle(regNum, slotID) {
    // Bike-specific initialization
}

// Overloaded constructor: forwards explicit entry time to Vehicle base
Bike::Bike(const std::string& regNum, int slotID,
           const std::chrono::system_clock::time_point& entry)
    : Vehicle(regNum, slotID, entry) {
    // Preserves historical entry time (used when loading state from disk)
}

Bike::~Bike() {
    // Bike-specific cleanup
}

// RUNTIME POLYMORPHISM: Bike has its own (lower) rate calculation
double Bike::calculateFee() const {
    double hours = calculateDurationHours();
    // Bike rate: Rs. 20.00 per hour
    return hours * HOURLY_RATE;
}

std::string Bike::getTypeName() const {
    return "Bike";
}

double Bike::getHourlyRate() const {
    return HOURLY_RATE;
}


// ============================================================
// TRUCK class implementation
// ============================================================

Truck::Truck(const std::string& regNum, int slotID) : Vehicle(regNum, slotID) {
    // Truck-specific initialization
}

// Overloaded constructor: forwards explicit entry time to Vehicle base
Truck::Truck(const std::string& regNum, int slotID,
             const std::chrono::system_clock::time_point& entry)
    : Vehicle(regNum, slotID, entry) {
    // Preserves historical entry time (used when loading state from disk)
}

Truck::~Truck() {
    // Truck-specific cleanup
}

// RUNTIME POLYMORPHISM: Truck has its own (higher) rate calculation
double Truck::calculateFee() const {
    double hours = calculateDurationHours();
    // Truck rate: Rs. 80.00 per hour
    return hours * HOURLY_RATE;
}

std::string Truck::getTypeName() const {
    return "Truck";
}

double Truck::getHourlyRate() const {
    return HOURLY_RATE;
}
