#ifndef DISK_H
#define DISK_H

#define DISK_BLOCK_SIZE 4096

/*This function must be invoked before calling other API functions.
It is only possible to have one active disk at some point in time.*/
int  disk_init( const char *filename, long nblocks );

/*Returns an integer with the total number of the blocks in the disk.*/
int  disk_size();

/*Reads the contents of the block disk numbered blocknum (4096 bytes) to a memory buffer that starts at address data.*/
void disk_read( long blocknum, char *data );

/*Writes, in the block blocknum of the disk, a total of 4096 bytes starting at memory address data.*/
void disk_write( long blocknum, const char *data );

/*Function to be called at the end of the program.*/
void disk_close();


#endif
