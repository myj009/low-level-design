#include <bits/stdc++.h>
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <unordered_set>
using namespace std;

class Observer {
public:
    virtual void update(string pName, string msg) = 0;
};

class Customer: public Observer {
    int id;
    string name;
public:
    Customer(int id, string name): id(id), name(name){}
    virtual void update(string pName, string msg) override {
        cout<<"Notification: Product - "<<pName<<", Message - "<<msg<<", User - "<<name<<"\n";
    }
};

class Product {
public:
    int id;
    string name;

    Product(int id, string name): id(id), name(name){}
};

struct ObserverList {
    list<Observer*> lst;
    unordered_map<Observer*, list<Observer*>::iterator> mp;
    ObserverList(){
        lst.clear();
        mp.clear();
    }
};

class Store {
    string name;
    int id;
    unordered_map<Product*, ObserverList*> products;
public:
    Store(int id, string name): name(name), id(id){
        products.clear();
    }

    virtual void registerObserver(Observer* o, Product* p) {
        if(products.find(p) == products.end()) return;
        auto obsList = products[p];
        if(obsList->mp.find(o) != obsList->mp.end()) return;
        obsList->lst.push_front(o);
        obsList->mp[o] = obsList->lst.begin();
        // cout<<"Registered\n";
    }

    virtual void unregisterObserver(Observer* o, Product *p) {
        if(products.find(p) == products.end()) return;
        auto obsList = products[p];
        if(obsList->mp.find(o) == obsList->mp.end()) return;
        obsList->lst.erase(obsList->mp[o]);
        obsList->mp.erase(o);
    }

    virtual void notifyObservers(Product* p, string msg){
        if(products.find(p) == products.end()) return;
        
        auto obsList = products[p];
        // cout<<obsList->mp.size()<<"\n";
        for(auto it = obsList->lst.begin(); it!=obsList->lst.end();it++){
            // cout<<"Notifying\n";
            (*it)->update(p->name, msg);
        }
    }

    void addProduct(Product* p){
        if(products.find(p) != products.end()) return;
        products[p] = new ObserverList();
    }

    void updateProduct(Product* p, string message){
        notifyObservers(p, message);
    }
};

int main() {
    Store* walmart = new Store(1, "Walmart");

    Customer* alex = new Customer(1, "Alex");
    Customer* mj = new Customer(2, "MJ");
    Customer* zed = new Customer(3, "Zed");

    Product* iphone = new Product(1, "Iphone");
    Product* mac = new Product(2, "Macbook");

    walmart->addProduct(iphone);
    walmart->addProduct(mac);

    walmart->registerObserver(alex, iphone);
    walmart->registerObserver(alex, mac);
    walmart->registerObserver(mj, mac);

    walmart->updateProduct(mac, "New version launched");

    walmart->registerObserver(zed, iphone);
    walmart->unregisterObserver(mj, mac);

    walmart->updateProduct(iphone, "New phone launch");
    walmart->updateProduct(mac, "Discounts");
}