#include <iostream>
#include <string>
#include "NamespaceOS.h"
using namespace std;

int main()
{
	using namespace OS;
	OS::Shell shell;
    shell.start();
}