#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include "structures.h"

int main(int argc, char const *argv[]) {
    
    if(argc != 2) {
        perror("Specify 2 arguments as: fget filename");
        return 1;
    }

    int callingUID = getuid();
    // printf("setacl called using User ID: %d\n", callingUID);

    int stUID = setuid(0);
    if (stUID<0)
    {
        perror("Error in setting UID");
        return 1;
    }

    // Open the file in read mode
    FILE *filePointer;
    filePointer = fopen(argv[1], "r");
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

    struct aclNode *acl = (struct aclNode *)malloc(aclLen*sizeof(struct aclNode));
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
        if(acl[i].uid == callingUID && acl[i].perms[0] == 'r') {
            validUser = 1;
        }
    }

    if(validUser == 0) {
        perror("Permission Denied for fget (read) on this file");
        return 1;
    }
    printf("Data Length: %d\n", dataLen);
    printf("File Content:\n%s\n", data);
    
    return 0;
}