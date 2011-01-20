#ifndef NAMESPACEOS_H
#define NAMESPACEOS_H

#include "OpSystem.h"

#include <iostream>
#include <sstream>
#include <boost\filesystem\operations.hpp>
#include <boost\filesystem\path.hpp>

using namespace std;

namespace fs = boost::filesystem;

namespace OS{
    class Shell
    {
    public:
        Shell(OpSystem* os) : sys(os), cwd(fs::initial_path<fs::path>() ) {}
        ~Shell() {}

        void validate(string command)
        {
            istringstream s(command);
            string temp;
            s >> temp;
            if(temp == "help")
            {
                help();
            }
            else if(temp == "dir")
            {
                ls();
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
                {
                    fs::path hf = cwd/hexFile;
                    
                    sys->load(hf.string());
                }
            }
            else if (temp == "run")
            {
                int pid;
                s >> pid;
                sys->run(pid);
            }
            else if (temp == "ps")
            {
                sys->ps();
            }
            else
                cout <<"command not defined" << endl;
        }

        void help(){
            cout <<"help: prints a listing of commands and their descriptions.\n";
            cout <<"exit: exit the shell.\n";
            cout <<"ls: list the contents of a directory.\n";
            cout <<"cd: change working directory. cd (path)\n";
        }

        void ls(){
            string line;
            fs::directory_iterator dirItr( cwd );
            fs::directory_iterator endIter;
            for ( dirItr;dirItr != endIter;++dirItr )
            {
                cout << dirItr->path().filename() << endl;
            }
        }

        void cd(string cdDir){
            cwd = fs::system_complete( fs::path( cdDir ) );
            if ( !fs::exists( cwd ) )
            {
                cout << "Path not found" << endl;
                cout << "cd (path)" << endl;
                cwd = fs::initial_path<fs::path>();
            }
            else if ( !fs::is_directory( cwd ) )
            {
                cout << "Not a directory" << endl;
                cout << "cd (path)" << endl;
                cwd = fs::initial_path<fs::path>();
            }
            else
                fs::current_path(cwd);
        }

        void start()
        {
            bool runShell = true;
            string line;
            while(runShell)
            {
                cout <<"\n" << cwd.string() + " # ";
                getline(cin, line);
                if(line == "exit")
                {
                    runShell = false;
                }
                else
                {
                    try {
                        Shell::validate(line);
                    } catch (const std::exception &ex) {
                        cout << "Error executing command: " << ex.what() << endl;
                    }
                }
            }
        }
    private:
        OpSystem *sys;
        // cwd is a standard acronymn for "Current Working Directory"
        // it seemed easier to understand to me
        fs::path cwd;
    };

}
#endif