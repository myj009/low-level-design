#include <unordered_map>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <mutex>

using namespace std;

class StockExchange;

struct Order {
public:
    static int nextId;
    int orderId;
    string ticker;
    double price;
    int quantity;
    Order(string t, double p, int q): orderId(nextId++), ticker(t), price(p), quantity(q) {}
};
int Order::nextId{1};

class IStockObserver {
public:
    virtual void update(Order* o) = 0;
    virtual int getId() = 0;
};

class IStockSubject {
public:
    virtual void addObserver(string ticker, IStockObserver* o) = 0;
    virtual void removeObserver(string ticker, IStockObserver* o) = 0;
    virtual void notifyObservers(Order* o) = 0;
};

struct ObserverList {
    list<IStockObserver*> lst;
    unordered_map<int, list<IStockObserver*>::iterator> mp;
};

class Account {
public:
    static int nextId;
    int accountId;
    Account(): accountId(nextId++) {}
    virtual void displayAccountInfo() = 0;
};
int Account::nextId{1};

class UserAccount: public Account {
public:
    UserAccount(): Account() {}
    virtual void displayAccountInfo() override {
        cout<<"User Account: "<<accountId<<"\n";
    }
};

class InstitutionalAccount: public Account, public IStockObserver {
public:
    InstitutionalAccount(): Account() {}
    virtual void displayAccountInfo() override {
        cout<<"Institution Account: "<<accountId<<"\n";
    }

    virtual void update(Order* o) override {
        cout<<"Account Id: "<<accountId<<"\nNotified for stock: "<<o->ticker<<"\nPrice: "<<o->price<<"\n";
    }

    virtual int getId() override {
        return accountId;
    }
};

struct Stock {
    string ticker;
    double price;
    Stock(string t, double p): ticker(t), price(p) {}
};

class StockExchange: public IStockSubject {
    static StockExchange* instance;
    static mutex mtx;
    
    unordered_map<string, ObserverList*> observers;
    unordered_map<string, Stock*> stocks;
    StockExchange() = default;
public:
    static StockExchange* getInstance() {
        if(instance == nullptr) {
            lock_guard<mutex> lock(mtx);
            if(instance == nullptr) {
                instance = new StockExchange();
            }
        }
        return instance;
    }

    void addStock(Stock* s) {
        stocks[s->ticker] = s;
        observers[s->ticker] = new ObserverList();
    }

    virtual void addObserver(string ticker, IStockObserver* o) override {
        if(!stocks.count(ticker)) throw invalid_argument("Invalid stock");
        auto obs = observers[ticker];
        if(obs->mp.count(o->getId())) {
            cout<<"Observer already exists\n";
            return;
        }
        obs->lst.push_front(o);
        obs->mp[o->getId()] = obs->lst.begin();
    }

    virtual void removeObserver(string ticker, IStockObserver* o) override {
        if(!stocks.count(ticker)) throw invalid_argument("Invalid stock");
        auto obs = observers[ticker];
        if(!obs->mp.count(o->getId())) {
            cout<<"Observer does not exists\n";
            return;
        }

        auto it = obs->mp[o->getId()];
        obs->lst.erase(it);
        obs->mp.erase(o->getId());
    }

    virtual void notifyObservers(Order* o) override {
        string ticker = o->ticker;
        if(!stocks.count(ticker)) throw invalid_argument("Invalid stock");
        if(!observers.count(ticker)) {
            cout<<"No one listening";
            return;
        }
        
        auto obs = observers[ticker]->lst;
        for(auto it:obs) {
            it->update(o);
        }
    }

    void watchStock(Account* acc, string ticker) {
        auto account = dynamic_cast<InstitutionalAccount*>(acc);
        if(account == nullptr) {
            cout<<"Not an institutional investor";
            return;
        }
        addObserver(ticker, account);
    }

    void processTransaction(Order* o) {
        string ticker = o->ticker;
        if(!stocks.count(ticker)) throw invalid_argument("Invalid stock");
        // Process transaction
        notifyObservers(o);
    }
};
StockExchange* StockExchange::instance = nullptr;
mutex StockExchange::mtx;

class AccountFactory{
public:
    static Account* createAccount(string type) {
        Account* acc = nullptr;
        if(type == "user") {
            acc = new UserAccount();
        } else {
            acc = new InstitutionalAccount();
        }
        return acc;
    }
};

int main() {
    auto instance = StockExchange::getInstance();
    Stock* apple = new Stock("apl", 100);
    Stock* google = new Stock("ggl", 50);
    Stock* amazon = new Stock("amz", 80);
    instance->addStock(apple);
    instance->addStock(google);
    instance->addStock(amazon);
    
    auto user1 = AccountFactory::createAccount("user");
    auto ins1 = AccountFactory::createAccount("ins");
    auto ins2 = AccountFactory::createAccount("ins");
    
    instance->watchStock(ins1, "apl");
    instance->watchStock(ins1, "ggl");
    instance->watchStock(ins2, "ggl");
    instance->watchStock(ins2, "amz");
    
    auto o1 = new Order("apl", 101, 10);
    instance->processTransaction(o1);
    auto o2 = new Order("ggl", 52, 20);
    instance->processTransaction(o2);
    auto o3 = new Order("amz", 80, 20);
    instance->processTransaction(o3);

}