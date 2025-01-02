#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <map>
using namespace std;

enum VehicleSize {
    SMALL,
    MEDIUM,
    LARGE
};

enum TicketStatus {
    ISSUED,
    PAID
};

class Vehicle {
    string plateNumber;
    VehicleSize sz;
public:
    Vehicle(string pn, VehicleSize s): plateNumber(pn), sz(s) {}
    string getNumberPlate() {
        return plateNumber;
    }

    VehicleSize getSize() {
        return sz;
    }
    virtual void drive() = 0;
    virtual ~Vehicle() = default;
};

class Bike: public Vehicle {
public:
    Bike(string pn): Vehicle(pn, VehicleSize::SMALL) {}

    virtual void drive() override {}
};

class Car: public Vehicle {
public:
    Car(string pn): Vehicle(pn, VehicleSize::MEDIUM) {}

    virtual void drive() override {}
};

class Truck: public Vehicle {
public:
    Truck(string pn): Vehicle(pn, VehicleSize::LARGE) {}

    virtual void drive() override {}
};

class Slot {
    int id;
    VehicleSize sz;
    Vehicle* vehicle;
public:
    Slot(int id, VehicleSize s): id(id), sz(s), vehicle(nullptr) {}

    bool isEmpty() {
        return vehicle == nullptr;
    }

    void fillSlot(Vehicle* v) {
        if(!isEmpty()) throw logic_error("Slot is already full");
        vehicle = v;
    }

    void emptySlot() {
        if(isEmpty()) throw logic_error("Slot already empty");
        vehicle = nullptr;
    }
};

class PricingStrategy {
public:
    virtual double getCost(VehicleSize sz, string entryTime, string exitTime) = 0;
};

class FixedPricingStrategy {
public:
    virtual double getCost(VehicleSize sz, string entryTime, string exitTime) {};
};

class Ticket {
    static int ids;
    int id;
    int level;
    Vehicle* vh;
    int slot;
    string entryTime;
    string exitTime;
    double cost;
    TicketStatus status;
public:
    Ticket(Vehicle* v, int s, string et, int l): id(ids++), vh(v), slot(s), entryTime(et), cost(0), 
        exitTime(""), level(l), status(TicketStatus::ISSUED) {}


    int getID() {}

    int getLevel() {}

    int getSlot() {}

    VehicleSize getSize() {}

    string getEntryTime() {}

    void exitVehicle(string et, double c) {
        exitTime = et;
        cost = c;
        status = TicketStatus::PAID;
    }
};

int Ticket::ids = 1;

class SlotSelectionStrategy {
public:
    virtual int getSlotForVehicle(VehicleSize sz, vector<Slot*> slots) = 0;
};

class NearestSlotSelectionStrategy {
public:
    virtual int getSlotForVehicle(VehicleSize sz, vector<Slot*> slots) {}
};

class Level {
    vector<Slot*> slots;
    SlotSelectionStrategy* strategy;
public:
    Level() {
        slots.resize(100);
        for(int i=0;i<50;i++){
            auto slt = new Slot(i, VehicleSize::SMALL);
        }

        for(int i=50;i<80;i++){
            auto slt = new Slot(i, VehicleSize::MEDIUM);
        }

        for(int i=80;i<100;i++){
            auto slt = new Slot(i, VehicleSize::LARGE);
        }
    }

    void SetStrategy(SlotSelectionStrategy* st){
        strategy = st;
    }

    int Park(Vehicle* vh) {
        int slotNumber = strategy->getSlotForVehicle(vh->getSize(), slots);
        slots[slotNumber]->fillSlot(vh);
        return slotNumber;
    }

    void unPark(int slotNumber) {
        slots[slotNumber]->emptySlot();
    }
};

class ParkingLot {
    vector<Level*> levels;
    map<int, Ticket*> tickets;

    PricingStrategy* strategy;
    // SlotSelectionStrategy* strategy;
    ParkingLot(int n) {
        levels.resize(n);
    };
public:
    static ParkingLot& getInstance(int n) {
        static ParkingLot instance(n);
        for(int i=0;i<n;i++){
            instance.levels[i] = new Level();
        }
        return instance;
    }

    void addLevel() {
        auto newLevel = new Level();
        levels.push_back(newLevel);
    }

    // void setStrategy(SlotSelectionStrategy* st){
    //     strategy = st;
    //     for(auto it:levels) {
    //         it->SetStrategy(st);
    //     }
    // }

    void setPricingStrategy(PricingStrategy* ps) {
        strategy = ps;
    }

    int Park(Vehicle* vh) {
        for(int i=0;i<levels.size();i++) {
            int slot = levels[i]->Park(vh);
            if(slot != -1) {
                auto ticket = new Ticket(vh, slot, "some time", i);
                tickets[ticket->getID()] = ticket;
                return ticket->getID();
            }
        }
        return -1;
    }

    double unPark(int ticketID) {
        auto ticket = tickets[ticketID];
        levels[ticket->getLevel()]->unPark(ticket->getSlot());
        double cost = strategy->getCost(ticket->getSize(), ticket->getEntryTime(), "exit time");
        ticket->exitVehicle("exit time", cost);
        return cost;
    } 
};

int main() {

}