#ifndef FS_H
#define FS_H

/*#Prints detailed information about the file system.
Reports the contents of the i-node table.*/
void fs_debug();

/*#Formats the disk.
Creates a new file system (FS) in the disk, destroying all the disk's content.
Reserves 10% of the blocks for the i-node table; this table is initialized with all the i-nodes (non-valid).
Please note that formatting a disk does not imply that the disk is accessible.
This is performed by the mount operation.
Trying to format a mounted disk is not allowed; invoking fs_format with the disk in use should do nothing and return an error.*/
int  fs_format();

/*#Mounts the filesystem (reads the superblock and the i-node table; builds the block map).
Verifies if there is a valid FS in the disk.
If the FS in the disk is valid, this operation reads the superblock and, using the i-node table in disk, builds in RAM the map of free/occupied blocks.
Please note that all the following operations should fail if the disk is not mounted.*/
int  fs_mount();

/*#Creates a new file; returns the i-node number.
Marks the first free i-node as occupied by a file of length 0.
Returns the number of the allocated i-node. In error, returns -1*/
int  fs_create();

/*#Deletes the file with inode inumber.
Removes the file with i-node inumber.
Frees the i-node entry and declares all the blocks associated with it as free by updating the map of free/occupied blocks.
Returns 0 if success; -1 if an error occurs.*/
int  fs_delete( long inumber );

/*#Returns the size of the file inumber.
Returns the length of the file associated with the i-node.
In error, returns -1.*/
int  fs_getsize(long inumber);

/*#Reads length bytes, starting at offset, from file inode, and transfers the bytes to a buffer that starts on address data.
Transfers data from a file (identified by a valid i-node) to memory.
Copies length bytes from the i-node inode to the address data pointer, starting at offset in the file.
Returns the effective number of bytes read.
This number of bytes read can be lower than the number of bytes requested if the distance from offset to the end of the file is less than length.
In case of error, returns -1.*/
int  fs_read( long inumber, char *data, long length, long offset );

/*#Writes length bytes, starting at offset, into file inode by transferring the bytes from a buffer that starts in data.
Transfers data between memory and the file designated by inode.
Copies length bytes from the address data to the file starting at position defined in offset.
This operation will allocate the necessary disk blocks.
Returns the number of bytes really written to the file; this number of written bytes can be lower than the length, in case there are no free disk blocks.
In case of other errors, returns -1.*/
int  fs_write( long inumber, char *data, long length, long offset );

#endif
