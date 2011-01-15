#include <iostream>
#include <string>
#include "NamespaceOS.h"
using namespace std;

int main()
{
	using namespace OS;
	bool runShell = true;
	string line;
	while(runShell)
	{
		cout << "#";
		getline(cin, line);
		if(line == "exit")
		{
			runShell = false;
		}
		else
		{
			
			Shell::validate(line);
		}
	}
	system("PAUSE");
}