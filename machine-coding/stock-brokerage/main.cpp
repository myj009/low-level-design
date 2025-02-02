#include <atomic>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <map>
#include <thread>
#include <csignal>

using namespace std;

atomic<bool> running{true};

class Account {
    static int id;
    int accountId;
    atomic<double> balance;
    mutex mtx;
public:
    Account(): accountId(id++), balance(0) {}

    void depositMoney(double amt) {
        balance = balance + amt;
    }

    void debitAmount(double amt) {
        lock_guard<mutex> lock(mtx);
        if(balance < amt) {
            throw logic_error("Insufficient balance");
        }

        balance = balance - amt;
    }

    double getBalance() {
        return balance.load();
    }
};
int Account::id = 1;

class Stock {
    string ticker;
    double price;
public:
    Stock(string tkr, double p): ticker(tkr), price(p) {}

    void updatePrice(double p) {
        price = p;
    }

    double getPrice() {
        return price;
    }

    string getTicker() {
        return ticker;
    }
};

enum OrderStatus {
    Pending,
    Completed,
    Rejected
};

enum OrderType {
    Buy,
    Sell
};

class IOrder {
public:
    static atomic<int> nextId;
    int orderId;
    string stockTicker;
    int quantity;
    double price;
    OrderStatus status;
    OrderType orderType;
    IOrder(string ticker, int qt, OrderType ot, double p): orderId(nextId.fetch_add(1)), stockTicker(ticker), quantity(qt), status(Pending), orderType(ot), price(p) {
        cout<<orderId<<"\n";
    }
};
atomic<int> IOrder::nextId{1};

class OrderQueue {
    queue<IOrder*> q;
    mutex mtx;
    condition_variable cond;
public:
    void push(IOrder* order) {
        lock_guard<mutex> lock(mtx);
        q.push(order);
        cond.notify_one();
    }

    IOrder* pop() {
        unique_lock<mutex> lock(mtx);
        cond.wait(lock, [this] { return !q.empty(); });
        auto order = q.front();
        q.pop();
        return order;
    }

    bool empty() {
        lock_guard<mutex> lock(mtx);
        return q.empty();
    }

    IOrder* front() {
        unique_lock<mutex> lock(mtx);
        cond.wait(lock, [this] {return !q.empty();});
        return q.front();
    }
};

class MatchingEngine {
    unordered_map<string,multimap<double, IOrder*>> buyOrders;
    unordered_map<string,multimap<double, IOrder*>> sellOrders;
public:
    void addOrder(IOrder* order) {
        cout<<"addOrder- "<<order->orderId<<"\n";
        if(order->orderType == OrderType::Buy) {
            buyOrders[order->stockTicker].insert({order->price, order});
        } else {
            sellOrders[order->stockTicker].insert({order->price, order});
        }
        matchOrders(order->stockTicker);
    }

    void matchOrders(string ticker) {
        auto& bOrders = buyOrders[ticker];
        auto& sOrders = sellOrders[ticker];
        cout<<"here\n";

        while(!bOrders.empty() && !sOrders.empty()) {
            auto bb = bOrders.rbegin();
            auto bs = sOrders.begin();
            auto bestBuy = bb->second;
            auto bestSell = bs->second;
            cout<<bestBuy->orderId<<"-"<<bestSell->orderId<<"\n";
            if(bestBuy->price >= bestSell->price) {
                int matchedQuantity = min(bestBuy->quantity, bestSell->quantity);
                cout<<"Stock: "<<ticker<<"\nBuy Order Id: "<<bestBuy->orderId<<"\nSell Order Id: "<<bestSell->orderId<<"\nQuantity: "<<matchedQuantity<<"\nPrice: "<<bestSell->price<<"\n";

                bestBuy->quantity -= matchedQuantity;
                bestSell->quantity -= matchedQuantity;

                if(bestBuy->quantity == 0) bOrders.erase(next(bb).base());
                if(bestSell->quantity == 0) sOrders.erase(next(bb).base());
            } else {
                break;
            }
        }  
    }
};

void ProcessOrders(OrderQueue* q, MatchingEngine* engine) {
    while(running) {
        if(!q->empty()) {
            auto order = q->pop();
            cout<<"Popped order: "<<order->orderId<<"\n";
            engine->addOrder(order);
        } else {
            std::this_thread::sleep_for(chrono::milliseconds(10));
        }
    }
    cout<<"Processing thread stopped\n";
}

void signalHandler(int signal) {
    if(signal == SIGINT) {
        cout<<"Received interrupt signal. Stopping process\n";
        running = false;
    }
}

class Exchange {
    static mutex mtx;
    static Exchange* instance;
    unordered_map<string, Stock*> stocks;
    unordered_map<int, IOrder*> orders;
    Exchange() = default;
    OrderQueue* oq;
public:
    static Exchange* getInstance() {
        if(instance == nullptr) {
            lock_guard<mutex> lock(mtx);
            if(instance == nullptr) {
                instance = new Exchange();
            }
        }
        return instance;
    }

    void setOrderQueue(OrderQueue* o) {
        oq = o;
    }

    void placeOrder(IOrder* o) {
        if(!oq) throw runtime_error("No order queue set");
        if(!stocks.count(o->stockTicker)) throw logic_error("stock does not exist");
        oq->push(o);
        cout<<"Pushed Order: "<<o->orderId<<"\n";
    }

    void addStock(Stock* st) {
        stocks[st->getTicker()] = st;
    }
};
Exchange* Exchange::instance = nullptr;
mutex Exchange::mtx;

int main() {
    try {
        signal(SIGINT, signalHandler);
        OrderQueue* oq = new OrderQueue();
        MatchingEngine* me = new MatchingEngine();
        thread processingThread(ProcessOrders, oq, me);

        auto exchange = Exchange::getInstance();
        exchange->setOrderQueue(oq);
        auto apple = new Stock("apl", 100);
        exchange->addStock(apple);

        auto order1 = new IOrder("apl", 10, OrderType::Buy, 99);
        exchange->placeOrder(order1);
        
        auto order2 = new IOrder("apl", 9, OrderType::Sell, 99);
        cout<<order1->orderId<<"-"<<order2->orderId<<"\n";
        exchange->placeOrder(order2);

        processingThread.join();
        cout<<"Shutting down\n";

    } catch (const exception& e) {
        cout<<"Exception: "<<e.what() <<endl;
    }
}