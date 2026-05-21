# ✈️ Airline Reservation System

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=cplusplus"/>
  <img src="https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows"/>
  <img src="https://img.shields.io/badge/Database-SQLite-003B57?style=for-the-badge&logo=sqlite"/>
  <img src="https://img.shields.io/badge/IDE-Visual%20Studio-5C2D91?style=for-the-badge&logo=visualstudio"/>
  <img src="https://img.shields.io/badge/UI-Win32%20API-gray?style=for-the-badge"/>
</p>

<p align="center">
  A fully functional <strong>Windows Desktop Application</strong> for managing airline flights, passengers, and reservations.<br/>
  Built with <strong>C++ Win32 API</strong> and <strong>SQLite</strong> as the embedded local database.
</p>

---

## 📋 Table of Contents

- [About the Project](#-about-the-project)
- [Screenshots](#-screenshots)
- [Features](#-features)
- [Database Structure](#-database-structure)
- [Tech Stack](#-tech-stack)
- [Getting Started](#-getting-started)
- [How to Use](#-how-to-use)
- [Project Structure](#-project-structure)
- [Author](#-author)

---

## 📌 About the Project

This project is a **Windows Programming** desktop application developed using **C++** and the **Win32 API** through Visual Studio. It simulates a real airline reservation system where administrators can manage flights, register passengers, and handle bookings — all stored locally in a **SQLite** database with no need for an internet connection or external database server.

The project was built as part of the **Windows Programming** course, demonstrating:
- Native Windows GUI development using Win32 API
- Embedded database integration with SQLite
- Full CRUD operations (Create, Read, Update, Delete)
- Multiple search strategies on database records

---

## 📸 Screenshots

### 🏠 Main Window
<!-- 📂 Save your screenshot as: screenshots/main_window.png -->
![Main Window](screenshots/main_window.png)

> The main dashboard where you can navigate between Flights, Passengers, and Reservations.

---

### ✈️ Flights Management
<!-- 📂 Save your screenshot as: screenshots/flights.png -->
![Flights Management](screenshots/flights.png)

> Add, edit, delete, and view all available flights. Includes seat availability tracking.

---

### 🔍 Search Flights
<!-- 📂 Save your screenshot as: screenshots/search_flights.png -->
![Search Flights](screenshots/search_flights.png)

> **Search Method 1:** Search by Flight Number — find an exact or partial flight code (e.g. `MS-4`).  
> **Search Method 2:** Search by Destination — find all flights going to a specific city.

---

### 🎫 Reservations
<!-- 📂 Save your screenshot as: screenshots/reservations.png -->
![Reservations](screenshots/reservations.png)

> Book a seat on a flight for a passenger. The system automatically checks seat availability and updates the count upon booking or cancellation.

---

## ⚙️ Features

| Feature | Description |
|---|---|
| ✅ **Add Flight** | Insert new flight with number, origin, destination, date, time, seats, and price |
| ✅ **Edit Flight** | Update any flight's information |
| ✅ **Delete Flight** | Remove a flight from the system |
| ✅ **View All Flights** | Display all flights in a list/table |
| ✅ **Search by Flight Number** | Partial or full match on flight code |
| ✅ **Search by Destination** | Find all flights to a specific city |
| ✅ **Add Passenger** | Register passenger with name, national ID, phone, email |
| ✅ **Edit / Delete Passenger** | Full passenger record management |
| ✅ **Book Reservation** | Link a passenger to a flight with seat assignment |
| ✅ **Cancel Reservation** | Mark a booking as cancelled and restore the seat |
| ✅ **Seat Availability Tracking** | Auto-decrements/increments available seats on booking or cancellation |
| ✅ **SQLite Integration** | All data persists in a local `.db` file — no server required |

---

## 🗄️ Database Structure

The application uses **3 tables** in SQLite:

```
Flights
├── id             (INTEGER, Primary Key, Auto-increment)
├── flightNumber   (TEXT, Unique)   e.g. "MS-401"
├── origin         (TEXT)
├── destination    (TEXT)
├── departureDate  (TEXT)           "YYYY-MM-DD"
├── departureTime  (TEXT)           "HH:MM"
├── totalSeats     (INTEGER)
├── availableSeats (INTEGER)
└── ticketPrice    (REAL)

Passengers
├── id           (INTEGER, Primary Key, Auto-increment)
├── name         (TEXT)
├── nationalId   (TEXT, Unique)
├── phone        (TEXT)
└── email        (TEXT)

Reservations
├── id            (INTEGER, Primary Key, Auto-increment)
├── flightId      (INTEGER, Foreign Key → Flights)
├── passengerId   (INTEGER, Foreign Key → Passengers)
├── bookingDate   (TEXT)
├── seatNumber    (TEXT)
└── status        (TEXT)           "Confirmed" / "Cancelled"
```

---

## 🛠️ Tech Stack

| Component | Technology |
|---|---|
| Programming Language | C++ |
| GUI Framework | Win32 API (Windows Desktop Application) |
| Database | SQLite 3 (embedded, single `.db` file) |
| IDE | Microsoft Visual Studio |
| Target Platform | Windows 10 / 11 |

---

## 🚀 Getting Started

### Prerequisites

- Windows 10 or 11
- Visual Studio 2019 or later (with Desktop development with C++ workload)
- SQLite3 — already included in the project (`sqlite3.h` + `sqlite3.c`)

### Build & Run

1. **Clone the repository:**
   ```bash
   git clone https://github.com/XONepTon/airline_reservation_system.git
   cd airline_reservation_system
   ```

2. **Open in Visual Studio:**
   - Open `airline_reservation_system.sln`

3. **Make sure SQLite files are in the project:**
   - `sqlite3.h` and `sqlite3.c` must be in the project directory
   - Both files must be added to the Visual Studio project

4. **Build the solution:**
   - Press `Ctrl + Shift + B` or go to **Build → Build Solution**

5. **Run the application:**
   - Press `F5` or click **Local Windows Debugger**
   - The database file `airline.db` will be created automatically on first run

---

## 📖 How to Use

### Managing Flights
- Click **"Flights"** from the main menu
- Use **Add** to insert a new flight — fill in all required fields
- Select a flight from the list and click **Edit** to modify it
- Click **Delete** to remove a flight
- Use the **Search by Flight Number** field or **Search by Destination** field to filter results

### Managing Passengers
- Click **"Passengers"** from the main menu
- Add passengers with their National ID (must be unique)
- Search by name or by National ID

### Making a Reservation
- Click **"Reservations"** from the main menu
- Select a **Flight** and a **Passenger** from the dropdowns
- Enter a **Seat Number** and click **Book**
- To cancel, select a reservation and click **Cancel Booking**

---

## 📁 Project Structure

```
airline_reservation_system/
│
├── airline_db.h          # Database header — structs & function declarations
├── airline_db.cpp        # Database implementation — all SQLite CRUD logic
├── main.cpp              # WinMain entry point & main window procedure
├── resource.h            # Windows resource IDs
├── airline.rc            # Resource file (dialogs, menus, icons)
├── sqlite3.h             # SQLite amalgamation header
├── sqlite3.c             # SQLite amalgamation source
│
├── screenshots/          # 📂 Put your app screenshots here
│   ├── main_window.png
│   ├── flights.png
│   ├── search_flights.png
│   ├── passengers.png
│   └── reservations.png
│
└── README.md
```

---

## 👨‍💻 Author

**ramadan ragab **  
Faculty of Industrial Technology and Energy — Delta Technological University  
Windows Programming Course — [Year]

[![GitHub](https://img.shields.io/badge/GitHub-XONepTon-181717?style=flat&logo=github)](https://github.com/XONepTon)

---

## 📄 License

This project is for educational purposes.  
Feel free to use or reference it for your own learning.
