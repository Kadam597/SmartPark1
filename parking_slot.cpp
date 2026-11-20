#include "parking_slot.h"
#include <iostream>

// ============================================================
// ParkingSlot class implementation
// Manages individual parking slot state and compatibility
// ============================================================

// Default constructor - creates an empty/uninitialized slot
ParkingSlot::ParkingSlot() : slotID(-1), slotType(SlotType::UNIVERSAL), isAvailable(true) {
    // Default slot: marked as available but with invalid ID
}

// Parameterized constructor - creates a slot with specific ID and type
ParkingSlot::ParkingSlot(int id, SlotType type)
    : slotID(id), slotType(type), isAvailable(true) {
    // New slots always start as available (vacant)
}

// Destructor
ParkingSlot::~ParkingSlot() {
    // No dynamic memory to clean up in ParkingSlot
}

// Get slot's unique ID number
int ParkingSlot::getSlotID() const {
    return slotID;
}

// Get what type of vehicles this slot accepts
SlotType ParkingSlot::getSlotType() const {
    return slotType;
}

// Check if slot is currently free
bool ParkingSlot::getAvailability() const {
    return isAvailable;
}

// Get slot type as a user-friendly string
std::string ParkingSlot::getSlotTypeName() const {
    return slotTypeToString(slotType);
}

// Mark this slot as occupied (a vehicle is parking here)
void ParkingSlot::occupy() {
    isAvailable = false;
}

// Mark this slot as vacant (vehicle has left)
void ParkingSlot::vacate() {
    isAvailable = true;
}

// ===== Compatibility Check =====
// Determines if a particular vehicle type can park in this slot
// This simulates real-world parking where:
//   - Bike slots are too small for cars/trucks
//   - Car slots fit bikes but not trucks
//   - Truck slots fit everything
//   - Universal slots fit all
bool ParkingSlot::canFitVehicle(const std::string& vehicleTypeStr) const {
    // Only check availability when slot is occupied
    // (Caller should handle availability check separately)

    if (slotType == SlotType::UNIVERSAL) {
        // Universal slots accept all vehicle types
        return true;
    }

    if (vehicleTypeStr == "Bike") {
        // Bikes can fit in almost any slot except dedicated truck slots
        return (slotType == SlotType::BIKE_SLOT ||
                slotType == SlotType::CAR_SLOT ||
                slotType == SlotType::UNIVERSAL);
    }

    if (vehicleTypeStr == "Car") {
        // Cars need car-sized or truck-sized or universal slots
        return (slotType == SlotType::CAR_SLOT ||
                slotType == SlotType::TRUCK_SLOT ||
                slotType == SlotType::UNIVERSAL);
    }

    if (vehicleTypeStr == "Truck") {
        // Trucks only fit in truck slots or universal slots
        return (slotType == SlotType::TRUCK_SLOT ||
                slotType == SlotType::UNIVERSAL);
    }

    // Unknown vehicle type
    return false;
}

// Print slot status information
void ParkingSlot::displaySlotInfo() const {
    std::cout << "  Slot #" << slotID
              << " | Type: " << getSlotTypeName()
              << " | Status: " << (isAvailable ? "[AVAILABLE]" : "[OCCUPIED]")
              << std::endl;
}


// ============================================================
// Helper functions for SlotType conversion
// ============================================================

// Given a vehicle type name, return the minimum slot type required
SlotType getRequiredSlotType(const std::string& vehicleTypeStr) {
    if (vehicleTypeStr == "Bike")  return SlotType::BIKE_SLOT;
    if (vehicleTypeStr == "Car")   return SlotType::CAR_SLOT;
    if (vehicleTypeStr == "Truck") return SlotType::TRUCK_SLOT;
    return SlotType::UNIVERSAL;
}

// Convert SlotType enum value to a readable string
std::string slotTypeToString(SlotType type) {
    switch (type) {
        case SlotType::BIKE_SLOT:    return "Bike Slot";
        case SlotType::CAR_SLOT:     return "Car Slot";
        case SlotType::TRUCK_SLOT:   return "Truck Slot";
        case SlotType::UNIVERSAL:    return "Universal Slot";
        default:                      return "Unknown";
    }
}
