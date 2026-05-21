#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>
using namespace std;

// ==================== CẤU TRÚC DỮ LIỆU ====================

// Cấu trúc Dịch vụ
struct Service {
    string serviceName;
    double price;
    int quantity;
    Service* next;
    
    Service(string name, double p, int q) : serviceName(name), price(p), quantity(q), next(nullptr) {}
};

// Cấu trúc Phòng
struct Room {
    string roomId;
    string roomType; // Standard, Deluxe, VIP
    double pricePerDay;
    bool isAvailable;
    Service* serviceList;
    
    Room() : serviceList(nullptr), isAvailable(true) {}
    Room(string id, string type, double price) 
        : roomId(id), roomType(type), pricePerDay(price), isAvailable(true), serviceList(nullptr) {}
};

// Cấu trúc Khách hàng
struct Customer {
    string customerId;
    string fullName;
    string idCard;
    string phoneNumber;
    Customer* next;
    
    Customer() : next(nullptr) {}
    Customer(string id, string name, string card, string phone)
        : customerId(id), fullName(name), idCard(card), phoneNumber(phone), next(nullptr) {}
};

// Cấu trúc Đặt phòng
struct Reservation {
    string reservationId;
    string customerId;
    string roomId;
    int checkInDay, checkInMonth, checkInYear;
    int checkOutDay, checkOutMonth, checkOutYear;
    bool isCheckedIn;
    
    Reservation() : isCheckedIn(false) {}
};

// Cấu trúc Hóa đơn
struct Invoice {
    string invoiceId;
    string customerId;
    string roomId;
    int checkInDay, checkInMonth, checkInYear;
    int checkOutDay, checkOutMonth, checkOutYear;
    double roomCharge;
    double serviceCharge;
    double totalAmount;
    
    Invoice() : roomCharge(0), serviceCharge(0), totalAmount(0) {}
};

// ==================== QUẢN LÝ PHÒNG ====================
class RoomManager {
private:
    Room* rooms;
    int capacity;
    int count;
    
    void resize() {
        capacity *= 2;
        Room* newRooms = new Room[capacity];
        for (int i = 0; i < count; i++) {
            newRooms[i] = rooms[i];
        }
        delete[] rooms;
        rooms = newRooms;
    }
    
public:
    RoomManager(int cap = 10) : capacity(cap), count(0) {
        rooms = new Room[capacity];
    }
    
    ~RoomManager() {
        for (int i = 0; i < count; i++) {
            Service* curr = rooms[i].serviceList;
            while (curr) {
                Service* temp = curr;
                curr = curr->next;
                delete temp;
            }
        }
        delete[] rooms;
    }
    
    bool addRoom(string id, string type, double price) {
        // Kiểm tra trùng mã phòng
        for (int i = 0; i < count; i++) {
            if (rooms[i].roomId == id) {
                cout << "Loi: Ma phong da ton tai!\n";
                return false;
            }
        }
        
        if (price <= 0) {
            cout << "Loi: Gia phong phai lon hon 0!\n";
            return false;
        }
        
        if (count == capacity) resize();
        
        rooms[count] = Room(id, type, price);
        count++;
        cout << "Them phong thanh cong!\n";
        return true;
    }
    
    void displayAllRooms() {
        if (count == 0) {
            cout << "Khong co phong nao!\n";
            return;
        }
        
        // Sắp xếp theo mã phòng (Bubble Sort)
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (rooms[j].roomId > rooms[j + 1].roomId) {
                    Room temp = rooms[j];
                    rooms[j] = rooms[j + 1];
                    rooms[j + 1] = temp;
                }
            }
        }
        
        cout << "\n========== DANH SACH PHONG ==========\n";
        cout << left << setw(10) << "Ma phong" 
             << setw(12) << "Loai phong" 
             << setw(15) << "Gia/ngay" 
             << setw(15) << "Trang thai" << endl;
        cout << string(52, '-') << endl;
        
        for (int i = 0; i < count; i++) {
            cout << left << setw(10) << rooms[i].roomId
                 << setw(12) << rooms[i].roomType
                 << setw(15) << rooms[i].pricePerDay
                 << setw(15) << (rooms[i].isAvailable ? "Trong" : "Dang thue") << endl;
        }
        cout << string(52, '=') << endl;
    }
    
    Room* findRoom(string id) {
        for (int i = 0; i < count; i++) {
            if (rooms[i].roomId == id) {
                return &rooms[i];
            }
        }
        return nullptr;
    }
    
    void searchRoom(string keyword) {
        bool found = false;
        cout << "\n========== KET QUA TIM KIEM ==========\n";
        
        for (int i = 0; i < count; i++) {
            if (rooms[i].roomId.find(keyword) != string::npos || 
                rooms[i].roomType.find(keyword) != string::npos) {
                cout << "Ma phong: " << rooms[i].roomId << endl;
                cout << "Loai phong: " << rooms[i].roomType << endl;
                cout << "Gia: " << rooms[i].pricePerDay << endl;
                cout << "Trang thai: " << (rooms[i].isAvailable ? "Trong" : "Dang thue") << endl;
                cout << string(38, '-') << endl;
                found = true;
            }
        }
        
        if (!found) {
            cout << "Khong tim thay phong!\n";
        }
    }
    
    void addServiceToRoom(string roomId, string serviceName, double price, int quantity) {
        Room* room = findRoom(roomId);
        if (!room) {
            cout << "Khong tim thay phong!\n";
            return;
        }
        
        if (room->isAvailable) {
            cout << "Phong chua duoc thue!\n";
            return;
        }
        
        Service* newService = new Service(serviceName, price, quantity);
        newService->next = room->serviceList;
        room->serviceList = newService;
        
        cout << "Them dich vu thanh cong!\n";
    }
    
    double calculateServiceCharge(string roomId) {
        Room* room = findRoom(roomId);
        if (!room) return 0;
        
        double total = 0;
        Service* curr = room->serviceList;
        while (curr) {
            total += curr->price * curr->quantity;
            curr = curr->next;
        }
        return total;
    }
    
    void clearServices(string roomId) {
        Room* room = findRoom(roomId);
        if (!room) return;
        
        Service* curr = room->serviceList;
        while (curr) {
            Service* temp = curr;
            curr = curr->next;
            delete temp;
        }
        room->serviceList = nullptr;
    }
    
    int getRoomCount() { return count; }
    
    void displayStatistics() {
        int available = 0, occupied = 0;
        for (int i = 0; i < count; i++) {
            if (rooms[i].isAvailable) available++;
            else occupied++;
        }
        
        cout << "\n========== THONG KE PHONG ==========\n";
        cout << "Tong so phong: " << count << endl;
        cout << "Phong trong: " << available << endl;
        cout << "Phong dang thue: " << occupied << endl;
        cout << string(36, '=') << endl;
    }
};

// ==================== QUẢN LÝ KHÁCH HÀNG ====================
class CustomerManager {
private:
    Customer* head;
    int count;
    
public:
    CustomerManager() : head(nullptr), count(0) {}
    
    ~CustomerManager() {
        Customer* curr = head;
        while (curr) {
            Customer* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
    
    bool addCustomer(string id, string name, string idCard, string phone) {
        // Kiểm tra trùng
        Customer* curr = head;
        while (curr) {
            if (curr->customerId == id || curr->idCard == idCard) {
                cout << "Loi: Ma khach hoac CCCD da ton tai!\n";
                return false;
            }
            curr = curr->next;
        }
        
        Customer* newCustomer = new Customer(id, name, idCard, phone);
        newCustomer->next = head;
        head = newCustomer;
        count++;
        
        cout << "Them khach hang thanh cong!\n";
        return true;
    }
    
    void displayAllCustomers() {
        if (!head) {
            cout << "Khong co khach hang nao!\n";
            return;
        }
        
        cout << "\n========== DANH SACH KHACH HANG ==========\n";
        cout << left << setw(10) << "Ma KH" 
             << setw(20) << "Ho ten" 
             << setw(15) << "CCCD" 
             << setw(15) << "SDT" << endl;
        cout << string(60, '-') << endl;
        
        Customer* curr = head;
        while (curr) {
            cout << left << setw(10) << curr->customerId
                 << setw(20) << curr->fullName
                 << setw(15) << curr->idCard
                 << setw(15) << curr->phoneNumber << endl;
            curr = curr->next;
        }
        cout << string(60, '=') << endl;
    }
    
    Customer* findCustomer(string id) {
        Customer* curr = head;
        while (curr) {
            if (curr->customerId == id) {
                return curr;
            }
            curr = curr->next;
        }
        return nullptr;
    }
    
    void searchCustomer(string keyword) {
        bool found = false;
        cout << "\n========== KET QUA TIM KIEM ==========\n";
        
        Customer* curr = head;
        while (curr) {
            if (curr->customerId.find(keyword) != string::npos ||
                curr->fullName.find(keyword) != string::npos ||
                curr->idCard.find(keyword) != string::npos) {
                cout << "Ma KH: " << curr->customerId << endl;
                cout << "Ho ten: " << curr->fullName << endl;
                cout << "CCCD: " << curr->idCard << endl;
                cout << "SDT: " << curr->phoneNumber << endl;
                cout << string(38, '-') << endl;
                found = true;
            }
            curr = curr->next;
        }
        
        if (!found) {
            cout << "Khong tim thay khach hang!\n";
        }
    }
    
    int getCustomerCount() { return count; }
};

// ==================== QUẢN LÝ ĐẶT PHÒNG ====================
class ReservationManager {
private:
    Reservation* reservations;
    int capacity;
    int count;
    
    void resize() {
        capacity *= 2;
        Reservation* newRes = new Reservation[capacity];
        for (int i = 0; i < count; i++) {
            newRes[i] = reservations[i];
        }
        delete[] reservations;
        reservations = newRes;
    }
    
public:
    ReservationManager(int cap = 10) : capacity(cap), count(0) {
        reservations = new Reservation[capacity];
    }
    
    ~ReservationManager() {
        delete[] reservations;
    }
    
    bool makeReservation(string resId, string custId, string roomId, 
                        int inD, int inM, int inY, int outD, int outM, int outY,
                        CustomerManager& custMgr, RoomManager& roomMgr) {
        // Kiểm tra khách tồn tại
        if (!custMgr.findCustomer(custId)) {
            cout << "Loi: Khach hang khong ton tai!\n";
            return false;
        }
        
        // Kiểm tra phòng tồn tại và trống
        Room* room = roomMgr.findRoom(roomId);
        if (!room) {
            cout << "Loi: Phong khong ton tai!\n";
            return false;
        }
        
        if (!room->isAvailable) {
            cout << "Loi: Phong da duoc thue!\n";
            return false;
        }
        
        if (count == capacity) resize();
        
        reservations[count].reservationId = resId;
        reservations[count].customerId = custId;
        reservations[count].roomId = roomId;
        reservations[count].checkInDay = inD;
        reservations[count].checkInMonth = inM;
        reservations[count].checkInYear = inY;
        reservations[count].checkOutDay = outD;
        reservations[count].checkOutMonth = outM;
        reservations[count].checkOutYear = outY;
        reservations[count].isCheckedIn = false;
        count++;
        
        cout << "Dat phong thanh cong!\n";
        return true;
    }
    
    bool checkIn(string roomId, RoomManager& roomMgr) {
        Room* room = roomMgr.findRoom(roomId);
        if (!room) {
            cout << "Khong tim thay phong!\n";
            return false;
        }
        
        // Tìm reservation
        for (int i = 0; i < count; i++) {
            if (reservations[i].roomId == roomId && !reservations[i].isCheckedIn) {
                reservations[i].isCheckedIn = true;
                room->isAvailable = false;
                cout << "Nhan phong thanh cong!\n";
                return true;
            }
        }
        
        cout << "Khong tim thay dat phong!\n";
        return false;
    }
    
    Reservation* findReservationByRoom(string roomId) {
        for (int i = 0; i < count; i++) {
            if (reservations[i].roomId == roomId && reservations[i].isCheckedIn) {
                return &reservations[i];
            }
        }
        return nullptr;
    }
    
    void displayAllReservations() {
        if (count == 0) {
            cout << "Khong co dat phong nao!\n";
            return;
        }
        
        cout << "\n========== DANH SACH DAT PHONG ==========\n";
        for (int i = 0; i < count; i++) {
            cout << "Ma dat phong: " << reservations[i].reservationId << endl;
            cout << "Ma khach: " << reservations[i].customerId << endl;
            cout << "Ma phong: " << reservations[i].roomId << endl;
            cout << "Ngay nhan: " << reservations[i].checkInDay << "/" 
                 << reservations[i].checkInMonth << "/" << reservations[i].checkInYear << endl;
            cout << "Ngay tra: " << reservations[i].checkOutDay << "/" 
                 << reservations[i].checkOutMonth << "/" << reservations[i].checkOutYear << endl;
            cout << "Trang thai: " << (reservations[i].isCheckedIn ? "Da nhan phong" : "Chua nhan") << endl;
            cout << string(41, '-') << endl;
        }
    }
};

// ==================== QUẢN LÝ HÓA ĐƠN ====================
class InvoiceManager {
private:
    Invoice* invoices;
    int capacity;
    int count;
    
    void resize() {
        capacity *= 2;
        Invoice* newInv = new Invoice[capacity];
        for (int i = 0; i < count; i++) {
            newInv[i] = invoices[i];
        }
        delete[] invoices;
        invoices = newInv;
    }
    
    int calculateDays(int d1, int m1, int y1, int d2, int m2, int y2) {
        // Tính đơn giản (có thể cải tiến)
        int days = (y2 - y1) * 365 + (m2 - m1) * 30 + (d2 - d1);
        return days > 0 ? days : 1;
    }
    
public:
    InvoiceManager(int cap = 10) : capacity(cap), count(0) {
        invoices = new Invoice[capacity];
    }
    
    ~InvoiceManager() {
        delete[] invoices;
    }
    
    bool checkOut(string roomId, RoomManager& roomMgr, ReservationManager& resMgr) {
        Room* room = roomMgr.findRoom(roomId);
        if (!room) {
            cout << "Khong tim thay phong!\n";
            return false;
        }
        
        if (room->isAvailable) {
            cout << "Phong chua duoc thue!\n";
            return false;
        }
        
        Reservation* res = resMgr.findReservationByRoom(roomId);
        if (!res) {
            cout << "Khong tim thay thong tin dat phong!\n";
            return false;
        }
        
        if (count == capacity) resize();
        
        // Tạo hóa đơn
        invoices[count].invoiceId = "INV" + to_string(count + 1);
        invoices[count].customerId = res->customerId;
        invoices[count].roomId = roomId;
        invoices[count].checkInDay = res->checkInDay;
        invoices[count].checkInMonth = res->checkInMonth;
        invoices[count].checkInYear = res->checkInYear;
        invoices[count].checkOutDay = res->checkOutDay;
        invoices[count].checkOutMonth = res->checkOutMonth;
        invoices[count].checkOutYear = res->checkOutYear;
        
        int days = calculateDays(res->checkInDay, res->checkInMonth, res->checkInYear,
                                res->checkOutDay, res->checkOutMonth, res->checkOutYear);
        
        invoices[count].roomCharge = days * room->pricePerDay;
        invoices[count].serviceCharge = roomMgr.calculateServiceCharge(roomId);
        invoices[count].totalAmount = invoices[count].roomCharge + invoices[count].serviceCharge;
        
        cout << "\n========== HOA DON ==========\n";
        cout << "Ma hoa don: " << invoices[count].invoiceId << endl;
        cout << "Ma khach: " << invoices[count].customerId << endl;
        cout << "Ma phong: " << invoices[count].roomId << endl;
        cout << "So ngay thue: " << days << endl;
        cout << "Tien phong: " << invoices[count].roomCharge << endl;
        cout << "Tien dich vu: " << invoices[count].serviceCharge << endl;
        cout << "TONG TIEN: " << invoices[count].totalAmount << endl;
        cout << string(29, '=') << endl;
        
        count++;
        
        // Trả phòng
        room->isAvailable = true;
        roomMgr.clearServices(roomId);
        
        return true;
    }
    
    void displayAllInvoices() {
        if (count == 0) {
            cout << "Khong co hoa don nao!\n";
            return;
        }
        
        cout << "\n========== DANH SACH HOA DON ==========\n";
        for (int i = 0; i < count; i++) {
            cout << "Ma HD: " << invoices[i].invoiceId << endl;
            cout << "Ma KH: " << invoices[i].customerId << endl;
            cout << "Ma phong: " << invoices[i].roomId << endl;
            cout << "Tong tien: " << invoices[i].totalAmount << endl;
            cout << string(38, '-') << endl;
        }
    }
    
    void sortByTotal() {
        // Quick Sort
        if (count <= 1) return;
        
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (invoices[j].totalAmount < invoices[j + 1].totalAmount) {
                    Invoice temp = invoices[j];
                    invoices[j] = invoices[j + 1];
                    invoices[j + 1] = temp;
                }
            }
        }
    }
    
    double calculateRevenue(int month, int year) {
        double total = 0;
        for (int i = 0; i < count; i++) {
            if (invoices[i].checkOutMonth == month && invoices[i].checkOutYear == year) {
                total += invoices[i].totalAmount;
            }
        }
        return total;
    }
    
    void displayRevenueStatistics() {
        cout << "\n========== THONG KE DOANH THU ==========\n";
        cout << "Nhap thang: ";
        int month; cin >> month;
        cout << "Nhap nam: ";
        int year; cin >> year;
        
        double revenue = calculateRevenue(month, year);
        cout << "Doanh thu thang " << month << "/" << year << ": " << revenue << endl;
        cout << string(40, '=') << endl;
    }
};

// ==================== HỆ THỐNG CHÍNH ====================
class HotelManagementSystem {
private:
    RoomManager roomMgr;
    CustomerManager custMgr;
    ReservationManager resMgr;
    InvoiceManager invMgr;
    
    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    
public:
    void run() {
        int choice;
        do {
            displayMenu();
            cout << "Chon chuc nang: ";
            cin >> choice;
            clearInput();
            
            switch (choice) {
                case 1: menuRoom(); break;
                case 2: menuCustomer(); break;
                case 3: menuReservation(); break;
                case 4: menuCheckOut(); break;
                case 5: menuService(); break;
                case 6: menuInvoice(); break;
                case 7: menuStatistics(); break;
                case 0: cout << "Tam biet!\n"; break;
                default: cout << "Lua chon khong hop le!\n";
            }
        } while (choice != 0);
    }
    
private:
    void displayMenu() {
        cout << "\n";
        cout << "╔════════════════════════════════════════╗\n";
        cout << "║   HE THONG QUAN LY KHACH SAN          ║\n";
        cout << "╠════════════════════════════════════════╣\n";
        cout << "║  1. Quan ly phong                      ║\n";
        cout << "║  2. Quan ly khach hang                 ║\n";
        cout << "║  3. Dat phong / Nhan phong             ║\n";
        cout << "║  4. Tra phong                          ║\n";
        cout << "║  5. Quan ly dich vu                    ║\n";
        cout << "║  6. Quan ly hoa don                    ║\n";
        cout << "║  7. Thong ke                           ║\n";
        cout << "║  0. Thoat                              ║\n";
        cout << "╚════════════════════════════════════════╝\n";
    }
    
    void menuRoom() {
        int choice;
        cout << "\n--- QUAN LY PHONG ---\n";
        cout << "1. Them phong\n";
        cout << "2. Hien thi danh sach phong\n";
        cout << "3. Tim kiem phong\n";
        cout << "Chon: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            string id, type;
            double price;
            cout << "Ma phong: "; getline(cin, id);
            cout << "Loai phong (Standard/Deluxe/VIP): "; getline(cin, type);
            cout << "Gia/ngay: "; cin >> price;
            roomMgr.addRoom(id, type, price);
        } else if (choice == 2) {
            roomMgr.displayAllRooms();
        } else if (choice == 3) {
            string keyword;
            cout << "Nhap tu khoa: "; getline(cin, keyword);
            roomMgr.searchRoom(keyword);
        }
    }
    
    void menuCustomer() {
        int choice;
        cout << "\n--- QUAN LY KHACH HANG ---\n";
        cout << "1. Them khach hang\n";
        cout << "2. Hien thi danh sach khach\n";
        cout << "3. Tim kiem khach hang\n";
        cout << "Chon: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            string id, name, idCard, phone;
            cout << "Ma khach: "; getline(cin, id);
            cout << "Ho ten: "; getline(cin, name);
            cout << "CCCD: "; getline(cin, idCard);
            cout << "SDT: "; getline(cin, phone);
            custMgr.addCustomer(id, name, idCard, phone);
        } else if (choice == 2) {
            custMgr.displayAllCustomers();
        } else if (choice == 3) {
            string keyword;
            cout << "Nhap tu khoa: "; getline(cin, keyword);
            custMgr.searchCustomer(keyword);
        }
    }
    
    void menuReservation() {
        int choice;
        cout << "\n--- DAT PHONG / NHAN PHONG ---\n";
        cout << "1. Dat phong\n";
        cout << "2. Nhan phong\n";
        cout << "3. Hien thi danh sach dat phong\n";
        cout << "Chon: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            string resId, custId, roomId;
            int inD, inM, inY, outD, outM, outY;
            
            cout << "Ma dat phong: "; getline(cin, resId);
            cout << "Ma khach: "; getline(cin, custId);
            cout << "Ma phong: "; getline(cin, roomId);
            cout << "Ngay nhan (dd mm yyyy): "; 
            cin >> inD >> inM >> inY;
            cout << "Ngay tra (dd mm yyyy): "; 
            cin >> outD >> outM >> outY;
            
            resMgr.makeReservation(resId, custId, roomId, inD, inM, inY, 
                                  outD, outM, outY, custMgr, roomMgr);
        } else if (choice == 2) {
            string roomId;
            cout << "Ma phong: "; getline(cin, roomId);
            resMgr.checkIn(roomId, roomMgr);
        } else if (choice == 3) {
            resMgr.displayAllReservations();
        }
    }
    
    void menuCheckOut() {
        string roomId;
        cout << "Ma phong tra: ";
        getline(cin, roomId);
        invMgr.checkOut(roomId, roomMgr, resMgr);
    }
    
    void menuService() {
        string roomId, serviceName;
        double price;
        int quantity;
        
        cout << "Ma phong: "; getline(cin, roomId);
        cout << "Ten dich vu: "; getline(cin, serviceName);
        cout << "Gia: "; cin >> price;
        cout << "So luong: "; cin >> quantity;
        
        roomMgr.addServiceToRoom(roomId, serviceName, price, quantity);
    }
    
    void menuInvoice() {
        int choice;
        cout << "\n--- QUAN LY HOA DON ---\n";
        cout << "1. Hien thi tat ca hoa don\n";
        cout << "2. Sap xep hoa don theo tong tien\n";
        cout << "Chon: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            invMgr.displayAllInvoices();
        } else if (choice == 2) {
            invMgr.sortByTotal();
            invMgr.displayAllInvoices();
        }
    }
    
    void menuStatistics() {
        int choice;
        cout << "\n--- THONG KE ---\n";
        cout << "1. Thong ke phong\n";
        cout << "2. Thong ke doanh thu\n";
        cout << "Chon: ";
        cin >> choice;
        clearInput();
        
        if (choice == 1) {
            roomMgr.displayStatistics();
        } else if (choice == 2) {
            invMgr.displayRevenueStatistics();
        }
    }
};

// ==================== MAIN ====================
int main() {
    HotelManagementSystem system;
    system.run();
    return 0;
}