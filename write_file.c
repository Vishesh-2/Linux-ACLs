#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include "structures.h"

int main(int argc, char const *argv[]) {
    
    if(argc != 3) {
        perror("Specify 3 arguments as: fput filename text");
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

    char *data = malloc((dataLen+strlen(argv[2])) * sizeof(char));
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
        perror("Permission Denied for fput on this file");
        return 1;
    }

    dataLen += strlen(argv[2]);
    // printf("New Data Length: %d\n", dataLen);
    
    
    // Open file in write mode:
    filePointer = fopen(argv[1], "w");
    if(filePointer == NULL) {
        perror("Error reading from file\n");
        return 1;
    }

    strcat(data, argv[2]);
    // printf("Data before writing: %s\n", data);
    fwrite(&aclLen, sizeof(int), 1, filePointer);
    fwrite(acl, sizeof(struct aclNode), aclLen, filePointer);
    fwrite(&dataLen, sizeof(int), 1, filePointer);
    fwrite(data, sizeof(char), dataLen, filePointer);

    return 0;
}