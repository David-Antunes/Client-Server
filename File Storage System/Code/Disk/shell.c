#include "fs.h"
#include "disk.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int do_copyin( const char *filename, long inumber );
static int do_copyout( long inumber, const char *filename );

int main( int argc, char *argv[] )
{
	char line[1024];
	char cmd[1024];
	char arg1[1024];
	char arg2[1024];
	long inumber, result, args;

	if(argc!=3) {
		printf("use: %s <diskfile> <nblocks>\n",argv[0]);
		return 1;
	}

	if(!disk_init(argv[1],atoi(argv[2]))) {
		printf("couldn't initialize %s: %s\n",argv[1],strerror(errno));
		return 1;
	}

	printf("opened emulated disk image %s with %d blocks\n",argv[1],disk_size());

	while(1) {
		printf("sf-1920> ");
		fflush(stdout);

		if(fgets(line,sizeof(line),stdin)==NULL) break;

		if(line[0]=='\n') continue;
		line[strlen(line)-1] = (char)0;

		args = sscanf(line,"%s %s %s",cmd,arg1,arg2);
		if(args==0) continue;

		if(!strcmp(cmd,"format")) {
			if(args==1) {
				if(!fs_format()) {
					printf("disk formatted.\n");
				} else {
					printf("format failed!\n");
				}
			} else {
				printf("use: format\n");
			}
		} else if(!strcmp(cmd,"mount")) {
			if(args==1) {
				if(!fs_mount()) {
					printf("disk mounted.\n");
				} else {
					printf("mount failed!\n");
				}
			} else {
				printf("use: mount\n");
			}
		} else if(!strcmp(cmd,"debug")) {
			if(args==1) {
				fs_debug();
			} else {
				printf("use: debug\n");
			}
		} else if(!strcmp(cmd,"getsize")) {
			if(args==2) {
				inumber = atoi(arg1);
				result = fs_getsize(inumber);
				if(!result) {
					printf("inode %ld has size %ld\n",inumber,result);
				} else {
					printf("getsize failed!\n");
				}
			} else {
				printf("use: getsize <inumber>\n");
			}

		} else if(!strcmp(cmd,"create")) {
			if(args==1) {
				inumber = fs_create();
				if(inumber>=0) {
					printf("created inode %ld\n",inumber);
				} else {
					printf("create failed!\n");
				}
			} else {
				printf("use: create\n");
			}
		} else if(!strcmp(cmd,"delete")) {
			if(args==2) {
				inumber = atoi(arg1);
				if(!fs_delete(inumber)) {
					printf("inode %ld deleted.\n",inumber);
				} else {
					printf("delete failed!\n");
				}
			} else {
				printf("use: delete <inumber>\n");
			}
		} else if(!strcmp(cmd,"cat")) {
			if(args==2) {
				inumber = atoi(arg1);
				if(do_copyout(inumber,"/dev/stdout") < 0) {
					printf("cat failed!\n");
				}
			} else {
				printf("use: cat <inumber>\n");
			}

		} else if(!strcmp(cmd,"copyin")) {
			if(args==3) {
				inumber = atoi(arg2);
				if(do_copyin(arg1,inumber)>0) {
					printf("copied file %s to inode %ld\n",arg1,inumber);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyin <filename> <inumber>\n");
			}

		} else if(!strcmp(cmd,"copyout")) {
			if(args==3) {
				inumber = atoi(arg1);
				if(do_copyout(inumber,arg2)>=0) {
					printf("copied inode %ld to file %s\n",inumber,arg2);
				} else {
					printf("copy failed!\n");
				}
			} else {
				printf("use: copyout <inumber> <filename>\n");
			}

		} else if(!strcmp(cmd,"help")) {
			printf("Commands are:\n");
			printf("    format\n");
			printf("    mount\n");
			printf("    debug\n");
			printf("    create\n");
			printf("    delete  <inode>\n");
			printf("    cat     <inode>\n");
			printf("    copyin  <file> <inode>\n");
			printf("    copyout <inode> <file>\n");
			printf("    help\n");
			printf("    quit\n");
			printf("    exit\n");
		} else if(!strcmp(cmd,"quit")) {
			break;
		} else if(!strcmp(cmd,"exit")) {
			break;
		} else {
			printf("unknown command: %s\n",cmd);
			printf("type 'help' for a list of commands.\n");
			result = 1;
		}
	}

	printf("closing emulated disk.\n");
	disk_close();

	return 0;
}

static int do_copyin( const char *filename, long inumber )
{
	FILE *file;
	long offset=0;
	int result, actual;
	char buffer[16384];

	file = fopen(filename,"r");
	if(!file) {
		printf("couldn't open %s: %s\n",filename,strerror(errno));
		return 0;
	}

	while(1) {
		result = fread(buffer,1,sizeof(buffer),file);
		if(result<=0) break;
		if(result>0) {
			actual = fs_write(inumber,buffer,result,offset);
			if(actual<0) {
				printf("ERROR: fs_write return invalid result %d\n",actual);
				break;
			}
			offset += actual;
			if(actual!=result) {
				printf("WARNING: fs_write only wrote %d bytes, not %d bytes\n",actual,result);
				break;
			}
		}
	}

	printf("%ld bytes copied\n",offset);

	fclose(file);
	return 1;
}

static int do_copyout( long inumber, const char *filename )
{
	FILE *file;
	long offset=0;
	int result;
	char buffer[16384];

	file = fopen(filename,"w");
	if(!file) {
		printf("couldn't open %s: %s\n",filename,strerror(errno));
		return 0;
	}

	while(1) {
		result = fs_read(inumber,buffer,sizeof(buffer),offset);
		if(result<=0) break;
		fwrite(buffer,1,result,file);
		offset += result;
	}

	printf("%ld bytes copied\n",offset);

	fclose(file);
	return 1;
}
