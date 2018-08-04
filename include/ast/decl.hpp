#ifndef decl_hpp
#define decl_hpp

#include "base.hpp"

#include <cmath>
#include <sstream>

class Parameter : public Node
{
private:
    std::string type, id;
public:
    Parameter(const std::string &_type, const std::string &_id)
        : type(_type),
        id(_id)
    {}

    const std::string getId() const
    { return id; }

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        context.add_binding(type,id);
        if(context.get_current_register()<3){
            std::string reg = "a"+std::to_string(context.next_register());
            context.set_binding(id,reg,dst,0);
        } else {
            dst<<"\tlw\t$s0,"<<context.next_register()*4<<"($t1)\n";
            context.set_binding(id,"s0",dst,0);
        }
    }
};

class ParameterList : public Node
{
protected:
    NodePtr list, parameter;
public:
    ParameterList (NodePtr _list, NodePtr _parameter)
            : list(_list),
            parameter(_parameter)
        {}

    unsigned int get_num() const
    {
        const ParameterList* params = dynamic_cast<const ParameterList *>(list);
        unsigned int num = 1;
        if (list != nullptr) {
            num += params->get_num();
        }
        return num;
    }

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        const Parameter* param = dynamic_cast<const Parameter *>(parameter);
        // parameter list could contain one parameter
        if (list != nullptr) {
            list->generate_assembly(dst, context);
        }
        parameter->generate_assembly(dst, context);
        // paremter list cannot containg more than 4 parameters (assuming 4 bytes parameters)
        // if (context.get_binding(param->Parameter::getId()) > 12) {
        //     throw std::runtime_error("More than 4 parameters not supported");
        // }
    }
};



class List : public Node {
public:
    virtual void generate_assembly(std::ostream &dst, Context &context) const override //required by Node
    {
    }
    virtual void generate_assembly(std::ostream &dst, Context &context, const std::string &type) const =0;
};

class DeclList : public List
{
private:
    std::string id;
    NodePtr list, value;
public:
    DeclList(NodePtr _list, const std::string &_id, NodePtr _value)
        : list(_list),
        id(_id),
        value(_value)
    {}


    virtual void generate_assembly(std::ostream &dst, Context &context, const std::string &type) const override
    {
        context.add_binding(type, id);
        if (value != nullptr) {
            value->generate_assembly(dst, context);
            dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
            context.set_binding(id,"s0",dst,0);
        }
        if (list != nullptr) {
            const List* declarations = dynamic_cast<const List *>(list);
            declarations->generate_assembly(dst, context,type);
        }
    }
};


class VariableDecl : public Node
{
private:
    std::string type;
    NodePtr list;
public:
    VariableDecl(const std::string &_type, NodePtr _list)
        : type(_type),
        list(_list)
    {}


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        const List* declarations = dynamic_cast<const List *>(list);
        declarations->generate_assembly(dst, context,type);
    }
};

class InitParams : public Node
{
protected:
    NodePtr list, expr;
public:
    InitParams(NodePtr _list, NodePtr _expr)
            : list(_list),
            expr(_expr)
        {}

    virtual void generate_assembly(std::ostream &dst, Context &context,const int& mem,const int& type_size, int& current) const
    {
        if (list != nullptr) {
            const InitParams* params = dynamic_cast<const InitParams *>(list);
            params->generate_assembly(dst, context, mem,type_size, current);
            current++;
        }
        expr->generate_assembly(dst,context);
        dst<<"\tlw\t$s0"<<","<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\tsw\t$s0"<<","<<mem+type_size*current<<"($fp)"<<std::endl;
    }
};


class GlobalDeclList : public List
{
private:
    std::string id;
    NodePtr list;
public:
    GlobalDeclList(NodePtr _list, const std::string &_id)
        : list(_list),
        id(_id)
    {
        getGlobals().push_back(id);
    }

    virtual void generate_assembly(std::ostream &dst, Context &context, const std::string &type) const override
    {
        dst<<"\t.comm\t"<<id<<","<<context.get_size(type)<<","<<context.get_size(type)<<"\n";
        if (list != nullptr) {
            const List* declarations = dynamic_cast<const List *>(list);
            declarations->generate_assembly(dst, context,type);
        }
    }
};

class GlobalDeclList2 : public List
{
private:
    std::string id;
    NodePtr list;
    int value;
public:
    GlobalDeclList2(NodePtr _list, const std::string &_id, const int _value)
        : list(_list),
        id(_id),
        value(_value)
    {
        getGlobals().push_back(id);
    }

    virtual void generate_assembly(std::ostream &dst, Context &context, const std::string &type) const override
    {
        dst<<"\t.globl\t"<<id<<std::endl;
        if (context.is_first_global) {
            dst<<"\t.data"<<std::endl;
            context.is_first_global = false;
        }
        dst<<"\t.align\t2"<<std::endl;
        dst<<"\t.type\t"<<id<<", @object"<<std::endl;
        dst<<"\t.size\t"<<id<<", "<<context.get_size(type)<<std::endl;
        dst<<id<<":"<<std::endl;
        dst<<"\t.word\t"<<(int)value<<std::endl;
        if (list != nullptr) {
            const List* declarations = dynamic_cast<const List *>(list);
            declarations->generate_assembly(dst, context,type);
        }
    }
};


class GlobalInitParams : public Node
{
protected:
    NodePtr list;
    int value;
public:
    GlobalInitParams(NodePtr _list, int _value)
            : list(_list),
            value(_value)
        {}


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        if (list != nullptr) {
            const GlobalInitParams* params = dynamic_cast<const GlobalInitParams *>(list);
            params->generate_assembly(dst, context);
        }
        dst<<"\t.word\t"<<value<<std::endl;
    }
};

#endif
