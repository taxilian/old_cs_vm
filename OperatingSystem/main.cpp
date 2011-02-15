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
	//CPU utilization variable.
	ptime t2(microsec_clock::local_time());
	time_duration overallSystem = t2-t1;
	cout << "              Scheduling Statistical Data\n";
	int sysMicroseconds = overallSystem.total_microseconds();
	int runningMicroseconds = vm.runningTime.total_microseconds();
	double percent = (runningMicroseconds/sysMicroseconds) * 100;
	char temp = 0;
	cout <<"Overall system time: " <<sysMicroseconds <<  endl;
	cout <<"CPU running time: "  << runningMicroseconds << endl;
	cout <<"CPU percentage: " << percent << endl;
	cin.get(temp);
}