#pragma once
#ifndef Interrupts_h
#define Interrupts_h
#include <boost/noncopyable.hpp>
namespace VM{
class Interrupts : boost::noncopyable
{
public:
	virtual int sysNew(int size) = 0;
	virtual void sysDelete(int addr) = 0;
	virtual void yield() = 0;
};
}
#endif