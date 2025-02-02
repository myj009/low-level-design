#include <string>
#include <stdexcept>
#include <unordered_map>

using namespace std;

class Account {
    static int nextAccountNumber;
    string accountNumber;
    double balance;
public:
    Account(): accountNumber(to_string(nextAccountNumber++)), balance(0) {}

    string getAccountNumber() {
        return accountNumber;
    }

    double getBalance() {
        return balance;
    }

    void Debit(double amount) {
        if(balance < amount) {
            throw invalid_argument("Insufficient balance");
        }
        balance-=amount;
    }

    void Credit(double amount) {
        balance += amount;
    }
};

int Account::nextAccountNumber = 1;

class Bank {
    unordered_map<string, Account*> accounts;
public:
    Bank() {
        accounts.clear();
    }

    string addAccount() {
        auto newAccount = new Account();
        accounts[newAccount->getAccountNumber()] = newAccount;
        return newAccount->getAccountNumber();
    }

    void deleteAccount(string id) {
        if(!accounts.count(id)) {
            throw logic_error("Account does not exist");
        }
        delete accounts[id];
        accounts.erase(id);
    }

    void addMoney(string id, double amount) {
        if(!accounts.count(id)) {
            throw logic_error("Account does not exist");
        }
        auto acc = accounts[id];
        acc->Credit(amount);
    }
};

class CashDispenser {
    double balance;
public:
    CashDispenser(double amt): balance(amt) {}

    void addMoney(double amt) {
        balance += amt;
    }

     void dispenseMoney(double amt) {
        if(balance < amt) {
            throw logic_error("Cash not available");
        }
        balance -= amt;
    }

};

class Transaction {
public:
    int transactionId;
    Account* account;
    double amount;
    static int id;

    Transaction(Account* acc, double amt): transactionId(id++), account(acc), amount(amt) {}

    virtual ~Transaction() = default;
    virtual void execute() = 0;
};

class WithdrawTransaction: public Transaction {
public:
    WithdrawTransaction(Account* acc, double amt): Transaction(acc, amt) {}
    virtual void execute() override {
        
    }
};

class DepositTransaction: public Transaction {
public:
    DepositTransaction(Account* acc, double amt): Transaction(acc, amt) {}
    virtual void execute() override {
        
    }
};

int Transaction::id = 1;

class Card {
    string cardNumber;
    int pin;
public:
    Card(string cn, int pn): cardNumber(cn), pin(pn) {}
};

class ATM {

};

int main() {

}
