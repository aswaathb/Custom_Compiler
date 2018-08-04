#ifndef statement_hpp
#define statement_hpp

#include "base.hpp"

#include <string>
#include <cmath>
#include <iostream>
#include <vector>

static std::vector<std::string> condTracker;
static std::vector<std::string> endTracker;
static std::vector<NodePtr> switchTracker;


class PrintStat : public Node
{
protected:
    NodePtr expr;
public:
    PrintStat(NodePtr _expr)
            : expr(_expr)
        {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        expr->generate_assembly(dst,context);
//MIPS code for printf

      // 	lw	$2,%got(y)($28)
      // 	nop
      // 	lw	$2,0($2)
      // 	nop
      // 	move	$5,$2
      // 	lw	$2,%got($LC0)($28)
      // 	nop
      // 	addiu	$4,$2,%lo($LC0)
      // 	lw	$2,%call16(printf)($28)
      // 	nop
      // 	move	$25,$2
      // 	.reloc	1f,R_MIPS_JALR,printf
      // 1:	jalr	$25
      // 	nop
      //  lw	$28,16($fp)
      // 	nop

        dst<<"\tlw\t$2,%got("<<context.get_current_mem()<<")($fp)\n"
           <<"\tnop\n"<<"\tlw\t$2,0($2)\n"<<"\tnop\n\tmove\t$5,$2\n"
           <<"\tlw\t$2,%got($LC0)($28)\n"<<"\tnop\n"
        	 <<"\taddiu\t$4,$2,%lo($LC0)\n"<<"\tlw\t$2,%call16(printf)($28)\n"
        	 <<"\tnop\n"<<"\tmove\t$25,$2\n"<<"\t.reloc\t1f,R_MIPS_JALR,printf\n"
           <<"1:\tjalr\t$25\n"<<"\tnop\n"<<"\tlw\t$28,16($fp)\n"<<"\tnop\n";

    }
};

class CompoundStat : public Node
{
protected:
    NodePtr seq;
public:
    CompoundStat(NodePtr _seq)
            : seq(_seq)
        {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        if (seq != nullptr) { //compound statement could be empty
            seq->generate_assembly(dst,context);
        }
    }
};

class Sequence : public Node
{
protected:
    NodePtr sequence_nest, next;
public:
    Sequence(NodePtr _sequence_nest, NodePtr _next)
            : sequence_nest(_sequence_nest),
            next(_next)
        {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        if (sequence_nest != nullptr) { //sequence could be only 1 statement
            sequence_nest->generate_assembly(dst,context);
        }
        next->generate_assembly(dst,context);
    }
};

class Stat : public Node
{
protected:
    NodePtr expr;
public:
    Stat(NodePtr _expr)
            : expr(_expr)
        {}

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        expr->generate_assembly(dst,context);
        // context.reset_mem();
    }
};

class ifStat : public Node
{
protected:
    NodePtr condition, sequence;
    std::string endLabel;
public:
    static int ifCounter;
    ifStat(NodePtr _condition, NodePtr _sequence)
        : condition(_condition),
        sequence(_sequence)
    {  endLabel = makeIfLabel();  }

    std::string makeIfLabel(){
        return "$IL"+std::to_string(ifCounter++);
    }

    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        condition->generate_assembly(dst,context);
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\tbeq\t$s0,$0,"<<endLabel;
        dst<<std::endl<<"\tnop"<<std::endl;
        sequence->generate_assembly(dst,context);
        dst<<endLabel<<":"<<std::endl;
    }
};

class ifElseStat : public Node
{
protected:
    NodePtr condition, ifSequence, elseSequence;
    std::string elseLabel, endLabel;
public:
    static int ifElseCounter;
    ifElseStat(NodePtr _condition, NodePtr _ifSequence, NodePtr _elseSequence)
            : condition(_condition),
            ifSequence(_ifSequence),
            elseSequence(_elseSequence)
        {
            elseLabel = makeIfElseLabel();
            endLabel = makeIfElseLabel();
        }

    std::string makeIfElseLabel()
    {
        return "$IEL"+std::to_string(ifElseCounter++);
    }


    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        condition->generate_assembly(dst,context);
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\tbeq\t$s0,$0,"<<elseLabel;
        dst<<std::endl<<"\tnop"<<std::endl;
        ifSequence->generate_assembly(dst,context);
        dst<<"\tbeq\t$0,$0,"<<endLabel;
        dst<<std::endl<<"\tnop"<<std::endl;
        dst<<elseLabel<<":"<<std::endl;
        elseSequence->generate_assembly(dst,context);
        dst<<endLabel<<":"<<std::endl;
    }

};

class whileStat : public Node
{
protected:
    NodePtr condition, sequence;
    std::string seqLabel, condLabel, endLabel;
public:
    static int whileCounter;
    std::string makeWhileLabel()
    {
        return "$WL"+std::to_string(whileCounter++);
    }
    whileStat(NodePtr _condition, NodePtr _sequence)
            : condition(_condition),
            sequence(_sequence)
        {
            seqLabel = makeWhileLabel();
            condLabel = makeWhileLabel();
            endLabel = makeWhileLabel();
            condTracker.push_back(condLabel);
            endTracker.push_back(endLabel);
        }



    virtual void generate_assembly(std::ostream &dst, Context &context) const override
    {
        dst<<"\tb\t"<<condLabel<<std::endl;
        dst<<"\tnop\n";
        dst<<seqLabel<<":"<<std::endl;
        sequence->generate_assembly(dst,context);
        dst<<condLabel<<":"<<std::endl;
        condition->generate_assembly(dst,context);
        dst<<"\tlw\t$s0,"<<context.get_current_mem()<<"($fp)"<<std::endl;
        dst<<"\tbne\t$s0,$0,"<<seqLabel;
        dst<<std::endl<<"\tnop"<<std::endl;
        dst<<endLabel<<":"<<std::endl;
        condTracker.erase(condTracker.begin());
        endTracker.erase(endTracker.begin());
    }
};


#endif
