#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>
#include <iomanip>
using namespace std;

// ==================== JSON HELPER ====================
class JsonHelper {
public:
    static string escapeString(const string& str) {
        string result;
        for (char c : str) {
            if (c == '"') result += "\\\"";
            else if (c == '\\') result += "\\\\";
            else if (c == '\n') result += "\\n";
            else if (c == '\r') result += "\\r";
            else if (c == '\t') result += "\\t";
            else result += c;
        }
        return result;
    }
    
    static string unescapeString(const string& str) {
        string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                if (str[i+1] == '"') { result += '"'; i++; }
                else if (str[i+1] == '\\') { result += '\\'; i++; }
                else if (str[i+1] == 'n') { result += '\n'; i++; }
                else if (str[i+1] == 'r') { result += '\r'; i++; }
                else if (str[i+1] == 't') { result += '\t'; i++; }
                else result += str[i];
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    static string extractValue(const string& json, const string& key) {
        string searchKey = "\"" + key + "\":";
        size_t pos = json.find(searchKey);
        if (pos == string::npos) return "";
        
        pos += searchKey.length();
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
        
        if (pos >= json.length()) return "";
        
        if (json[pos] == '"') {
            pos++;
            size_t endPos = pos;
            while (endPos < json.length() && json[endPos] != '"') {
                if (json[endPos] == '\\') endPos++;
                endPos++;
            }
            return unescapeString(json.substr(pos, endPos - pos));
        } else {
            size_t endPos = pos;
            while (endPos < json.length() && json[endPos] != ',' && json[endPos] != '}') {
                endPos++;
            }
            return json.substr(pos, endPos - pos);
        }
    }
    
    // Format số với 3 chữ số thập phân
    static string formatPrice(double price) {
        stringstream ss;
        ss << fixed << setprecision(3) << price;
        return ss.str();
    }
};

// ==================== CẤU TRÚC DỮ LIỆU ====================
struct Service {
    string serviceName;
    double price;
    int quantity;
    Service* next;
    
    Service(string name, double p, int q) : serviceName(name), price(p), quantity(q), next(nullptr) {}
    
    string toJson() const {
        return "{\n      \"serviceName\": \"" + JsonHelper::escapeString(serviceName) + 
               "\",\n      \"price\": " + JsonHelper::formatPrice(price) + 
               ",\n      \"quantity\": " + to_string(quantity) + "\n    }";
    }
};

struct Room {
    string roomId;
    string roomType;
    double pricePerDay;
    bool isAvailable;
    Service* serviceList;
    
    Room() : serviceList(nullptr), isAvailable(true) {}
    Room(string id, string type, double price) 
        : roomId(id), roomType(type), pricePerDay(price), isAvailable(true), serviceList(nullptr) {}
    
    string toJson() const {
        string json = "  {\n";
        json += "    \"roomId\": \"" + JsonHelper::escapeString(roomId) + "\",\n";
        json += "    \"roomType\": \"" + JsonHelper::escapeString(roomType) + "\",\n";
        json += "    \"pricePerDay\": " + JsonHelper::formatPrice(pricePerDay) + ",\n";
        json += "    \"isAvailable\": " + string(isAvailable ? "true" : "false") + ",\n";
        json += "    \"services\": [";
        
        Service* curr = serviceList;
        bool first = true;
        while (curr) {
            if (!first) json += ",";
            json += "\n    " + curr->toJson();
            first = false;
            curr = curr->next;
        }
        if (!first) json += "\n    ";
        json += "]\n  }";
        return json;
    }
};

struct Customer {
    string customerId;
    string fullName;
    string idCard;
    string phoneNumber;
    Customer* next;
    
    Customer() : next(nullptr) {}
    Customer(string id, string name, string card, string phone)
        : customerId(id), fullName(name), idCard(card), phoneNumber(phone), next(nullptr) {}
    
    string toJson() const {
        string json = "  {\n";
        json += "    \"customerId\": \"" + JsonHelper::escapeString(customerId) + "\",\n";
        json += "    \"fullName\": \"" + JsonHelper::escapeString(fullName) + "\",\n";
        json += "    \"idCard\": \"" + JsonHelper::escapeString(idCard) + "\",\n";
        json += "    \"phoneNumber\": \"" + JsonHelper::escapeString(phoneNumber) + "\"\n";
        json += "  }";
        return json;
    }
};

struct Reservation {
    string reservationId;
    string customerId;
    string roomId;
    int checkInDay, checkInMonth, checkInYear;
    int checkOutDay, checkOutMonth, checkOutYear;
    bool isCheckedIn;
    
    Reservation() : isCheckedIn(false) {}
    
    string toJson() const {
        string json = "  {\n";
        json += "    \"reservationId\": \"" + JsonHelper::escapeString(reservationId) + "\",\n";
        json += "    \"customerId\": \"" + JsonHelper::escapeString(customerId) + "\",\n";
        json += "    \"roomId\": \"" + JsonHelper::escapeString(roomId) + "\",\n";
        json += "    \"checkInDay\": " + to_string(checkInDay) + ",\n";
        json += "    \"checkInMonth\": " + to_string(checkInMonth) + ",\n";
        json += "    \"checkInYear\": " + to_string(checkInYear) + ",\n";
        json += "    \"checkOutDay\": " + to_string(checkOutDay) + ",\n";
        json += "    \"checkOutMonth\": " + to_string(checkOutMonth) + ",\n";
        json += "    \"checkOutYear\": " + to_string(checkOutYear) + ",\n";
        json += "    \"isCheckedIn\": " + string(isCheckedIn ? "true" : "false") + "\n";
        json += "  }";
        return json;
    }
};

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
    
    string toJson() const {
        string json = "  {\n";
        json += "    \"invoiceId\": \"" + JsonHelper::escapeString(invoiceId) + "\",\n";
        json += "    \"customerId\": \"" + JsonHelper::escapeString(customerId) + "\",\n";
        json += "    \"roomId\": \"" + JsonHelper::escapeString(roomId) + "\",\n";
        json += "    \"checkInDay\": " + to_string(checkInDay) + ",\n";
        json += "    \"checkInMonth\": " + to_string(checkInMonth) + ",\n";
        json += "    \"checkInYear\": " + to_string(checkInYear) + ",\n";
        json += "    \"checkOutDay\": " + to_string(checkOutDay) + ",\n";
        json += "    \"checkOutMonth\": " + to_string(checkOutMonth) + ",\n";
        json += "    \"checkOutYear\": " + to_string(checkOutYear) + ",\n";
        json += "    \"roomCharge\": " + JsonHelper::formatPrice(roomCharge) + ",\n";
        json += "    \"serviceCharge\": " + JsonHelper::formatPrice(serviceCharge) + ",\n";
        json += "    \"totalAmount\": " + JsonHelper::formatPrice(totalAmount) + "\n";
        json += "  }";
        return json;
    }
};

// ==================== ROOM COMBINATION SOLVER (BACKTRACKING) ====================
class RoomCombinationSolver {
private:
    struct RoomRequest {
        string roomType;
        int quantity;
    };
    
    vector<RoomRequest> requests;
    vector<Room*> availableRooms;
    vector<vector<Room*>> solutions;
    vector<Room*> currentSolution;
    
    bool backtrack(int requestIndex) {
        if (requestIndex == requests.size()) {
            solutions.push_back(currentSolution);
            return true;
        }
        
        RoomRequest& req = requests[requestIndex];
        int found = 0;
        
        for (Room* room : availableRooms) {
            if (room->isAvailable && room->roomType == req.roomType) {
                bool alreadyUsed = false;
                for (Room* used : currentSolution) {
                    if (used->roomId == room->roomId) {
                        alreadyUsed = true;
                        break;
                    }
                }
                
                if (!alreadyUsed) {
                    currentSolution.push_back(room);
                    found++;
                    
                    if (found == req.quantity) {
                        if (backtrack(requestIndex + 1)) {
                            return true;
                        }
                        for (int i = 0; i < req.quantity; i++) {
                            currentSolution.pop_back();
                        }
                        return false;
                    }
                }
            }
        }
        
        for (int i = 0; i < found; i++) {
            currentSolution.pop_back();
        }
        return false;
    }
    
public:
    bool findRoomCombination(const vector<pair<string, int>>& reqs, Room* rooms, int roomCount) {
        requests.clear();
        availableRooms.clear();
        solutions.clear();
        currentSolution.clear();
        
        for (const auto& r : reqs) {
            requests.push_back({r.first, r.second});
        }
        
        for (int i = 0; i < roomCount; i++) {
            if (rooms[i].isAvailable) {
                availableRooms.push_back(&rooms[i]);
            }
        }
        
        return backtrack(0);
    }
    
    vector<Room*> getSolution() {
        return solutions.empty() ? vector<Room*>() : solutions[0];
    }
};

// ==================== PRICE OPTIMIZATION (DYNAMIC PROGRAMMING) ====================
class PriceOptimizer {
public:
    struct RoomOption {
        string roomId;
        string roomType;
        double price;
        int daysAvailable;
    };
    
    static double findMinCost(vector<RoomOption>& rooms, int totalDays, vector<string>& selectedRooms) {
        int n = rooms.size();
        if (n == 0 || totalDays <= 0) return 0;
        
        vector<vector<double>> dp(n + 1, vector<double>(totalDays + 1, 1e9));
        vector<vector<int>> choice(n + 1, vector<int>(totalDays + 1, -1));
        
        for (int i = 0; i <= n; i++) {
            dp[i][0] = 0;
        }
        
        for (int i = 1; i <= n; i++) {
            for (int j = 0; j <= totalDays; j++) {
                dp[i][j] = dp[i-1][j];
                choice[i][j] = 0;
                
                int maxDays = min(j, rooms[i-1].daysAvailable);
                for (int k = 1; k <= maxDays; k++) {
                    double cost = dp[i-1][j-k] + rooms[i-1].price * k;
                    if (cost < dp[i][j]) {
                        dp[i][j] = cost;
                        choice[i][j] = k;
                    }
                }
            }
        }
        
        selectedRooms.clear();
        int days = totalDays;
        for (int i = n; i > 0 && days > 0; i--) {
            if (choice[i][days] > 0) {
                selectedRooms.push_back(rooms[i-1].roomId + " (" + to_string(choice[i][days]) + " ngay)");
                days -= choice[i][days];
            }
        }
        
        return dp[n][totalDays];
    }
};

// ==================== QUẢN LÝ PHÒNG ====================
class RoomManager {
private:
    Room* rooms;
    int capacity;
    int count;
    const string ROOM_FILE = "rooms.json";
    
    void resize() {
        capacity *= 2;
        Room* newRooms = new Room[capacity];
        for (int i = 0; i < count; i++) {
            newRooms[i] = rooms[i];
        }
        delete[] rooms;
        rooms = newRooms;
    }
    
    void saveToFile() {
        ofstream file(ROOM_FILE);
        if (!file.is_open()) {
            cout << "Loi: Khong the luu du lieu phong!\n";
            return;
        }
        
        file << "[\n";
        for (int i = 0; i < count; i++) {
            file << rooms[i].toJson();
            if (i < count - 1) file << ",\n";
            else file << "\n";
        }
        file << "]\n";
        
        file.close();
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
        saveToFile();
        return true;
    }
    
    bool deleteRoom(string id) {
        for (int i = 0; i < count; i++) {
            if (rooms[i].roomId == id) {
                if (!rooms[i].isAvailable) {
                    cout << "Loi: Khong the xoa phong dang duoc thue!\n";
                    return false;
                }
                
                Service* curr = rooms[i].serviceList;
                while (curr) {
                    Service* temp = curr;
                    curr = curr->next;
                    delete temp;
                }
                
                for (int j = i; j < count - 1; j++) {
                    rooms[j] = rooms[j + 1];
                }
                count--;
                cout << "Xoa phong thanh cong!\n";
                saveToFile();
                return true;
            }
        }
        cout << "Loi: Khong tim thay phong!\n";
        return false;
    }
    
    void displayAllRooms() {
        if (count == 0) {
            cout << "Khong co phong nao!\n";
            return;
        }
        
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
                 << setw(15) << fixed << setprecision(3) << rooms[i].pricePerDay
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
                cout << "Gia: " << fixed << setprecision(3) << rooms[i].pricePerDay << endl;
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
        saveToFile();
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
    
    void updateRoomStatus(string roomId, bool available) {
        Room* room = findRoom(roomId);
        if (room) {
            room->isAvailable = available;
            saveToFile();
        }
    }
    
    int getRoomCount() { return count; }
    Room* getRooms() { return rooms; }
    
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
    
    void loadFromJson(const string& json) {
        size_t pos = 1;
        while (pos < json.length()) {
            size_t start = json.find("{", pos);
            if (start == string::npos) break;
            
            int braceCount = 1;
            size_t end = start + 1;
            while (end < json.length() && braceCount > 0) {
                if (json[end] == '{') braceCount++;
                else if (json[end] == '}') braceCount--;
                end++;
            }
            
            string obj = json.substr(start, end - start);
            
            string id = JsonHelper::extractValue(obj, "roomId");
            string type = JsonHelper::extractValue(obj, "roomType");
            double price = stod(JsonHelper::extractValue(obj, "pricePerDay"));
            
            if (count == capacity) resize();
            rooms[count] = Room(id, type, price);
            
            string availStr = JsonHelper::extractValue(obj, "isAvailable");
            rooms[count].isAvailable = (availStr == "true");
            
            count++;
            pos = end;
        }
    }
    
    void loadFromFile() {
        ifstream file(ROOM_FILE);
        if (!file.is_open()) {
            return;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string json = buffer.str();
        file.close();
        
        loadFromJson(json);
    }
};

// ==================== QUẢN LÝ KHÁCH HÀNG ====================
class CustomerManager {
private:
    Customer* head;
    int count;
    const string CUSTOMER_FILE = "customers.json";
    
    void saveToFile() {
        ofstream file(CUSTOMER_FILE);
        if (!file.is_open()) {
            cout << "Loi: Khong the luu du lieu khach hang!\n";
            return;
        }
        
        file << "[\n";
        Customer* curr = head;
        bool first = true;
        while (curr) {
            if (!first) file << ",\n";
            file << curr->toJson();
            first = false;
            curr = curr->next;
        }
        file << "\n]\n";
        
        file.close();
    }
    
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
        saveToFile();
        return true;
    }
    
    bool deleteCustomer(string id) {
        if (!head) {
            cout << "Loi: Danh sach khach hang rong!\n";
            return false;
        }
        
        if (head->customerId == id) {
            Customer* temp = head;
            head = head->next;
            delete temp;
            count--;
            cout << "Xoa khach hang thanh cong!\n";
            saveToFile();
            return true;
        }
        
        Customer* curr = head;
        while (curr->next) {
            if (curr->next->customerId == id) {
                Customer* temp = curr->next;
                curr->next = curr->next->next;
                delete temp;
                count--;
                cout << "Xoa khach hang thanh cong!\n";
                saveToFile();
                return true;
            }
            curr = curr->next;
        }
        
        cout << "Loi: Khong tim thay khach hang!\n";
        return false;
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
    
    void loadFromJson(const string& json) {
        size_t pos = 1;
        while (pos < json.length()) {
            size_t start = json.find("{", pos);
            if (start == string::npos) break;
            
            size_t end = json.find("}", start);
            if (end == string::npos) break;
            
            string obj = json.substr(start, end - start + 1);
            
            string id = JsonHelper::extractValue(obj, "customerId");
            string name = JsonHelper::extractValue(obj, "fullName");
            string card = JsonHelper::extractValue(obj, "idCard");
            string phone = JsonHelper::extractValue(obj, "phoneNumber");
            
            Customer* newCust = new Customer(id, name, card, phone);
            newCust->next = head;
            head = newCust;
            count++;
            
            pos = end + 1;
        }
    }
    
    void loadFromFile() {
        ifstream file(CUSTOMER_FILE);
        if (!file.is_open()) {
            return;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string json = buffer.str();
        file.close();
        
        loadFromJson(json);
    }
};

// ==================== QUẢN LÝ ĐẶT PHÒNG ====================
class ReservationManager {
private:
    Reservation* reservations;
    int capacity;
    int count;
    const string RESERVATION_FILE = "reservations.json";
    
    void resize() {
        capacity *= 2;
        Reservation* newRes = new Reservation[capacity];
        for (int i = 0; i < count; i++) {
            newRes[i] = reservations[i];
        }
        delete[] reservations;
        reservations = newRes;
    }
    
    void saveToFile() {
        ofstream file(RESERVATION_FILE);
        if (!file.is_open()) {
            cout << "Loi: Khong the luu du lieu dat phong!\n";
            return;
        }
        
        file << "[\n";
        for (int i = 0; i < count; i++) {
            file << reservations[i].toJson();
            if (i < count - 1) file << ",\n";
            else file << "\n";
        }
        file << "]\n";
        
        file.close();
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
        if (!custMgr.findCustomer(custId)) {
            cout << "Loi: Khach hang khong ton tai!\n";
            return false;
        }
        
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
        saveToFile();
        return true;
    }
    
    bool checkIn(string roomId, RoomManager& roomMgr) {
        Room* room = roomMgr.findRoom(roomId);
        if (!room) {
            cout << "Khong tim thay phong!\n";
            return false;
        }
        
        for (int i = 0; i < count; i++) {
            if (reservations[i].roomId == roomId && !reservations[i].isCheckedIn) {
                reservations[i].isCheckedIn = true;
                roomMgr.updateRoomStatus(roomId, false);
                cout << "Nhan phong thanh cong!\n";
                saveToFile();
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
    
    void loadFromJson(const string& json) {
        size_t pos = 1;
        while (pos < json.length()) {
            size_t start = json.find("{", pos);
            if (start == string::npos) break;
            
            size_t end = json.find("}", start);
            if (end == string::npos) break;
            
            string obj = json.substr(start, end - start + 1);
            
            if (count == capacity) resize();
            
            reservations[count].reservationId = JsonHelper::extractValue(obj, "reservationId");
            reservations[count].customerId = JsonHelper::extractValue(obj, "customerId");
            reservations[count].roomId = JsonHelper::extractValue(obj, "roomId");
            reservations[count].checkInDay = stoi(JsonHelper::extractValue(obj, "checkInDay"));
            reservations[count].checkInMonth = stoi(JsonHelper::extractValue(obj, "checkInMonth"));
            reservations[count].checkInYear = stoi(JsonHelper::extractValue(obj, "checkInYear"));
            reservations[count].checkOutDay = stoi(JsonHelper::extractValue(obj, "checkOutDay"));
            reservations[count].checkOutMonth = stoi(JsonHelper::extractValue(obj, "checkOutMonth"));
            reservations[count].checkOutYear = stoi(JsonHelper::extractValue(obj, "checkOutYear"));
            
            string checkedIn = JsonHelper::extractValue(obj, "isCheckedIn");
            reservations[count].isCheckedIn = (checkedIn == "true");
            
            count++;
            pos = end + 1;
        }
    }
    
    void loadFromFile() {
        ifstream file(RESERVATION_FILE);
        if (!file.is_open()) {
            return;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string json = buffer.str();
        file.close();
        
        loadFromJson(json);
    }
};

// ==================== QUẢN LÝ HÓA ĐƠN ====================
class InvoiceManager {
private:
    Invoice* invoices;
    int capacity;
    int count;
    const string INVOICE_FILE = "invoices.json";
    
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
        int days = (y2 - y1) * 365 + (m2 - m1) * 30 + (d2 - d1);
        return days > 0 ? days : 1;
    }
    
    void saveToFile() {
        ofstream file(INVOICE_FILE);
        if (!file.is_open()) {
            cout << "Loi: Khong the luu du lieu hoa don!\n";
            return;
        }
        
        file << "[\n";
        for (int i = 0; i < count; i++) {
            file << invoices[i].toJson();
            if (i < count - 1) file << ",\n";
            else file << "\n";
        }
        file << "]\n";
        
        file.close();
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
        cout << "Tien phong: " << fixed << setprecision(3) << invoices[count].roomCharge << endl;
        cout << "Tien dich vu: " << fixed << setprecision(3) << invoices[count].serviceCharge << endl;
        cout << "TONG TIEN: " << fixed << setprecision(3) << invoices[count].totalAmount << endl;
        cout << string(29, '=') << endl;
        
        count++;
        
        roomMgr.updateRoomStatus(roomId, true);
        roomMgr.clearServices(roomId);
        
        saveToFile();
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
            cout << "Tong tien: " << fixed << setprecision(3) << invoices[i].totalAmount << endl;
            cout << string(38, '-') << endl;
        }
    }
    
    void sortByTotal() {
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
        cout << "Doanh thu thang " << month << "/" << year << ": " 
             << fixed << setprecision(3) << revenue << endl;
        cout << string(40, '=') << endl;
    }
    
    void loadFromJson(const string& json) {
        size_t pos = 1;
        while (pos < json.length()) {
            size_t start = json.find("{", pos);
            if (start == string::npos) break;
            
            size_t end = json.find("}", start);
            if (end == string::npos) break;
            
            string obj = json.substr(start, end - start + 1);
            
            if (count == capacity) resize();
            
            invoices[count].invoiceId = JsonHelper::extractValue(obj, "invoiceId");
            invoices[count].customerId = JsonHelper::extractValue(obj, "customerId");
            invoices[count].roomId = JsonHelper::extractValue(obj, "roomId");
            invoices[count].checkInDay = stoi(JsonHelper::extractValue(obj, "checkInDay"));
            invoices[count].checkInMonth = stoi(JsonHelper::extractValue(obj, "checkInMonth"));
            invoices[count].checkInYear = stoi(JsonHelper::extractValue(obj, "checkInYear"));
            invoices[count].checkOutDay = stoi(JsonHelper::extractValue(obj, "checkOutDay"));
            invoices[count].checkOutMonth = stoi(JsonHelper::extractValue(obj, "checkOutMonth"));
            invoices[count].checkOutYear = stoi(JsonHelper::extractValue(obj, "checkOutYear"));
            invoices[count].roomCharge = stod(JsonHelper::extractValue(obj, "roomCharge"));
            invoices[count].serviceCharge = stod(JsonHelper::extractValue(obj, "serviceCharge"));
            invoices[count].totalAmount = stod(JsonHelper::extractValue(obj, "totalAmount"));
            
            count++;
            pos = end + 1;
        }
    }
    
    void loadFromFile() {
        ifstream file(INVOICE_FILE);
        if (!file.is_open()) {
            return;
        }
        
        stringstream buffer;
        buffer << file.rdbuf();
        string json = buffer.str();
        file.close();
        
        loadFromJson(json);
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
    
    void loadAllData() {
        roomMgr.loadFromFile();
        custMgr.loadFromFile();
        resMgr.loadFromFile();
        invMgr.loadFromFile();
        cout << "Tai du lieu thanh cong!\n";
    }
    
public:
    void run() {
        loadAllData();
        
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
                case 8: menuAdvanced(); break;
                case 0: 
                    cout << "Tam biet!\n"; 
                    break;
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
        cout << "║  8. Chuc nang nang cao                 ║\n";
        cout << "║  0. Thoat                              ║\n";
        cout << "╚════════════════════════════════════════╝\n";
    }
    
    void menuRoom() {
        int choice;
        do {
            cout << "\n--- QUAN LY PHONG ---\n";
            cout << "1. Them phong\n";
            cout << "2. Xoa phong\n";
            cout << "3. Hien thi danh sach phong\n";
            cout << "4. Tim kiem phong\n";
            cout << "0. Quay lai\n";
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
                string id;
                cout << "Ma phong can xoa: "; getline(cin, id);
                roomMgr.deleteRoom(id);
            } else if (choice == 3) {
                roomMgr.displayAllRooms();
            } else if (choice == 4) {
                string keyword;
                cout << "Nhap tu khoa: "; getline(cin, keyword);
                roomMgr.searchRoom(keyword);
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
    }
    
    void menuCustomer() {
        int choice;
        do {
            cout << "\n--- QUAN LY KHACH HANG ---\n";
            cout << "1. Them khach hang\n";
            cout << "2. Xoa khach hang\n";
            cout << "3. Hien thi danh sach khach\n";
            cout << "4. Tim kiem khach hang\n";
            cout << "0. Quay lai\n";
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
                string id;
                cout << "Ma khach can xoa: "; getline(cin, id);
                custMgr.deleteCustomer(id);
            } else if (choice == 3) {
                custMgr.displayAllCustomers();
            } else if (choice == 4) {
                string keyword;
                cout << "Nhap tu khoa: "; getline(cin, keyword);
                custMgr.searchCustomer(keyword);
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
    }
    
    void menuReservation() {
        int choice;
        do {
            cout << "\n--- DAT PHONG / NHAN PHONG ---\n";
            cout << "1. Dat phong\n";
            cout << "2. Nhan phong\n";
            cout << "3. Hien thi danh sach dat phong\n";
            cout << "0. Quay lai\n";
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
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
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
        do {
            cout << "\n--- QUAN LY HOA DON ---\n";
            cout << "1. Hien thi tat ca hoa don\n";
            cout << "2. Sap xep hoa don theo tong tien\n";
            cout << "0. Quay lai\n";
            cout << "Chon: ";
            cin >> choice;
            clearInput();
            
            if (choice == 1) {
                invMgr.displayAllInvoices();
            } else if (choice == 2) {
                invMgr.sortByTotal();
                invMgr.displayAllInvoices();
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
    }
    
    void menuStatistics() {
        int choice;
        do {
            cout << "\n--- THONG KE ---\n";
            cout << "1. Thong ke phong\n";
            cout << "2. Thong ke doanh thu\n";
            cout << "0. Quay lai\n";
            cout << "Chon: ";
            cin >> choice;
            clearInput();
            
            if (choice == 1) {
                roomMgr.displayStatistics();
            } else if (choice == 2) {
                invMgr.displayRevenueStatistics();
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
    }
    
    void menuAdvanced() {
        int choice;
        do {
            cout << "\n--- CHUC NANG NANG CAO ---\n";
            cout << "1. Tim to hop phong (Backtracking)\n";
            cout << "2. Toi uu chi phi thue phong (Dynamic Programming)\n";
            cout << "0. Quay lai\n";
            cout << "Chon: ";
            cin >> choice;
            clearInput();
            
            if (choice == 1) {
                findRoomCombination();
            } else if (choice == 2) {
                optimizeRoomCost();
            } else if (choice == 0) {
                break;
            }
        } while (choice != 0);
    }
    
    void findRoomCombination() {
        cout << "\n--- TIM TO HOP PHONG CHO DOAN KHACH ---\n";
        cout << "Nhap so loai phong can tim: ";
        int n;
        cin >> n;
        clearInput();
        
        vector<pair<string, int>> requests;
        for (int i = 0; i < n; i++) {
            string type;
            int quantity;
            cout << "Loai phong " << (i+1) << ": ";
            getline(cin, type);
            cout << "So luong: ";
            cin >> quantity;
            clearInput();
            requests.push_back({type, quantity});
        }
        
        RoomCombinationSolver solver;
        if (solver.findRoomCombination(requests, roomMgr.getRooms(), roomMgr.getRoomCount())) {
            cout << "\n=== TIM THAY TO HOP PHONG PHU HOP ===\n";
            vector<Room*> solution = solver.getSolution();
            for (Room* room : solution) {
                cout << "- Phong " << room->roomId << " (" << room->roomType 
                     << ") - Gia: " << fixed << setprecision(3) << room->pricePerDay << "/ngay\n";
            }
        } else {
            cout << "\nKhong tim thay to hop phong phu hop!\n";
        }
    }
    
    void optimizeRoomCost() {
        cout << "\n--- TOI UU CHI PHI THUE PHONG ---\n";
        cout << "Nhap tong so ngay can thue: ";
        int totalDays;
        cin >> totalDays;
        clearInput();
        
        vector<PriceOptimizer::RoomOption> options;
        Room* rooms = roomMgr.getRooms();
        int count = roomMgr.getRoomCount();
        
        for (int i = 0; i < count; i++) {
            if (rooms[i].isAvailable) {
                PriceOptimizer::RoomOption opt;
                opt.roomId = rooms[i].roomId;
                opt.roomType = rooms[i].roomType;
                opt.price = rooms[i].pricePerDay;
                opt.daysAvailable = totalDays;
                options.push_back(opt);
            }
        }
        
        if (options.empty()) {
            cout << "Khong co phong trong!\n";
            return;
        }
        
        vector<string> selectedRooms;
        double minCost = PriceOptimizer::findMinCost(options, totalDays, selectedRooms);
        
        if (minCost >= 1e9) {
            cout << "Khong the tim duoc giai phap phu hop!\n";
        } else {
            cout << "\n=== GIAI PHAP TOI UU ===\n";
            cout << "Chi phi toi thieu: " << fixed << setprecision(3) << minCost << " VND\n";
            cout << "Cac phong duoc chon:\n";
            for (const string& room : selectedRooms) {
                cout << "- " << room << "\n";
            }
        }
    }
};

// ==================== MAIN ====================
int main() {
    HotelManagementSystem system;
    system.run();
    return 0;
}