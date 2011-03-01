#ifndef FILESYSTEM_H
#define FILESYSTEM_H
const size_t fileDataBlks = 20;//every block is 512 bytes which gives a maximum of 10KB file size.
const size_t lenghtPath = 60;
const size_t blkSize = 512;
const size_t numOfEntries = 10;//maximum number of directory or file entries in a directory.
enum iNType{directory,file,link};
struct Inode{
	size_t creationDate;//YYYYMMDD
	size_t lastChDate;//YYYYMMDD
};
struct iNFile: Inode{
	size_t fileSize;
	size_t dataBLKS[fileDataBlks];
};
struct iNLink: Inode{
	char pathName[lenghtPath];
};
struct Data{
	char rawData[blkSize]; 
	Data(){
		for(int i = 0; i < blkSize; i++)
		{rawData[i] = '3';}//initialize bytes of data with some char.
	}
};
struct Entry{
	iNType type;
	char name[60];
	Inode* iNptr;
};
struct Directory{
	size_t size;//number of entries in directory.
	Inode* next;
	Entry entries[numOfEntries];//if entries exceed 10 then Inode points to next dir block.
};
class FileSystem
{

};

#endif