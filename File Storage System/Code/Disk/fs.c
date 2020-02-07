#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#define FS_MAGIC           0xf0f03410
#define INODES_PER_BLOCK   64
#define POINTERS_PER_INODE 14
#define POINTERS_PER_BLOCK 1024

struct fs_superblock {
	unsigned int magic;
	unsigned int nblocks;
	unsigned int ninodeblocks;
	unsigned int ninodes;
};
struct fs_superblock my_super;
#define NUM_SUPERBLOCKS 1

struct fs_inode {
	unsigned int isvalid;
	unsigned int size;
	unsigned int direct[POINTERS_PER_INODE];
};

union fs_block {
	struct fs_superblock super;
	struct fs_inode inode[INODES_PER_BLOCK];
	char data[DISK_BLOCK_SIZE];
};

#define FALSE 0
#define TRUE 1

#define VALID 1
#define NON_VALID 0

#define FREE 0
#define NOT_FREE 1
unsigned char * blockBitMap;

struct fs_inode inode;

int fs_format()
{
  union fs_block block;
  unsigned long i, nblocks;
  unsigned long ninodeblocks;

  if(my_super.magic == FS_MAGIC){
    printf("Cannot format a mounted disk!\n");
    return -1;
  }
  nblocks = disk_size();
  block.super.magic = FS_MAGIC;
  block.super.nblocks = nblocks;
  ninodeblocks = (int)ceil((float)nblocks*0.1);
  block.super.ninodeblocks = ninodeblocks;
  block.super.ninodes = block.super.ninodeblocks * INODES_PER_BLOCK;

  printf("superblock:\n");
  printf("    %d blocks\n",block.super.nblocks);
  printf("    %d inode blocks\n",block.super.ninodeblocks);
  printf("    %d inodes\n",block.super.ninodes);

  /* escrita do superbloco */
  disk_write(0,block.data);

  /* preparacao da tabela de inodes */
  bzero( block.data, DISK_BLOCK_SIZE);
  for( i = 0; i < INODES_PER_BLOCK; i++ )
    block.inode[i].isvalid = NON_VALID;

  /* escrita da tabela de inodes */
  for( i = 1; i <= ninodeblocks; i++)
    disk_write( i, block.data );

  return 0;
}

void fs_debug()
{
	union fs_block sBlock;
	union fs_block iBlock;
	unsigned int i, j, k;

	disk_read(0, sBlock.data);

	if (sBlock.super.magic != FS_MAGIC) {
		printf("disk unformatted !\n");
		return;
	}
	printf("superblock:\n");
	printf("    %d blocks\n", sBlock.super.nblocks);
	printf("    %d inode blocks\n", sBlock.super.ninodeblocks);
	printf("    %d inodes\n", sBlock.super.ninodes);

	for (i = 1; i <= sBlock.super.ninodeblocks; i++) {
		disk_read(i, iBlock.data);
		for (j = 0; j < INODES_PER_BLOCK; j++)
			if (iBlock.inode[j].isvalid == VALID) {
				printf("-----\n inode: %d\n", (i - 1) * INODES_PER_BLOCK + j);
				printf("size: %d \n", iBlock.inode[j].size);
				printf("blocks:");
				for (k = 0; k < POINTERS_PER_INODE; k++)
					if (iBlock.inode[j].direct[k] != 0)
						printf("  %d", iBlock.inode[j].direct[k]);
				printf("\n");
			}
	}
}

int fs_mount()
{
	union fs_block block;

	if(my_super.magic == FS_MAGIC){
		printf("disc already mounted!\n");
		return -1;
	}

	disk_read(0,block.data);
	if(block.super.magic != FS_MAGIC){
		printf("cannot mount an unformatted disc!\n");
		return -1;
	}
	if(block.super.nblocks != disk_size()){
		printf("file system size and disk size differ!\n");
		return -1;
	}
	//mounts disk
	my_super.magic = block.super.magic;
	my_super.nblocks = block.super.nblocks;
	my_super.ninodeblocks = block.super.ninodeblocks;
	my_super.ninodes = block.super.ninodes;

	blockBitMap = (unsigned char *)malloc(block.super.nblocks*sizeof(char));

	// This registers the superblock and inodeblocks with NOT_FREE on the blockBitMap
	for (long i = 0; i < NUM_SUPERBLOCKS + my_super.ninodeblocks; i++) {
		blockBitMap[i] = NOT_FREE;
	}

	//This sweeps the inode blocks to register the various used datablocks
	for (long i = NUM_SUPERBLOCKS; i < NUM_SUPERBLOCKS + my_super.ninodeblocks; i++) {

		// Reads inodeBlock
		disk_read(i,block.data);

		//Sweeps every inode
		for (int j = 0; j < INODES_PER_BLOCK; j++) {

			if(block.inode[j].isvalid) {

				//Finds the number of blocks used by inode
				int pointToBlock = (block.inode[j].size / DISK_BLOCK_SIZE);
				int remainder = block.inode[j].size % DISK_BLOCK_SIZE;
				if (remainder != 0) {
					pointToBlock++;
				}

				//Registers which blocks are NOT_FREE
				for (int k = 0; k < pointToBlock; k++) {
					blockBitMap[block.inode[j].direct[k]] = NOT_FREE;
				}
			}
		}
	}
	return 0;
}

int fs_create()
{
	if(my_super.magic != FS_MAGIC){
		printf("disc not mounted\n");
		return -1;
	}

	union fs_block block;

	//This sweeps the inode blocks to register the various used datablocks
	for (long blockNumber = NUM_SUPERBLOCKS; blockNumber < NUM_SUPERBLOCKS + my_super.ninodeblocks; blockNumber++) {
		disk_read(blockNumber, block.data);
		for (int inodeIndex = 0; inodeIndex < INODES_PER_BLOCK; inodeIndex++) {
			if(!block.inode[inodeIndex].isvalid) {
				block.inode[inodeIndex].isvalid = VALID;
				block.inode[inodeIndex].size = 0;
				for (size_t i = 0; i < POINTERS_PER_INODE; i++) {
					block.inode[inodeIndex].direct[i] = 0;
				}
				disk_write(blockNumber, block.data);
				return (blockNumber - NUM_SUPERBLOCKS) * INODES_PER_BLOCK + inodeIndex;
			}
		}
	}
	return -1;
}

void inode_load( long inumber, struct fs_inode *inode ){
	long inodeBlock;
	union fs_block block;

	if( inumber > my_super.ninodes ){
		printf("inode number too big \n");
		abort();
	}
	inodeBlock = 1 + (inumber/INODES_PER_BLOCK);
	disk_read( inodeBlock, block.data );
	*inode = block.inode[inumber % INODES_PER_BLOCK];
}

void inode_save(long inumber, struct fs_inode* inode) {
	long inodeBlock;
	union fs_block block;

	if (inumber > my_super.ninodes) {
		printf("inode number too big \n");
		abort();
	}
	inodeBlock = 1 + (inumber / INODES_PER_BLOCK);
	disk_read(inodeBlock, block.data);
	block.inode[inumber % INODES_PER_BLOCK] = *inode;
	disk_write(inodeBlock, block.data);
}

int fs_delete( long inumber )
{
	if(my_super.magic != FS_MAGIC){
		printf("disc not mounted\n");
		return -1;
	}
	// CHECKS IF THE INODE NUMBER IS LOWER THAN THE TOTAL NUMBER OF INODES
	if (my_super.ninodes < inumber) {
		return -1;
	}

	inode_load(inumber, &inode);
	if (inode.isvalid == NON_VALID) {
		return -1;
	}

	//Number of blocks occupied of the file
	long numBlocks = (int)ceil((float)inode.size/DISK_BLOCK_SIZE);

	//Updating BitMap
	for (long i = 0; i < numBlocks; i++) {
		blockBitMap[inode.direct[i]] = FREE;
	}

	inode.isvalid = NON_VALID;
	inode_save(inumber, &inode);

	return 0;
}

int fs_getsize( long inumber )
{
	if(my_super.magic != FS_MAGIC){
		printf("disc not mounted\n");
		return -1;
	}
	// CHECKS IF THE INODE NUMBER IS LOWER THAN THE TOTAL NUMBER OF INODES
	if (my_super.ninodes < inumber) {
		return -1;
	}
	inode_load(inumber, &inode);
	return inode.size;
}


/**************************************************************/
int min(int a, int b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

/*Writes dataLimit of data starting at dataOffset in data
up to (bufferLimit - bufferOffset) in buffer starting at bufferOffset
Returns the number of bytes written*/
int writeDataInBuffer(char* buffer, int bufferOffset, int bufferLimit, char* data, int dataOffset, int dataLimit) {
	int limit = min(bufferLimit, dataLimit);
	for (int i = 0; i < limit; i++) {
		buffer[i + bufferOffset] = data[i + dataOffset];
	}
	return limit;
}

int fs_read( long inumber, char *data, long length, long offset )
{
	long currentBlock, offsetCurrent, offsetInBlock;
	long bytesLeft, nCopy, bytesToRead;
	char *dst;
	union fs_block buff;

	if(my_super.magic != FS_MAGIC){
		printf("disc not mounted\n");
		return -1;
	}
	inode_load( inumber, &inode );
	if( inode.isvalid == NON_VALID ){
		printf("inode is not valid\n");
		return -1;
	}
	if( offset > inode.size ){
		printf("offset bigger that file size !\n");
		return -1;
	}
	if (inode.size == 0) {
		return 0;
	}

	// Start
	bytesToRead = 0;
	bytesLeft = length;
	currentBlock = offset / DISK_BLOCK_SIZE;
	offsetInBlock = offset % DISK_BLOCK_SIZE;
	dst = data;
	offsetCurrent = offset;

	// Start, Mid and End
	while (inode.size - offsetCurrent > 0 && bytesLeft > 0) {
		disk_read(inode.direct[currentBlock++], buff.data);
		nCopy = writeDataInBuffer(dst, bytesToRead, min(bytesLeft, inode.size - offsetCurrent), buff.data, offsetInBlock, DISK_BLOCK_SIZE - offsetInBlock);
		bytesToRead += nCopy;
		bytesLeft -= nCopy;
		offsetCurrent += nCopy;
		offsetInBlock = 0;
	}
	return bytesToRead;
}

/******************************************************************/
int getFreeBlock(){
	int i, found;

	i = 0;
	found = FALSE;
	do{
		if(blockBitMap[i] == FREE){
			found = TRUE;
			blockBitMap[i] = NOT_FREE;
		}
		else i++;
	}while((!found) && (i < my_super.nblocks));

	if(i == my_super.nblocks) return -1; /* nao ha' blocos livres */
	else return i;
}

int fs_write( long inumber, char *data, long length, long offset )
{
	long currentBlock, offsetInBlock;
	long bytesLeft, nCopy, bytesToWrite, newEntry;
	long originalNBlocks;
	char *src;
	union fs_block buff;

	if(my_super.magic != FS_MAGIC){
		printf("disc not mounted\n");
		return -1;
	}
	inode_load( inumber, &inode );
	if( inode.isvalid == NON_VALID ){
		printf("inode is not valid\n");
		return -1;
	}
	if( offset > inode.size ){
		printf("starting to write after end of file\n");
		return -1;
	}

	// Start
	bytesToWrite = 0;
	bytesLeft = length;
	currentBlock = offset / DISK_BLOCK_SIZE;
	offsetInBlock = offset % DISK_BLOCK_SIZE;
	src = data;

	originalNBlocks = (inode.size / DISK_BLOCK_SIZE) - 1;
	if (inode.size % DISK_BLOCK_SIZE > 0) {
		originalNBlocks++;
	}

	// Start, Mid and End
	while (bytesLeft > 0 && currentBlock < POINTERS_PER_INODE) {
		if (currentBlock > originalNBlocks || inode.size == 0) {
			newEntry = getFreeBlock();
			if (newEntry == -1) {
				inode_save(inumber, &inode);
				return bytesToWrite;
			}
			inode.direct[currentBlock] = newEntry;
		} else {
			disk_read(inode.direct[currentBlock], buff.data);
		}
		nCopy = writeDataInBuffer(buff.data, offsetInBlock, DISK_BLOCK_SIZE - offsetInBlock, src, bytesToWrite, bytesLeft);
		disk_write(inode.direct[currentBlock++], buff.data);
		if (currentBlock - 1 == originalNBlocks) {
			inode.size -= (inode.size % DISK_BLOCK_SIZE) - offsetInBlock;
		}
		if (currentBlock - 1 < originalNBlocks) {
			inode.size -= DISK_BLOCK_SIZE - offsetInBlock;
		}
		inode.size += nCopy;
		bytesToWrite += nCopy;
		bytesLeft -= nCopy;
		offsetInBlock = 0;
	}
	inode_save( inumber, &inode );
	return bytesToWrite;
}
