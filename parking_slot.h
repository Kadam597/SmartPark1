#ifndef PARKING_SLOT_H
#define PARKING_SLOT_H

#include <string>

// Enum to represent the type of vehicle a parking slot can accommodate
// Uses C++11 enum class for type safety
enum class SlotType {
    BIKE_SLOT,    // Small slot for bikes/scooters
    CAR_SLOT,     // Medium slot for cars
    TRUCK_SLOT,   // Large slot for trucks/buses
    UNIVERSAL     // Can accommodate any vehicle type
};

// ParkingSlot class: represents an individual parking space in the lot
// Demonstrates ENCAPSULATION - data is private, accessed via public methods
class ParkingSlot {
private:
    int slotID;          // Unique identifier for this slot
    SlotType slotType;   // Type of vehicles allowed (Bike/Car/Truck/Universal)
    bool isAvailable;    // True = free to use, False = occupied

public:
    // Default constructor
    ParkingSlot();

    // Parameterized constructor
    ParkingSlot(int id, SlotType type);

    // Destructor
    ~ParkingSlot();

    // === Getter Methods (Encapsulation) ===
    int getSlotID() const;
    SlotType getSlotType() const;
    bool getAvailability() const;

    // Convert SlotType enum to human-readable string
    std::string getSlotTypeName() const;

    // === Setter Methods ===
    // Mark slot as occupied (vehicle parked)
    void occupy();

    // Mark slot as free (vehicle left)
    void vacate();

    // Check if this slot is compatible with a given vehicle type
    // Parameters: vehicleTypeStr = "Car", "Bike", or "Truck"
    bool canFitVehicle(const std::string& vehicleTypeStr) const;

    // Display slot information
    void displaySlotInfo() const;
};

// Helper: Convert a vehicle type name (Car/Bike/Truck) to compatible SlotType
SlotType getRequiredSlotType(const std::string& vehicleTypeStr);

// Helper: Convert SlotType enum to string name
std::string slotTypeToString(SlotType type);

#endif // PARKING_SLOT_H
