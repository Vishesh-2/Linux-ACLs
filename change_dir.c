#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include "structures.h"

int main(int argc, char const *argv[])
{
    if(argc != 2) {
        perror("Enter in this format cd_acl directory_name");
        return EXIT_FAILURE;
    }
    const char *destDir = argv[1];

    int callingUID = getuid();

    // Check if directory
    struct stat fileInfo;
    if(stat(argv[1], &fileInfo) == 0) {
        if(!S_ISDIR(fileInfo.st_mode)) {
            perror("Not a directory\n");
            return EXIT_FAILURE;
        }
    }
    
    const char *attr_1 = "user.acl_len";
    const char *attr_2 = "user.acl_data";
    int acl_len;

    // TODO: check if current file is created using ACL system

    ssize_t bytes_read = getxattr(argv[1], attr_1, &acl_len, sizeof(int));
    if(bytes_read < 0) {
        perror("Error in reading ACL length");
        return EXIT_FAILURE;
    }

    struct aclNode *acl = malloc(acl_len * sizeof(struct aclNode));

    bytes_read = getxattr(argv[1], attr_2, acl, acl_len * sizeof(struct aclNode));
    if(bytes_read < 0) {
        perror("Error in reading ACL");
        return EXIT_FAILURE;
    }

    int isPermittedUser = 0;
    for(int i=0; i<acl_len; ++i) {
        if(acl[i].uid == callingUID && acl[i].perms[2] == 'x') {
            isPermittedUser = 1;
        }
    }

    if(isPermittedUser == 0) {
        perror("Permission denied to enter in this directory\n");
        return EXIT_FAILURE;
    }
    else {
        if(chdir(destDir) != 0) {
            perror("Error in using chdir()\n");
            return EXIT_FAILURE;
        }
        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("Error in calling getcwd()\n");
            return EXIT_FAILURE;
        }
        printf("Successfully changed directory to: %s\n", cwd);
    }
    
    return 0;
}