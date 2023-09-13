# ACLs_and_setuid in LINUX

## Description of the Systems
* The provided directory `ACLs_and_setuid` implements custom ACL and its correponding custom sudo in a linux system keeping emphasis on `setuid` system call.

* Custom sudo program is referred as `simple_sudo` and is owned by `root`. It allows executing commands of ACLs by a user who does not have relevant permission. It is analogous to the default sudo in linux that gives over and above access to the user than he currently has. It enables the current user to perform an action as the owner of the file on which action needs to be performed.

* Implementation done on an Ubuntu 20.04 VM.

* You need to have access to a linux machine with a user having sudo privileges in order to successfully run and test the system.

## Steps to execute
* `cd` into the directory `ACLs_and_setuid`.
* Compile the files using `sudo make`. sudo is required for using the make file to ensure, we can set the setuid bit.

* Run the command `source cd_helper.sh`. 
This needs to be executed in each instance of the shell you want to run the programs.

* Go into the desired user's shell, let's say to that of Larry and again execute `source cd_helper.sh`. Keep in mind that you have to specify the complete path of `cd_helper.sh` file while running this command.

* `create_dir DIRECTORY_NAME` to create a directory with desired name.

* You can cd into that directory using the normal `cd DIRECTORY_NAME` command of bash or the custom made `aclcd DIRECTORY_NAME` to change the directory.

* To create a new file for the ACL system, use the command `create_file FILE_NAME`.

* To view the ACL system of a directory/file: 
`getacl FILE/DIR_PATH`.

* To set or modify the ACL for a user:
`setacl USER PERM FILE/DIR`
    * `USER` is the username to whose ACL you wish to modify/add in the ACL of the file.
    
    * `PERM` refers to the permission given to USER. It must necessarily be 3 characters long, with first character representing `r- read`, second representing `w- write` and third representing `x- execute` permission. If you wish to give read, and execute permission to `bill` on a directory named `sample_dir`, the command would be: `setacl bill r-x sample_dir`.

* `fget FILE_NAME` command to get the contents stored in the file.

* `fput FILE_NAME text` - to write text to file. The command appends data on repeated usage.

* `aclcd DIRECTORY_NAME` - to change directory to an ACL maintained directory. This changes the directory in the shell as well.

* `change_dir DIRECTORY_NAME` - changes the directory but is not reflected. The change is confirmed by printing the output of getcwd(). This has been implemented because the implementation of `aclcd` cannot be implemented with `simple_sudo`.

* Command to execute a program using `simple_sudo`: `simple_sudo ACL_CMD PROG_ARGS`

    * `ACL_CMD` is one command out of those listed below, supported in Linux ACLs implementation.
    
    * `PROG_ARGS` are the command line arguments to be provided to the executible that needs to be run via simple_sudo program.


## Assumptions

* The directory created using `create_dir` would give the default `rwx` permissions to the user who created the directory in the ACL. This is same as that done in DAC.

* Other users need to be granted permissions explicitly using the `setacl` command.

* A file has to be created by using `create_file` command to initialize the ACLs for that. File created by this command can only be manipulated by ACLs.

* The file would have `rw-` default permissions for the user as is the case for DAC system when a file is created using the `touch` command.

* The data would be appended on using the `fput` command on a file.

* Only text data can be stored in the file.

* The files cannot be executed using `./file_name` command.

* The permissions specified in setacl need to be specified in a 3 character format: `rwx`. In case you do not want to grant a permission, enter `-` in its position.

* On re-entering the permission of a user, their permissions are updated as per the new permissions provided.

* The `create_file` and `create_dir` commands, when used with simple_sudo, make the owner of the directory as their owner.

* Only the following commands can be used with simple_sudo:

    * `create_file`
    
    * `create_dir`

    * `fput`

    * `fget`

    * `change_dir`

    * `getacl`

    * `setacl`

* In case the owner also does not have appropriate permissions despite using `simple_sudo`, a permission denied message would be printed.
