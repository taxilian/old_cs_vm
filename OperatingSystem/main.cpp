#include <iostream>
#include <string>
#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\filesystem\operations.hpp>
#include <boost\filesystem\path.hpp>
#include "NamespaceOS.h"
#include "VirtualMachine.h"
#include "VirtualDisk.h"
using namespace std;

int main()
{
    fs::path cwd(fs::initial_path<fs::path>());
	using namespace boost::posix_time;
	using namespace OS;
	ptime t1(microsec_clock::local_time());
	VM::VirtualMachine vm;
    fs::path df = cwd/"DISK";
	VM::VirtualDisk vd(df.string());
	OpSystem system(&vm,&vd);
	Shell shell(&system);
    shell.start();
}