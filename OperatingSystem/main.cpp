#include <iostream>
#include <string>
#include "NamespaceOS.h"
#include "VirtualMachine.h"
using namespace std;

int main()
{
	using namespace OS;
	VM::VirtualMachine vm;
	OpSystem system(&vm);
	Shell shell(&system);
    shell.start();
}