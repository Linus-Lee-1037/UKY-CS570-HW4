**Build Instructions**
Note: The GNU readline library is required for this project to build.

To build the programs, simply run "make".  Running rpcgen is not necessary, as it is run by the Makefile.

To delete all programs, build files, and virtual disk files, simply run "make clean".

To start RPC service, you need to use command "sudo service rpcbind start" to enable it first.

To unregister an RPC service, you need to use "rpcinfo -p" to obtain the program number (program) and version number (vers), then use the command "sudo rpcinfo -d program vers" to unregister the service.

**Usage Instructions**
To run the server program, simply run "./server".  If your rpc binder is not running in insecure mode, you may need to run as "sudo ./server".

To run the client program, simply run "./client hostname", where hostname is the name of the host running the server, e.g., "./client localhost" if the server is running on the same host.

The client has equipped with all required functions: Open, Read, Write, List, Seek, Delete, Close. The sample code given in the requirement of Assignment 4 is integrated into the main() function and I add a call of List() to test in line 184 before the call of Close(fd2) so that you could test all the required functions to see if they work correctly.
