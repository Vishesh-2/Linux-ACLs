all: create_file getacl setacl fput fget create_dir aclcd

create_file: create_file.c
	gcc -o /usr/bin/create_file create_file.c

getacl: get_acl.c
	gcc -o /usr/bin/getacl get_acl.c && chmod u+s /usr/bin/getacl

setacl: set_acl.c
	gcc -o /usr/bin/setacl set_acl.c && chmod u+s /usr/bin/setacl

fput: write_file.c
	gcc -o /usr/bin/fput write_file.c && chmod u+s /usr/bin/fput

fget: read_file.c
	gcc -o /usr/bin/fget read_file.c && chmod u+s /usr/bin/fget

create_dir: create_dir.c
	gcc -o /usr/bin/create_dir create_dir.c && chmod u+s /usr/bin/create_dir

aclcd: cd_helper.sh change_dir.c
	gcc -o /usr/bin/change_dir change_dir.c && chmod u+s /usr/bin/change_dir && chmod u+s cd_helper.sh

clean:
	rm create_file getacl setacl fput fget
