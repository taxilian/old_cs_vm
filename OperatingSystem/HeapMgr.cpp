#include "HeapMgr.h"

using namespace OS;
HeapMgr::HeapMgr()
{
	totalMem = 0;
	freeMem = 0;
	nxtID = 0;
}
HeapMgr::HeapMgr(int _totalMem, int baseAddrs)
{
	totalMem = _totalMem;
	freeMem = totalMem;
	nxtID = 0;
	initial.blkID = nxtID;
	initial.free = true;
	initial.size = totalMem;
	initial.address = baseAddrs;
	blocks.push_back(initial);
}
void HeapMgr::display()
{
	std::cout << "Amount of free memory: " << freeMemAmount() << "\n";
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
	freeMem = 0;
	for(blkIter=blocks.begin(); blkIter != blocks.end(); blkIter++)
	{
		if(blkIter->free == true)
		{
			freeMem = freeMem + blkIter->size;
		}
	}
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
		if((blkIter->free == true) && (blkIter->size > size))
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
		if((blkIter->free == true) && (blkIter->size == size))
		{
			blkIter->free = false;
			freeMem = freeMem - size;
			return blkIter->address;
		}
	}
	return NULL;
}
void HeapMgr::de_allocate(int addr)
{
	std::list<blkInfo>::iterator temp;
	for(blkIter=blocks.begin(); blkIter != blocks.end(); blkIter++)
	{
		if(blkIter->address == addr)
		{
			blkIter->free = true;
			if(blkIter == blocks.begin())
			{
				temp = blkIter;
				temp++;
				if(temp->free==true)
				{
					blkIter->size = blkIter->size + temp->size;
					blocks.erase(temp);
				}
			}
			else if(blkIter == blocks.end())
			{
				temp = blkIter;
				temp--;
				if(temp->free == true)
				{
					blkIter->size = blkIter->size + temp->size;
					blocks.erase(temp);
				}
			}
			else
			{
				temp = blkIter;
				temp--;
				if(temp->free == true)
				{
					blkIter->size = blkIter->size + temp->size;
					blocks.erase(temp);
				}
				temp = blkIter;
				temp++;
				if(temp->free == true)
				{
					blkIter->size = blkIter->size + temp->size;
					blocks.erase(temp);
				}
			}
			break;
		}
	}
} 
void HeapMgr::setMem(int _totalMem, int baseAddrs)
{
	totalMem = _totalMem;
	freeMem = totalMem;
	nxtID = 0;
	initial.blkID = nxtID;
	initial.free = true;
	initial.size = totalMem;
	initial.address = baseAddrs;
	blocks.push_back(initial);
}