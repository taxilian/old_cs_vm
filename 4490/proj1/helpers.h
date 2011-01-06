

#pragma once
#ifndef H_VM_HELPERS_RB_
#define H_VM_HELPERS_RB_

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "VMConfig.h"
//typedef char REGISTER;
typedef unsigned short ADDRESS;
typedef int IMMEDIATE;
typedef unsigned int REGISTER;

typedef boost::function<void (const instructionBlock&)> CallInstructionFunctor;

class VirtualMachine;

template<class F>
struct method_wrapper_ADDR
{
    typedef void result_type;
    F f;
    method_wrapper_ADDR(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(in.uint16_param2);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(ADDRESS))
{
    return boost::bind(method_wrapper_ADDR<void (VirtualMachine::*)(ADDRESS)>(function), instance, _1);
}

template<class F>
struct method_wrapper_REG_ADDR
{
    typedef void result_type;
    F f;
    method_wrapper_REG_ADDR(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(VM->reg[in.uint8_param], in.uint16_param2);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, ADDRESS))
{
    return boost::bind(method_wrapper_REG_ADDR<void (VirtualMachine::*)(REGISTER&, ADDRESS)>(function), instance, _1);
}

template<class F>
struct method_wrapper_REG
{
    typedef void result_type;
    F f;
    method_wrapper_REG(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(VM->reg[in.uint8_param]);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&))
{
    return boost::bind(method_wrapper_REG<void (VirtualMachine::*)(REGISTER&)>(function), instance, _1);
}

template<class F>
struct method_wrapper_REG_IMM
{
    typedef void result_type;
    F f;
    method_wrapper_REG_IMM(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(VM->reg[in.uint8_param], in.uint16_param2);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, IMMEDIATE))
{
    return boost::bind(method_wrapper_REG_ADDR<void (VirtualMachine::*)(REGISTER&, IMMEDIATE)>(function), instance, _1);
}

template<class F>
struct method_wrapper_IMM
{
    typedef void result_type;
    F f;
    method_wrapper_IMM(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)((int)in.uint16_param2);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(IMMEDIATE))
{
    return boost::bind(method_wrapper_IMM<void (VirtualMachine::*)(IMMEDIATE)>(function), instance, _1);
}

template<class F>
struct method_wrapper_REG_REG
{
    typedef void result_type;
    F f;
    method_wrapper_REG_REG(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(VM->reg[in.uint8_param], VM->reg[in.uint8_param2]);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, REGISTER&))
{
    return boost::bind(method_wrapper_REG_REG<void (VirtualMachine::*)(REGISTER&, REGISTER&)>(function), instance, _1);
}

template<class F>
struct method_wrapper_REG_REG_REG
{
    typedef void result_type;
    F f;
    method_wrapper_REG_REG_REG(F f) : f(f) {}
    void operator()(VirtualMachine* VM, const instructionBlock& in)
    {
        return (VM->*f)(VM->reg[in.uint8_param], VM->reg[in.uint8_param2], VM->reg[in.uint8_param3]);
    }
};
inline CallInstructionFunctor
make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, REGISTER&, REGISTER&))
{
    return boost::bind(method_wrapper_REG_REG_REG<void (VirtualMachine::*)(REGISTER&, REGISTER&, REGISTER&)>(function), instance, _1);
}

#endif
