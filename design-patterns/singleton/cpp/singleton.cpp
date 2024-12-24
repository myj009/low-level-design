#include <iostream>
#include <mutex>
using namespace std;

class Singleton2 {
    static Singleton2* instance;
    static mutex mtx;
    Singleton2() {
        cout<<"Singleton2 Created\n";
    }
    ~Singleton2() {
        cout<<"Singleton2 Destroyed\n";
    }

    Singleton2(const Singleton2&) = delete;
    Singleton2& operator=(const Singleton2&) = delete;

public:
    static Singleton2* getInstance() {
        if(instance == nullptr) {
            lock_guard<mutex> lock(mtx);
            if(instance == nullptr) {
                instance = new Singleton2();
            }
        }
        return instance;
    }

    void print(){
        cout<<"I am a singleton\n";
    }

    // Optional: Cleanup method to delete the instance manually
    static void cleanup() {
        std::lock_guard<std::mutex> lock(mtx);
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }
};

// Initialize static members
Singleton2* Singleton2::instance = nullptr;
mutex Singleton2::mtx;

class Singleton {
    Singleton() {
        cout<<"Singleton Created\n";
    };
    ~Singleton() {
        cout<<"Singleton Destroyed\n";
    }
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }

    void print(){
        cout<<"I am a singleton\n";
    }
};

int main() {
    Singleton& instance1 = Singleton::getInstance();
    instance1.print();

    Singleton& instance2 = Singleton::getInstance();
    instance2.print();

    Singleton2* ins1 = Singleton2::getInstance();
    ins1->print();

    Singleton2* ins2 = Singleton2::getInstance();
    ins2->print();

    Singleton2::cleanup();
}