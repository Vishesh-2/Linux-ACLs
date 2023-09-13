## Advantages of ACL implementation

ACLs are better suited over Discretionary Action Control system.

The ACLs are used to successfully defend against the following bugs/errors/attacks:

* ACLs provide a more fine-grained, user-level access control over an object. The permissions can be maintained at a per-user basis for each object. This ensures that the owner/object-manager has give explicit permission for each user to have some access.

* ACLs prevent `Unauthorized access` to groups or user from accessing certain data. This leads to prevention of `Leakage of Sensitive Data` and unnecessary File Modifications.

* It also prevents `Malware Propagation` as execution and write permissions are more carefully given to users and hence their corresponding owned programs.

* ACLs also preven `DoS` attacks as the access to the resources is controlled.

* ACLs can provide `Multilevel Security Environments` that separates out data access and modification based on different criteria, more precisely than default DAC in linux.