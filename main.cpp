// ============================================================
// SmartPark - Smart Parking Management System
// College-level OOP Project Demonstrating Runtime Polymorphism
// ============================================================
//
// RUNTIME POLYMORPHISM DEMONSTRATED IN THIS PROJECT:
// ---------------------------------------------------
// 1. Base class 'Vehicle' has PURE VIRTUAL function calculateFee()
// 2. Derived classes 'Car', 'Bike', 'Truck' OVERRIDE calculateFee()
//    with different hourly rates
// 3. Vehicles are stored in vector<unique_ptr<Vehicle>>
//    (base class smart pointers holding derived class objects)
// 4. When we call vehicle->calculateFee() through the base pointer,
//    the CORRECT derived class version runs AUTOMATICALLY at runtime!
// 5. Virtual destructor ensures proper cleanup of derived objects
//    deleted through base class pointers
// ============================================================

#include <iostream>
#include <string>
#include <limits>     // for numeric_limits (input clearing)
#include <iomanip>

#include "parking_lot.h"
#include "vehicle.h"

// ============================================================
// Helper: Clear any bad input from cin and discard the rest
// of the line. Prevents infinite loops on invalid input.
// ============================================================
void clearInputStream() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// ============================================================
// Display the interactive menu to the user
// ============================================================
void displayMenu() {
    std::cout << "\n" << std::endl;
    std::cout << "  =================================================" << std::endl;
    std::cout << "    ##########  SMARTPARK  SYSTEM  ##########       " << std::endl;
    std::cout << "  =================================================" << std::endl;
    std::cout << "    1.  Register Vehicle Entry (Park)" << std::endl;
    std::cout << "    2.  Release Vehicle Exit  (Unpark + Pay)" << std::endl;
    std::cout << "    3.  Search Parked Vehicle" << std::endl;
    std::cout << "    4.  Show All Parking Slots" << std::endl;
    std::cout << "    5.  Show All Parked Vehicles" << std::endl;
    std::cout << "    6.  Show Parking History (All Exited Vehicles)" << std::endl;
    std::cout << "    7.  Show Revenue Summary" << std::endl;
    std::cout << "    8.  Show System Summary Dashboard" << std::endl;
    std::cout << "    9.  Run Demonstration (Polymorphism Example)" << std::endl;
    std::cout << "    0.  Exit SmartPark System" << std::endl;
    std::cout << "  =================================================" << std::endl;
    std::cout << "    Enter your choice: ";
}

// ============================================================
// Helper to get a string input from user with a prompt
// ============================================================
std::string getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return input;
}

// ============================================================
// Get vehicle type choice from user
// ============================================================
std::string getVehicleTypeFromUser() {
    std::cout << "\n  Select vehicle type:" << std::endl;
    std::cout << "    1. Car    (Rs. 40.00/hr)" << std::endl;
    std::cout << "    2. Bike   (Rs. 20.00/hr)" << std::endl;
    std::cout << "    3. Truck  (Rs. 80.00/hr)" << std::endl;
    std::cout << "    Enter choice (1-3): ";

    int choice;
    if (!(std::cin >> choice)) {
        clearInputStream();
        return "";
    }
    clearInputStream(); // consume newline after number

    switch (choice) {
        case 1: return "Car";
        case 2: return "Bike";
        case 3: return "Truck";
        default:
            std::cout << "  [ERROR] Invalid vehicle type choice." << std::endl;
            return "";
    }
}

// ============================================================
// Load sample demo data into the parking lot for quick testing
// ============================================================
void loadSampleData(ParkingLot& lot) {
    std::cout << "\n  [LOADING SAMPLE DATA] Please wait..." << std::endl;

    // Register some sample vehicles
    lot.registerVehicle("Bike", "MH-12-AB-1234");
    lot.registerVehicle("Car",  "MH-12-CD-5678");
    lot.registerVehicle("Bike", "MH-12-EF-9012");
    lot.registerVehicle("Truck","MH-12-GH-3456");
    lot.registerVehicle("Car",  "DL-01-XY-9999");

    std::cout << "  [DONE] 5 sample vehicles have been parked for testing." << std::endl;
    std::cout << "         Note: Fees are calculated from this moment forward," << std::endl;
    std::cout << "         so exit a vehicle now to see fees! (They will be small." << std::endl;
    std::cout << "         In real usage, vehicles would be parked for hours.)" << std::endl;
}

// ============================================================
// Run a self-contained demonstration that explicitly shows
// runtime polymorphism with clear, printed explanations
// ============================================================
void runPolymorphismDemo() {
    std::cout << "\n\n" << std::endl;
    std::cout << "  ========================================================" << std::endl;
    std::cout << "          RUNTIME POLYMORPHISM  DEMONSTRATION            " << std::endl;
    std::cout << "  ========================================================" << std::endl;

    std::cout << "\n  What is Runtime Polymorphism?" << std::endl;
    std::cout << "  -----------------------------" << std::endl;
    std::cout << "  Runtime polymorphism means: calling a VIRTUAL function" << std::endl;
    std::cout << "  through a BASE CLASS POINTER, and the CORRECT DERIVED" << std::endl;
    std::cout << "  CLASS implementation is AUTOMATICALLY selected at" << std::endl;
    std::cout << "  RUNTIME based on the ACTUAL object type stored." << std::endl;
    std::cout << std::endl;
    std::cout << "  In SmartPark, Vehicle* pointers point to Car/Bike/Truck" << std::endl;
    std::cout << "  objects. Calling ->calculateFee() runs the correct fee" << std::endl;
    std::cout << "  formula for each vehicle type - WITHOUT using if/else!" << std::endl;

    // Now let's create a small demo with its own vehicles
    std::cout << "\n  Demo: Creating 3 vehicles (Car, Bike, Truck)..." << std::endl;
    std::cout << "  Storing them all as unique_ptr<Vehicle> (base class!)" << std::endl;
    std::cout << std::endl;

    // Create derived class objects stored in base-class smart pointers
    std::unique_ptr<Vehicle> car1   = std::make_unique<Car>("DEMO-CAR-01", 101);
    std::unique_ptr<Vehicle> bike1  = std::make_unique<Bike>("DEMO-BIKE-02", 102);
    std::unique_ptr<Vehicle> truck1 = std::make_unique<Truck>("DEMO-TRK-03", 103);

    // Put them all in a vector of base-class pointers (just like ParkingLot does!)
    std::vector<std::unique_ptr<Vehicle>> demoVehicles;
    demoVehicles.push_back(std::move(car1));
    demoVehicles.push_back(std::move(bike1));
    demoVehicles.push_back(std::move(truck1));

    // Now the MAGIC: iterate through BASE CLASS pointers and call methods
    std::cout << "  Iterating through vector<unique_ptr<Vehicle>>...\n" << std::endl;

    for (const auto& vehicle : demoVehicles) {
        // Even though variable is Vehicle*, these all dispatch to
        // the ACTUAL DERIVED CLASS implementation at runtime!
        std::cout << "    Object Type:     " << vehicle->getTypeName() << std::endl;
        std::cout << "    Registration:    " << vehicle->getRegistrationNumber() << std::endl;
        std::cout << "    Hourly Rate:     Rs. " << std::fixed << std::setprecision(2)
                  << vehicle->getHourlyRate() << "  <-- POLYMORPHIC!" << std::endl;
        std::cout << "    calculateFee():  Rs. " << std::fixed << std::setprecision(2)
                  << vehicle->calculateFee() << "  <-- POLYMORPHIC!" << std::endl;
        std::cout << "    (Notice: different rates without any if-else!)" << std::endl;
        std::cout << std::endl;
    }

    std::cout << "  ========================================================" << std::endl;
    std::cout << "  Key Proof of Runtime Polymorphism in SmartPark:" << std::endl;
    std::cout << "  ========================================================" << std::endl;
    std::cout << "  1. class Vehicle has PURE VIRTUAL: virtual double" << std::endl;
    std::cout << "     calculateFee() const = 0;" << std::endl;
    std::cout << "  2. class Car   overrides: double calculateFee() const override;" << std::endl;
    std::cout << "     (Rs. 40/hr)" << std::endl;
    std::cout << "  3. class Bike  overrides: double calculateFee() const override;" << std::endl;
    std::cout << "     (Rs. 20/hr)" << std::endl;
    std::cout << "  4. class Truck overrides: double calculateFee() const override;" << std::endl;
    std::cout << "     (Rs. 80/hr)" << std::endl;
    std::cout << "  5. ParkingLot stores: vector<unique_ptr<Vehicle>> parkedVehicles;" << std::endl;
    std::cout << "  6. When releaseVehicle() calls vehicle->calculateFee()," << std::endl;
    std::cout << "     the correct version is selected automatically!" << std::endl;
    std::cout << "  ========================================================" << std::endl;

    // unique_ptr auto-deletes; virtual destructors run correctly.
    // Each derived destructor (~Car, ~Bike, ~Truck) runs in reverse order,
    // then base ~Vehicle() - this is the virtual destructor working!
}

// ============================================================
// MAIN PROGRAM - Entry point
// ============================================================
int main() {
    // Print welcome banner
    std::cout << "\n\n";
    std::cout << "  ###########################################################" << std::endl;
    std::cout << "  #                                                         #" << std::endl;
    std::cout << "  #         WELCOME  TO  SMARTPARK  SYSTEM                  #" << std::endl;
    std::cout << "  #                                                         #" << std::endl;
    std::cout << "  #  An OOP College Project Demonstrating:                  #" << std::endl;
    std::cout << "  #    - Runtime Polymorphism (Virtual Functions)           #" << std::endl;
    std::cout << "  #    - Inheritance  (Vehicle -> Car / Bike / Truck)       #" << std::endl;
    std::cout << "  #    - Encapsulation  (Private Data + Getters/Setters)    #" << std::endl;
    std::cout << "  #    - Smart Pointers  (std::unique_ptr<Vehicle>)         #" << std::endl;
    std::cout << "  #    - Function Overriding                                #" << std::endl;
    std::cout << "  #    - std::chrono (Time-based fee calc)                  #" << std::endl;
    std::cout << "  #                                                         #" << std::endl;
    std::cout << "  ###########################################################" << std::endl;

    // Create the main parking lot object
    ParkingLot smartParkLot("SmartPark Main Campus Lot");

    // Optional: ask user if they want to load sample data
    std::string ans = getStringInput("\n  Load sample demonstration data? (y/n): ");
    if (ans == "y" || ans == "Y" || ans == "yes" || ans == "YES") {
        loadSampleData(smartParkLot);
    }

    // =============================================
    // Main menu loop
    // =============================================
    int menuChoice = -1;

    while (menuChoice != 0) {
        displayMenu();

        // Read menu choice safely
        if (!(std::cin >> menuChoice)) {
            // Non-numeric input
            std::cout << "\n  [ERROR] Please enter a valid NUMBER (0-9)." << std::endl;
            clearInputStream();
            continue;
        }
        clearInputStream(); // consume the newline

        switch (menuChoice) {
            case 1: {
                // === REGISTER VEHICLE ENTRY ===
                std::cout << "\n  --- Register Vehicle Entry ---" << std::endl;
                std::string vType = getVehicleTypeFromUser();
                if (vType.empty()) break;

                std::string regNum = getStringInput("    Enter registration number: ");
                smartParkLot.registerVehicle(vType, regNum);
                break;
            }

            case 2: {
                // === RELEASE VEHICLE EXIT ===
                std::cout << "\n  --- Release Vehicle Exit ---" << std::endl;
                std::string regNum = getStringInput("    Enter registration number: ");
                smartParkLot.releaseVehicle(regNum);
                break;
            }

            case 3: {
                // === SEARCH VEHICLE ===
                std::cout << "\n  --- Search Parked Vehicle ---" << std::endl;
                std::string regNum = getStringInput("    Enter registration number: ");
                Vehicle* found = smartParkLot.searchVehicle(regNum);
                if (found) {
                    std::cout << "\n  [FOUND] Vehicle details:" << std::endl;
                    found->displayDetails();
                } else {
                    std::cout << "\n  [NOT FOUND] Vehicle with reg '" << regNum
                              << "' is not currently parked." << std::endl;
                    std::cout << "          (It may have already exited -"
                              << " check Parking History)" << std::endl;
                }
                break;
            }

            case 4: {
                // === SHOW ALL SLOTS ===
                smartParkLot.displayAllSlots();
                break;
            }

            case 5: {
                // === SHOW PARKED VEHICLES ===
                smartParkLot.displayParkedVehicles();
                break;
            }

            case 6: {
                // === PARKING HISTORY ===
                smartParkLot.displayParkingHistory();
                break;
            }

            case 7: {
                // === REVENUE SUMMARY ===
                smartParkLot.displayTotalRevenue();
                break;
            }

            case 8: {
                // === SYSTEM SUMMARY ===
                smartParkLot.displaySummary();
                break;
            }

            case 9: {
                // === POLYMORPHISM DEMONSTRATION ===
                runPolymorphismDemo();
                break;
            }

            case 0: {
                // === EXIT SYSTEM ===
                std::cout << "\n\n  =======================================================" << std::endl;
                std::cout << "      THANK YOU FOR USING SMARTPARK SYSTEM!            " << std::endl;
                std::cout << "  =======================================================" << std::endl;
                smartParkLot.displaySummary();
                std::cout << "\n  Goodbye!\n" << std::endl;
                break;
            }

            default: {
                std::cout << "\n  [ERROR] Invalid menu choice. Please enter 0-9." << std::endl;
                break;
            }
        }
    }

    return 0;
}
