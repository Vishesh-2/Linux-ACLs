#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <pwd.h>
#include "structures.h"

void getDefaultACL(int uid, char* uname, struct aclNode *node) {
    node->uid = uid;
    strcpy(node->uname, uname);
    strcpy(node->perms, "rwx");
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
        seteuid(0);
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

int main(int argc, char const *argv[])
{
    if(argc != 2) {
        perror("Invalid number of arguments. Expected Usage- create_dir dir_name");
        return EXIT_FAILURE;
    }

    // TODO: check if allowed to create directory or not.
    int wrAccess = canWriteDir();
    if(wrAccess != 0) {
        perror("Permission Denied to create a directory!\n");
        return EXIT_FAILURE;
    }

    // Use the mkdir function to create the directory
    if (mkdir(argv[1], 0777) == 0) {
        printf("Directory '%s' created successfully.\n", argv[1]);

        // Get the current user's UID using getuid()
        uid_t uid = getuid();
        gid_t gid = getgid();

        if (chmod(argv[1], S_IRWXU | S_IRWXG | S_IRWXO) == 0) {
            // printf("Permissions set to rwx for all users.\n");
        } else {
            perror("Error setting permissions");
            return 1;
        }

        // Use the chown function to change the owner of the directory to the current user
        if (chown(argv[1], uid, gid) == 0) {
            // printf("Owner of directory '%s' changed to UID %d.\n", argv[1], uid);
        } else {
            perror("Error in using chown");
            return EXIT_FAILURE;
        }

        // Initialize the ACL as attribute in file system
        const char *attribute_name = "user.acl_data";
        const char *attr_1 = "user.acl_len";
        int acl_len = 1;
        struct aclNode defNode;
        char username[1024];
        struct passwd *userInfo = getpwuid(getuid());
        if(userInfo == NULL) {
            perror("Error in invoking getpwd(), maybe user not found\n");
            return EXIT_FAILURE;
        }
        else {
            strcpy(username, userInfo->pw_name);
        }
        getDefaultACL(getuid(), username, &defNode);
        
        // Set the extended attribute for the directory
        if (setxattr(argv[1], attr_1, &acl_len, sizeof(int), 0) != 0) {
            perror("Error in setting ACL length");
            return EXIT_FAILURE;
        }

        if (setxattr(argv[1], attribute_name, &defNode, sizeof(struct aclNode), 0) == 0) {
            // printf("Attribute '%s' set successfully for directory '%s'.\n", attribute_name, argv[1]);
        } else {
            perror("Error in setting default ACL");
            return EXIT_FAILURE;
        }
    }
    else {
        perror("Error in creating directory\n");
        return EXIT_FAILURE;
    }

    return 0;
}