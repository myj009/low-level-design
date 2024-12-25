#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
using namespace std;

enum class TimeUnit {
    Hour,
    Day
};

class Bookable {
public:
    virtual int GetMaxBookingsPerSlot() = 0;
    virtual string GetName() = 0;
    virtual double GetCost() = 0;
    virtual TimeUnit GetSlotTimeUnit() = 0;
    virtual int GetID() = 0;
    virtual ~Bookable() = default;
};

class Hotel: public Bookable {
    static int i;
    int id;
    string name;
    string city;
    int roomCnt;
    string owner;
    double cost;
public:
    Hotel(string name, string city, int rooms, string owner, double cost):
        id(i++),name(name),city(city),roomCnt(rooms), owner(owner), cost(cost) {}

    void print() {
        cout<<id<<" - "<<name<<"\n";
    }

    virtual int GetMaxBookingsPerSlot() override {
        return roomCnt;
    }
    virtual string GetName() override {
        return name;
    }
    virtual double GetCost() override {
        return cost;
    }
    virtual TimeUnit GetSlotTimeUnit() override {
        return TimeUnit::Day;
    }
    virtual int GetID() override {
        return id;
    }
};

int Hotel::i = 1;

class Restaurant: public Bookable {
    static int i;
    int id;
    string name;
    string city;
    int tableCnt;
    string owner;
    double cost;
public:
    Restaurant(string name, string city, int tables, string owner, double cost):
        id(i++),name(name),city(city),tableCnt(tables), owner(owner), cost(cost) {}

    void print() {
        cout<<id<<" - "<<name<<"\n";
    }

    virtual int GetMaxBookingsPerSlot() override {
        return tableCnt;
    }
    virtual string GetName() override {
        return name;
    }
    virtual double GetCost() override {
        return cost;
    }
    virtual TimeUnit GetSlotTimeUnit() override {
        return TimeUnit::Hour;
    }
    virtual int GetID() override {
        return id;
    }
};

int Restaurant::i = 1;

class BookingMgr {
public:
    virtual void Register(Bookable* bookable) = 0;
    virtual void Unregister(int id) = 0;
    virtual vector<Bookable*> ListBookables() = 0;
    virtual vector<Bookable*> SearchByName(string name) = 0;
    virtual vector<Bookable*> SearchByCity(string name) = 0;
    virtual vector<Bookable*> SearchByCost(double minn, double maxx) = 0;
    virtual ~BookingMgr() = default;
};

class RestaurantBookingMgr: public BookingMgr {
    RestaurantBookingMgr() = default;
    ~RestaurantBookingMgr() = default;
    RestaurantBookingMgr(const RestaurantBookingMgr&) = delete;
    RestaurantBookingMgr& operator=(const RestaurantBookingMgr&) = delete;

    list<Restaurant*> restaurants;
    unordered_map<int, list<Restaurant*>::iterator> mp;

public:
    static RestaurantBookingMgr& getInstance() {
        static RestaurantBookingMgr instance;
        return instance;
    }

    virtual void Register(Bookable* bookable) override {
        int id = bookable->GetID();
        if(mp.find(id) != mp.end()) cout<<"Already Registered\n";

        restaurants.push_front(dynamic_cast<Restaurant*>(bookable));
        mp[bookable->GetID()] = restaurants.begin();
    }

    virtual void Unregister(int id) override {
        if(mp.find(id) == mp.end()) cout<<"Restaurant not registered.\n";

        auto it = mp[id];
        restaurants.erase(it);
        mp.erase(id);
    }
    virtual vector<Bookable*> ListBookables() override {
        vector<Bookable*> res;
        for(auto it:restaurants){
            res.push_back(it);
            cout<<it->GetName()<<" ";
        }
        cout<<"\n";

        return res;
    }

    virtual vector<Bookable*> SearchByName(string name) override {
        return {};
    }
    virtual vector<Bookable*> SearchByCity(string name) override {
        return {};
    }
    virtual vector<Bookable*> SearchByCost(double minn, double maxx) {
        return {};
    }

};

class System {
    System(BookingMgr* mgr): bookingMgr(mgr) {};
    ~System() = default;

    BookingMgr* bookingMgr;
public:
    System(const System&) = delete;
    System operator=(const System&) = delete;

    static System& getInstance(BookingMgr* mgr) {
        static System instance(mgr);
        return instance;
    }

    void SetBookingMgr(BookingMgr* mgr) {
        bookingMgr = mgr;
    }
};

int main() {
    auto mcd = new Restaurant("McDonalds", "Mumbai", 10, "MJ", 100.0);
    auto br = new Restaurant("Baskin Robbins", "Pune", 8, "SS", 150.0);

    RestaurantBookingMgr& resMgr = RestaurantBookingMgr::getInstance();
    resMgr.Register(mcd);
    resMgr.Register(br);
    resMgr.ListBookables();
    resMgr.Unregister(mcd->GetID());
    resMgr.ListBookables();

    // mcd->print();
    // br->print();
}