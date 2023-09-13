#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include "structures.h"

// export PATH=/home/visheshrangwani/NSS/A1/Q1:$PATH
// setacl username rwx filename

int getNewNode(struct aclNode *node, char username[50], char permissions[3]) {
    struct passwd *pw;
    errno = 0;
    pw = getpwnam(username);

    if(pw == NULL) {
        if (errno != 0) {
            perror("getpwnam - ");
            return -1;
        } else {
            fprintf(stderr, "User not found: %s\n", username);
            return -1;
        }
    }

    node->uid = pw->pw_uid;
    strcpy(node->uname, username);
    strcpy(node->perms, permissions);
}

int main(int argc, char const *argv[])
{
    if(argc != 4) {
        perror("Specify 3 arguments as: username +rwx filename");
        return 1;
    }

    int callingUID = getuid();
    // printf("setacl called using User ID: %d | EUID: %d\n", callingUID, geteuid());

    int isDir = 0;
    struct stat file_info;
    if(stat(argv[3], &file_info) == 0) {
        if(S_ISDIR(file_info.st_mode)) {
            isDir = 1;
        }
    }
    else {
        perror("Error in stat of file.\n");
        return EXIT_FAILURE;
    }
    
    // printf("setacl set to: %d | effective uid: %d\n", getuid(), geteuid());
    if(isDir) {
        const char *attr_1 = "user.acl_len";
        const char *attr_2 = "user.acl_data";
        int acl_len;

        ssize_t bytes_read = getxattr(argv[3], attr_1, &acl_len, sizeof(int));
        if(bytes_read < 0) {
            perror("Error in reading ACL length");
            return EXIT_FAILURE;
        }

        struct aclNode *acl = malloc((acl_len+1) * sizeof(struct aclNode));

        bytes_read = getxattr(argv[3], attr_2, acl, acl_len * sizeof(struct aclNode));
        if(bytes_read < 0) {
            perror("Error in reading ACL");
            return EXIT_FAILURE;
        }

        // Check if the user is authorized to edit acl
        int isValidUser = 0;

        for(int i=0; i<acl_len; ++i) {
            if(acl[i].uid == callingUID && acl[i].perms[1] == 'w') {
                isValidUser = 1;
            }
        }

        if(isValidUser == 0) {
            perror("Permission denied for setacl on this directory\n");
            return EXIT_FAILURE;
        }

        // Setting EUID to that of owner at index 0.
        if(seteuid(acl[0].uid) < 0) {
            perror("Unable to set EUID to that of owner\n");
            return EXIT_FAILURE;
        }

        // printf("setacl set to: %d | effective uid: %d\n", getuid(), geteuid());

        // Checking if owner has write permissions or not
        if(acl[0].perms[1] != 'w') {
            perror("Owner does not have write permission on this directory");
            return EXIT_FAILURE;
        }

        // Check if new user
        char inpUser[50], inpPerms[20];
        strcpy(inpUser, argv[1]);
        int aclIdx = -1;
        strcpy(inpPerms, argv[2]);
        for(int i=0; i<acl_len; ++i) {
            if(strcmp(inpUser, acl[i].uname)==0) {
                aclIdx = i;
            }
        }

        if(aclIdx == -1) {
            acl_len++;
            int ret = getNewNode(&(acl[acl_len-1]), inpUser, inpPerms);
            if(ret == -1) {
                perror("Couldn't create a new acl node");
                return EXIT_FAILURE;
            }
        }
        else {
            strcpy(acl[aclIdx].perms, inpPerms);
        }

        if(setxattr(argv[3], attr_1, &acl_len, sizeof(int), 0) != 0) {
            perror("Error in setting ACL length\n");
            return EXIT_FAILURE;
        }

        if(setxattr(argv[3], attr_2, acl, acl_len * sizeof(struct aclNode), 0) != 0) {
            perror("Error in setting ACL\n");
            return EXIT_FAILURE;
        }


        // for(int i=0; i<acl_len; ++i) {
        //     printf("%d | %s | %s\n", acl[i].uid, acl[i].uname, acl[i].perms);
        // }

        // printf("Length of ACL: %d\n", acl_len);
        printf("ACL modified successfully!\n");
    }
    else {
        // Open the file in read mode
        FILE *filePointer;
        filePointer = fopen(argv[3], "r");
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

        struct aclNode *acl = (struct aclNode *)malloc((aclLen+1)*sizeof(struct aclNode));
        bytesRead = fread(acl, sizeof(struct aclNode), aclLen, filePointer);
        if (bytesRead == 0) {
            perror("Error reading from the file");
            return 1;
        }

        int dataLen;
        bytesRead = fread(&dataLen, sizeof(int), 1, filePointer);
        if (bytesRead == 0) {
            perror("Error reading from the file");
            return 1;
        }

        char *data = malloc(dataLen*sizeof(char));
        fread(data, sizeof(char), dataLen, filePointer);

        fclose(filePointer);

        // Check if valid user
        int validUser = 0;

        for(int i=0; i<aclLen; ++i) {
            if(acl[i].uid == callingUID && acl[i].perms[1] == 'w') {
                validUser = 1;
            }
        }

        if(validUser == 0) {
            perror("Permission Denied for setacl on this file");
            return 1;
        }

        // Setting EUID to that of owner at index 0.
        if(seteuid(acl[0].uid) < 0) {
            perror("Unable to set EUID to that of owner\n");
            return EXIT_FAILURE;
        }

        // printf("setacl set to: %d | effective uid: %d\n", getuid(), geteuid());

        // Checking if owner has write permissions or not
        if(acl[0].perms[1] != 'w') {
            perror("Owner does not have write permission on this file");
            return EXIT_FAILURE;
        }

        char inpUser[50], inpPerm[50];
        strcpy(inpUser, argv[1]);
        strcpy(inpPerm, argv[2]);

        // printf("Input Details: %s\n", inpPerm);


        // TODO: Validate the input letters and length(>1) in permissions:
        
        int aclIdx = -1;
        for(int i=0; i<aclLen; ++i) {
            if(strcmp(inpUser, acl[i].uname)==0) {
                aclIdx = i;
            }
        }

        filePointer = fopen(argv[3], "w");
        if(filePointer == NULL) {
            perror("Error opening file for writing\n");
            return 1;
        }

        if(aclIdx == -1) {
            aclLen++;

            struct aclNode newNode;
            // printf("Calling with values: %s | %s\n", inpUser, inpPerm);
            int ret = getNewNode(&newNode, inpUser, inpPerm);
            if(ret == -1) {
                perror("Couldn't create a new node");
                return 1;
            }

            // printf("New Node details: %d %s %s\n", newNode.uid, newNode.uname, newNode.perms);
            fwrite(&aclLen, sizeof(int), 1, filePointer);
            fwrite(acl, sizeof(struct aclNode), aclLen-1, filePointer);
            fwrite(&newNode, sizeof(struct aclNode), 1, filePointer);
            fwrite(&dataLen, sizeof(int), 1, filePointer);
            fwrite(data, sizeof(char), dataLen, filePointer);
        }
        else {
            strcpy(acl[aclIdx].perms, inpPerm);

            fwrite(&aclLen, sizeof(int), 1, filePointer);
            fwrite(acl, sizeof(struct aclNode), aclLen, filePointer);
            fwrite(&dataLen, sizeof(int), 1, filePointer);
            fwrite(data, sizeof(char), dataLen, filePointer);
        }

        fclose(filePointer);
        // printf("ACL Length: %d\n", aclLen);
        printf("ACL modified successfully!\n");
    }
    return 0;
}