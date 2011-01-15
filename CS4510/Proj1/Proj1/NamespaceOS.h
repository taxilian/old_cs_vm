#ifndef NAMESPACEOS_H
#define NAMESPACEOS_H
#include <iostream>
#include <sstream>
#include <boost\filesystem\operations.hpp>
#include <boost\filesystem\path.hpp>
using namespace std;
namespace OS{
	static class Shell
	{
	public:
		static void validate(string command)
		{
			istringstream s(command);
			string temp;
			s >> temp;
			if(temp == "help")
			{
				help();
			}
			else if(temp == "ls")
			{
				ls();
			}
			else
				cout <<"command not defined" << endl;
		}
		static void help(){
		cout <<"help: prints a listing of commands and their descriptions.\n";
		cout <<"ls: list the contents of a directory.\n";
		}
		static void ls(){
			string line;
			namespace fs = boost::filesystem;
			fs::path fullPath(fs::initial_path<fs::path>() );
			fs::directory_iterator dirItr( fullPath );
			fs::directory_iterator endIter;
			for ( dirItr;dirItr != endIter;++dirItr )
			{
				cout << dirItr->path().filename() << endl;
			}
		}
	};
	

}
#endif