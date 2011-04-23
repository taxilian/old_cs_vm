#pragma once
#ifndef OpSystem_h__
#define OpSystem_h__

#include <string>
#include <list>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include "ProcessControlBlock.h"
#include "VMCore.h"
#include "HeapMgr.h"
#include "VirtualDisk.h"
#include "FileSystem.h"
#include "VMemMngr.h"
namespace OS {
    class ProcScheduler;
    class OpSystem : boost::noncopyable 
    {
    public:
		OpSystem(VM::VMCore* vm, VM::VirtualDisk* vd);
        ~OpSystem(void);

        void nvm_load(const std::string& pathfileName,const std::string& name);
        int getNextPid() { return m_lastPid++; }
        void ps() const;
        void load(const std::string& pathfileName);
        void run(int pid);
        void runall();
        void setAlgorithm( int alg_no );
        void setPriority( int pid, int priority );
        void printAlgorithm();
		void mem();
		void mem(int pid);
		void free();
		void free(int pid);

		//Disk management stuff
        void nvm_loadToFile( const std::string& nvmFname, const std::string fname );
		void nvm_loadTxtFile( const std::string& nvmFname, const std::string fname );
        void cat( std::string fname );
		void head(std::string fname, int n);
		void tail(std::string fname, int n);
		void cp(std::string fname, std::string location);
		void mv(std::string fname, std::string location);
		void formatDisk();
        int chdir(const std::string& dir);
        void mkdir(const std::string& dir);
        void ls();
		void rm(const std::string& name);
		void df();
		void du();
		void touch(std::string fname, std::string date);
		void ln(std::string fname, std::string lname, std::string sym);
        // Gets current working directory
        std::string pwd();

		//interrupts

		void processNew(VM::VMCore* vm);
		void processFree(VM::VMCore* vm);
		void processYield(VM::VMCore* vm);
        void processEnd(VM::VMCore* vm);

        void fsOpen(VM::VMCore* vm);
        void fsClose(VM::VMCore* vm);
        void fsSeek(VM::VMCore* vm);
        void fsTell(VM::VMCore* vm);
        void fsWrite(VM::VMCore* vm);
        void fsRead(VM::VMCore* vm);

        void runScheduler(VM::VMCore* vm);

    protected:
        // General utilites, used mainly by the scheduler
        void saveContext();
        void freeProcess(const ProcessControlBlockPtr& proc);

    protected:
        ProcessControlBlockPtr getProcess(int pid);
        template <class CONT>
        void getPCBList(CONT& container) {
            for (std::list<ProcessControlBlockPtr>::const_iterator it = m_processList.begin();
                it != m_processList.end(); ++it) {
                container.push_back(*it);
            }
        }
        bool fileOpen( const std::string& filename );
    private:
        static const int stackSize = 4096;
		static const int heapSize = 1024;
        uint32_t m_lastLoadAddr;
        VM::VMCore* m_vm;
		FileSystem fileSystem;
        std::list<ProcessControlBlockPtr> m_processList;
        int m_lastPid;
		HeapMgr sysMemMgr;
        boost::scoped_ptr<ProcScheduler> m_scheduler;
        int m_processCount;
        friend class ProcScheduler;
        int cwd;
        int lastFH;
        std::map<int, FileHandlePtr> fileTable;
		VMemMngr vmemMngr;
		
    };
	
}
#endif // OpSystem_h__