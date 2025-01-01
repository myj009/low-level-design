#include <string>
#include <vector>
#include <iostream>
using namespace std;

class Editor {
    string text;
    int start;
    int end;
public:
    Editor(string txt): text(txt), start(-1), end(-1) {}

    void doSelection(int s, int e) {
        start = s;
        end = e;
    }

    string getSelectedText() {
        return text.substr(start, end-start);
    }

    string removeSelectedText() {
        string txt = text.substr(start, end-start);
        text = text.substr(0, start) + text.substr(end);
        start=-1, end=-1;
        return txt;
    }

    void pasteSelectedText(string txt) {
        text = text.substr(0, start) + txt + text.substr(start);
    }

    void print() {
        cout<<text<<"\n";
    }

};

class Application {
    vector<Editor*> editors;
    Editor* curEditor;
    string clipboard;

public:
    void addEditor(Editor* e) {
        editors.push_back(e);
    }

    void setEditor(Editor* e) {
        curEditor = e;
    }

    void copyToClipboard(string txt) {
        clipboard = txt;
    }

    string getClipboardText() {
        return clipboard;
    }

};

class Command {
protected:
    Editor* editor;
    Application* app;
public:
    Command(Editor* e): editor(e) {}
    virtual bool execute() = 0;
    virtual ~Command() = default;
};

class CopyText: public Command {
public:
    CopyText(Editor* e): Command(e) {}
    virtual bool execute() {
        string txt = editor->getSelectedText();
        app->copyToClipboard(txt);
        return false;
    }
};

class CutText: public Command {
public:
    CutText(Editor* e): Command(e) {}
    virtual bool execute() {
        string txt = editor->removeSelectedText();
        app->copyToClipboard(txt);
        return true;
    }
};

class PasteText: public Command {
public:
    PasteText(Editor* e): Command(e) {}
    virtual bool execute() {
        editor->pasteSelectedText(app->getClipboardText());
        return true;
    }
};

class Button {
    Command* command;
public:
    Button(Command* c): command(c) {}
    void click() {
        if(command->execute()) {
            
        }
    };

    void setCommand(Command* c) {
        command = c;
    }

    virtual void draw() = 0;
    virtual ~Button() = default;
};

class CopyButton: public Button {
public:
    CopyButton(Command* c): Button(c) {} 

    virtual void draw() override {
        cout<<"Drawing copy button\n";
    }
};

class CutButton: public Button {
public:
    CutButton(Command* c): Button(c) {}
    virtual void draw() override {
        cout<<"Drawing cut button\n";
    }
};

class PasteButton: public Button {
public:
    PasteButton(Command* c): Button(c) {}
    virtual void draw() override {
        cout<<"Drawing paste button\n";
    }
};

int main() {

}