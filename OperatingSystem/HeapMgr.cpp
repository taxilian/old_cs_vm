#include "HeapMgr.h"

using namespace OS;

HeapMgr::HeapMgr(int _totalMem)
{
	totalMem = _totalMem;
	freeMem = totalMem;
	nxtID = 0;
	initial.blkID = nxtID;
	initial.free = true;
	initial.size = totalMem;
	initial.address = 0;
	blocks.push_back(initial);
}
void HeapMgr::display()
{
	std::cout << "Amount of free memory: " << freeMem << "\n";
	for(blkIter=blocks.begin(); blkIter != blocks.end(); blkIter++)
	{
		std::cout <<"block " << blkIter->blkID << ": ";
		std::cout << blkIter->size << " bytes ";
		if(blkIter->free == true)
		{
			std::cout << "free\n";
		}
		else
			std::cout << "used\n";
	}
}
int HeapMgr::freeMemAmount()
{
	return freeMem;
}
int HeapMgr::allocate(int size)
{
	if(freeMem < size)
	{
		return NULL;
	}
	for(blkIter=blocks.begin(); blkIter != blocks.end(); blkIter++)
	{
		if((blkIter->free == true) && (blkIter->size >= size))
		{
			nxtID++;
			blkInfo temp;
			temp.blkID = nxtID;
			temp.address = blkIter->address + size;
			temp.size = blkIter->size - size;
			temp.free = true;
			blkIter->free = false;
			blkIter->size = size;
			blocks.push_back(temp);
			freeMem = freeMem- size;
			return blkIter->address;
		}
	}
	return NULL;
}