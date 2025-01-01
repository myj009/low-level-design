#include <string>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <vector>
using namespace std;

enum CommsType {
    SMS,
    EMAIL,
    SOUNDBOX
};

enum CommsPriority {
    TRANSACTIONAL,
    PROMOTIONAL
};

enum CommsStatus {
    SUCCESSFUL,
    FAILED,
    PROCESSING
};

struct BasicAuth {
    string username;
    string password;

    BasicAuth(string username, string password): username(username), password(password) {}
};

struct CommsRequest {
    CommsType requestType;
    CommsPriority priority;
    unordered_map<string, string> payload;
    BasicAuth* auth;
    CommsRequest(CommsType type, CommsPriority p, unordered_map<string, string>& payload): requestType(type), priority(p), payload(payload), auth(nullptr) {}
};

struct CommsProvider;

class Provider {
    unordered_set<CommsType> supportedRequestTypes;
    unordered_map<string, string> authPairs;
    string id;
    string name;
public:
    Provider(string id, string name, vector<CommsType>& supportedRequests): id(id), name(name) {
        supportedRequestTypes.clear();
        for(auto it:supportedRequests) {
            supportedRequestTypes.insert(it);
        }
    }

    BasicAuth* addBasicAuth(string username, string password) {
        if(authPairs.find(username) != authPairs.end()) {
            throw invalid_argument("Username already exists");
        }
        auto auth = new BasicAuth(username, password);
        authPairs[username] = password;
        return auth;
    }

    bool authenticate(BasicAuth* auth) {
        if(auth == nullptr) throw invalid_argument("No auth provided");

        if(authPairs.find(auth->username) == authPairs.end()) throw invalid_argument("No such auth username found");

        if(authPairs[auth->username] != auth->password) throw invalid_argument("Invalid auth password");

        return true;
    }

    void removeAuth(string username) {
        if(authPairs.find(username) == authPairs.end()) {
            throw invalid_argument("Username does not exist");
        }
        authPairs.erase(username);
    }

    string getName() {
        return name;
    }

    string getID() {
        return id;
    }

    void addSupportedRequestType(CommsType requestType) {
        supportedRequestTypes.insert(requestType);
    }

    void removeSupportedRequestType(CommsType requestType) {
        if(supportedRequestTypes.find(requestType) == supportedRequestTypes.end()) {
            throw invalid_argument("Request type not found");
        }
        supportedRequestTypes.erase(requestType);
    }

    virtual bool doesSupport(const CommsType requestType) {
        // for (auto it:supportedRequestTypes){
        //     cout<<it<<" ";
        // }
        // cout<<"\n";
        return supportedRequestTypes.find(requestType) != supportedRequestTypes.end();
    }

    virtual void processSMS(const CommsRequest* request) {
        throw logic_error("SMS is not supported in this provider");
    }

    virtual void processEmail(const CommsRequest* request) {
        throw logic_error("Email is not supported in this provider");
    }

    virtual void processSoundbox(const CommsRequest* request) {
        throw logic_error("Soundbox is not supported in this provider");
    }

    virtual double getSuccessPercent() = 0;

    virtual ~Provider() = default;
};

// class SMSHandler {
// public:
//     virtual void processSMS(const CommsRequest* request) = 0;
// };

// class EmailHandler {
// public:
//     virtual void processEmail(const CommsRequest* request) = 0;
// };

// class SoundboxHandler {
// public:
//     virtual void processSoundbox(const CommsRequest* request) = 0;
// };

class TwilioProvider: public Provider {
public:
    TwilioProvider(string id, string name, vector<CommsType>& supportedRequests): Provider(id, name, supportedRequests) {}

    virtual void processEmail(const CommsRequest* request) override {
        if(!authenticate(request->auth)) return;
        cout<<"Sending Email"<<"\n";
    }

    virtual void processSMS(const CommsRequest* request) override {
        if(!authenticate(request->auth)) return;
        cout<<"Sending SMS"<<"\n";
    }

    virtual double getSuccessPercent() {
        return 0.8;
    }
};

class AWSProvider: public Provider {
public:
    AWSProvider(string id, string name, vector<CommsType>& supportedRequests): Provider(id, name, supportedRequests) {}

    virtual void processEmail(const CommsRequest* request) override {
        if(!authenticate(request->auth)) return;
        cout<<"Sending Email"<<"\n";
    }

    virtual void processSoundbox(const CommsRequest* request) override {
        if(!authenticate(request->auth)) return;
        cout<<"Sending Soundbox"<<"\n";
    }

    virtual double getSuccessPercent() {
        return 0.99;
    }
};

struct CommsProvider {
    Provider* instance;
    bool isActive;
    BasicAuth* auth;
    CommsProvider(Provider* instance, BasicAuth* auth): instance(instance), isActive(true), auth(auth) {}
};

struct RoutingStrategy {
public:
    virtual Provider* getProviderForRequestType(unordered_map<string, CommsProvider*>& providers, CommsType requestType) = 0;
};

struct RandomRoutingStrategy: public RoutingStrategy {
public:
    virtual Provider* getProviderForRequestType(unordered_map<string, CommsProvider*>& providers, CommsType requestType) override {
        cout<<providers.size()<<"\n";
        for(auto& [providerId, provider]: providers){
            // cout<<providerId<<" "; 
            // cout<<"here";
            if(provider->instance->doesSupport(requestType)){
                return provider->instance;
            }
        }
        throw logic_error("No provider supports given request type");
    }
};

struct DynamicRoutingStrategy: public RoutingStrategy {
public:
    virtual Provider* getProviderForRequestType(unordered_map<string, CommsProvider*>& providers, CommsType requestType) override {
        double maxSuccessRate = 0;
        CommsProvider* p = nullptr;
        for(auto& [providerId, provider]: providers){
            if(provider->instance->doesSupport(requestType)) {
                if(provider->instance->getSuccessPercent() > maxSuccessRate) {
                    maxSuccessRate = provider->instance->getSuccessPercent();
                    p = provider;
                }
            }
        }
        if(p==nullptr) throw logic_error("No provider supports given request type");
        return p->instance;
    }
};

struct CommsMessages {
    static int ids;
    int id;
    CommsRequest* metadata;
    string providerId;
    CommsStatus status;

    CommsMessages(CommsRequest* request, string pID, CommsStatus status): id(ids++), metadata(request), providerId(pID), status     (status) {}

    void setStatus(CommsStatus s) {
        status = s;
    }
};

int CommsMessages::ids = 1;

class CommsSystem {
    unordered_map<string, CommsProvider*> providers;
    unordered_map<int, CommsMessages*> comms;
    RoutingStrategy* routingStrategy;
    CommsSystem() {
        providers.clear();
        RoutingStrategy* random = new RandomRoutingStrategy();
        routingStrategy = random;
    }
    CommsSystem(CommsSystem&) = delete;
    CommsSystem& operator=(CommsSystem&) = delete;
public:
    static CommsSystem& getInstance() {
        static CommsSystem instance;
        return instance;

    }

    void setRoutingStrategy(RoutingStrategy* rs) {
        routingStrategy = rs;
    }

    void addProvider(Provider* p, BasicAuth* auth) {
        if(providers.find(p->getID()) != providers.end()) {
            invalid_argument("Provider already present");
        }
        providers[p->getID()] = new CommsProvider(p, auth);
    }

    Provider* getProvider(string providerID) {
        if(providers.find(providerID) == providers.end()) {
            throw invalid_argument("Provider not registered");
        }
        return providers[providerID]->instance;
    }

    void updateState(string providerID, bool active) {
        if(providers.find(providerID) == providers.end()) {
            throw invalid_argument("Provider not registered");
        }
        providers[providerID]->isActive = active;
    }

    void updateProvider(Provider* p) {
        if(providers.find(p->getID()) == providers.end()) {
            throw invalid_argument("Provider not registered");
        }
        providers[p->getID()]->instance = p;
    }

    void processRequest(CommsRequest* request) {
        // for(auto [pid, p]: providers){
        //     cout<<pid<<" ";
        // }
        // cout<<"\n";
        auto p = routingStrategy->getProviderForRequestType(providers, request->requestType);
        auto provider = providers[p->getID()];
        request->auth = provider->auth;
        if(request->requestType == CommsType::EMAIL) {
            provider->instance->processEmail(request);
        } else if(request->requestType == CommsType::SMS) {
            provider->instance->processSMS(request);
        } else {
            provider->instance->processSoundbox(request);
        }

        CommsMessages* comms = new CommsMessages(request, provider->instance->getID(), CommsStatus::PROCESSING);
    }
};

int main() {
    try{
    CommsSystem& system = CommsSystem::getInstance();

    vector<CommsType> twilioTypes = {CommsType::EMAIL, CommsType::SMS};
    auto twilio = new TwilioProvider("twilio", "twilio", twilioTypes);

    vector<CommsType> awsTypes = {CommsType::EMAIL, CommsType::SOUNDBOX};
    auto aws = new AWSProvider("aws", "aws", awsTypes);

    auto twilioAuth = twilio->addBasicAuth("comms", "password");
    system.addProvider(twilio, twilioAuth);

    auto awsAuth = aws->addBasicAuth("aws", "password");
    system.addProvider(aws, awsAuth);

    unordered_map<string, string> payload = {
        {"from", "abc"},
        {"to", "def"},
        {"subject", "hello"},
        {"message", "msg"}
    };

    auto request = new CommsRequest(CommsType::EMAIL, CommsPriority::TRANSACTIONAL, payload);
    system.processRequest(request);
    } catch (const exception& e) {
        cout<<"Exception: "<<e.what() <<endl;
    }
}