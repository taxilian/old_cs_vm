#include <iostream>
#include <string>
#include <boost\date_time\posix_time\posix_time.hpp>
#include "NamespaceOS.h"
#include "VirtualMachine.h"
using namespace std;

int main()
{
	using namespace boost::posix_time;
	using namespace OS;
	ptime t1(microsec_clock::local_time());
	VM::VirtualMachine vm;
	OpSystem system(&vm);
	Shell shell(&system);
    shell.start();
}