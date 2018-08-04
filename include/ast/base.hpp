#ifndef base_hpp
#define base_hpp

#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <sstream>

#include <memory>
#include <regex>

class Node;
class Context;

typedef const Node *NodePtr;
static const std::regex reNum("^-?[0-9]+$");
static const std::regex reId("^[a-z][a-z0-9]*$");

static int NameUnq=0;
static std::vector<std::string> FnTracker;

static std::string make_name(std::string base)
{
    return "$"+base+std::to_string(NameUnq++);
}

class Node
{
public:
    static std::vector<std::string>& getGlobals()  { static std::vector<std::string> globals; return globals; }
    static std::vector<std::string>& getGlobalsArray()  { static std::vector<std::string> globals; return globals; }
    static std::stringstream& getGlobalDec()  { static std::stringstream global; return global; }
    static bool& getRData()  { static bool has_r; return has_r; }
    virtual ~Node()
    {}

    //! Generate the mips code to the given stream
    virtual void generate_assembly(std::ostream &dst, Context &context) const
    { throw std::runtime_error("Not implemented yet"); }
};




class Context {
private:
    unsigned int _size = 52;
    int current_mem = -4;
    int current_register = -1;
    std::unordered_map<std::string,unsigned int> bindings;
    std::unordered_map<std::string,std::string> types;
    std::unordered_map<std::string,std::pair<std::string,unsigned int> > Arrtypes;
    std::unordered_map<std::string,unsigned int> functions;
    std::vector<std::string> declarations;
    Context* parent;
public:
    bool is_first_global = true;
    bool is_first_global_ptr = true;
    bool is_first_text = true;

    Context(Context* _parent)
        : parent(_parent)
    {
        if(_parent != NULL) current_mem = (*_parent).mem_init();
    }

    unsigned int set_binding(std::string key, std::string reg, std::ostream &dst, int offset) {
        int address = get_binding(key);
        if (address < 0) { //global
            dst<<"\tla\t$t0,"<<key<<std::endl;
            dst<<"\tsw\t$"<<reg<<",0($t0)"<<std::endl;
        } else {
            dst<<"\tsw\t$"<<reg<<","<<get_binding(key)+offset<<"($fp)"<<std::endl;
        }
    }

    void load_binding(std::string key, std::string reg, std::ostream &dst, int offset) {
        int address = get_binding(key);
        if (address < 0) { //global
            dst<<"\tla\t$t0,"<<key<<std::endl;
            dst<<"\tlw\t$"<<reg<<","<<offset<<"($t0)"<<std::endl;
        } else {
            dst<<"\tlw\t$"<<reg<<","<<get_binding(key)+offset<<"($fp)"<<std::endl;
        }
    }

    void load_array(std::string key, std::string reg, std::ostream &dst) {
        int address = get_binding(key);
        if (address < 0) { //global
            dst<<"\tlw\t$s2,%got("<<key<<")($28)"<<std::endl;
            dst<<"\taddu\t$"<<reg<<",$s1,$s2"<<std::endl;
        } else {
            dst<<"\tli\t$s2,"<<get_binding(key)<<std::endl;
            dst<<"\taddu\t$"<<reg<<",$s1,$s2"<<std::endl;
            dst<<"\taddu\t$"<<reg<<",$fp,$t0"<<std::endl;
        }
    }

    void add_declaration(std::string id) {
        declarations.push_back(id);
    }

    int get_binding(std::string key) {
        std::unordered_map<std::string,unsigned int>::iterator it = bindings.find(key);

        if (it == bindings.end()) {
            if (parent != nullptr) return parent->get_binding(key);
            else if (std::find(Node::getGlobals().begin(), Node::getGlobals().end(),key) != Node::getGlobals().end()) return -1;
            else if (std::find(Node::getGlobalsArray().begin(), Node::getGlobalsArray().end(),key) != Node::getGlobalsArray().end()) return -1;
            else throw std::runtime_error("error: '" + key + "' undeclared");
        } else return it->second;
    }

    std::string get_type(std::string key) const {
        auto it = types.find(key);

        if (it == types.end()) {
            if (parent != nullptr) return parent->get_type(key);
            else throw std::runtime_error("error: '" + key + "' undeclared");
        } else return it->second;
    }

    std::string get_arr_type(std::string key) {
        std::unordered_map<std::string,std::pair<std::string,unsigned int> >::iterator it = Arrtypes.find(key);

        if (it == Arrtypes.end()) {
            if (parent != nullptr) return parent->get_arr_type(key);
            else throw std::runtime_error("error: '" + key + "' undeclared");
        } else return it->second.first; //get the type within the pair
    }

    unsigned int get_size_bind(std::string const& key) const {
        unsigned int size = get_size(get_type(key));
        if (size != 0) return size;
        else return get_arr_size(key);
    }

    unsigned int get_size (std::string const& type) const {
        if (type == "int") return 4;
        if (type == "short") return 2;
        if (type == "char") return 1;
        else return 0;
    }

    unsigned int get_arr_size (std::string const& key) const {
        auto it = Arrtypes.find(key);

        if (it == Arrtypes.end()) {
            if (parent != nullptr) return parent->get_arr_size(key);
            else throw std::runtime_error("error: '" + key + "' undeclared");
        } else return it->second.second; //get the size within the pair
    }

    unsigned int get_function(std::string key) {
        if (parent != nullptr) parent->get_function(key);
    }

    void add_binding(std::string type, std::string key) {

        std::unordered_map<std::string,unsigned int>::iterator it = bindings.find(key);

        if (it == bindings.end()) {
            bindings[key] = _size;
            _size += 4;

            types[key] = type;
        } else throw std::runtime_error("redefinition of '"+key+"'");
    }

    void add_arr_binding(std::string type, std::string key, unsigned int size) {

        std::unordered_map<std::string,unsigned int>::iterator it = bindings.find(key);

        if (it == bindings.end()) {
            bindings[key] = _size;
            _size += size*get_size(type);

            Arrtypes[key] = std::make_pair(type, size);
        } else throw std::runtime_error("redefinition of '"+key+"'");
    }

    void add_arr_type(std::string type, std::string key, unsigned int size) {
        Arrtypes[key] = std::make_pair(type, size);
    }

    void add_function(std::string key, unsigned int param_num) {

        std::unordered_map<std::string,unsigned int>::iterator it = functions.find(key);
        auto dec = std::find(declarations.begin(), declarations.end(), key);

        if (it == functions.end() || dec != declarations.end()) {
            functions[key] = param_num;
            if (dec != declarations.end()) {
                declarations.erase(dec);
            }
        } else throw std::runtime_error("conflicting types for ‘"+key+"’");
    }

    unsigned int size() {
        return _size+current_mem+4;
    }

    int next_register() {
        current_register++;
        return current_register;
    }

    int get_current_register() {
        return current_register;
    }

    int reset_last_register(){
        current_register--;
        return current_register;
    }

    void reset_registers() {
        current_register = -1;
    }

    int next_mem() {
        current_mem = current_mem + 4;
        return _size+current_mem;
    }

    int get_current_mem() {
        return _size+current_mem;
    }

    int mem_init() {
        return current_mem;
    }

    void reset_mem() {
        current_mem = 0;
    }

    void set_mem(int mem){
        current_mem = mem;
    }
};


class Variable : public Node
{
private:
    std::string id;
public:
    Variable(const std::string &_id)
        : id(_id)
    {}

    const std::string getId() const
    { return id; }


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        context.load_binding(id,"s0",dst,0);
        dst << "\tsw\t$s0,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};

class Number : public Node
{
private:
    int value;
public:
    Number(int _value)
        : value(_value)
    {}

    int getValue() const
    { return value; }


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        dst<<"\tli\t$s0,"<<value<<std::endl;
        dst<<"\tsw\t$s0,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};


class ExprList : public Node
{
protected:
    NodePtr expr_list, expr;
public:
    ExprList(NodePtr _expr_list, NodePtr _expr)
            : expr_list(_expr_list),
            expr(_expr)
        {}
};





#endif
