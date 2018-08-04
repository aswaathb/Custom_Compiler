#ifndef operations_hpp
#define operations_hpp

#include "base.hpp"

#include <string>
#include <cmath>
#include <iostream>

class Operator : public Node
{
protected:
    NodePtr left;
    NodePtr right;

    Operator(NodePtr _left, NodePtr _right)
        : left(_left)
        , right(_right)
    {}
public:
    virtual const char *getOpcode() const =0;
    virtual const char *getOp() const {
        throw std::runtime_error("getOp() not implemented");
    };

    NodePtr getLeft() const
    { return left; }

    NodePtr getRight() const
    { return right; }


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        left->generate_assembly(dst,context);
        int res = context.get_current_mem();
        right->generate_assembly(dst,context);
        dst<<"\tlw\t$s1,"<<res<<"($fp)"<<std::endl;
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\t"<<getOp()<<"\t$s2,$s1,$s0"<<std::endl;
        dst<<"\tsw\t$s2,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};

class AddOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "+"; }
    virtual const char *getOp() const override
    { return "addu"; }
public:
    AddOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}
};

class SubOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "-"; }
    virtual const char *getOp() const override
    { return "sub"; }
public:
    SubOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}
};


class MulOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "*"; }
    virtual const char *getOp() const override
    { return "mul"; }
public:
    MulOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}
};

class DivOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "/"; }
    virtual const char *getOp() const override
    { return "div"; }
public:
    DivOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        left->generate_assembly(dst,context);
        int res = context.get_current_mem();
        right->generate_assembly(dst,context);
        dst<<"\tlw\t$s1,"<<res<<"($fp)"<<std::endl;
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\t"<<getOp()<<"\t$0,$s1,$s0"<<std::endl;
        dst<<"\tteq\t$s0,$0,7"<<std::endl; //trap with code 7 if denominator is eqaul to zero
        dst<<"\tmflo\t$s0"<<std::endl;
        dst<<"\tsw\t$s0,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};



class EqualsOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "=="; }
public:
    EqualsOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        left->generate_assembly(dst,context);
        int res = context.get_current_mem();
        right->generate_assembly(dst,context);
        dst<<"\tlw\t$s1,"<<res<<"($fp)"<<std::endl;
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\txor\t$s0,$s1,$s0"<<std::endl;
        dst<<"\tsltu\t$s0,$s0,1"<<std::endl;
        dst<<"\tandi\t$s0,$s0,0x00ff"<<std::endl;
        dst<<"\tsw\t$s0,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};


class LessOp : public Operator
{
protected:
    virtual const char *getOpcode() const override
    { return "<"; }
public:
    LessOp(NodePtr _left, NodePtr _right)
        : Operator(_left, _right)
    {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        left->generate_assembly(dst,context);
        int res = context.get_current_mem();
        right->generate_assembly(dst,context);
        dst<<"\tlw\t$s1,"<<res<<"($fp)"<<std::endl;
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\tsltu\t$s0,$s1,$s0"<<std::endl;
        dst<<"\tandi\t$s0,$s0,0x00ff"<<std::endl;
        dst<<"\tsw\t$s0,"<<context.next_mem()<<"($fp)"<<std::endl;
    }
};



class AssignOp : public Node
{
protected:
    std::string id;
    NodePtr offset, right;

    virtual const std::string getOpcode() const
    { return ":="; }

public:
    AssignOp(std::string &_id, NodePtr _offset, NodePtr _right)
        : id(_id),
        offset(_offset),
        right(_right)
    {}


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {   right->generate_assembly(dst, context);
        dst<<"\tlw\t$s3,"<<context.get_current_mem()<<"($fp)"<<std::endl;


        if (offset != nullptr) {
            offset->generate_assembly(dst, context);
            dst<<"\tlw\t$s1,"<<context.get_current_mem()<<"($fp)"<<std::endl;
            dst<<"\tli\t$s2,"<<context.get_size(context.get_arr_type(id))<<std::endl;

            dst<<"\tmul\t$s1,$s1,$s2"<<std::endl;
            context.load_array(id, "t0",dst);
            dst<<"\tsw\t$s3,($t0)"<<std::endl;
        }

        else context.set_binding(id, "s3", dst, 0);
    }
};

#endif
