#pragma once
#ifndef Interrupts_h
#define Interrupts_h
#include <boost/noncopyable.hpp>
namespace VM{
class Interrupts : boost::noncopyable
{
public:
	virtual int sysNew(int size) = 0; 
};
}
#endif