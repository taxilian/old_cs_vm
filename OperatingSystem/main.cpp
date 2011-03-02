#include <iostream>
#include <string>
#include <boost\date_time\posix_time\posix_time.hpp>
#include "NamespaceOS.h"
#include "VirtualMachine.h"
#include "VirtualDisk.h"
using namespace std;

int main()
{
	using namespace boost::posix_time;
	using namespace OS;
	ptime t1(microsec_clock::local_time());
	VM::VirtualMachine vm;
	VM::VirtualDisk vd("DISK.txt");
	OpSystem system(&vm,&vd);
	Shell shell(&system);
    shell.start();
}