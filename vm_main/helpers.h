#pragma once
#ifndef H_VM_HELPERS_RB_
#define H_VM_HELPERS_RB_

#include "VMCore.h"
#include "VMConfig.h"
//typedef char REGISTER;

namespace VM {

    class VirtualMachine;

    template<class F>
    struct method_wrapper_RAW
    {
        typedef void result_type;
        F f;
        method_wrapper_RAW(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)();
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)())
    {
        instructionDef def = { PT_RAW, boost::bind(method_wrapper_RAW<void (VirtualMachine::*)()>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_ADDR
    {
        typedef void result_type;
        F f;
        method_wrapper_ADDR(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)(in.uint32_param2);
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(ADDRESS))
    {
        instructionDef def = { PT_ADDR, boost::bind(method_wrapper_ADDR<void (VirtualMachine::*)(ADDRESS)>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_REG_ADDR
    {
        typedef void result_type;
        F f;
        method_wrapper_REG_ADDR(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)(VM->reg[in.uint16_param], in.uint32_param2);
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, ADDRESS))
    {
        instructionDef def = { PT_REGADDR, boost::bind(method_wrapper_REG_ADDR<void (VirtualMachine::*)(REGISTER&, ADDRESS)>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_REG
    {
        typedef void result_type;
        F f;
        method_wrapper_REG(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)(VM->reg[in.uint16_param]);
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&))
    {
        instructionDef def = { PT_REG, boost::bind(method_wrapper_REG<void (VirtualMachine::*)(REGISTER&)>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_REG_IMM
    {
        typedef void result_type;
        F f;
        method_wrapper_REG_IMM(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)(VM->reg[in.uint16_param], static_cast<short>(in.uint32_param2));
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, IMMEDIATE))
    {
        instructionDef def = { PT_REGIMMEDIATE, boost::bind(method_wrapper_REG_IMM<void (VirtualMachine::*)(REGISTER&, IMMEDIATE)>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_IMM
    {
        typedef void result_type;
        F f;
        method_wrapper_IMM(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
            return (VM->*f)((int)in.uint32_param2);
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(IMMEDIATE))
    {
        instructionDef def = { PT_IMMEDIATE, boost::bind(method_wrapper_IMM<void (VirtualMachine::*)(IMMEDIATE)>(function), instance, _1) };
        return def;
    }

    template<class F>
    struct method_wrapper_REG_REG
    {
        typedef void result_type;
        F f;
        method_wrapper_REG_REG(F f) : f(f) {}
        void operator()(VirtualMachine* VM, const instructionBlock& in)
        {
#define REG_OR_PC(param) (param == 0x7F ? VM->pc : VM->reg[param])
            return (VM->*f)(REG_OR_PC(in.uint16_param), REG_OR_PC(in.uint16_param2));
#undef REG_OR_PC
        }
    };
    inline instructionDef
        make_method(VirtualMachine* instance, void (VirtualMachine::*function)(REGISTER&, REGISTER&))
    {
        instructionDef def = { PT_REGREG, boost::bind(method_wrapper_REG_REG<void (VirtualMachine::*)(REGISTER&, REGISTER&)>(function), instance, _1) };
        return def;
    }

}

#endif
