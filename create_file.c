#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/xattr.h>
#include <pwd.h>
#include "structures.h"

void getDefaultACL(int uid, char* uname, struct aclNode *node) {
    node->uid = uid;
    strcpy(node->uname, uname);
    strcpy(node->perms, "rw-");
}

int canWriteDir() {
    const char *attr_1 = "user.acl_len";
    const char *attr_2 = "user.acl_data";
    int acl_len;

    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error in using getcwd\n");
        return EXIT_FAILURE;
    }

    // Check if parent directory not created using create_dir
    ssize_t bytes_read = getxattr(cwd, attr_1, &acl_len, sizeof(int));
    if(bytes_read < 0) {
        seteuid(getuid());
        int perm = access(cwd, W_OK | X_OK);    // Write to create a directory, X to check if can be in directory
        // seteuid(0);
        if(perm != 0) return -1;
    }
    else { // Parent directory created using create_dir
        struct aclNode *acl = malloc(acl_len * sizeof(struct aclNode));

        bytes_read = getxattr(cwd, attr_2, acl, acl_len * sizeof(struct aclNode));
        if(bytes_read < 0) {
            perror("Error in reading ACL");
            return EXIT_FAILURE;
        }

        int isPermittedUser = 0;
        for(int i=0; i<acl_len; ++i) {
            if(acl[i].uid == getuid() && acl[i].perms[1] == 'w') {
                isPermittedUser = 1;
            }
        }

        if(isPermittedUser == 0) {
            return EXIT_FAILURE;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 2) {
        perror("Specify 1 argument as filename\n");
        return EXIT_FAILURE;
    }

    int wrAcc = canWriteDir();
    if(wrAcc != 0) {
        perror("Permission Denied to create a file in this directory!\n");
        return EXIT_FAILURE;
    }

    char cwd[1024];

    // Creating the file
    FILE *filePointer; // Declare a file pointer

    // Open a file in write mode (creates the file if it doesn't exist)
    filePointer = fopen(argv[1], "w");

    if (filePointer == NULL) {
        perror("Error opening the file.\n");
        return EXIT_FAILURE;
    }

    // Get the current user's username:
    // char *username = getenv("USER");
    // if (username == NULL) {
    //     perror("Unable to determine the username using getenv.\n");
    // }

    char username[1024];
    struct passwd *userInfo = getpwuid(getuid());
    if(userInfo == NULL) {
        perror("Unable to get user details\n");
        return EXIT_FAILURE;
    }
    else {
        strcpy(username, userInfo->pw_name);
    }

    // Use the structure with default ACL:
    struct aclNode defaultACLNode;
    getDefaultACL(getuid(), username, &defaultACLNode);

    // Write the data and ACL onto the file
    int defaultDataLength = 0, defaultACLLength = 1;
    char *defaultData = "";
    fwrite(&defaultACLLength, sizeof(int), 1, filePointer);
    fwrite(&defaultACLNode, sizeof(struct aclNode), defaultACLLength, filePointer);
    fwrite(&defaultDataLength, sizeof(int), 1, filePointer);
    fwrite(defaultData, 1, defaultDataLength, filePointer);

    // printf("Default Node values: %d | %s | %s\n", defaultACLNode.uid, defaultACLNode.uname, defaultACLNode.perms);

    fclose(filePointer);
    printf("Successfully created file\n");

    return 0;
}