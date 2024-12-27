#include <string>
#include <iostream>
#include <algorithm>
using namespace std;

class TextProcessor {
public:
    virtual string process(string txt) = 0;
};

class SimpleTextProcessor: public TextProcessor {
public:
    virtual string process(string txt) {
        return txt;
    }
};

class TextProcessorDecorator: public TextProcessor {
public:
    TextProcessor* tp;
    TextProcessorDecorator(TextProcessor* tpp): tp(tpp) {}
};

class LowerCaseDecorator: public TextProcessorDecorator {
public:
    LowerCaseDecorator(TextProcessor* tpp): TextProcessorDecorator(tpp) {}
    virtual string process(string txt) {
        string newTxt = tp->process(txt);
        for(auto& c: newTxt){
            c = tolower(c);
        }
        return newTxt;
    }
};

class UnderscoreDecorator: public TextProcessorDecorator {
    public:
    UnderscoreDecorator(TextProcessor* tpp): TextProcessorDecorator(tpp) {}
    virtual string process(string txt) {
        string newTxt = tp->process(txt);
        for(auto &c:newTxt) {
            if(c==' ') c='_';
        }
        return newTxt;
    }
};

int main() {
    TextProcessor* tp = new SimpleTextProcessor();
    tp = new LowerCaseDecorator(tp);
    tp = new UnderscoreDecorator(tp);

    cout<<tp->process("Hello World")<<"\n";
    
}