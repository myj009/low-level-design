#include <unordered_map>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <queue>
#include <stack>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <map>
#include <thread>
#include <csignal>

using namespace std;

class IContainer {
    string name;
public:
    IContainer(string n): name(n) {}
    virtual void ls() = 0;
    string getName() {
        return name;
    }
};

class File: public IContainer {
public:
    File(string n): IContainer(n) {}
    virtual void ls() override {
        cout<<getName()<<"\n";
    }
};

class Folder: public IContainer {
    vector<IContainer*> items;
public:
    Folder(string n): IContainer(n) {}
    virtual void ls() override {
        cout<<getName()<<"\n";
        for(auto item:items) {
            cout<<"  ";
            item->ls();
        }
    }

    void addItem(IContainer* c) {
        items.push_back(c);
    }
};

int main() {
    auto root = new Folder("root");
    auto subRoot = new Folder("subRoot");
    root->addItem(subRoot);
    auto file1 = new File("readme.md");
    root->addItem(file1);
    auto file2 = new File("openapi.yml");
    subRoot->addItem(file2);

    subRoot->ls();
    cout<<"\n-----\n";
    root->ls();

}