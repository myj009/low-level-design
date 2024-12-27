#include <string>
#include <unordered_map>
#include <iostream>
#include <random>
#include <list>
#include <unordered_set>
using namespace std;

enum PaymentStatus {
    Success,
    Failed,
    Processing
};

enum PaymentMethod {
    UPI,
    NB,
    CARD
};

string PaymentMethodtoString(PaymentMethod pm){
    if(pm == PaymentMethod::UPI) return "UPI";
    if(pm == PaymentMethod::NB) return "NB";
    if(pm == PaymentMethod::CARD) return "CARD";
    return "UPI";
}

string PaymentStatusToString(PaymentStatus ps){
    if(ps == PaymentStatus::Success) return "Success";
    if(ps == PaymentStatus::Failed) return "Failed";
    return "Processing";
}

class PaymentGateway;

class Bank {
    string name;
public:
    Bank(string name): name(name) {}
    string getName(){ return name; }
    virtual PaymentStatus processPayment() = 0;
    virtual double GetSuccessRate() = 0;
    virtual ~Bank() = default;
};

class HDFC: public Bank {
public:
    HDFC(): Bank("HDFC") {}
    virtual PaymentStatus processPayment() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0.0, 1.0);
        double num = dis(gen);

        if(num>0.95) return PaymentStatus::Failed;
        return PaymentStatus::Success;
    }

    virtual double GetSuccessRate() {
        return 0.95;
    }
};

class SBI: public Bank {
public:
    SBI(): Bank("SBI") {}
    virtual PaymentStatus processPayment() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0.0, 1.0);
        double num = dis(gen);

        if(num>0.7) return PaymentStatus::Failed;
        return PaymentStatus::Success;
    }

    virtual double GetSuccessRate() {
        return 0.7;
    }
};

class Client {
public:
    virtual void UpdatePaymentStatus(int id, PaymentStatus status) = 0;
    virtual void makePayment() = 0;
};

class Flipkart: public Client {
    // PaymentGateway* pg;
    unordered_map<int, PaymentStatus> payments;
    Flipkart() {
        payments.clear();
        // pg = nullptr;
    }
    Flipkart(const Flipkart&) = delete;
    Flipkart& operator=(Flipkart&) = delete;
public:
    virtual void UpdatePaymentStatus(int id, PaymentStatus status) {

    }

    virtual void makePayment() {
        
    }

    static Flipkart& getInstance() {
        static Flipkart instance;
        return instance;
    }

    // void setPaymentGateway(PaymentGateway* pg) {
    //     pg = pg;
    // }
};

class PaymentStrategy {
    PaymentMethod name;
public:
    Bank* bank;
    PaymentStrategy(PaymentMethod name): bank(nullptr), name(name) {}
    virtual PaymentStatus ProcessPayment() = 0;

    void switchBank(Bank* bank1) {
        bank = bank1;
        // cout<<bank<<" "<<bank1<<"\n";
    }

    PaymentMethod GetName() {
        return name;
    }
};

class UPIStrategy: public PaymentStrategy {
    string vpa;
public:
    UPIStrategy(string vpa): PaymentStrategy(PaymentMethod::UPI), vpa(vpa) {}

    virtual PaymentStatus ProcessPayment() {
        cout<<"Sending to bank for processing payment\n";
        // cout<<bank<<"\n";
        auto status = bank->processPayment();
        cout<<"Payment "<<PaymentStatusToString(status)<<"\n";
        return status;
    };
};

struct Card {
    string number;
    string name;
    string cvc;
    string expiryDate;

    Card(string name, string number, string cvc, string expiryDate): name(name), number(number), cvc(cvc), expiryDate(expiryDate) {}
};

class CardStrategy: public PaymentStrategy {
    Card card;
public:
    CardStrategy(Card card): PaymentStrategy(PaymentMethod::CARD), card(card) {}

    virtual PaymentStatus ProcessPayment() {
        return PaymentStatus::Success;
    };
};

struct PGClient {
    Client* client;
    unordered_set<PaymentMethod> paymentMethods;
    PGClient(Client* client): client(client), paymentMethods({}) {}
};

class Payment {
    static int ids;
    int id;
    PaymentMethod pm;
    string bank;
    PaymentStatus status;
    double amount;

public:
    Payment(PaymentMethod pm, string bank, double amt): id(ids++), pm(pm), bank(bank), status(PaymentStatus::Processing), amount(amt) {}

    void setStatus(PaymentStatus ps) {
        status = ps;
    }

    int getID() {
        return id;
    }
};

int Payment::ids = 1;

class RoutingStrategy {

public:
    virtual Bank* DecidePaymentBank(PaymentMethod pm, unordered_map<string, Bank*> banks) = 0;
};

class FixedStrategy: public RoutingStrategy {
public:
    virtual Bank* DecidePaymentBank(PaymentMethod pm, unordered_map<string, Bank*> banks) {
        cout<<"Deciding bank\n";
        if(pm == PaymentMethod::CARD) return banks["SBI"];
        if(pm == PaymentMethod::UPI) return banks["HDFC"];
        if(pm == PaymentMethod::NB) return banks["HDFC"];
        return banks["HDFC"];
    }
};

class DynamicStrategy: public RoutingStrategy {
public:
    virtual Bank* DecidePaymentBank(PaymentMethod pm, unordered_map<string, Bank*> banks) {
        Bank* best = nullptr;
        double bestSuccessRate = 0;

        for(auto& [name, bank]: banks) {
            if(bank->GetSuccessRate() > bestSuccessRate) {
                best = bank;
                bestSuccessRate = bank->GetSuccessRate();
            }
        }
        return best;
    }
};

class PaymentGateway {
    unordered_set<PaymentMethod> paymentMethods;
    static int clientID;
    unordered_map<int, PGClient*> clients;
    unordered_map<int, Payment*> payments;
    unordered_map<string, Bank*> banks;
    RoutingStrategy* routingStrategy;
    PaymentGateway() {
        paymentMethods.clear();
        clients.clear();
        banks.clear();
        payments.clear();
    }

public:
    static PaymentGateway& getInstance() {
        static PaymentGateway instance;
        return instance;
    }

    void setRoutingStrategy(RoutingStrategy* rs){
        routingStrategy = rs;
    }

    int addClient(Client* client) {
        auto pgclient = new PGClient(client);
        int id = clientID++;
        clients[id] = pgclient;
        return id;
    }

    void removeClient(int id) {
        if(clients.find(id) == clients.end()){
            cout<<"Client does not exist.\n";
            return;
        }
        clients.erase(id);
    }

    void addPaymentMethod(PaymentMethod pm) {
        if(paymentMethods.find(pm) !=paymentMethods.end()){
            cout<<"Payment method already added.\n";
            return;
        }
        paymentMethods.insert(pm);
    }

    void removePaymentMethod(PaymentMethod pm){
        if(paymentMethods.find(pm) == paymentMethods.end()){
            cout<<"Payment method not found.\n";
            return;
        }
        paymentMethods.erase(pm);
    }

    vector<PaymentMethod> listPaymentMethods() {
        vector<PaymentMethod> res;
        for(auto& pm: paymentMethods) {
            cout<<PaymentMethodtoString(pm)<<" ";
            res.push_back(pm);
        }
        cout<<"\n";
        return res;
    }

    void addPaymentMethodForClient(int id, PaymentMethod pm) {
        if(clients.find(id) == clients.end()){
            cout<<"Client does not exist.\n";
            return;
        }

        if(paymentMethods.find(pm) == paymentMethods.end()){
            cout<<"Payment method not found.\n";
            return;
        }

        clients[id]->paymentMethods.insert(pm);
    }

    void removePaymentMethodForClient(int id, PaymentMethod pm){
        if(clients.find(id) == clients.end()){
            cout<<"Client does not exist.\n";
            return;
        }

        auto clientPMs = clients[id]->paymentMethods;
        clientPMs.erase(pm);
    }

    void addBank(Bank* bank) {
        if(banks.find(bank->getName()) != banks.end()){
            cout<<"Bank already registered\n";
            return;
        }

        banks[bank->getName()] = bank;
    }

    PaymentStatus makePayment(int clientID, PaymentStrategy* ps, double amount) {
        auto client = clients[clientID];
        if(client->paymentMethods.find(ps->GetName()) == client->paymentMethods.end()){
            cout<<"Payment method not added for client\n";
            return PaymentStatus::Failed;
        }

        Bank* paymentBank = routingStrategy->DecidePaymentBank(ps->GetName(), banks);
        // cout<<paymentBank<<"\n";
        ps->switchBank(paymentBank);
        // cout<<ps->bank<<"\n";
        auto payment = new Payment(ps->GetName(), paymentBank->getName(), amount);

        cout<<"Processing payment\n";
        auto status = ps->ProcessPayment();
        payment->setStatus(status);
        payments[payment->getID()] = payment;
        return status;
    }
};

int PaymentGateway::clientID = 1;

int main () {
    PaymentGateway& bestPG = PaymentGateway::getInstance();
    RoutingStrategy* fixed = new FixedStrategy();
    bestPG.setRoutingStrategy(fixed);

    Flipkart& flip = Flipkart::getInstance();
    int clientID = bestPG.addClient(&flip);
    
    bestPG.addPaymentMethod(PaymentMethod::UPI);
    bestPG.addPaymentMethod(PaymentMethod::CARD);
    bestPG.listPaymentMethods();

    Bank* hdfc = new HDFC();
    Bank* sbi = new SBI();
    bestPG.addBank(hdfc);
    bestPG.addBank(sbi);

    auto upiPayment = new UPIStrategy("abcd@ybl");

    bestPG.addPaymentMethodForClient(clientID, PaymentMethod::UPI);

    cout<<"Making payment\n";

    auto status = bestPG.makePayment(clientID, upiPayment, 100.0);
}