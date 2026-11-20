# SmartPark - Smart Parking Management System

A college-level Object-Oriented Programming (OOP) project in C++ that demonstrates **runtime polymorphism** using inheritance and virtual functions.

---

## 🏗️ Project Overview

SmartPark manages vehicle entry, slot allocation, fee calculation, and exit for a multi-type parking lot. It supports three vehicle categories (Car, Bike, Truck), each with its own hourly parking rate.

### Key Features
- Register and park vehicles (Car / Bike / Truck)
- Automatic suitable slot allocation
- Duplicate vehicle registration prevention
- Parking fee calculation based on vehicle type and real duration
- Vehicle exit processing with slot release and receipt
- Search for parked vehicles by registration number
- Real-time display of slot availability
- Complete parking history and total revenue tracking
- Load sample demonstration data on startup
- Built-in Runtime Polymorphism demonstration (Menu Option 9)

---

## 📚 OOP Concepts Demonstrated

This project is specifically designed to showcase these OOP principles:

| Concept | How it is used |
|---------|---------------|
| **Runtime Polymorphism** | `Vehicle` base pointer stores `Car`/`Bike`/`Truck`; `calculateFee()` dispatches to correct derived implementation at runtime via virtual functions. |
| **Inheritance** | `Car`, `Bike`, `Truck` all inherit from the abstract `Vehicle` base class. |
| **Pure Virtual Function / Abstract Class** | `Vehicle::calculateFee() = 0` makes `Vehicle` abstract; cannot be instantiated directly. |
| **Function Overriding** | Each derived class overrides `calculateFee()`, `getTypeName()`, and `getHourlyRate()` with `override` keyword. |
| **Virtual Destructor** | `~Vehicle()` is virtual, so `~Car()` / `~Bike()` / `~Truck()` run when objects are deleted through `Vehicle*`. |
| **Encapsulation** | All class data members are `private` or `protected`; accessed via public getter/setter methods. |
| **Smart Pointers** | `std::unique_ptr<Vehicle>` safely manages polymorphic vehicle objects; auto-deleted on scope exit. |
| **RAII / Resource Management** | Constructors initialize; destructors clean up; `std::vector` and `std::unique_ptr` manage memory automatically. |
| **Abstraction** | ParkingLot exposes a clean menu interface while hiding internal vehicle/slot management. |

---

## 🧱 Class Hierarchy

```
                    Vehicle (abstract base class)
                    ├── registrationNumber
                    ├── entryTime
                    ├── parkingSlotID
                    ├── calculateFee()       [pure virtual]
                    ├── getTypeName()        [virtual]
                    ├── getHourlyRate()      [virtual]
                    └── ~Vehicle()           [virtual destructor]
                          │
           ┌──────────────┼──────────────┐
           │              │              │
         Car             Bike           Truck
   (Rs. 40/hr)      (Rs. 20/hr)     (Rs. 80/hr)
   └─ calculateFee() └─ calculateFee() └─ calculateFee()


                   ParkingSlot
                   ├── slotID
                   ├── slotType (BIKE/CAR/TRUCK/UNIVERSAL)
                   └── isAvailable


                     ParkingLot (Manager Class)
          ─────────────────────────────────────
          Stores:
            vector<unique_ptr<Vehicle>> parkedVehicles   ← polymorphism here!
            vector<ParkingSlot>         parkingSlots
            vector<ParkingRecord>       parkingHistory
            double                      totalRevenue

          Operations:
            registerVehicle()   releaseVehicle()
            searchVehicle()     displayParkedVehicles()
            displayAllSlots()   displayParkingHistory()
            displayTotalRevenue()  displaySummary()
```

---

## 🚗 Parking Rates

| Vehicle Type | Hourly Rate (Rs.) | Minimum Charge |
|--------------|-------------------|----------------|
| Bike         | 20.00             | ~Rs. 0.20      |
| Car          | 40.00             | ~Rs. 0.40      |
| Truck        | 80.00             | ~Rs. 0.80      |

Parking duration is computed using `std::chrono::system_clock` from the time of registration until exit.

---

## 🧩 Short Explanation of Each C++ Class

| Class | Header | Purpose / Explanation |
|---|---|---|
| **Vehicle** (abstract) | [vehicle.h](file:///d:/SmartPark/vehicle.h) | The abstract *base class* that every concrete vehicle inherits from. Stores the three shared fields (`registrationNumber`, `entryTime`, `parkingSlotID`) and declares the *three pure virtual functions* (`calculateFee()`, `getTypeName()`, `getHourlyRate()`) that every derived class **must** override. Because of the pure virtual, you can never write `new Vehicle(...)` — it only exists to be inherited from. Also provides shared helpers like `calculateDurationHours()` and getters so derived classes don't reimplement the same plumbing. |
| **Car** | [vehicle_types.h:11](file:///d:/SmartPark/vehicle_types.h#L11-L36) | A *derived class* that IS-A Vehicle. Introduces the compile-time constant `HOURLY_RATE = 40.00` (Rs. 40 per hour). Overrides all three pure virtuals so `calculateFee()` returns `duration × 40.00` and `getTypeName()` returns `"Car"`. |
| **Bike** | [vehicle_types.h:44](file:///d:/SmartPark/vehicle_types.h#L44-L61) | A *derived class* that IS-A Vehicle. Uses the cheapest rate: `HOURLY_RATE = 20.00` Rs/hr. Structurally identical to `Car`, but the different `HOURLY_RATE` constant is what makes `calculateFee()` dispatch to different math at runtime through polymorphism. |
| **Truck** | [vehicle_types.h:69](file:///d:/SmartPark/vehicle_types.h#L69-L86) | A *derived class* that IS-A Vehicle. Uses the most expensive rate: `HOURLY_RATE = 80.00` Rs/hr (4× Bike, 2× Car). Demonstrates that *adding a new type only requires a new derived class + one new line in the factory function*; `ParkingLot` code never changes. |
| **ParkingSlot** | [parking_slot.h](file:///d:/SmartPark/parking_slot.h) | Represents *one physical parking slot* in the lot. Holds `slotID`, a `SlotType` enum (Bike Slot / Car Slot / Truck Slot / Universal), an `isAvailable` flag, and the parked vehicle reg/type. The key helper `canFitVehicle(type)` encodes the compatibility rules: a Bike can fit anywhere, a Car needs Car/Truck/Universal, a Truck only fits in Truck or Universal slots — preventing the system from ever parking a 16-wheeler in a bicycle-sized slot. |
| **ParkingLot** | [parking_lot.h](file:///d:/SmartPark/parking_lot.h) | The *central manager / façade class* of the whole system. Knows about every slot, every parked vehicle, every history record, and the revenue total. The **most important line in the project** is its `std::vector<std::unique_ptr<Vehicle>> parkedVehicles;` — this container of base-class smart pointers is what *enables* runtime polymorphism. Provides `registerVehicle(type, reg)`, `releaseVehicle(reg)` (which calls `calculateFee()` through `Vehicle*`), persistence (`saveStateToFile` / `loadStateFromFile`), and enumeration accessors used by the HTTP server to build JSON. Think of it as the "parking lot brain" — all user operations eventually call into this class. |
| `ParkingRecord` (struct) | [parking_lot.h:14](file:///d:/SmartPark/parking_lot.h#L14-L22) | Plain data struct that records one completed exit. Stored inside `ParkingLot::parkingHistory` for the History tab and revenue audit. Not polymorphic itself — just a record of the fee that polymorphism computed. |

---

## 📂 Project Structure (Modular .h / .cpp Files)

```
SmartPark/
├── main.cpp               ← Menu-driven console UI + entry point (console version)
├── server.cpp             ← HTTP server + API endpoints (web version - cpp-httplib)
├── parking_api.h          ← JSON helpers: extractEpoch(), extractIntField(), JSON builders
├── httplib.h              ← cpp-httplib single-header HTTP library (vendor)
├── vehicle.h              ← Vehicle abstract base class declaration
├── vehicle.cpp            ← Vehicle class implementation
├── vehicle_types.h        ← Car / Bike / Truck derived class declarations
├── vehicle_types.cpp      ← Car / Bike / Truck implementations
├── parking_slot.h         ← ParkingSlot class + SlotType enum declaration
├── parking_slot.cpp       ← ParkingSlot implementation + helper functions
├── parking_lot.h          ← ParkingLot manager class declaration
├── parking_lot.cpp        ← ParkingLot implementation (core logic + persistence)
├── 📁 web/
│   ├── index.html         ← 5-tab dashboard UI (Dashboard / Register / Layout / Parked / History)
│   ├── style.css          ← Navy + orange theme, cards, responsive grid, mobile breakpoints
│   └── app.js             ← Vanilla JS: fetch API calls, tab switching, auto-refresh, toasts
├── 📁 .trae/specs/        ← Spec mode plan (spec.md + tasks.md) - approval trace
├── 📓 TEST_REPORT.md      ← ⭐ Full 10-case test report (actual results, curl commands, JSON evidence, OOP refs)
├── 🎤 DEMO_SCRIPT.md      ← ⭐ 10-minute presentation script for showing to the OOP teacher
├── smartpark_state.dat    ← (generated on run) file-based persistence (no DB)
└── README.md              ← This file!
```

---

## ⚙️ Compile & Run (VS Code + MinGW g++)

### Prerequisites
- **C++ Compiler**: `g++` (MinGW-w64 or equivalent) supporting C++17 or later
- Verify install: open terminal and run
  ```
  g++ --version
  ```
  You should see version info (GCC 8+ or any modern version works).

### Step 1: Open Project in VS Code
```
File → Open Folder → select the SmartPark folder
```
Open an **integrated terminal**: `Terminal → New Terminal` in VS Code.

### Step 2: Compile All Files
From the SmartPark directory run **this single command**:

```powershell
g++ -std=c++17 -Wall -Wextra main.cpp vehicle.cpp vehicle_types.cpp parking_slot.cpp parking_lot.cpp -o SmartPark.exe
```

**Explanation of flags**:
| Flag | Meaning |
|------|---------|
| `-std=c++17` | Use the C++17 standard (for modern smart pointers, `constexpr`, etc.) |
| `-Wall -Wextra` | Enable all common warnings — catches bugs early |
| `-o SmartPark.exe` | Name the executable `SmartPark.exe` (Windows) |

On Linux/macOS use `-o SmartPark` (no `.exe` extension).

### Step 3: Run the Program
After successful compilation:
```powershell
.\SmartPark.exe
```
(On Linux/macOS: `./SmartPark`)

---

## 🎮 How to Use (Console Menu)

On startup you will see:

```
  Load sample demonstration data? (y/n):
```
Type `y` to quickly load 5 sample vehicles (recommended for testing).

Then the main menu:
```
  1.  Register Vehicle Entry (Park)
  2.  Release Vehicle Exit  (Unpark + Pay)
  3.  Search Parked Vehicle
  4.  Show All Parking Slots
  5.  Show All Parked Vehicles
  6.  Show Parking History (All Exited Vehicles)
  7.  Show Revenue Summary
  8.  Show System Summary Dashboard
  9.  Run Demonstration (Polymorphism Example)   ← Try this first!
  0.  Exit SmartPark System
```

### Suggested Testing Flow
1. Run option **9** first to clearly see how runtime polymorphism works.
2. Choose **y** on startup to load sample data.
3. Use option **5** to see parked vehicles (notice polymorphic `calculateFee()` in use).
4. Use option **2** → enter a sample plate (e.g. `MH-12-AB-1234`) to see exit processing with receipt.
5. Use option **6** to view the record in Parking History.
6. Use option **7** to see revenue increase.
7. Use option **1** to park your own vehicles.
8. Try registering a vehicle with a duplicate plate — error handling will prevent it.

---

## 🧪 Runtime Polymorphism — Explanation with Code

This is the *core OOP concept* the project is meant to show.

### Setup
In `parking_lot.h` we declare:
```cpp
std::vector<std::unique_ptr<Vehicle>> parkedVehicles;
```
Each element is a **base class smart pointer**, but the actual object stored can be a `Car`, `Bike`, or `Truck`.

### Registering vehicles (in `registerVehicle()`)
```cpp
std::unique_ptr<Vehicle> newVehicle;
if (type == "Car")   newVehicle = std::make_unique<Car>(reg, slot);
if (type == "Bike")  newVehicle = std::make_unique<Bike>(reg, slot);
if (type == "Truck") newVehicle = std::make_unique<Truck>(reg, slot);
parkedVehicles.push_back(std::move(newVehicle));
```
Now we have derived objects stored as base pointers.

### The Magic: Releasing a Vehicle
When we release a vehicle, we call:
```cpp
Vehicle* vehicle = parkedVehicles[i].get();  // base pointer!
double fee = vehicle->calculateFee();        // VIRTUAL FUNCTION CALL
```

At **runtime**, C++ looks up the **actual** object type stored in that `Vehicle*`:
- If it's really a `Car`  → `Car::calculateFee()` runs  @ Rs. 40/hr
- If it's really a `Bike` → `Bike::calculateFee()` runs @ Rs. 20/hr
- If it's really a `Truck`→ `Truck::calculateFee()` runs @ Rs. 80/hr

**No `if` / `else` / `switch` needed!** This is exactly what runtime polymorphism buys you — easy extensibility: add a new vehicle type (e.g. `Scooter`) by creating a new derived class and overriding one function; you never touch `ParkingLot` or existing code (Open/Closed Principle).

---

## ✅ OOP Verification in SmartPark (with Exact Code References)

Use this section with your OOP teacher. Every claim below is anchored to a specific file and line.

### 1. Base Class and Derived Classes
| Role | Class | Declaration |
|---|---|---|
| Abstract BASE | `Vehicle` | [vehicle.h:11](file:///d:/SmartPark/vehicle.h#L11-L52) |
| Derived (is-a Vehicle) | `Car   : public Vehicle` | [vehicle_types.h:11](file:///d:/SmartPark/vehicle_types.h#L11-L36) |
| Derived (is-a Vehicle) | `Bike  : public Vehicle` | [vehicle_types.h:44](file:///d:/SmartPark/vehicle_types.h#L44-L61) |
| Derived (is-a Vehicle) | `Truck : public Vehicle` | [vehicle_types.h:69](file:///d:/SmartPark/vehicle_types.h#L69-L86) |

### 2. Where Inheritance Is Used
All three concrete types use **public inheritance** from `Vehicle`. Public inheritance means "is-a":
- [vehicle_types.h:11](file:///d:/SmartPark/vehicle_types.h#L11-L11) → `class Car   : public Vehicle`
- [vehicle_types.h:44](file:///d:/SmartPark/vehicle_types.h#L44-L44) → `class Bike  : public Vehicle`
- [vehicle_types.h:69](file:///d:/SmartPark/vehicle_types.h#L69-L69) → `class Truck : public Vehicle`

Derived-class constructors chain to the `Vehicle` base constructor so `registrationNumber`, `entryTime`, and `parkingSlotID` are initialized only once in the base.

### 3. Virtual Functions and Their Overrides
| Virtual declared in `Vehicle` (base) | Override in `Car` | Override in `Bike` | Override in `Truck` |
|---|---|---|---|
| `virtual double calculateFee() const = 0;` → pure virtual [vehicle.h:34](file:///d:/SmartPark/vehicle.h#L34-L34) | `double calculateFee() const override;` [vehicle_types.h:29](file:///d:/SmartPark/vehicle_types.h#L29-L29) impl @ [vehicle_types.cpp:27–31](file:///d:/SmartPark/vehicle_types.cpp#L27-L31) × 40.00 | [vehicle_types.h:58](file:///d:/SmartPark/vehicle_types.h#L58-L58) impl × 20.00 | [vehicle_types.h:83](file:///d:/SmartPark/vehicle_types.h#L83-L83) impl × 80.00 |
| `virtual std::string getTypeName() const = 0;` [vehicle.h:37](file:///d:/SmartPark/vehicle.h#L37-L37) | [vehicle_types.h:32](file:///d:/SmartPark/vehicle_types.h#L32-L32) returns `"Car"` | [vehicle_types.h:59](file:///d:/SmartPark/vehicle_types.h#L59-L59) returns `"Bike"` | [vehicle_types.h:84](file:///d:/SmartPark/vehicle_types.h#L84-L84) returns `"Truck"` |
| `virtual double getHourlyRate() const = 0;` [vehicle.h:40](file:///d:/SmartPark/vehicle.h#L40-L40) | [vehicle_types.h:35](file:///d:/SmartPark/vehicle_types.h#L35-L35) → 40.00 | [vehicle_types.h:60](file:///d:/SmartPark/vehicle_types.h#L60-L60) → 20.00 | [vehicle_types.h:85](file:///d:/SmartPark/vehicle_types.h#L85-L85) → 80.00 |
| `virtual ~Vehicle();` [vehicle.h:29](file:///d:/SmartPark/vehicle.h#L29-L29)  **virtual destructor — MANDATORY for safe polymorphic delete** | `~Car() override;` [vehicle_types.h:24](file:///d:/SmartPark/vehicle_types.h#L24-L24) | `~Bike() override;` | `~Truck() override;` |

### 4. How Runtime Polymorphism Is Achieved Through Base-Class Pointers
This is the step-by-step proof:

1. **Storage** — All parked vehicles are stored as *pointers to the base class*, never as derived-value objects.
   [parking_lot.h:41](file:///d:/SmartPark/parking_lot.h#L41-L41)
   ```cpp
   std::vector<std::unique_ptr<Vehicle>> parkedVehicles;
   ```

2. **Factory construction** — Objects are built as the correct derived type, then *sliced* into the base pointer for storage (no slicing actually occurs because we store *pointers*, not values — that's the whole trick!).
   [parking_lot.cpp `createVehicleObject()`](file:///d:/SmartPark/parking_lot.cpp):
   ```cpp
   if (type == "Car")   return std::make_unique<Car>  (reg, slot);
   if (type == "Bike")  return std::make_unique<Bike> (reg, slot);
   if (type == "Truck") return std::make_unique<Truck>(reg, slot);
   ```

3. **Base-pointer extraction at the call site** — when you exit a vehicle, you deliberately reduce the type back to the raw `Vehicle*` so you lose compile-time type information:
   [parking_lot.cpp:229](file:///d:/SmartPark/parking_lot.cpp#L229-L229)
   ```cpp
   Vehicle* vehicle = parkedVehicles[foundIndex].get();
   ```

4. **Virtual dispatch call** — the *actual* override is chosen at *runtime* by dereferencing the object's v-table pointer.
   [parking_lot.cpp:239](file:///d:/SmartPark/parking_lot.cpp#L239-L239)
   ```cpp
   double fee = vehicle->calculateFee();
   ```
   Secondary dispatch site in the HTTP server: [server.cpp:335](file:///d:/SmartPark/server.cpp#L335-L335) `exitRecord.feePaid = v->calculateFee();`.

5. **Evidence the dispatch worked** — in the actual HTTP test suite (see [TEST_REPORT.md](file:///d:/SmartPark/TEST_REPORT.md) TEST 3), three vehicles parked for the same duration produced fees in the exact ratio `Car(2.00) : Bike(1.00) : Truck(4.00)` — mathematically this can only happen if each derived class's own `calculateFee()` body ran, because the `HOURLY_RATE` constants are 40, 20, 80.

6. **Safe polymorphic destruction** — when the vehicle is erased from the vector, `std::unique_ptr<Vehicle>` calls `delete` on the base pointer. Because `~Vehicle()` is declared `virtual`, the derived destructor (`~Car() / ~Bike() / ~Truck()`) is invoked first, then the base destructor — zero leak, zero UB.
   [parking_lot.cpp:294](file:///d:/SmartPark/parking_lot.cpp#L294-L294) → `parkedVehicles.erase(parkedVehicles.begin() + foundIndex);`

### 5. Encapsulation, Constructors, Destructors
| Pillar | Evidence |
|---|---|
| **Encapsulation** | Data members are never `public`. Vehicle uses `protected:` for fields only derived classes need [vehicle.h:12–16](file:///d:/SmartPark/vehicle.h#L12-L16); all `private:` in Car/Bike/Truck [vehicle_types.h:12–13](file:///d:/SmartPark/vehicle_types.h#L12-L13) and ParkingLot [parking_lot.h:32–50](file:///d:/SmartPark/parking_lot.h#L32-L50). External code reads data *only* via public getters such as `getRegistrationNumber()` [vehicle.h:43](file:///d:/SmartPark/vehicle.h#L43-L43) and `getTotalRevenue()` [parking_lot.h:125](file:///d:/SmartPark/parking_lot.h#L125-L125). |
| **Constructors** | `Vehicle` has two ctors: one that sets `entryTime = now()` for new parkings [vehicle.h:20](file:///d:/SmartPark/vehicle.h#L20-L20) and one with explicit `time_point` for persistence restore [vehicle.h:23](file:///d:/SmartPark/vehicle.h#L23-L24). Each derived class mirrors both overloads and chains to the base. `ParkingLot(const std::string& name)` [parking_lot.h:82](file:///d:/SmartPark/parking_lot.h#L82-L82) seeds the 15 default slots and initialises `totalRevenue = 0.0`. |
| **Destructors** | `virtual ~Vehicle()` [vehicle.h:29](file:///d:/SmartPark/vehicle.h#L29-L29) plus `override` in each derived class. `~ParkingLot()` [parking_lot.h:85](file:///d:/SmartPark/parking_lot.h#L85-L85) (implicit body — `std::vector` and `std::unique_ptr` do all cleanup). RAII throughout: no raw `new`/`delete` pairs written by hand. |

---

## 🧾 Slot Configuration

The parking lot is initialized in `ParkingLot` constructor with **15 slots**:

| Slot IDs | Type | Suitable For |
|----------|------|-------------|
| 1 – 5    | Bike Slot    | Bike only (small) |
| 6 – 10   | Car Slot     | Bike, Car |
| 11 – 13  | Truck Slot   | Bike, Car, Truck |
| 14 – 15  | Universal    | All vehicle types |

Allocation searches in order, so Bikes use Bike slots first, freeing larger slots for larger vehicles.

---

## ⚠️ Error Handling

- Rejects invalid vehicle type choices.
- Prevents duplicate registration of same number.
- Warns when no suitable slot is available.
- Warns when searching / releasing a vehicle that is not parked.
- Clears `std::cin` on invalid numeric input to prevent infinite loops.
- Empty registration numbers are rejected.

---

## 🛠️ Troubleshooting

| Problem | Fix |
|---------|-----|
| `g++ is not recognized` | Install MinGW-w64 and add its `bin/` folder to Windows PATH; restart VS Code terminal. |
| Errors about `unique_ptr`, `make_unique` | Make sure you compile with `-std=c++17` or `-std=c++20`. |
| Fee is always very small (~Rs. 0.40) | Fees are real-time-based. Park a vehicle, wait a few minutes, then exit — or modify the code to simulate entry time offset (for testing). |
| Linker errors (undefined reference to...) | Make sure you list ALL `.cpp` files in the compile command (`main.cpp`, `vehicle.cpp`, `vehicle_types.cpp`, `parking_slot.cpp`, `parking_lot.cpp`). Missing any .cpp will cause linker errors. |

---

## 🎓 Student Notes

If you're studying OOP for an exam, pay attention to these lines in the code:
- [vehicle.h line 38](vehicle.h) — `virtual double calculateFee() const = 0;` (pure virtual → abstract class)
- [vehicle.h line 35](vehicle.h) — `virtual ~Vehicle();` (virtual destructor — *essential*!)
- [vehicle_types.h line 36](vehicle_types.h) — `double calculateFee() const override;` (override keyword)
- [parking_lot.h line 48](parking_lot.h) — `std::vector<std::unique_ptr<Vehicle>> parkedVehicles;` (the polymorphic container)
- [parking_lot.cpp line 203](parking_lot.cpp) — `double fee = vehicle->calculateFee();` (the runtime-dispatch call!)

Happy coding! 🚗🏍️🚛

---

## 🌐 Web Dashboard – Modern Responsive UI (Recommended)

SmartPark now includes a **modern, professional, responsive web dashboard** on top of the *exact same C++ core logic*. All vehicle registration, slot allocation, parking duration, and fee calculation still go through the C++ `ParkingLot` + polymorphic `Vehicle` classes — the frontend is purely a visual layer that calls C++ HTTP APIs.

### Technology Stack
| Layer | Tech |
|-------|------|
| **Backend (source of truth)** | C++17 + `cpp-httplib` (single-header HTTP server, no extra build steps) |
| **Communication** | JSON over HTTP — no duplicated JS logic |
| **Frontend** | HTML5 + CSS3 + Vanilla JavaScript (no frameworks) |
| **Persistence** | `smartpark_state.dat` – custom pipe-delimited format, no DB |
| **Thread Safety** | `std::mutex` guards `ParkingLot` across HTTP requests |

### Architecture
```
Browser (web/index.html + app.js + style.css)
        │  fetch()  ← JSON
        ▼
C++ HTTP Server (server.cpp  ← cpp-httplib)
        │  locks ParkingLot mutex
        ▼
ParkingLot (parking_lot.cpp) ─► registerVehicle() / releaseVehicle() / searchVehicle()
        │
        └──► vector<unique_ptr<Vehicle>> ─┐
                ├── Car::calculateFee()   │  ← RUNTIME POLYMORPHISM
                ├── Bike::calculateFee()  │
                └── Truck::calculateFee() ┘
```

---

## 🚀 Start the Web Dashboard in VS Code (Windows)

Follow these **exact steps** to build and launch the web UI from the C++ backend.

### Prerequisites (same as console version)
- MinGW `g++` (C++17 compatible) in your PATH
- VS Code
- Any modern web browser (Chrome / Edge / Firefox / Safari)

### Step 1 – Open project in VS Code
```
File → Open Folder → select  d:\SmartPark
```

Open the **integrated terminal**: `Terminal → New Terminal`
(It should open PowerShell inside `d:\SmartPark`)

### Step 2 – Build the C++ Web Server

Paste **this single command** into the terminal and press Enter:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -I. server.cpp vehicle.cpp vehicle_types.cpp parking_slot.cpp parking_lot.cpp -o SmartParkServer.exe -lws2_32
```

**Flag breakdown:**
| Flag | Why needed |
|------|-----------|
| `-std=c++17` | Modern C++ (smart pointers, `make_unique`, etc.) |
| `-O2` | Optimized build (server runs fast) |
| `-Wall -Wextra` | Warnings-as-sanity-checks |
| `-I.` | Include current directory (finds `httplib.h`) |
| `-o SmartParkServer.exe` | Output binary name |
| **`-lws2_32`** | **Windows only** — links Winsock2 (sockets). *Mandatory on Windows for cpp-httplib to work.* |

> `CPPHTTPLIB_IMPLEMENTATION` is already `#define`d inside `server.cpp` (line 14) so no extra `-D` flag is needed.

✅ **Success indicator** — no output (clean build) and `SmartParkServer.exe` appears in `d:\SmartPark`.

> If you see `undefined reference to socket/bind/listen`, you **forgot `-lws2_32`**. Add it!

### Step 3 – Start the Server

```powershell
.\SmartParkServer.exe
```

You should see:
```
[SmartPark] No saved state found. Starting with empty lot.
[SmartPark] Parked: 0  Revenue: Rs. 0
=================================================
  SmartPark Backend Server - starting...
  C++ Core Logic: ParkingLot + Vehicle (polymorphic)
  Listening on: http://127.0.0.1:8080
  Dashboard URL: http://127.0.0.1:8080/
  API health :   http://127.0.0.1:8080/api/health
  Press Ctrl+C to stop.
=================================================
```

✅ State is saved **every 30 seconds** + on every register/exit + on Ctrl+C shutdown. State file: `d:\SmartPark\smartpark_state.dat`.

### Step 4 – Open the Web Dashboard

**Two ways (both work, pick your favourite):**

#### ✅ Method A – Server-hosted (Recommended)
Open your browser and go to:
```
http://127.0.0.1:8080/
```
The server serves `web/index.html` directly.

#### ✅ Method B – Double-click the file
In Windows Explorer, go to `d:\SmartPark\web\` and **double-click `index.html`**.
- The JS auto-detects you're running under `file://` and talks to `http://127.0.0.1:8080` (CORS is already enabled).

---

## 🖥️ Using the Web Dashboard (5 Tabs)

### 1️⃣ Dashboard
- Shows **Total / Occupied / Available** slots + **Total Revenue** cards
- **By-Type** breakdown bars (Cars / Bikes / Trucks parked)
- **Recent Activity** list (exited vehicles + fees paid)
- Auto-refreshes every 20 seconds (and after every operation)

### 2️⃣ Register Vehicle
- Choose vehicle type from the dropdown (Car / Bike / Truck)
- Enter a registration number (e.g. `MH-05-CQ-1234`)
- Click **Register & Park Vehicle**
- The C++ backend **automatically allocates the best-fit slot**
- Success screen shows the assigned slot, entry time, and running fee

### 3️⃣ Live Parking Layout
- 15 slots rendered as **coloured cards in a grid**
- **Green** = available, **Orange** = occupied
- Each card shows: Slot #, Type, and (if occupied) vehicle registration + type
- Hit **Refresh** to get the latest state (or it auto-updates after operations)

### 4️⃣ Vehicle Exit
- Type a registration number → click **Lookup**
- Backend responds with vehicle type, slot number, entry time
- Click **Confirm Exit & Charge Fee**
  - **Fee is calculated in C++** via the polymorphic `calculateFee()` override (Bike=20/hr, Car=40/hr, Truck=80/hr)
- Receipt card shows entry/exit/duration/fee
- Slot is **released immediately** in the same operation

### 5️⃣ Parking History
- Table with: Type, Registration, Slot, Entry, Exit, Duration, Fee, Status
- Filter by: All Types / Active (Parked) / Completed (Exited) / Cars / Bikes / Trucks
- **Search plate…** box filters rows live
- Shows both currently-parked and completed records

---

## 🔌 HTTP API Endpoints (C++ Backend)

All responses are JSON with a consistent envelope:
```json
{ "success": true | false, "message": "...", "data": {...} }
```

| Method | Endpoint | Purpose |
|--------|----------|---------|
| `GET`  | `/api/health` | Server health + lot name |
| `GET`  | `/api/dashboard` | Totals, by-type counts, recent activity |
| `GET`  | `/api/slots` | All 15 slots (ID, type, available flag, parked reg/type) |
| `POST` | `/api/vehicles/register` | **Body:** `{"type":"Car","registrationNumber":"ABC"}` → registers + allocates slot |
| `GET`  | `/api/vehicles/{number}` | Lookup one parked vehicle |
| `GET`  | `/api/vehicles` | List all currently parked vehicles |
| `POST` | `/api/vehicles/exit` | **Body:** `{"registrationNumber":"ABC"}` → computes fee polymorphically, releases slot |
| `GET`  | `/api/history` | Returns `{ completed: [...], active: [...] }` records |

### Try an API test from PowerShell
(Use a temporary JSON file to avoid PowerShell quote-escaping issues)
```powershell
Set-Content reg.json '{"type":"Bike","registrationNumber":"API-BIKE-01"}'
curl.exe -s -X POST -H "Content-Type: application/json" --data-binary "@reg.json" http://127.0.0.1:8080/api/vehicles/register
Remove-Item reg.json
```

---

## 🛠️ Troubleshooting the Web Setup

| Problem | Fix |
|---------|-----|
| `g++ is not recognized` | Install MinGW-w64, add its `bin/` to PATH, restart VS Code terminal. |
| Linker errors: `undefined reference to socket@…` | You forgot `-lws2_32` on the end of the compile command. Re-add it. |
| `Address already in use: 8080` | Another program (or an old SmartParkServer.exe) still has port 8080. Close it or change the port in `server.cpp` last line. |
| Browser shows `Cannot GET /` | Server not running, or wrong URL. Confirm `.\SmartParkServer.exe` is running, then open `http://127.0.0.1:8080/`. |
| Open `index.html` double-click → red "Server OFFLINE" | C++ server isn't running. Start `.\SmartParkServer.exe` first. |
| curl.exe POST always returns `"Invalid vehicle type"` | PowerShell is mangling quotes in `-d "..."` args. Use `--data-binary "@file.json"` instead (shown above). The dashboard UI never has this issue because `fetch()` + `JSON.stringify()` build JSON correctly. |
| Stale data in Dashboard / Layout | Click **Refresh**, or wait 20 seconds for auto-refresh. |
| Want to reset all data | Stop the server (Ctrl+C), then delete `d:\SmartPark\smartpark_state.dat`, then restart. |

---

## 🔁 Rebuilding the Console Version

If you still want the original menu-driven console UI, it still works:
```powershell
g++ -std=c++17 -Wall -Wextra main.cpp vehicle.cpp vehicle_types.cpp parking_slot.cpp parking_lot.cpp -o SmartPark.exe
.\SmartPark.exe
```
The two binaries (`SmartPark.exe` console, `SmartParkServer.exe` web) share **all the same core .cpp files** — polymorphism is demonstrated in both.

---

## 📚 Project Rules Recap (per requirements)

✅ **C++ is the single source of truth** — vehicle registration, slot allocation, duration, and fee calculation all happen in `ParkingLot` / `Vehicle` classes. No JS re-implementation of any business logic.

✅ **`extractEpoch` & `extractIntField` (in [parking_api.h](parking_api.h)) accept exactly 3 parameters each** as required:
```cpp
bool extractEpoch(const system_clock::time_point& timePoint, const string& fieldName, long long& outEpochMs);
bool extractIntField(int value, const string& fieldName, int& outValue);
```

✅ **`httplib.h` is a single-header download** (cpp-httplib v0.16.2). No build system / package manager needed.

✅ **No database.** State is persisted to `smartpark_state.dat` by the C++ backend.

✅ **Responsive design.** Breakpoints at 980px (tablet) and 640px (mobile). Works on phones, tablets, and desktops.

✅ **Navy + orange theme.** White/light-grey background, dark-navy text, orange accents, cards with soft shadows and rounded corners (see [web/style.css](web/style.css)).

---

## 🎯 Quick Start Cheat Sheet (copy-paste)

```powershell
# 1. Build server
g++ -std=c++17 -O2 -Wall -Wextra -I. server.cpp vehicle.cpp vehicle_types.cpp parking_slot.cpp parking_lot.cpp -o SmartParkServer.exe -lws2_32

# 2. Start server
.\SmartParkServer.exe

# 3. Open browser →  http://127.0.0.1:8080/
#                         DONE! 🚗✨
```
