#include <bits/stdc++.h>
using namespace std;

class PaymentStrategy {
public:
    virtual void ProcessPayment(string fromId, string toId, double amount) = 0;
    virtual ~PaymentStrategy() = default;
};

class CreditCard: public PaymentStrategy {
    virtual void ProcessPayment(string fromId, string toId, double amount) override{
        cout<<"Paying using CC\n";
    }
};

class Bitcoin: public PaymentStrategy {
    virtual void ProcessPayment(string fromId, string toId, double amount) override{
        cout<<"Paying using Bitcoin\n";
    }
};

class Paypal: public PaymentStrategy {
    virtual void ProcessPayment(string fromId, string toId, double amount) override{
        cout<<"Paying using Paypal\n";
    }
};

class PaymentProcessor{
    PaymentStrategy* paymentStrategy;
public:
    PaymentProcessor(PaymentStrategy* paymentStrategy):paymentStrategy(paymentStrategy){}
    void setStrategy(PaymentStrategy* ps){
        paymentStrategy = ps;
    }
    void executePayment(string fromId, string toId, double amount){
        paymentStrategy->ProcessPayment(fromId, toId, amount);
    }
};

int main(){
    auto ps = new Bitcoin();
    auto processor = PaymentProcessor(ps);
    processor.executePayment("mj", "ss", 100);

    auto ps1 = new CreditCard();
    processor.setStrategy(ps1);
    processor.executePayment("mj", "ss", 400);

    delete ps;
    delete ps1;
}