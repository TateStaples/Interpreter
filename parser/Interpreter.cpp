//
// Created by Staples Tate on 4/20/20.
//
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cctype>
#include <sstream>
#include <unistd.h>

using namespace std;

void print(const string& s = "", const string& end= "\n") {
    cout << s + end;
}

int count(string s, char c) {
    int val = 0;
    for (char c2 : s) {
        if (c2 == c) val++;
    }
    return val;
}

bool contains(const string& s, const string& check) {
    long index = s.find_first_of(check);
    return index != string::npos;
}

int index(string s, string key, int startIndex = 0, int stopIndex = -1) {
    //if (stopIndex == -1) stopIndex = s.length();
    s = s.substr(startIndex, stopIndex);
    if (contains(s, key)) return s.find(key)+startIndex;
    return -1;
}

string trim(string s) {  // removes spaces before and after
    while (s.back() == ' ' || s.front() == '\n') {s.pop_back();}
    while (s.front() == ' ') {s = s.substr(1, s.length());}
    return s;
}

map<string, string> set_to_custom;
map<string, string> custom_to_set;
vector<string> operators;
bool chain_activated = false;

class SourceFile {
public:
    fstream file;
    vector<string> lines;
    int lineIndex = 1;
    int fileLength = 0;

    explicit SourceFile(const string& filePath) {
        file.open(filePath);
        lines = loadFile(&file);
        fileLength = lines.size();
        lines.max_size();
        setUp();
    }

    static vector<string> loadFile(fstream *f) {
        vector<string> lines;
        string line;
        while(f->is_open() && !f->eof()) {
            getline(*f, line);
            lines.push_back(line);
        }
        return lines;
    }

    static void sortOperators() {
        int len = operators.size();
        int i2;
        for (int i=1; i<len; i++) {
            i2 = i;
            while (i2 > 0 && operators[i2].length() > operators[i2-1].length()) {
                string s1 = operators[i2];
                string s2 = operators[i2-1];
                operators[i2] = s2;
                operators[i2-1] = s1;
                i2--;
            }
        }
    }

    void setUp() {
        string keywordPath = getLine(0);
        fstream setUpFile;
        setUpFile.open(keywordPath);
        string line;
        string set;
        string custom;
        bool op = true;
        while(!setUpFile.eof() && setUpFile.is_open()) {
            getline(setUpFile, line);
            if (line.length() > 0 && line[0] != '~') {
                int splitIndex = line.find_first_of('#');
                custom = line.substr(0, splitIndex);
                custom = trim(custom);
                if (custom.length() == 0) { continue;}
                set = line.substr(splitIndex+1, line.length());
                set = trim(set);
                set_to_custom[set] = custom;
                custom_to_set[custom] = set;
                if (op) operators.push_back(custom);
                if (set == "in") op = false;
            }
        }
        setUpFile.close();
        sortOperators();
    }

    static string removeComments(string line) {
        string comment = set_to_custom["comment"];
        int i = index(line, comment);
        if (i != -1) {
            line = line.substr(0, i);
        }
        return line;
    }

    string getLine(int line=-1) {
        if (line == -1) line = lineIndex;
        return lines[line];
    }

    static int countIndents(string s) {
        int val = 0;
        while(s.front() == '\t' || s.front() == ' ') {
            if (s.front() == '\t') val += 4;
            else val++;
            s = s.substr(1, s.length());
        }
        return val/4;
    }

    static vector<string> split(const string& str, char delimiter=' ') {
        vector<string> internal;
        stringstream ss(str); // Turn the string into a stream.
        string tok;
        while(getline(ss, tok, delimiter)) {
            if (tok.length() > 0)
                internal.push_back(tok);
        }
        return internal;
    }

    static vector<string> getWords(string line) {
        line = removeSyntax(line);
        // split on groups, commas, and spaces
        int grouperCounter = 0;
        char start = ' ';
        char end = ' ';
        bool super = false;
        string placeholder;
        vector<string> final;
        vector<string> spaced = split(line);
        for (string section : spaced) {
            if (!super) {
                // check for groups
                if (count(section, set_to_custom["parameters"].front())) {
                    start = set_to_custom["parameters"].front();
                    end = set_to_custom["parameters"].back();
                    grouperCounter += count(section, start);
                    grouperCounter -= count(section, end);
                    super = grouperCounter > 0;
                    int i = index(section, *new string(1, start));
                    string before = section.substr(0, i);
                    for (string result : getWords(before)) final.push_back(result);
                    if (super)
                        placeholder = section.substr(i, -1);
                    else {
                        int i2 = index(section, *new string(1, end));
                        string main = section.substr(i, i2+1);
                        final.push_back(section.substr(i, i2+1));
                        string after = section.substr(i2+1, -1);
                        for (string result : getWords(after)) final.push_back(result);
                    }
                    continue;
                }
                if (count(section, set_to_custom["list"].front())) {
                    start = set_to_custom["list"].front();
                    end = set_to_custom["list"].back();
                    grouperCounter += count(section, start);
                    grouperCounter -= count(section, end);
                    super = grouperCounter > 0;
                    int i = index(section, *new string(1, start));
                    string before = section.substr(0, i);
                    for (string result : getWords(before)) final.push_back(result);
                    if (super)
                        placeholder = section.substr(i, -1);
                    else {
                        int i2 = index(section, *new string(1, end));
                        final.push_back(section.substr(i, i2+1));
                        string after = section.substr(i2+1, -1);
                        for (string result : getWords(after)) final.push_back(result);
                    }
                    continue;
                }
                if (count(section, set_to_custom["map"].front())) {
                    start = set_to_custom["map"].front();
                    end = set_to_custom["map"].back();
                    grouperCounter += count(section, start);
                    grouperCounter -= count(section, end);
                    super = grouperCounter > 0;
                    int i = index(section, *new string(1, start));
                    string before = section.substr(0, i);
                    for (string result : getWords(before)) final.push_back(result);
                    if (super)
                        placeholder = section.substr(i, -1);
                    else {
                        int i2 = index(section, *new string(1, end));
                        final.push_back(section.substr(i, i2+1));
                        string after = section.substr(i2+1, -1);
                        for (string result : getWords(after)) final.push_back(result);
                    }
                    continue;
                }
                if (count(section, set_to_custom["string"].front())) {
                    start = set_to_custom["string"].front();
                    end = set_to_custom["string"].back();
                    grouperCounter += count(section, start);
                    grouperCounter -= count(section.substr(1, -1), end);
                    super = grouperCounter > 0;
                    int i = index(section, *new string(1, start));
                    string before = section.substr(0, i);
                    for (string result : getWords(before)) final.push_back(result);
                    if (super)
                        placeholder = section.substr(i, -1);
                    else {
                        int i2 = index(section, *new string(1, end), i+1);
                        string main = section.substr(i, i2+1);
                        final.push_back(main);
                        string after = section.substr(i2+1, -1);
                        for (string result : getWords(after)) final.push_back(result);
                    }
                    continue;
                }
                // todo add other groups
                for (string part : splitOperators(section)) {
                    final.push_back(part);
                }
            }
            else {
                section = ' ' + section;
                grouperCounter += count(section, start);
                grouperCounter -= count(section, end);
                if (start == end && count(section, start)) grouperCounter = 0;
                if (grouperCounter <= 0) {
                    int i = section.find_last_of(end);
                    string before = section.substr(0, i+1);
                    //string after = section.substr(i+1, -1);
                    if (!before.empty()) final.push_back(placeholder + before);
                } else {
                    placeholder += section;
                }
            }
            // remove syntax
        }
//        for (string word : final) print("word2: " + word);
        return final;
    }

    static string removeSyntax(string s) {
        string s2;
        string syntax = ",;:";
        for (char c : s) if (!count(syntax, c)) s2 += c; else s += ' ';
        return s2;
    }

    static vector<string> splitOperators(string section) {
        vector<string> terms;
        section = trim(section);
        if (section.length() == 0) return terms;
        if (find(operators.begin(), operators.end(), section) != operators.end()) {
            terms.push_back(section);
            return terms;
        }
        bool og = true;
        for (string op : operators) {
            int i = index(section, op);
            //todo add special cases (. for decimals and - for negs)
            if (i != -1) {
                og = false;
                string s1 = section.substr(0, i);
                string s2 = section.substr(i+op.length(), -1);
                for (const string& result : splitOperators(s1)) terms.push_back(result);
                terms.push_back(op);
                for (const string& result : splitOperators(s2)) terms.push_back(result);
            }
        }
        if (og) terms.push_back(section);
        return terms;
    }

    bool isOver() {return lineIndex >= fileLength;}

    vector<string> parseLine(string s) {
        s = removeComments(s);
        return getWords(s);
    }

    vector<string> parseNext() {
        vector<string> v = parseLine(getLine());
        lineIndex++;
        return v;
    }

    void shutdown() {
        file.close();
    }
};

class Object {
public:
    static Object None;
    string name = "";
    virtual string toString() {if (name == "") return set_to_custom["null"]; return name;}
    map<string, Object*> dict;
    Object() = default;
    explicit Object (string s) {name = s;}
    virtual string type() {string s = typeid(*this).name(); return s.substr(1, s.length());}
    virtual ~Object() = default;
    virtual double getDouble(){return 0.0;};
    virtual vector<Object*>* getArray (){return nullptr;}
    virtual bool getBool() {return false;}

    virtual Object* activate(Object* o1, Object* o2) {return &None;}
    virtual Object* __and__(Object* o1, Object* o2) {return &None;}
    virtual Object* __or__(Object* o1, Object* o2) {return &None;}
    virtual Object* __add__(Object* o1, Object* o2) {return &None;}
    virtual Object* __sub__(Object* o1, Object* o2) {return &None;}
    virtual Object* __mul__(Object* o1, Object* o2) {return &None;}
    virtual Object* __div__(Object* o1, Object* o2) {return &None;}
    virtual Object* __mod__(Object* o1, Object* o2) {return &None;}
    Object* __get_attribute__(Object* o1, Object* o2) {return o1->dict[o2->toString()];}
    virtual Object* __equal__(Object* o1, Object* o2) {return &None;}
    virtual Object* __not_equal__(Object* o1, Object* o2) {
        return o1->__not__(o1->__equal__(o1, o2));
    }
    virtual Object* __greater_than__(Object* o1, Object* o2) {return &None;}
    virtual Object* __less_than__(Object* o1, Object* o2) {return &None;}
    virtual Object* __greater_than_or_equal_to___(Object* o1, Object* o2) {
        Object g = *o1->__greater_than__(o1, o2);
        Object e = *o1->__equal__(o1, o2);
        return g.__or__(&g, &e);
    }
    virtual Object* __less_than_or_equal_to__(Object* o1, Object* o2) {
        Object g = *o1->__less_than__(o1,o2);
        Object e = *o1->__equal__(o1,o2);
        return g.__or__(&g,&e);
    }
    virtual Object* __in__(Object* o1, Object* o2) {return &None;}
    virtual Object* __not__(Object* o1) {return &None;}

    virtual Object* __call__(Object* parameter) {return &None;}
};
Object Object::None;

map<string, Object*> variables;
typedef Object* (*functiontype2) (Object*);

class Word : public Object {
public:
    string val;
    explicit Word(string s){val = s;}
    string toString() override {return val;}
    Object* __add__(Object* o1, Object* o2) override {return new Word(o1->toString() + o2->toString());}
};
class Truth : public Object{
public:
    bool val;
    explicit Truth(bool b) : val(b) {val = b;}
    string toString() override {if(val) return set_to_custom["True"]; return set_to_custom["False"];}
    bool getBool() override {return val;}
    Object* __and__(Object* o1, Object* o2) override {
        return new Truth(o1->getBool() && o2->getBool());
    }
    Object* __or__(Object* o1, Object* o2) override {
        return new Truth(o1->getBool() || o2->getBool());
    }
    Object* __not__(Object* o1) override {return new Truth(!val);}
};
class Number : public Object {
public:
    double val;
    Number(){val = 0.0;}
    explicit Number(double num){val = num;}
    double getDouble() override {return val;}

    Object* __add__ (Object* o1, Object* o2) override {return new Number(o1->getDouble() + o2->getDouble());}
    Object* __sub__(Object* o1, Object* o2) override {return new Number(o1->getDouble() - o2->getDouble());}
    Object* __mul__(Object* o1, Object* o2) override {return new Number(o1->getDouble() * o2->getDouble());}
    Object* __div__(Object* o1, Object* o2) override {return new Number(o1->getDouble() / o2->getDouble());}
    Object* __mod__(Object* o1, Object* o2) override {return new Number(int (o1->getDouble()) % int (o2->getDouble()) + 0.0);}
    Object* __equal__(Object* o1, Object* o2) override {return new Truth(o1->getDouble() == o2->getDouble());}
    Object* __greater_than__(Object* o1, Object* o2) override {return new Truth(o1->getDouble() > o2->getDouble());}
    Object* __less_than__(Object* o1, Object* o2) override {return new Truth(o1->getDouble() < o2->getDouble());}
    string toString() override {
        string s = to_string(val);
        int i = s.size() - 1;
        while (s[i] == '0' || s[i] == '.') {i--; if (s[i] == '.') {i--; break;}}
        if (i > 0) return s.substr(0, i+1);
        return s.substr(0, 1);
    }
};
class Iterable : public Object{
public:
    string borders;
    vector<Object*> val;
    virtual string toString() override {
        if (borders == "") {
            string t = this->type();
            transform(t.begin(), t.end(), t.begin(), ::tolower);
            borders = set_to_custom[t];
        }
        string s;
        s += borders.front();
        for (Object* o : val) {
            s += o->toString();
            if (o != val.back()) s += ", ";
        }
        s += borders.back();
        return s;
    }
    vector<Object*>* getArray() override {return &val;}
    Object* __in__(Object* o1, Object* o2) override {
        vector<Object*>* array = o1->getArray();
        if (array != nullptr && find(array->begin(), array->end(), o2) != array->end()) {
            return new Truth(true);
        }
        return new Truth(false);
    }
};
class List : public Iterable{
public:
    explicit List(vector<Object*> l) {val = l;}
    List() = default;
};
class Map: public Iterable {
public:
    Map(vector<pair<Object, Object>> stuff){}
};
class Parameters: public List {
public:
    string type() override {string s = typeid(*this).name(); return s.substr(2, s.length());}
    explicit Parameters(vector<Object*> p){val = p;}
};
class Function: public Object {
public:
    bool builtin = false;
    int* activator;
    functiontype2 func;
    vector<string> argNames;
    int start=0;
    Function(int s, int* activation) {
        start = s;
        activator = activation;
    }
    Function(Object* (*f)(Object*)) {
        builtin = true;
        func = f;
    }
    //Function(Object (*f)Parameters);
    Object* __call__(Object* args) override {
        if (builtin)
            return func(args);
        return activate(args);
    }
    string toString() override {return "a function";}

    Object *activate(Object *parameters);
};
class Operator : public Object {
public:
    string code;
    double priority = 1;
    explicit Operator(string key) {
        code = trim(key);
        setUp();
    }

    double getDouble() override{return priority;}

    void setUp() {
        if (code == set_to_custom["addition"])
            priority = 2;
        else if (code == set_to_custom["subtraction"])
            priority = 2;
        else if (code == set_to_custom["mod"])
            priority = 3;
        else if (code == set_to_custom["division"])
            priority = 3;
        else if (code == set_to_custom["multiplication"])
            priority = 3;
        else if (code == set_to_custom["assignment"])
            priority = 0;
        else if (code == set_to_custom["attribute"])
            priority = 6;
    }

    Object* activate(Object* o1, Object* o2) override {
        if (o1->type() == "Parameters")
            o1 = o1->getArray()->front();
        if (o2->type() == "Parameters")
            o2 = o2->getArray()->front();
        if (code == set_to_custom["addition"])
            return o1->__add__(o1, o2);
        if (code == set_to_custom["subtraction"])
            return o1->__sub__(o1, o2);
        if (code == set_to_custom["mod"])
            return o1->__mod__(o1, o2);
        if (code == set_to_custom["division"])
            return o1->__div__(o1, o2);
        if (code == set_to_custom["multiplication"])
            return o1->__mul__(o1, o2);
        if (code == set_to_custom["assignment"]) {
            if (o1->type() == "Object") {
                variables[o1->name] = o2;
            } else {
                o1 = o2;
            }
            return o2;
        }
        if (code == set_to_custom["attribute"])
            return o1->__get_attribute__(o1, o2);
        if (code == set_to_custom["equal"])
            return o1->__equal__(o1, o2);
        if (code == set_to_custom["not_equal"])
            return o1->__not_equal__(o1, o2);
        if (code == set_to_custom["greater_than"])
            return o1->__greater_than__(o1, o2);
        if (code == set_to_custom["less_than"])
            return o1->__less_than__(o1, o2);
        if (code == set_to_custom["greater_than_or_equal_to"])
            return o1->__greater_than_or_equal_to___(o1, o2);
        if (code == set_to_custom["less_than_or_equal_to"])
            return o1->__less_than_or_equal_to__(o1, o2);
        if (code == set_to_custom["and"])
            return o1->__and__(o1, o2);
        if (code == set_to_custom["or"])
            return o1->__or__(o1, o2);
        if (code == set_to_custom["in"])
            return o1->__in__(o1, o2);
        return &None;
    }

    string toString() override {
        return code;
    }
};
class Metaclass : public Object {};

class CodeBlock {
public:
    // todo: local variables
    int active = 1;
    int startLine;
    int terminationLine{};
    explicit CodeBlock(int s) {startLine = s;}
    bool firstTime = true;
    virtual bool shouldEnter(vector<Object*>) = 0;
    virtual void onEnter() {firstTime = false;}
    virtual Object* onTermination() { return nullptr;};
    virtual bool shouldRemove() {return true;}
    string toString() {return typeid(*this).name();}
};
class ConditionalStatement : public CodeBlock {
public:
    explicit ConditionalStatement(int i) : CodeBlock(i) {}

    void onEnter() override {
        firstTime = false;
        chain_activated = true;
    }
};
class ifStatement : public ConditionalStatement {
public:
    explicit ifStatement(int i) : ConditionalStatement(i) {}
    bool shouldEnter(vector<Object*> args) override {
        assert(args.size() == 1);
        bool check = args.front()->getBool();
        bool answer = firstTime && check;
        if (firstTime & !check && chain_activated)
            chain_activated = false;
        return answer;
    }
};
class elifStatement : public ifStatement {
public:
    explicit elifStatement(int i) : ifStatement(i) {}
    bool shouldEnter(vector<Object*> args) override {
        assert(args.size() == 1);
        bool check = args.front()->getBool();
        return firstTime && check && !chain_activated;
    }
};
class elseStatement : public ConditionalStatement {
public:
    explicit elseStatement(int i) : ConditionalStatement(i) {}
    bool shouldEnter(vector<Object*>) override {return firstTime && !chain_activated;}
};
class Loop : public CodeBlock {
public:
    static Loop* activeLoop;
    explicit Loop(int i) : CodeBlock(i) {activeLoop = this;}
};
Loop* Loop::activeLoop = nullptr;
class forLoop : public Loop {
public:
    int index = 0;
    string arg;
    bool shouldEnter(vector<Object*> stuff) override {
        if (firstTime) arg = stuff[0]->name;
        vector<Object*>* iter = stuff[2]->getArray();
        bool enter = index < iter->size();
        if (enter) {
            variables[arg] = iter->at(index);
        }
        return enter;
    }

    void onEnter() override {
        firstTime = false;
        index++;
    }

    forLoop(int i) : Loop(i) {}
};
class whileLoop : public Loop {
public:
    explicit whileLoop(int i) : Loop(i) {}
    bool shouldEnter(vector<Object *> args) override {
        assert(args.size() == 1);
        return args.front()->getBool();
    }
};
class Declaration : public CodeBlock {
public:
    explicit Declaration(int i) : CodeBlock(i) {
        active = 0;
    }

    bool shouldRemove() {return false;}

};
class functionDeclaration : public Declaration {
public:
    Function* func;

    explicit functionDeclaration(int i) : Declaration(i) {
        func = new Function(i+1, &active);
    }

    bool shouldEnter(vector<Object*> args) override {
        string name = args.front()->name;
        variables[name] = func;
        for (Object* parameterName : *args.back()->getArray())
            func->argNames.push_back(parameterName->name);
        return false;
    }

    Object* onTermination() override {return &Object::None;}
};
class classDeclaration : public Declaration {};

struct Builtins {
    static Object* print(Object* parameters) {
        for (Object* term : *parameters->getArray()) {
            cout << term->toString() + " ";
        }
        cout << endl;
        return &Object::None;
    }
    static Object* input(Object* parameters) {
        string response;
        getline(cin, response);
        return new Word(response);
    }
    static Object* range(Object* parameters) {
        vector<Object*>* args = parameters->getArray();
        Object* num1 = args->at(0);
        Object* num2 = args->at(1);
        vector<Object*> results;
        while (num1->getDouble() > num2->getDouble()) {
            results.push_back(num1);
            num1 = new Number(num1->getDouble()+1);
        }
        return new List(results);
    }
};

class Interpreter {
    const static bool debug = false;
public:
    map<int, vector<CodeBlock*>> terminations;
    SourceFile* sourceCode;
    static Interpreter* active;

    explicit Interpreter(const string &filePath) {
        active = this;
        sourceCode = new SourceFile (filePath);
        addBuiltins();
    }

    Object* runProgram() {
        while(!sourceCode->isOver()) {
            if (debug) sleep(1);
            if (debug) print("Line " + to_string(sourceCode->lineIndex) + " = " + sourceCode->getLine(sourceCode->lineIndex));
            CodeBlock* c = nullptr;
            if (terminations.count(sourceCode->lineIndex)) {
                c = terminations[sourceCode->lineIndex].back();
                if (c->active) {
                    Object *endOrders = c->onTermination();
                    if (endOrders != nullptr) { return endOrders; }
                    //terminations[sourceCode->lineIndex].pop_back();
                    sourceCode->lineIndex = c->startLine;
                } else c = nullptr;
            }
            vector<string> words = sourceCode->parseNext();
            if (debug) for (string word : words) print("word: " + word);
            if (words.empty()) continue;
            string firstWord = words[0];
            if (firstWord == set_to_custom["return"]) {
                words.erase(words.begin(), words.begin()+1);
                vector<Object*> terms = findTerms(words);
                vector<Object*> results = operate(terms);
                if (results.size() > 0)
                    return results.front();
                return &Object::None;
            }
            if (firstWord == set_to_custom["break"]) {
                c = getCurrentLoop();
                sourceCode->lineIndex = c->terminationLine;
                terminations[c->terminationLine].pop_back();
                if (terminations[c->terminationLine].empty())
                    terminations.erase(c->terminationLine);
                continue;
            }
            if (firstWord == set_to_custom["continue"]) {
                c = getCurrentLoop();
                sourceCode->lineIndex = c->terminationLine;
                continue;
            }
            if (c == nullptr) c = getCodeSource(firstWord);
            if (c == nullptr) {
                vector<Object*> terms = findTerms(words);
                operate(terms);
            }
            else {
                runCodeBlock(c, words);
            }
        }
        sourceCode->shutdown();
        return &Object::None;
    }

    void runCodeBlock(CodeBlock* c, vector<string> words) {
        int terminationLine = getTerminationLine();
        if (debug) print("terminationLine of " + sourceCode->getLine(sourceCode->lineIndex-1) + ": " + to_string(terminationLine));
        c->terminationLine = terminationLine;
        words.erase(words.begin(), words.begin()+1);
        vector<Object*> terms = findTerms(words);
        vector<Object*> results = operate(terms);
        if (terminations.count(terminationLine)) {
            if (terminations[terminationLine].back() != c) terminations[terminationLine].push_back(c);
        }
        else {
            vector<CodeBlock*> codes;
            codes.push_back(c);
            terminations[terminationLine] = codes;
        }
        if (c->shouldEnter(results)) {
            c->onEnter();
        }
        else {
            if (debug) print("Didn't enter: " + c->toString());
            sourceCode->lineIndex = terminationLine;
            if (terminations.count(terminationLine) && c->shouldRemove()) {
                terminations[terminationLine].pop_back();
                if (terminations[terminationLine].empty()) {
                    terminations.erase(terminationLine);
                }
            }
        }
    }

    static CodeBlock* getCurrentLoop() {return Loop::activeLoop;}

    int getTerminationLine() const {
        int line = sourceCode->lineIndex-1;
        int baseIndent = SourceFile::countIndents(sourceCode->getLine(line));
        int indent;
        do {
            line++;
            indent = SourceFile::countIndents(sourceCode->getLine(line));
        } while (indent > baseIndent);
        return line;
    }

    static void addBuiltins() {
        variables["print"] = new Function(Builtins::print);
        variables["input"] = new Function(Builtins::input);
        variables["range"] = new Function(Builtins::range);
    }

    CodeBlock* getCodeSource(const string& s) const {
        if (s == set_to_custom["for"]) return new forLoop(sourceCode->lineIndex-1);
        if (s == set_to_custom["while"]) return new whileLoop(sourceCode->lineIndex-1);
        if (s == set_to_custom["if"]) return new ifStatement(sourceCode->lineIndex-1);
        if (s == set_to_custom["elif"]) return new elifStatement(sourceCode->lineIndex-1);
        if (s == set_to_custom["else"]) return new elseStatement(sourceCode->lineIndex-1);
        if (s == set_to_custom["declare_function"]) return new functionDeclaration(sourceCode->lineIndex-1);
        return nullptr;
    }

    vector<Object*> decode(const string& s) {
        return findTerms(SourceFile::getWords(s));
    }

    vector<Object*> findTerms(vector<string> words) {
        int numberOfTerms = words.size();
        vector<Object*> terms;
        Object *o;
        for (int i = 0; i<numberOfTerms; i++) {
            if (i < numberOfTerms-1 && words[i+1] == set_to_custom["assignment"])
                o = new Object(words[i]);
            else
                o = objectify(words[i]);
            terms.push_back(o);
        }
        return terms;
    }

    Object* objectify(string s) {
        Object* o;
        //groups
        if (s.front() == set_to_custom["list"].front()) {
            o  = new List(operate(decode(s.substr(1, s.length()-2))));
            return o;
        }
        if (s.front() == set_to_custom["map"].front()) {
            //o = new Map(decode(s.substr(1, s.length()-1)));
            //return o;
        }
        if (s.front() == set_to_custom["string"].front()) {
            o = new Word(s.substr(1, s.length()-2));
            return o;
        }
        if (s.front() == set_to_custom["parameters"].front()) {
            o  = new Parameters(operate(decode(s.substr(1, s.length()-2))));
            return o;
        }
        // nums
        if (isdigit(s.front())) {
            o  = new Number(stod(s));
            return o;
        }
        //bools
        if (s == set_to_custom["True"]) {
            o = new Truth(true);
            return o;
        }
        if (s == set_to_custom["False"]) {
            o = new Truth(false);
            return o;
        }
        // variables
        if (variables.count(s)) {
            return variables[s];
        }
        if (custom_to_set.count(s)) {
            o = new Operator(s);
            return o;
        }
        return new Object(s);
    }

    static vector<Object*> operate(vector<Object*> terms) {
        Object* o;
        vector<Object*> actions;
        vector<int> priorities;
        vector<int> indices;
        for (int i = 0; i < terms.size(); i++) {
            Object* term = terms[i];
            string t = term->type();
            if (t == "Function") {
                actions.push_back(term);
                indices.push_back(i);
                priorities.push_back(5);
            } else if (t == "Operator") {
                actions.push_back(term);
                indices.push_back(i);
                priorities.push_back(term->getDouble());
            }
        }
        while (actions.size() > 0) {
            double max = -1;
            int topIndex = 0;
            for (int i=0; i < priorities.size(); i++) {
                if (priorities[i] > max) {
                    max = priorities[i];
                    topIndex = i;
                }
            }
            int i = indices[topIndex];
            o = actions[topIndex];
            actions.erase(actions.begin()+topIndex, actions.begin()+topIndex+1);
            priorities.erase(priorities.begin()+topIndex, priorities.begin()+topIndex+1);
            indices.erase(indices.begin()+topIndex, indices.begin()+topIndex+1);
            if (o->type() == "Function") {
                Object* p = terms[i+1];
                if (p->type() !="Parameters") {
                    continue;
                }
                Object* result = o->__call__(p);
                terms[i] = result;
                terms.erase(terms.begin()+i+1, terms.begin()+i+2);
                for (int index=0; index < indices.size(); index++) {
                    if (indices[index] > i) {
                        indices[index] = indices[index] - 1;
                    }
                }
            }
            else {
                Object* o1 = terms[i-1];
                Object* o2 = terms[i+1];
                if (debug) print(o1->toString() + " does " + o->toString() + " to " + o2->toString());
                Object* result = o->activate(o1, o2);
                if (debug) print("result: " + result->toString());
                terms[i-1] = result;
                terms.erase(terms.begin()+i, terms.begin()+i+2);
                for (int index=0; index < indices.size(); index++) {
                    if (indices[index] > i) {
                        indices[index] = indices[index] - 2;
                    }
                }
            }

        }
        return terms;
        // todo: add keywords like loops and conditionals
    }
};
Interpreter* Interpreter::active = nullptr;

Object* Function::activate(Object* p) {
    *activator += 1;
    int initialLine = Interpreter::active->sourceCode->lineIndex;
    Interpreter::active->sourceCode->lineIndex = start;
    vector<Object*> args = *p->getArray();
    for (int i=0; i<args.size(); i++)
        variables[argNames[i]] = args[i];
    Object* answer = Interpreter::active->runProgram();
    Interpreter::active->sourceCode->lineIndex = initialLine;
    *activator -= 1;
    return answer;
}


int main() {
    string s = "text_documents/thingy.txt";
    Interpreter program(s);
    program.runProgram();
}


//todo: local variables
//todo: spacing
//todo: classes
//todo: iterables
//todo: for loops