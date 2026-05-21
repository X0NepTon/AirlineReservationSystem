#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <sstream>
#include "../sqlite3.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")

using namespace std;

// ==================== Color Palette ====================
#define CLR_BG       RGB(15, 23, 42)
#define CLR_SURFACE  RGB(30, 41, 59)
#define CLR_TEXT     RGB(248, 250, 252)
#define CLR_MUTED    RGB(148, 163, 184)
#define CLR_BTN_ADD  RGB(6, 182, 212)
#define CLR_BTN_UPD  RGB(245, 158, 11)
#define CLR_BTN_DEL  RGB(239, 68, 68)

// ==================== Control IDs ====================
#define IDC_TAB                    1001

#define IDC_EDIT_PNAME             2001
#define IDC_EDIT_PID               2002
#define IDC_EDIT_PPHONE            2003
#define IDC_BTN_ADD_PASSENGER      2004
#define IDC_BTN_UPDATE_PASSENGER   2005
#define IDC_BTN_DELETE_PASSENGER   2006
#define IDC_LIST_PASSENGERS        2007

#define IDC_EDIT_FLIGHTNUM         3001
#define IDC_EDIT_FROM              3002
#define IDC_EDIT_TO                3003
#define IDC_EDIT_PRICE             3004
#define IDC_BTN_ADD_FLIGHT         3005
#define IDC_BTN_UPDATE_FLIGHT      3006
#define IDC_BTN_DELETE_FLIGHT      3007
#define IDC_LIST_FLIGHTS           3008
#define IDC_EDIT_DEPARTURE         3009
#define IDC_EDIT_ARRIVAL           3010
#define IDC_COMBO_STATUS           3011

#define IDC_COMBO_PASSENGER        4001
#define IDC_COMBO_FLIGHT           4002
#define IDC_EDIT_SEAT              4003
#define IDC_BTN_ADD_BOOKING        4004
#define IDC_BTN_DELETE_BOOKING     4005
#define IDC_LIST_BOOKINGS          4006
#define IDC_BTN_GROUP_BOOKING      4007
#define IDC_LIST_SELECTED_PASSENGERS 4008
#define IDC_BTN_ADD_TO_GROUP       4009
#define IDC_BTN_REMOVE_FROM_GROUP  4010
#define IDC_BTN_CLEAR_GROUP        4011

#define IDC_EDIT_SEARCH_ID         5001
#define IDC_BTN_SEARCH_ID          5002
#define IDC_EDIT_SEARCH_NAME       5003
#define IDC_BTN_SEARCH_NAME        5004
#define IDC_LIST_SEARCH            5005

// ==================== Structs ====================
struct Passenger {
    int id;
    wstring name, nationalID, phone;
};

struct Flight {
    int id;
    wstring flightNumber, from, to, status;
    string departure;   // Format: "2026-05-20 14:30"
    string arrival;
    double price;
};

struct Booking {
    int id, passengerID, flightID;
    wstring passengerName, flightNumber, seat;
};
// ==================== Globals ====================
HINSTANCE hInst;
HWND hMainWnd = nullptr, hTab = nullptr;
HWND hPanels[4] = { nullptr };

HWND hListPassengers = nullptr;
HWND hListFlights = nullptr;
HWND hListBookings = nullptr;
HWND hListSearch = nullptr;
HWND hListSelectedPassengers = nullptr;

HWND hEditPName = nullptr, hEditPID = nullptr, hEditPPhone = nullptr;
HWND hEditFlightNum = nullptr, hEditFrom = nullptr, hEditTo = nullptr;
HWND hEditPrice = nullptr, hEditDeparture = nullptr, hEditArrival = nullptr;
HWND hComboStatus = nullptr;
HWND hComboPassenger = nullptr, hComboFlight = nullptr, hEditSeat = nullptr;
HWND hEditSearchID = nullptr, hEditSearchName = nullptr;

sqlite3* db = nullptr;

vector<Passenger> passengers;
vector<Flight> flights;
vector<Booking> bookings;
vector<int> selectedGroupPassengers;

int selectedPassengerID = -1;
int selectedFlightID = -1;
int selectedBookingID = -1;

HFONT hFontUI = nullptr, hFontList = nullptr, hFontHeader = nullptr, hFontBtn = nullptr;
HBRUSH hBrushBg = nullptr, hBrushSurface = nullptr;

// ==================== Helper Functions ====================
HFONT MakeFont(int pt, int weight, const wchar_t* face) {
    return CreateFont(-MulDiv(pt, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72),
        0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
}

void CreateResources() {
    hFontUI = MakeFont(10, FW_NORMAL, L"Segoe UI");
    hFontList = MakeFont(9, FW_NORMAL, L"Segoe UI");
    hFontHeader = MakeFont(13, FW_LIGHT, L"Segoe UI");
    hFontBtn = MakeFont(9, FW_SEMIBOLD, L"Segoe UI");
}

void DestroyResources() {
    DeleteObject(hFontUI);
    DeleteObject(hFontList);
    DeleteObject(hFontHeader);
    DeleteObject(hFontBtn);
    DeleteObject(hBrushBg);
    DeleteObject(hBrushSurface);
}

wstring Utf8ToWide(const char* s) {
    if (!s) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    wstring r(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &r[0], n);
    return r;
}

string WideToUtf8(const wstring& w) {
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, NULL, 0, NULL, NULL);
    string r(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &r[0], n, NULL, NULL);
    return r;
}

// ==================== Database ====================
void InitDatabase() {
    if (sqlite3_open("airline.db", &db) != SQLITE_OK) {
        MessageBox(NULL, L"Failed to open database!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    const char* sqls[] = {
        "CREATE TABLE IF NOT EXISTS Passengers(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, nationalID TEXT UNIQUE NOT NULL, phone TEXT);",
        "CREATE TABLE IF NOT EXISTS Flights(id INTEGER PRIMARY KEY AUTOINCREMENT, flightNumber TEXT NOT NULL UNIQUE, fromCity TEXT NOT NULL, toCity TEXT NOT NULL, departure DATETIME NOT NULL, arrival DATETIME NOT NULL, price REAL NOT NULL, status TEXT DEFAULT 'Scheduled');",
        "CREATE TABLE IF NOT EXISTS Bookings(id INTEGER PRIMARY KEY AUTOINCREMENT, passengerID INTEGER, flightID INTEGER, seat TEXT, bookingDate DATETIME DEFAULT CURRENT_TIMESTAMP, FOREIGN KEY(passengerID) REFERENCES Passengers(id), FOREIGN KEY(flightID) REFERENCES Flights(id));"
    };

    for (auto s : sqls) {
        char* err = nullptr;
        sqlite3_exec(db, s, nullptr, nullptr, &err);
        if (err) {
            MessageBoxW(NULL, Utf8ToWide(err).c_str(), L"Database Error", MB_OK | MB_ICONERROR);
            sqlite3_free(err);
        }
    }
}
// ==================== Load Data ====================
void LoadPassengers() {
    passengers.clear();
    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, "SELECT id,name,nationalID,phone FROM Passengers", -1, &s, nullptr);
    while (sqlite3_step(s) == SQLITE_ROW) {
        Passenger p;
        p.id = sqlite3_column_int(s, 0);
        p.name = Utf8ToWide((const char*)sqlite3_column_text(s, 1));
        p.nationalID = Utf8ToWide((const char*)sqlite3_column_text(s, 2));
        p.phone = Utf8ToWide((const char*)sqlite3_column_text(s, 3));
        passengers.push_back(p);
    }
    sqlite3_finalize(s);
}

void LoadFlights() {
    flights.clear();
    const char* sql = "SELECT id, flightNumber, fromCity, toCity, departure, arrival, price, status FROM Flights ORDER BY departure";
    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
    while (sqlite3_step(s) == SQLITE_ROW) {
        Flight f;
        f.id = sqlite3_column_int(s, 0);
        f.flightNumber = Utf8ToWide((const char*)sqlite3_column_text(s, 1));
        f.from = Utf8ToWide((const char*)sqlite3_column_text(s, 2));
        f.to = Utf8ToWide((const char*)sqlite3_column_text(s, 3));
        f.departure = (const char*)sqlite3_column_text(s, 4) ? (const char*)sqlite3_column_text(s, 4) : "";
        f.arrival = (const char*)sqlite3_column_text(s, 5) ? (const char*)sqlite3_column_text(s, 5) : "";
        f.price = sqlite3_column_double(s, 6);
        f.status = Utf8ToWide((const char*)sqlite3_column_text(s, 7));
        flights.push_back(f);
    }
    sqlite3_finalize(s);
}

void LoadBookings() {
    bookings.clear();
    const char* sql = "SELECT b.id, b.passengerID, b.flightID, p.name, f.flightNumber, b.seat "
        "FROM Bookings b JOIN Passengers p ON b.passengerID = p.id "
        "JOIN Flights f ON b.flightID = f.id";
    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
    while (sqlite3_step(s) == SQLITE_ROW) {
        Booking b;
        b.id = sqlite3_column_int(s, 0);
        b.passengerID = sqlite3_column_int(s, 1);
        b.flightID = sqlite3_column_int(s, 2);
        b.passengerName = Utf8ToWide((const char*)sqlite3_column_text(s, 3));
        b.flightNumber = Utf8ToWide((const char*)sqlite3_column_text(s, 4));
        b.seat = Utf8ToWide((const char*)sqlite3_column_text(s, 5));
        bookings.push_back(b);
    }
    sqlite3_finalize(s);
}

// ==================== Refresh Functions ====================
void RefreshPassengerList() {
    LoadPassengers();
    SendMessage(hListPassengers, LB_RESETCONTENT, 0, 0);
    for (auto& p : passengers) {
        wstringstream ss;
        ss << L" #" << p.id << L" " << p.name << L" | ID: " << p.nationalID << L" | Phone: " << p.phone;
        SendMessage(hListPassengers, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void RefreshFlightList() {
    LoadFlights();
    SendMessage(hListFlights, LB_RESETCONTENT, 0, 0);
    for (auto& f : flights) {
        wstringstream ss;
        ss << L" #" << f.id << L" " << f.flightNumber
            << L" | " << f.from << L" → " << f.to
            << L" | " << Utf8ToWide(f.departure.c_str())
            << L" | $" << f.price << L" | " << f.status;
        SendMessage(hListFlights, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void RefreshBookingList() {
    LoadBookings();
    SendMessage(hListBookings, LB_RESETCONTENT, 0, 0);
    for (auto& b : bookings) {
        wstringstream ss;
        ss << L" #" << b.id << L" " << b.passengerName
            << L" | F+light: " << b.flightNumber << L" | Seat: " << b.seat;
        SendMessage(hListBookings, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void RefreshCombos() {
    LoadPassengers();
    LoadFlights();

    SendMessage(hComboPassenger, CB_RESETCONTENT, 0, 0);
    for (auto& p : passengers) {
        wstringstream ss;
        ss << L"[" << p.id << L"] " << p.name;
        SendMessage(hComboPassenger, CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }

    SendMessage(hComboFlight, CB_RESETCONTENT, 0, 0);
    for (auto& f : flights) {
        wstringstream ss;
        ss << L"[" << f.id << L"] " << f.flightNumber
            << L" | " << f.from << L" → " << f.to
            << L" | " << Utf8ToWide(f.departure.c_str());
        SendMessage(hComboFlight, CB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
}

void RefreshSelectedGroupList() {
    SendMessage(hListSelectedPassengers, LB_RESETCONTENT, 0, 0);
    for (int pid : selectedGroupPassengers) {
        for (auto& p : passengers) {
            if (p.id == pid) {
                wstringstream ss;
                ss << L"[" << p.id << L"] " << p.name;
                SendMessage(hListSelectedPassengers, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
                break;
            }
        }
    }
}

// ==================== Group Booking Functions ====================
void AddToGroup() {
    int idx = (int)SendMessage(hComboPassenger, CB_GETCURSEL, 0, 0);
    if (idx == CB_ERR) {
        MessageBox(hMainWnd, L"Please select a passenger.", L"Warning", MB_OK | MB_ICONWARNING);
        return;
    }
    int pid = passengers[idx].id;

    for (int id : selectedGroupPassengers) {
        if (id == pid) {
            MessageBox(hMainWnd, L"Passenger already added to group.", L"Info", MB_OK | MB_ICONINFORMATION);
            return;
        }
    }

    if (selectedGroupPassengers.size() >= 9) {
        MessageBox(hMainWnd, L"Maximum 9 passengers allowed in one group.", L"Limit", MB_OK | MB_ICONWARNING);
        return;
    }

    selectedGroupPassengers.push_back(pid);
    RefreshSelectedGroupList();
}

void RemoveFromGroup() {
    int idx = (int)SendMessage(hListSelectedPassengers, LB_GETCURSEL, 0, 0);
    if (idx == LB_ERR) return;
    selectedGroupPassengers.erase(selectedGroupPassengers.begin() + idx);
    RefreshSelectedGroupList();
}

void ClearGroup() {
    selectedGroupPassengers.clear();
    RefreshSelectedGroupList();
}

void AddGroupBooking() {
    if (selectedGroupPassengers.empty()) {
        MessageBox(hMainWnd, L"Please add at least one passenger to the group.", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    int flightIdx = (int)SendMessage(hComboFlight, CB_GETCURSEL, 0, 0);
    if (flightIdx == CB_ERR) {
        MessageBox(hMainWnd, L"Please select a flight.", L"Error", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t seat[50] = L"";
    GetWindowText(hEditSeat, seat, 50);

    sqlite3_stmt* s = nullptr;
    const char* sql = "INSERT INTO Bookings(passengerID, flightID, seat) VALUES(?,?,?)";

    int success = 0;

    for (int pid : selectedGroupPassengers) {
        if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(s, 1, pid);
            sqlite3_bind_int(s, 2, flights[flightIdx].id);
            sqlite3_bind_text(s, 3, WideToUtf8(seat).c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(s) == SQLITE_DONE) success++;
            sqlite3_finalize(s);
        }
    }

    if (success > 0) {
        wstringstream msg;
        msg << L"Successfully booked " << success << L" passenger(s).";
        MessageBox(hMainWnd, msg.str().c_str(), L"Success", MB_OK | MB_ICONINFORMATION);

        ClearGroup();
        SetWindowText(hEditSeat, L"");
        RefreshBookingList();
    }
    else {
        MessageBox(hMainWnd, L"Failed to create bookings.", L"Error", MB_OK | MB_ICONERROR);
    }
}
// ==================== CRUD Operations ====================

void AddPassenger() {
    wchar_t name[256] = L"", nid[256] = L"", phone[256] = L"";
    GetWindowText(hEditPName, name, 256);
    GetWindowText(hEditPID, nid, 256);
    GetWindowText(hEditPPhone, phone, 256);

    if (!wcslen(name) || !wcslen(nid)) {
        MessageBox(hMainWnd, L"Name and National ID are required!", L"Validation Error", MB_OK | MB_ICONWARNING);
        return;
    }

    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, "INSERT INTO Passengers(name, nationalID, phone) VALUES(?,?,?)", -1, &s, nullptr);
    sqlite3_bind_text(s, 1, WideToUtf8(name).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 2, WideToUtf8(nid).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 3, WideToUtf8(phone).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(s);
    sqlite3_finalize(s);

    SetWindowText(hEditPName, L"");
    SetWindowText(hEditPID, L"");
    SetWindowText(hEditPPhone, L"");

    RefreshPassengerList();
    RefreshCombos();
    MessageBox(hMainWnd, L"Passenger added successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
}

void UpdatePassenger() {
    if (selectedPassengerID == -1) {
        MessageBox(hMainWnd, L"Please select a passenger to update.", L"No Selection", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t name[256], nid[256], phone[256];
    GetWindowText(hEditPName, name, 256);
    GetWindowText(hEditPID, nid, 256);
    GetWindowText(hEditPPhone, phone, 256);

    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, "UPDATE Passengers SET name=?, nationalID=?, phone=? WHERE id=?", -1, &s, nullptr);
    sqlite3_bind_text(s, 1, WideToUtf8(name).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 2, WideToUtf8(nid).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 3, WideToUtf8(phone).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(s, 4, selectedPassengerID);
    sqlite3_step(s);
    sqlite3_finalize(s);

    RefreshPassengerList();
    RefreshCombos();
    MessageBox(hMainWnd, L"Passenger updated successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
}

void DeletePassenger() {
    if (selectedPassengerID == -1) {
        MessageBox(hMainWnd, L"Please select a passenger to delete.", L"No Selection", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBox(hMainWnd, L"Are you sure you want to delete this passenger?", L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "DELETE FROM Passengers WHERE id=?", -1, &s, nullptr);
        sqlite3_bind_int(s, 1, selectedPassengerID);
        sqlite3_step(s);
        sqlite3_finalize(s);

        selectedPassengerID = -1;
        RefreshPassengerList();
        RefreshCombos();
        MessageBox(hMainWnd, L"Passenger deleted successfully!", L"Deleted", MB_OK | MB_ICONINFORMATION);
    }
}
void AddFlight() {
    wchar_t num[100] = L"", from[100] = L"", to[100] = L"",
        dep[100] = L"", arr[100] = L"", priceStr[50] = L"";

    GetWindowText(hEditFlightNum, num, 100);
    GetWindowText(hEditFrom, from, 100);
    GetWindowText(hEditTo, to, 100);
    GetWindowText(hEditDeparture, dep, 100);
    GetWindowText(hEditArrival, arr, 100);
    GetWindowText(hEditPrice, priceStr, 50);

    // Validation
    if (!wcslen(num) || !wcslen(from) || !wcslen(to) || !wcslen(dep) || !wcslen(arr)) {
        MessageBox(hMainWnd, L"All fields are required!", L"Validation Error", MB_OK | MB_ICONWARNING);
        return;
    }

    if (_wtof(priceStr) < 0) {
        MessageBox(hMainWnd, L"Price must be greater than zero.", L"Validation Error", MB_OK | MB_ICONWARNING);
        return;
    }

    sqlite3_stmt* s = nullptr;
    const char* sql = "INSERT INTO Flights(flightNumber, fromCity, toCity, departure, arrival, price, status) "
        "VALUES(?,?,?,?,?,?,?)";

    if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) != SQLITE_OK) {
        MessageBox(hMainWnd, L"Failed to prepare statement.", L"Database Error", MB_OK | MB_ICONERROR);
        return;
    }

    sqlite3_bind_text(s, 1, WideToUtf8(num).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 2, WideToUtf8(from).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 3, WideToUtf8(to).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 4, WideToUtf8(dep).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 5, WideToUtf8(arr).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(s, 6, _wtof(priceStr));

    // Status
    int statusIdx = (int)SendMessage(hComboStatus, CB_GETCURSEL, 0, 0);
    wchar_t statusText[50] = L"Scheduled";
    if (statusIdx != CB_ERR)
        SendMessage(hComboStatus, CB_GETLBTEXT, statusIdx, (LPARAM)statusText);

    sqlite3_bind_text(s, 7, WideToUtf8(statusText).c_str(), -1, SQLITE_TRANSIENT);

    // Execute
    int result = sqlite3_step(s);
    sqlite3_finalize(s);

    if (result == SQLITE_DONE) {
        MessageBox(hMainWnd, L"Flight added successfully!", L"Success", MB_OK | MB_ICONINFORMATION);

        // Clear fields
        SetWindowText(hEditFlightNum, L"");
        SetWindowText(hEditFrom, L"");
        SetWindowText(hEditTo, L"");
        SetWindowText(hEditDeparture, L"");
        SetWindowText(hEditArrival, L"");
        SetWindowText(hEditPrice, L"");

        RefreshFlightList();
        RefreshCombos();
    }
    else {
        const char* err = sqlite3_errmsg(db);
        wstring errMsg = L"Failed to add flight: " + Utf8ToWide(err);
        MessageBoxW(hMainWnd, errMsg.c_str(), L"Database Error", MB_OK | MB_ICONERROR);
    }
}

void UpdateFlight() {
    if (selectedFlightID == -1) {
        MessageBox(hMainWnd, L"Please select a flight to update.", L"No Selection", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t num[100] = L"", from[100] = L"", to[100] = L"",
        dep[100] = L"", arr[100] = L"", priceStr[50] = L"";

    GetWindowText(hEditFlightNum, num, 100);
    GetWindowText(hEditFrom, from, 100);
    GetWindowText(hEditTo, to, 100);
    GetWindowText(hEditDeparture, dep, 100);
    GetWindowText(hEditArrival, arr, 100);
    GetWindowText(hEditPrice, priceStr, 50);

    // Validation
    if (!wcslen(num) || !wcslen(from) || !wcslen(to) || !wcslen(dep) || !wcslen(arr)) {
        MessageBox(hMainWnd, L"All fields are required!", L"Validation Error", MB_OK | MB_ICONWARNING);
        return;
    }

    if (_wtof(priceStr) < 0) {
        MessageBox(hMainWnd, L"Price must be greater than zero.", L"Validation Error", MB_OK | MB_ICONWARNING);
        return;
    }

    sqlite3_stmt* s = nullptr;
    const char* sql = "UPDATE Flights SET flightNumber=?, fromCity=?, toCity=?, "
        "departure=?, arrival=?, price=?, status=? WHERE id=?";

    if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) != SQLITE_OK) {
        MessageBox(hMainWnd, L"Failed to prepare update statement.", L"Database Error", MB_OK | MB_ICONERROR);
        return;
    }

    sqlite3_bind_text(s, 1, WideToUtf8(num).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 2, WideToUtf8(from).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 3, WideToUtf8(to).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 4, WideToUtf8(dep).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 5, WideToUtf8(arr).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(s, 6, _wtof(priceStr));

    // Get selected status
    int statusIdx = (int)SendMessage(hComboStatus, CB_GETCURSEL, 0, 0);
    wchar_t statusText[50] = L"Scheduled";
    if (statusIdx != CB_ERR)
        SendMessage(hComboStatus, CB_GETLBTEXT, statusIdx, (LPARAM)statusText);

    sqlite3_bind_text(s, 7, WideToUtf8(statusText).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(s, 8, selectedFlightID);

    int result = sqlite3_step(s);
    sqlite3_finalize(s);

    if (result == SQLITE_DONE) {
        RefreshFlightList();
        RefreshCombos();
        MessageBox(hMainWnd, L"Flight updated successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
    }
    else {
        const char* err = sqlite3_errmsg(db);
        wstring errMsg = L"Update failed: " + Utf8ToWide(err);
        MessageBoxW(hMainWnd, errMsg.c_str(), L"Database Error", MB_OK | MB_ICONERROR);
    }
}
void DeleteFlight() {
    if (selectedFlightID == -1) {
        MessageBox(hMainWnd, L"Please select a flight to delete.", L"No Selection", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBox(hMainWnd, L"Are you sure you want to delete this flight?", L"Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "DELETE FROM Flights WHERE id=?", -1, &s, nullptr);
        sqlite3_bind_int(s, 1, selectedFlightID);
        sqlite3_step(s);
        sqlite3_finalize(s);

        selectedFlightID = -1;
        RefreshFlightList();
        RefreshCombos();
        MessageBox(hMainWnd, L"Flight deleted successfully!", L"Deleted", MB_OK | MB_ICONINFORMATION);
    }
}

void AddBooking() {
    int pi = (int)SendMessage(hComboPassenger, CB_GETCURSEL, 0, 0);
    int fi = (int)SendMessage(hComboFlight, CB_GETCURSEL, 0, 0);

    if (pi == CB_ERR || fi == CB_ERR) {
        MessageBox(hMainWnd, L"Please select a passenger and a flight.", L"Validation", MB_OK | MB_ICONWARNING);
        return;
    }

    wchar_t seat[256] = L"";
    GetWindowText(hEditSeat, seat, 256);

    sqlite3_stmt* s = nullptr;
    const char* sql = "INSERT INTO Bookings(passengerID, flightID, seat) VALUES(?,?,?)";

    if (sqlite3_prepare_v2(db, sql, -1, &s, nullptr) != SQLITE_OK) {
        const char* err = sqlite3_errmsg(db);
        wstring msg = L"Prepare failed: " + Utf8ToWide(err);
        MessageBoxW(hMainWnd, msg.c_str(), L"Database Error", MB_OK | MB_ICONERROR);
        return;
    }

    sqlite3_bind_int(s, 1, passengers[pi].id);
    sqlite3_bind_int(s, 2, flights[fi].id);
    sqlite3_bind_text(s, 3, WideToUtf8(seat).c_str(), -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(s);
    sqlite3_finalize(s);

    if (result == SQLITE_DONE) {
        SetWindowText(hEditSeat, L"");
        RefreshBookingList();
        MessageBox(hMainWnd, L"Booking confirmed successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
    }
    else {
        const char* err = sqlite3_errmsg(db);
        wstring msg = L"Failed to book: " + Utf8ToWide(err);
        MessageBoxW(hMainWnd, msg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}

void DeleteBooking() {
    if (selectedBookingID == -1) {
        MessageBox(hMainWnd, L"Please select a booking to cancel.", L"No Selection", MB_OK | MB_ICONWARNING);
        return;
    }
    if (MessageBox(hMainWnd, L"Are you sure you want to cancel this booking?", L"Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "DELETE FROM Bookings WHERE id=?", -1, &s, nullptr);
        sqlite3_bind_int(s, 1, selectedBookingID);
        sqlite3_step(s);
        sqlite3_finalize(s);

        selectedBookingID = -1;
        RefreshBookingList();
        MessageBox(hMainWnd, L"Booking cancelled successfully!", L"Cancelled", MB_OK | MB_ICONINFORMATION);
    }
}

void SearchByID();
void SearchByName();
// ==================== Search Functions ====================
void SearchByID() {
    wchar_t idStr[64] = L"";
    GetWindowText(hEditSearchID, idStr, 64);
    if (!wcslen(idStr)) {
        MessageBox(hMainWnd, L"Please enter a Booking ID.", L"Validation", MB_OK | MB_ICONWARNING);
        return;
    }

    SendMessage(hListSearch, LB_RESETCONTENT, 0, 0);
    const char* sql = "SELECT b.id, p.name, f.flightNumber, b.seat "
        "FROM Bookings b JOIN Passengers p ON b.passengerID=p.id "
        "JOIN Flights f ON b.flightID=f.id WHERE b.id=?";

    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
    sqlite3_bind_int(s, 1, _wtoi(idStr));

    if (sqlite3_step(s) == SQLITE_ROW) {
        wstringstream ss;
        ss << L" Booking #" << sqlite3_column_int(s, 0)
            << L" | Passenger: " << Utf8ToWide((const char*)sqlite3_column_text(s, 1))
            << L" | Flight: " << Utf8ToWide((const char*)sqlite3_column_text(s, 2))
            << L" | Seat: " << Utf8ToWide((const char*)sqlite3_column_text(s, 3));
        SendMessage(hListSearch, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
    else {
        SendMessage(hListSearch, LB_ADDSTRING, 0, (LPARAM)L" No results found.");
    }
    sqlite3_finalize(s);
}

void SearchByName() {
    wchar_t keyword[256] = L"";
    GetWindowText(hEditSearchName, keyword, 256);
    if (!wcslen(keyword)) {
        MessageBox(hMainWnd, L"Please enter a name or flight number.", L"Validation", MB_OK | MB_ICONWARNING);
        return;
    }

    SendMessage(hListSearch, LB_RESETCONTENT, 0, 0);
    const char* sql = "SELECT b.id, p.name, f.flightNumber, b.seat "
        "FROM Bookings b JOIN Passengers p ON b.passengerID=p.id "
        "JOIN Flights f ON b.flightID=f.id "
        "WHERE p.name LIKE ? OR f.flightNumber LIKE ?";

    sqlite3_stmt* s;
    sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
    string pattern = "%" + WideToUtf8(keyword) + "%";
    sqlite3_bind_text(s, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(s, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    while (sqlite3_step(s) == SQLITE_ROW) {
        found = true;
        wstringstream ss;
        ss << L" #" << sqlite3_column_int(s, 0) << L" "
            << Utf8ToWide((const char*)sqlite3_column_text(s, 1))
            << L" | Flight: " << Utf8ToWide((const char*)sqlite3_column_text(s, 2))
            << L" | Seat: " << Utf8ToWide((const char*)sqlite3_column_text(s, 3));
        SendMessage(hListSearch, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
    }
    if (!found) {
        SendMessage(hListSearch, LB_ADDSTRING, 0, (LPARAM)L" No results found.");
    }
    sqlite3_finalize(s);
}

// ==================== Panel Procedure ====================
LRESULT CALLBACK PanelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_ERASEBKGND) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, hBrushBg);
        return 1;
    }
    if (msg == WM_CTLCOLORSTATIC) {
        SetTextColor((HDC)wParam, CLR_MUTED);
        SetBkColor((HDC)wParam, CLR_BG);
        return (LRESULT)hBrushBg;
    }
    if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX) {
        SetTextColor((HDC)wParam, CLR_TEXT);
        SetBkColor((HDC)wParam, CLR_SURFACE);
        return (LRESULT)hBrushSurface;
    }
    if (msg == WM_COMMAND) {
        SendMessage(hMainWnd, WM_COMMAND, wParam, lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ==================== Button Subclassing ====================
static WNDPROC g_origBtnProc = nullptr;

COLORREF BtnColor(HWND h) {
    switch (GetDlgCtrlID(h)) {
    case IDC_BTN_ADD_PASSENGER: case IDC_BTN_ADD_FLIGHT:
    case IDC_BTN_ADD_BOOKING: case IDC_BTN_SEARCH_ID:
    case IDC_BTN_SEARCH_NAME: case IDC_BTN_ADD_TO_GROUP:
    case IDC_BTN_GROUP_BOOKING:
        return CLR_BTN_ADD;
    case IDC_BTN_UPDATE_PASSENGER: case IDC_BTN_UPDATE_FLIGHT:
        return CLR_BTN_UPD;
    default:
        return CLR_BTN_DEL;
    }
}

LRESULT CALLBACK BtnProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        HBRUSH br = CreateSolidBrush(BtnColor(hwnd));
        FillRect(dc, &rc, br);
        DeleteObject(br);

        wchar_t t[128];
        GetWindowText(hwnd, t, 128);

        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(255, 255, 255));
        SelectObject(dc, hFontBtn);
        DrawText(dc, t, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProc(g_origBtnProc, hwnd, msg, wp, lp);
}

void SubclassButtons(HWND panel) {
    EnumChildWindows(panel, [](HWND c, LPARAM)->BOOL {
        wchar_t cls[32];
        GetClassName(c, cls, 32);
        if (!wcscmp(cls, L"Button")) {
            if (!g_origBtnProc)
                g_origBtnProc = (WNDPROC)GetWindowLongPtr(c, GWLP_WNDPROC);
            SetWindowLongPtr(c, GWLP_WNDPROC, (LONG_PTR)BtnProc);
        }
        return TRUE;
        }, 0);
}

// ==================== Control Factory ====================
HWND Lbl(const wchar_t* t, int x, int y, int w, int h, HWND p) {
    HWND h2 = CreateWindow(L"STATIC", t, WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, w, h, p, 0, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontUI, TRUE);
    return h2;
}

HWND Edt(int x, int y, int w, int h, HWND p, UINT id) {
    HWND h2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y, w, h, p, (HMENU)(UINT_PTR)id, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontUI, TRUE);
    return h2;
}

HWND Btn(const wchar_t* t, int x, int y, int w, int h, HWND p, UINT id) {
    HWND h2 = CreateWindow(L"BUTTON", t, WS_CHILD | WS_VISIBLE | BS_FLAT,
        x, y, w, h, p, (HMENU)(UINT_PTR)id, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
    return h2;
}

HWND Cmb(int x, int y, int w, int h, HWND p, UINT id) {
    HWND h2 = CreateWindow(L"COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h, p, (HMENU)(UINT_PTR)id, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontUI, TRUE);
    return h2;
}

HWND Lst(int x, int y, int w, int h, HWND p, UINT id) {
    HWND h2 = CreateWindow(L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        x, y, w, h, p, (HMENU)(UINT_PTR)id, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontList, TRUE);
    SendMessage(h2, LB_SETITEMHEIGHT, 0, 24);
    return h2;
}

void SectionHdr(const wchar_t* t, int x, int y, HWND p) {
    HWND h2 = CreateWindow(L"STATIC", t, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, 500, 26, p, 0, hInst, 0);
    SendMessage(h2, WM_SETFONT, (WPARAM)hFontHeader, TRUE);
}
// ==================== Register Panel Class ====================
void RegisterPanelClass() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = PanelProc;
    wc.hInstance = hInst;
    wc.hbrBackground = hBrushBg;
    wc.lpszClassName = L"TabPanel";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);
}

// ==================== Build Panels ====================
void BuildPassengerPanel(HWND p) {
    SectionHdr(L"Passenger Management", 20, 16, p);
    Lbl(L"Full Name", 20, 54, 100, 20, p); hEditPName = Edt(130, 52, 230, 26, p, IDC_EDIT_PNAME);
    Lbl(L"National ID", 20, 88, 100, 20, p); hEditPID = Edt(130, 86, 230, 26, p, IDC_EDIT_PID);
    Lbl(L"Phone", 20, 122, 100, 20, p); hEditPPhone = Edt(130, 120, 230, 26, p, IDC_EDIT_PPHONE);

    Btn(L"+ Add", 20, 158, 110, 32, p, IDC_BTN_ADD_PASSENGER);
    Btn(L"Update", 140, 158, 110, 32, p, IDC_BTN_UPDATE_PASSENGER);
    Btn(L"Delete", 260, 158, 110, 32, p, IDC_BTN_DELETE_PASSENGER);

    hListPassengers = Lst(10, 204, 790, 310, p, IDC_LIST_PASSENGERS);
}

void BuildFlightPanel(HWND p) {
    SectionHdr(L"Flight Management", 20, 16, p);
    Lbl(L"Flight No.", 20, 54, 100, 20, p); hEditFlightNum = Edt(130, 52, 180, 26, p, IDC_EDIT_FLIGHTNUM);
    Lbl(L"From", 20, 88, 100, 20, p); hEditFrom = Edt(130, 86, 180, 26, p, IDC_EDIT_FROM);
    Lbl(L"To", 330, 88, 50, 20, p); hEditTo = Edt(390, 86, 180, 26, p, IDC_EDIT_TO);

    Lbl(L"Departure", 20, 122, 100, 20, p); hEditDeparture = Edt(130, 120, 180, 26, p, IDC_EDIT_DEPARTURE);
    Lbl(L"Arrival", 330, 122, 100, 20, p); hEditArrival = Edt(430, 120, 180, 26, p, IDC_EDIT_ARRIVAL);

    Lbl(L"Price ($)", 20, 156, 100, 20, p); hEditPrice = Edt(130, 154, 130, 26, p, IDC_EDIT_PRICE);

    Lbl(L"Status", 20, 190, 100, 20, p);
    hComboStatus = Cmb(130, 188, 180, 200, p, IDC_COMBO_STATUS);

    const wchar_t* statuses[] = { L"Scheduled", L"Delayed", L"Cancelled", L"Boarded", L"Completed" };
    for (auto st : statuses)
        SendMessage(hComboStatus, CB_ADDSTRING, 0, (LPARAM)st);
    SendMessage(hComboStatus, CB_SETCURSEL, 0, 0);

    Btn(L"+ Add", 20, 230, 110, 32, p, IDC_BTN_ADD_FLIGHT);
    Btn(L"Update", 140, 230, 110, 32, p, IDC_BTN_UPDATE_FLIGHT);
    Btn(L"Delete", 260, 230, 110, 32, p, IDC_BTN_DELETE_FLIGHT);

    hListFlights = Lst(10, 280, 790, 234, p, IDC_LIST_FLIGHTS);
}

void BuildBookingPanel(HWND p) {
    SectionHdr(L"Booking Management", 20, 16, p);

    Lbl(L"Passenger", 20, 54, 100, 20, p);
    hComboPassenger = Cmb(130, 52, 340, 220, p, IDC_COMBO_PASSENGER);

    Btn(L"+ Add to Group", 480, 52, 140, 28, p, IDC_BTN_ADD_TO_GROUP);
    Btn(L"Remove", 630, 52, 100, 28, p, IDC_BTN_REMOVE_FROM_GROUP);

    Lbl(L"Selected Passengers", 20, 90, 200, 20, p);
    hListSelectedPassengers = CreateWindow(L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        20, 112, 710, 110, p, (HMENU)IDC_LIST_SELECTED_PASSENGERS, hInst, NULL);
    SendMessage(hListSelectedPassengers, WM_SETFONT, (WPARAM)hFontList, TRUE);

    Lbl(L"Flight", 20, 235, 100, 20, p);
    hComboFlight = Cmb(130, 233, 340, 220, p, IDC_COMBO_FLIGHT);

    Lbl(L"Seat No. (optional)", 20, 270, 150, 20, p);
    hEditSeat = Edt(170, 268, 140, 26, p, IDC_EDIT_SEAT);

    Btn(L"Confirm Group Booking", 20, 310, 220, 35, p, IDC_BTN_GROUP_BOOKING);
    Btn(L"Cancel Booking", 260, 310, 150, 35, p, IDC_BTN_DELETE_BOOKING);
    Btn(L"Clear Group", 480, 310, 120, 35, p, IDC_BTN_CLEAR_GROUP);

    hListBookings = Lst(10, 360, 790, 180, p, IDC_LIST_BOOKINGS);
}

void BuildSearchPanel(HWND p) {
    SectionHdr(L"Search & Lookup", 20, 16, p);
    Lbl(L"Booking ID", 20, 56, 120, 20, p);
    hEditSearchID = Edt(150, 54, 160, 26, p, IDC_EDIT_SEARCH_ID);
    Btn(L"Search", 320, 54, 100, 26, p, IDC_BTN_SEARCH_ID);

    Lbl(L"Name / Flight No.", 20, 96, 130, 20, p);
    hEditSearchName = Edt(160, 94, 160, 26, p, IDC_EDIT_SEARCH_NAME);
    Btn(L"Search", 330, 94, 100, 26, p, IDC_BTN_SEARCH_NAME);

    hListSearch = Lst(10, 138, 790, 376, p, IDC_LIST_SEARCH);
}

void ShowPanel(int idx) {
    for (int i = 0; i < 4; i++)
        ShowWindow(hPanels[i], (i == idx) ? SW_SHOW : SW_HIDE);
}

// ==================== Main Window Procedure ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_ERASEBKGND) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, hBrushBg);
        return 1;
    }
    if (msg == WM_CTLCOLORSTATIC) {
        SetTextColor((HDC)wParam, CLR_MUTED);
        SetBkColor((HDC)wParam, CLR_BG);
        return (LRESULT)hBrushBg;
    }
    if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX) {
        SetTextColor((HDC)wParam, CLR_TEXT);
        SetBkColor((HDC)wParam, CLR_SURFACE);
        return (LRESULT)hBrushSurface;
    }

    if (msg == WM_CREATE) {
        CreateResources();
        InitDatabase();
        RegisterPanelClass();

        hTab = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE | TCS_FLATBUTTONS,
            6, 6, 826, 584, hwnd, (HMENU)IDC_TAB, hInst, NULL);
        SendMessage(hTab, WM_SETFONT, (WPARAM)hFontUI, TRUE);

        const wchar_t* tabs[] = { L" Passengers ", L" Flights ", L" Bookings ", L" Search " };
        TCITEM tie = {};
        tie.mask = TCIF_TEXT;
        for (int i = 0; i < 4; i++) {
            tie.pszText = (LPWSTR)tabs[i];
            TabCtrl_InsertItem(hTab, i, &tie);
        }

        RECT tabRC = { 0, 0, 820, 574 };
        TabCtrl_AdjustRect(hTab, FALSE, &tabRC);

        for (int i = 0; i < 4; i++) {
            hPanels[i] = CreateWindowEx(0, L"TabPanel", NULL, WS_CHILD,
                tabRC.left, tabRC.top, tabRC.right - tabRC.left, tabRC.bottom - tabRC.top,
                hTab, NULL, hInst, NULL);
        }

        BuildPassengerPanel(hPanels[0]);
        BuildFlightPanel(hPanels[1]);
        BuildBookingPanel(hPanels[2]);
        BuildSearchPanel(hPanels[3]);

        for (int i = 0; i < 4; i++)
            SubclassButtons(hPanels[i]);

        RefreshPassengerList();
        RefreshFlightList();
        RefreshBookingList();
        RefreshCombos();
        RefreshSelectedGroupList();

        ShowPanel(0);
    }

    if (msg == WM_NOTIFY) {
        NMHDR* nm = (NMHDR*)lParam;
        if (nm->idFrom == IDC_TAB && nm->code == TCN_SELCHANGE)
            ShowPanel((int)TabCtrl_GetCurSel(hTab));
    }

    if (msg == WM_COMMAND) {
        switch (LOWORD(wParam)) {
        case IDC_BTN_ADD_PASSENGER:     AddPassenger(); break;
        case IDC_BTN_UPDATE_PASSENGER:  UpdatePassenger(); break;
        case IDC_BTN_DELETE_PASSENGER:  DeletePassenger(); break;
        case IDC_BTN_ADD_FLIGHT:        AddFlight(); break;
        case IDC_BTN_UPDATE_FLIGHT:     UpdateFlight(); break;
        case IDC_BTN_DELETE_FLIGHT:     DeleteFlight(); break;
        case IDC_BTN_ADD_BOOKING:       AddBooking(); break;
        case IDC_BTN_DELETE_BOOKING:    DeleteBooking(); break;
        case IDC_BTN_GROUP_BOOKING:     AddGroupBooking(); break;
        case IDC_BTN_ADD_TO_GROUP:      AddToGroup(); break;
        case IDC_BTN_REMOVE_FROM_GROUP: RemoveFromGroup(); break;
        case IDC_BTN_CLEAR_GROUP:       ClearGroup(); break;
        case IDC_BTN_SEARCH_ID:         SearchByID(); break;
        case IDC_BTN_SEARCH_NAME:       SearchByName(); break;

            // List Selections
        case IDC_LIST_PASSENGERS:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int i = (int)SendMessage(hListPassengers, LB_GETCURSEL, 0, 0);
                if (i != LB_ERR && i < (int)passengers.size()) {
                    selectedPassengerID = passengers[i].id;
                    SetWindowText(hEditPName, passengers[i].name.c_str());
                    SetWindowText(hEditPID, passengers[i].nationalID.c_str());
                    SetWindowText(hEditPPhone, passengers[i].phone.c_str());
                }
            }
            break;

        case IDC_LIST_FLIGHTS:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int i = (int)SendMessage(hListFlights, LB_GETCURSEL, 0, 0);
                if (i != LB_ERR && i < (int)flights.size()) {
                    selectedFlightID = flights[i].id;
                    SetWindowText(hEditFlightNum, flights[i].flightNumber.c_str());
                    SetWindowText(hEditFrom, flights[i].from.c_str());
                    SetWindowText(hEditTo, flights[i].to.c_str());
                    SetWindowText(hEditDeparture, Utf8ToWide(flights[i].departure.c_str()).c_str());
                    SetWindowText(hEditArrival, Utf8ToWide(flights[i].arrival.c_str()).c_str());

                    wchar_t ps[50];
                    swprintf(ps, 50, L"%.2f", flights[i].price);
                    SetWindowText(hEditPrice, ps);

                    int count = (int)SendMessage(hComboStatus, CB_GETCOUNT, 0, 0);
                    for (int j = 0; j < count; j++) {
                        wchar_t buf[50];
                        SendMessage(hComboStatus, CB_GETLBTEXT, j, (LPARAM)buf);
                        if (wcscmp(buf, flights[i].status.c_str()) == 0) {
                            SendMessage(hComboStatus, CB_SETCURSEL, j, 0);
                            break;
                        }
                    }
                }
            }
            break;

        case IDC_LIST_BOOKINGS:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int i = (int)SendMessage(hListBookings, LB_GETCURSEL, 0, 0);
                if (i != LB_ERR && i < (int)bookings.size())
                    selectedBookingID = bookings[i].id;
            }
            break;
        }
    }

    if (msg == WM_DESTROY) {
        DestroyResources();
        if (db) sqlite3_close(db);
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ==================== WinMain ====================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    hBrushBg = CreateSolidBrush(CLR_BG);
    hBrushSurface = CreateSolidBrush(CLR_SURFACE);

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_TAB_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = hBrushBg;
    wc.lpszClassName = L"AirlineApp";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassEx(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int ww = 860, wh = 660;

    hMainWnd = CreateWindowEx(0, L"AirlineApp", L" Airline Reservation System",
        WS_OVERLAPPEDWINDOW,
        (sw - ww) / 2, (sh - wh) / 2, ww, wh,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
