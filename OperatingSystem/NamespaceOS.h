#ifndef NAMESPACEOS_H
#define NAMESPACEOS_H

#include "OpSystem.h"

#include <iostream>
#include <sstream>
#include <boost\filesystem\operations.hpp>
#include <boost\filesystem\path.hpp>

using namespace std;

namespace fs = boost::filesystem;

namespace OS {
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
            else if(temp == "!dir")
            {
                nvm_ls();
            }
            else if(temp == "!ls")
            {
                nvm_ls();
            }
            else if(temp == "!cd")
            {
                string directory;
                s >> directory;
                nvm_cd(directory);
            }
            else if(temp == "!pwd")
            {
                cout << cwd.string() << endl;
            }
            else if(temp == "cd")
            {
                string directory;
                s >> directory;
                cd(directory);
            }
            else if(temp == "md")
            {
                string directory;
                s >> directory;
                mkdir(directory);
            }
            else if(temp == "ls" || temp == "dir")
            {
                sys->ls();
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
                    
                    sys->nvm_load(hf.string(), hexFile);
                }
            }
            else if (temp == "run")
            {
                int pid;
                s >> pid;
                sys->run(pid);
            }
            else if (temp == "runall")
            {
                sys->runall();
            }
            else if (temp == "changeAlg")
            {
                int alg_no;
                s >> alg_no;
                sys->setAlgorithm(alg_no);
            }
            else if (temp == "priority")
            {
                int pid, priority;
                s >> pid >> priority;
                sys->setPriority(pid, priority);
            }
            else if (temp == "printAlg")
            {
                sys->printAlgorithm();
            }
            else if (temp == "ps")
            {
                sys->ps();
            }
			else if(temp == "mem")
			{
				int pid = 0;
				s >> pid;
				if(pid==0)
				{
					sys->mem();
				}
				else
					sys->mem(pid);
			}
			else if(temp == "free")
			{
				int pid = 0;
				s >> pid;
				if(pid==0)
				{
					sys->free();
				}
				else
					sys->free(pid); 
			}
            else
                cout <<"command not defined" << endl;
        }

        void help(){
            cout <<"help: prints a listing of commands and their descriptions.\n";
            cout <<"exit: exit the shell.\n";
            cout <<"ls: list the contents of a directory.\n";
            cout <<"cd: change working directory. cd (path)\n";
			cout <<"load: load a process into memory. load (process)\n";
			cout <<"run: run a process. run (process#)\n";
			cout <<"ps: shows the processes in memory\n";
			cout <<"mem: displays system memory list. mem (process#) displays process memory list\n";
			cout <<"free: displays the amount of system free memory. free (process#) displays free memory for process.\n";
            cout <<"changeAlg (1-4): Changes the scheduling algorithm\n";
            cout <<"printAlg: Displays the current scheduling algorithm\n";
            cout <<"priority: Sets the priority of a process. priority (process#) (priority)\n";
        }

        void nvm_ls(){
            string line;
            fs::directory_iterator dirItr( cwd );
            fs::directory_iterator endIter;
            for ( dirItr;dirItr != endIter;++dirItr )
            {
                cout << dirItr->path().filename() << endl;
            }
        }

        void cd(string cdDir){
            sys->chdir(cdDir);
        }

        void mkdir(string newDir) {
            sys->mkdir(newDir);
        }

        void nvm_cd(string cdDir){
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
            // perhaps should not always format; for now you can tell it not to
			sys->formatDisk();
			bool runShell = true;
            string line;
            while(runShell)
            {
                cout << endl << sys->pwd() << "/ # ";
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