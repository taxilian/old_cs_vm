#ifndef NAMESPACEOS_H
#define NAMESPACEOS_H
#include <iostream>
#include <sstream>
#include <boost\filesystem\operations.hpp>
#include <boost\filesystem\path.hpp>
using namespace std;
namespace OS{
	namespace fs = boost::filesystem;
	fs::path fullPath(fs::initial_path<fs::path>() );
	class System
	{
	public:
		static void load(string fileName)
		{
			string line;
			ifstream in(fileName);
			if(in)
			{
				while(getline(in,line))
				{//should load process to vm memory here.
					cout << line << endl;
				}
			}
			else
				cout <<"Unable to open file.\n" << endl;
		}
	};
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
			else if(temp == "cd")
			{
				string directory;
				s >> directory;
				cd(directory);
			}
			else if(temp == "load")
			{
				string hexFile;
				s >> hexFile;
				if(hexFile == "")
				{
					cout << "must be of form: load (process)" << endl;
				}
				else
					System::load(hexFile);
			}
			else
				cout <<"command not defined" << endl;
		}
		static void help(){
		cout <<"help: prints a listing of commands and their descriptions.\n";
		cout <<"exit: exit the shell.\n";
		cout <<"ls: list the contents of a directory.\n";
		cout <<"cd: change working directory. cd (path)\n";
		}
		static void ls(){
			string line;
			fs::directory_iterator dirItr( fullPath );
			fs::directory_iterator endIter;
			for ( dirItr;dirItr != endIter;++dirItr )
			{
				cout << dirItr->path().filename() << endl;
			}
		}
		static void cd(string cdDir){
			fullPath = fs::system_complete( fs::path( cdDir ) );
			if ( !fs::exists( fullPath ) )
			{
				cout << "Path not found" << endl;
				cout << "cd (path)" << endl;
				fullPath = fs::initial_path<fs::path>();
			}
			else if ( !fs::is_directory( fullPath ) )
			{
				cout << "Not a directory" << endl;
				cout << "cd (path)" << endl;
				fullPath = fs::initial_path<fs::path>();
			}
			else
				fs::current_path(fullPath);
		}
	};
	void startShell()
	{
		bool runShell = true;
		string line;
		while(runShell)
		{
			cout <<"\n" << fullPath.string() + "#";
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
	}
	

}
#endif