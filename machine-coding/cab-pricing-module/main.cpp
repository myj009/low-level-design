#include <string>
#include <iostream>
#include <unordered_map>
#include <mutex>

using namespace std;

class FeeStore {
    static FeeStore* instance;
    static mutex mtx;

    unordered_map<string, double> distanceRates;
    unordered_map<string, double> timeRates;
    unordered_map<string, double> surgeRates;
    unordered_map<string, double> taxRates;
    FeeStore() {
        distanceRates.clear();
        timeRates.clear();
        surgeRates.clear();
        taxRates.clear();
    }
public:
    static FeeStore* getFeeStore() {
        if(instance == nullptr) {
            lock_guard<mutex> lock(mtx);
            if(instance == nullptr) {
                instance = new FeeStore();
            }
        }
        return instance;
    }

    void upsertDistanceRates(string city, double amt) {
        distanceRates[city] = amt;
    }

    void upsertTimeRates(string city, double amt) {
        timeRates[city] = amt;
    }

    void upsertSurgeRates(string city, double amt) {
        surgeRates[city] = amt;
    }

    void upsertTaxRates(string city, double amt) {
        taxRates[city] = amt;
    }

    double getDistanceRate(string city) {
        return distanceRates[city];
    }

    double getTaxRate(string city) {
        return taxRates[city];
    }

    double getSurgeRate(string city) {
        return surgeRates[city];
    }

    double getTimeRate(string city) {
        return timeRates[city];
    }
};

FeeStore* FeeStore::instance = nullptr;
mutex FeeStore::mtx;

struct PricingContext {
    double distance;
    double minutes;
    string city;
    PricingContext(double dis, double mins, string ct): distance(dis), minutes(mins), city(ct) {}
};

class IPricingStrategy {
public:
    virtual double calculatePrice(PricingContext* pc) = 0;
    virtual void print(double amt) = 0;
};

class CabPricingStrategy: public IPricingStrategy {
    double basePrice;
public:
    CabPricingStrategy(double b): basePrice(b) {}
    double calculatePrice(PricingContext* pc) override {
        print(basePrice);
        return basePrice;
    }

    void print(double amt) override {
        cout<<"Base: "<<amt<<"\n";
    }

    void setBasePrice(double price) {
        basePrice = price;
    }
};

class PricingDecorator: public IPricingStrategy {
public:
    IPricingStrategy* pricingStrategy;
    FeeStore* feeStore;
    PricingDecorator(IPricingStrategy* ps): pricingStrategy(ps) {
        feeStore = FeeStore::getFeeStore();
    }
};

class DistancePricingDecorator: public PricingDecorator {
public:
    DistancePricingDecorator(IPricingStrategy* ps): PricingDecorator(ps) {}

    double calculatePrice(PricingContext* pc) override {
        double amt = pricingStrategy->calculatePrice(pc);
        double pricePerKm = feeStore->getDistanceRate(pc->city);
        double distanceFee = pc->distance * pricePerKm;
        print(distanceFee);
        return amt + distanceFee;
    }

    void print(double amt) override {
        cout<<"Distance Fees: "<<amt<<"\n";
    }
};

class TimePricingDecorator: public PricingDecorator {
public:
    TimePricingDecorator(IPricingStrategy* ps): PricingDecorator(ps) {}

    double calculatePrice(PricingContext* pc) override {
        double amt = pricingStrategy->calculatePrice(pc);
        double pricePerMin = feeStore->getTimeRate(pc->city);
        double timeFee = pc->minutes * pricePerMin;
        print(timeFee);
        return amt + timeFee;
    }

    void print(double amt) override {
        cout<<"Time Fees: "<<amt<<"\n";
    }
};

class SurgePricingDecorator: public PricingDecorator {
public:
    SurgePricingDecorator(IPricingStrategy* ps): PricingDecorator(ps) {}

    double calculatePrice(PricingContext* pc) override {
        double amt = pricingStrategy->calculatePrice(pc);
        double surgeRate = feeStore->getSurgeRate(pc->city);
        double surgePrice = amt * surgeRate;
        print(surgePrice - amt);
        return surgePrice;
    }

    void print(double amt) override {
        cout<<"Surge Fee: "<<amt<<"\n";
    }
};

class TaxFeeDecorator: public PricingDecorator {
public:
    TaxFeeDecorator(IPricingStrategy* ps): PricingDecorator(ps) {}

    double calculatePrice(PricingContext* pc) override {
        double amt = pricingStrategy->calculatePrice(pc);
        double taxRate = feeStore->getTaxRate(pc->city);
        double tax = amt * taxRate;
        print(tax);
        return amt + tax;
    }

    void print(double amt) override {
        cout<<"GST: "<<amt<<"\n";
    }
};

class PricingStrategyBuilder {
    IPricingStrategy* pricingStrategy;
public:
    PricingStrategyBuilder(double base) {
        pricingStrategy = new CabPricingStrategy(base);
    }

    PricingStrategyBuilder* addDistanceFee() {
        pricingStrategy = new DistancePricingDecorator(pricingStrategy);
        return this;
    }

    PricingStrategyBuilder* addTimeFee() {
        pricingStrategy = new TimePricingDecorator(pricingStrategy);
        return this;
    }

    PricingStrategyBuilder* addSurgeRate() {
        pricingStrategy = new SurgePricingDecorator(pricingStrategy);
        return this;
    }

    PricingStrategyBuilder* addTaxRate() {
        pricingStrategy = new TaxFeeDecorator(pricingStrategy);
        return this;
    }

    IPricingStrategy* build() {
        return pricingStrategy;
    }
};

class PriceCalculator {
    IPricingStrategy* pricingstrategy;
public:
    PriceCalculator(IPricingStrategy* ps): pricingstrategy(ps) {}

    void setStrategy(IPricingStrategy* strat) {
        pricingstrategy = strat;
    }

    double calculateRideFare(PricingContext* pc) {
        double amt = pricingstrategy->calculatePrice(pc);
        cout<<"Total: "<<amt<<"\n";
        return amt;
    }

};

int main() {
    auto strategy = PricingStrategyBuilder(100).addDistanceFee()->addTimeFee()->addSurgeRate()->addTaxRate()->build();
    auto fareCalculator = new PriceCalculator(strategy);

    auto feeStore = FeeStore::getFeeStore();
    feeStore->upsertDistanceRates("hyd", 3);
    feeStore->upsertTimeRates("hyd", 0.3);
    feeStore->upsertSurgeRates("hyd", 1);
    feeStore->upsertTaxRates("hyd", 0.09);

    PricingContext* newRide = new PricingContext(10, 22, "hyd");
    double fare = fareCalculator->calculateRideFare(newRide);

    feeStore->upsertSurgeRates("hyd", 1.2);
    fare = fareCalculator->calculateRideFare(newRide);
}