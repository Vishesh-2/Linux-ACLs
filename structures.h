

struct aclNode
{
    int uid;
    char uname[50];
    char perms[3];
};

struct fileData {
    int aclLen;
    struct aclNode *acl;
    int dataLen;
    char* data;
};