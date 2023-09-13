#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include "structures.h"

// export PATH=/home/visheshrangwani/NSS/A1/Q1:$PATH

int main(int argc, char const *argv[])
{
    if(argc != 2) {
        perror("Specify 1 argument as filename");
        return 1;
    }

    int callingUID = getuid();
    // printf("getacl called using User ID: %d\n", callingUID);

    int stUID = setuid(0);
    if (stUID<0)
    {
        perror("Error in setting UID");
        return 1;
    }
    
    int isDir = 0;
    struct stat file_info;
    if(stat(argv[1], &file_info) == 0) {
        if(S_ISDIR(file_info.st_mode)) {
            isDir = 1;
        }
    }
    else {
        perror("Error in stat of file.\n");
        return EXIT_FAILURE;
    }

    // Handle the case when the input filename is a directory
    if(isDir == 1) {
        const char *attr_1 = "user.acl_len";
        const char *attr_2 = "user.acl_data";
        int acl_len;

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

        int isValidUser = 0;

        for(int i=0; i<acl_len; ++i) {
            if(acl[i].uid == callingUID && acl[i].perms[0] == 'r') {
                isValidUser = 1;
            }
        }

        if(isValidUser == 0) {
            perror("Permission denied for getacl on this directory\n");
            return EXIT_FAILURE;
        }

        printf("UID | Username | Permissions\n");
        for(int i=0; i<acl_len; ++i) {
            printf("%d | %s | %s\n", acl[i].uid, acl[i].uname, acl[i].perms);
        }

        // printf("Length of ACL: %d\n", acl_len);
    }
    else {  // Handle the case when the input filename is not a directory
        // Open the file in read mode
        FILE *filePointer;
        filePointer = fopen(argv[1], "rb");
        if(filePointer == NULL) {
            perror("Error reading from file\n");
            return 1;
        }

        int aclLen;
        size_t bytesRead = fread(&aclLen, sizeof(int), 1, filePointer);
        if (bytesRead == 0) {
            perror("Error reading from the file");
            return 1;
        }

        struct aclNode *acl;
        acl = (struct aclNode *)malloc(aclLen*sizeof(struct aclNode));
        bytesRead = fread(acl, sizeof(struct aclNode), aclLen, filePointer);
        if (bytesRead == 0) {
            perror("Error reading from the file");
            return 1;
        }

        int validUser = 0;

        for(int i=0; i<aclLen; ++i) {
            if(acl[i].uid == callingUID && acl[i].perms[0] == 'r') {
                validUser = 1;
            }
        }

        if(validUser == 0) {
            perror("Permission Denied for getacl on this file");
            return -1;
        }
        else {
            printf("UID | Username | Permissions\n");
            for(int i=0; i<aclLen; ++i) {
                printf("%d | %s | %s\n", acl[i].uid, acl[i].uname, acl[i].perms);
            }
        }

        fclose(filePointer);
        // printf("ACL Length: %d\n", aclLen);
    }

    return 0;
}