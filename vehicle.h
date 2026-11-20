#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>
#include <chrono>
#include <iostream>

// Vehicle is the BASE CLASS demonstrating runtime polymorphism
// It contains a virtual function calculateFee() that will be
// overridden by derived classes (Car, Bike, Truck)
class Vehicle {
protected:
    // Protected members: accessible by derived classes but not from outside
    std::string registrationNumber;
    std::chrono::system_clock::time_point entryTime;
    int parkingSlotID;

public:
    // Constructor - initializes all vehicle attributes (entry time = now)
    Vehicle(const std::string& regNum, int slotID);

    // Overloaded constructor - with explicit entry time (for persistence restore)
    Vehicle(const std::string& regNum, int slotID,
            const std::chrono::system_clock::time_point& entry);

    // Virtual destructor - ESSENTIAL for polymorphism
    // When deleting a derived object through a base class pointer,
    // this ensures the correct destructor chain is called
    virtual ~Vehicle();

    // Pure virtual function makes Vehicle an ABSTRACT CLASS
    // = 0 means no implementation in base; MUST be overridden by derived classes
    // This is the KEY to runtime polymorphism
    virtual double calculateFee() const = 0;

    // Virtual function to get vehicle type name (overridden by derived classes)
    virtual std::string getTypeName() const = 0;

    // Virtual function to get the hourly rate specific to vehicle type
    virtual double getHourlyRate() const = 0;

    // Getter functions (encapsulation - read-only access to private/protected data)
    std::string getRegistrationNumber() const;
    std::chrono::system_clock::time_point getEntryTime() const;
    int getParkingSlotID() const;

    // Calculate parking duration in hours (used by calculateFee implementations)
    double calculateDurationHours() const;

    // Display vehicle details
    void displayDetails() const;
};

#endif // VEHICLE_H
