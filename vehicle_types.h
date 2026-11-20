#ifndef VEHICLE_TYPES_H
#define VEHICLE_TYPES_H

#include "vehicle.h"

// ============================================================
// CAR class - DERIVED from Vehicle
// Demonstrates FUNCTION OVERRIDING
// Hourly rate: Rs. 40.00 per hour
// ============================================================
class Car : public Vehicle {
private:
    static constexpr double HOURLY_RATE = 40.00;

public:
    // Constructor calls base class Vehicle constructor
    Car(const std::string& regNum, int slotID);

    // Overloaded constructor - with explicit entry time (for persistence restore)
    Car(const std::string& regNum, int slotID,
        const std::chrono::system_clock::time_point& entry);

    // Destructor - override base class virtual destructor
    ~Car() override;

    // OVERRIDE the pure virtual function from base class
    // Note: 'override' keyword tells compiler we intend to override,
    // causing compile error if signature doesn't match exactly
    double calculateFee() const override;

    // Return the name of this vehicle type
    std::string getTypeName() const override;

    // Return the hourly rate for Car
    double getHourlyRate() const override;
};


// ============================================================
// BIKE class - DERIVED from Vehicle
// Demonstrates FUNCTION OVERRIDING
// Hourly rate: Rs. 20.00 per hour (cheaper than Car)
// ============================================================
class Bike : public Vehicle {
private:
    static constexpr double HOURLY_RATE = 20.00;

public:
    Bike(const std::string& regNum, int slotID);

    // Overloaded constructor - with explicit entry time (for persistence restore)
    Bike(const std::string& regNum, int slotID,
         const std::chrono::system_clock::time_point& entry);

    ~Bike() override;

    // OVERRIDE virtual functions from Vehicle
    double calculateFee() const override;
    std::string getTypeName() const override;
    double getHourlyRate() const override;
};


// ============================================================
// TRUCK class - DERIVED from Vehicle
// Demonstrates FUNCTION OVERRIDING
// Hourly rate: Rs. 80.00 per hour (most expensive)
// ============================================================
class Truck : public Vehicle {
private:
    static constexpr double HOURLY_RATE = 80.00;

public:
    Truck(const std::string& regNum, int slotID);

    // Overloaded constructor - with explicit entry time (for persistence restore)
    Truck(const std::string& regNum, int slotID,
          const std::chrono::system_clock::time_point& entry);

    ~Truck() override;

    // OVERRIDE virtual functions from Vehicle
    double calculateFee() const override;
    std::string getTypeName() const override;
    double getHourlyRate() const override;
};

#endif // VEHICLE_TYPES_H
