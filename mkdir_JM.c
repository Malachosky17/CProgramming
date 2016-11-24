/*
 * mkdir_JM.c
 * 
 * Copyright 2016 Joe Malachosky <jsm2@Jinux>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
 /*
  * Example inputs:
  * 	mkdir mydir
  * 	mkdir -m rwx mydir -> "Makes mydir and sets permissions so that users may read, write and execute contents" 
  * 	mkdir -p /mydir/innerdir/a/b/c -> "If the parent directory /mydir/innerdir/a/b does not exist, mkdir will create that directory first"
  */

/*
 * Systems Programming Final Project
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define PROGRAM_NAME = "JM_mkdir"

#define AUTHORS = "Joseph Malachosky"

int mkRecursiveDir(const char *dir);

struct globalArgs_t {
	int modeChanged;
	int verbosity;			/* -v option to print dirs as created */
	int create_parents;		/* -p option to make parent dirs if they don't exist */
	mode_t mode;			/* -m option to chmod a dir, changing the permission */
} globalArgs;

const char *optString = "M:m:P:p:V::v::H::h::";

int main(int argc, char *argv[])
{
	int option = 0;
	struct stat sb;
	char str[256];
	
	/*
	 * Used for debugging
	 * Printing out program, followed by arguments to the terminal
	 */
	//printf("argc = %d\n", argc);
	//for(int i = 0; i < argc; i++) {
	//	printf("argv[%d] = \"%s\"\n", i, argv[i]);
	//}
	
	//Default mode for my global arguments
	globalArgs.modeChanged = 0;
	globalArgs.mode = S_IRWXU;
	
	/*
	 * If getopt doesn't match with any char in the optString it will return '?'
	 */
	
	while((option = getopt(argc, argv, optString)) != -1) {
		switch(option) {
			case 'M':
			case 'm':
				//printf("Mode option captured: %s\n", optarg);
				if(strcmp(optarg,"r") == 0) {
					globalArgs.modeChanged = 1;
					globalArgs.mode = S_IRUSR;
				} else if(strcmp(optarg, "w") == 0) {
					globalArgs.modeChanged = 2;
					globalArgs.mode = S_IWUSR;
				} else if(strcmp(optarg, "x") == 0) {
					globalArgs.modeChanged = 3;
					globalArgs.mode = S_IXUSR;
				} else if(strcmp(optarg, "rw") == 0 || strcmp(optarg, "wr") == 0) {
					globalArgs.modeChanged = 4;
					globalArgs.mode = (S_IRUSR | S_IWUSR);
				} else if(strcmp(optarg, "rwx") == 0 || strcmp(optarg, "rxw") == 0 ||
						  strcmp(optarg, "wrx") == 0 || strcmp(optarg, "wxr") == 0 ||
						  strcmp(optarg, "xrw") == 0 || strcmp(optarg, "xwr") == 0) {
					globalArgs.modeChanged = 0;
					globalArgs.mode = S_IRWXU;
				}
				break;
			
			case 'V':
			case 'v':
				//printf("Verbose option captured: %s\n", optarg);
				globalArgs.verbosity = 1;
				break;
				
			case 'P':
			case 'p':
				//printf("Parent option captured: %s\n", optarg);
				globalArgs.create_parents = 1;
				if(mkRecursiveDir(optarg) == -1) {
					return -1;
				}
				break;
				
			case 'H':
			case 'h':
				printf("\nUsage: %s [-mvph?] [dir...]\n\
-m, --mode=MODE   set permission mode (as in chmod), not rwxrwxrwx - umask\n\
-p, --parents     no error if existing, make parent directories as needed\n\
-v, --verbose     print a message for each created directory\n", argv[0]);
				return 0;
			case '?':
				//If a valid char is found, print and print usage
				if(isprint(optopt)) {
					printf("Unknown option %c\n", optopt);
				}
				printf("\nUsage: %s [-mvph?] [dir...]\n\
-m, --mode=MODE   set permission mode (as in chmod), not rwxrwxrwx - umask\n\
-p, --parents     no error if existing, make parent directories as needed\n\
-v, --verbose     print a message for each created directory\n", argv[0]);
				break;
			default:
				abort();
		}
		
	}
	
	if(argc == 1 || option == 0) {
		printf("Expected at least one of the following arguments:\n\
Usage: %s [-mvph?] [dir...]\n\
-m, --mode=MODE   set permission mode (as in chmod), not rwxrwxrwx - umask\n\
-p, --parents     no error if existing, make parent directories as needed\n\
-v, --verbose     print a message for each created directory\n", argv[0]);
	}
	
	if(globalArgs.create_parents != 1) {
		strncpy(str, argv[argc - 1], sizeof(str));
		if(stat(str, &sb) != 0) {
			if(mkdir(str, globalArgs.mode) < 0) {
				return -1;
			}
			if(globalArgs.verbosity) {
				printf("Created directory: %s\n", str);
			}
			if(globalArgs.modeChanged != 0) {
				printf("Changed permission of %s to %o\n", str, globalArgs.mode);
			}
		} else if(!S_ISDIR(sb.st_mode)) {
			return -1;
		}
	}
	
	return 0;
}

int mkRecursiveDir(const char *dir) {
	char tmp[256];
	char *p = NULL;
	struct stat sb;
	size_t len;
	
	strncpy(tmp, dir, sizeof(tmp));
	len = strlen(tmp);
	if(len >= sizeof(tmp)) {
		return -1;
	}
	
	if(tmp[len - 1] == '/') {
		tmp[len -1] = 0;
	}
	for(p = tmp + 1; *p; p++) {
		if(*p == '/') {
			*p = 0;
			//Test path
			if(stat(tmp, &sb) != 0) {
				//Path does not exist - create directory
				if(mkdir(tmp, globalArgs.mode) < 0) {
					return -1;
				}
				if(globalArgs.verbosity) {
					printf("Created directory: %s\n", tmp);
				}
			} else if(!S_ISDIR(sb.st_mode)) {
				//Not a directory
				return -1;
			}
			*p = '/';
		}
	}
	
	//Test path
	if(stat(tmp, &sb) != 0) {
		//Path does not exist - create directory
		if(mkdir(tmp, globalArgs.mode) < 0) {
			return -1;
		}
		if(globalArgs.verbosity) {
			printf("Created directory: %s\n", tmp);
		}
	} else if(!S_ISDIR(sb.st_mode)) {
		//Not a directory
		return -1;
	}
	if(globalArgs.modeChanged != 0) {
		printf("Changed permission of %s to %o\n", tmp, globalArgs.mode);
	}
	return 0;
}
