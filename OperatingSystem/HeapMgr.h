#ifndef HEAPMGR_H
#define HEAPMGR_H 
#include <list>
#include <iostream>
namespace OS{
	struct blkInfo{int blkID; bool free; int size; int address;};
	class HeapMgr
	{
	public:
		HeapMgr();
		HeapMgr(int _totalMem, int baseAddrs);
		void setMem(int _totalMem, int baseAddrs);
		void display();
		int freeMemAmount();
		int allocate(int size);
		void de_allocate(int addr);
	private:
		int totalMem;
		int freeMem;
		int nxtID;
		std::list<blkInfo> blocks;
		std::list<blkInfo>::iterator blkIter;
		blkInfo initial;
	};

}
#endif