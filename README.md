==================
Build Instructions
==================
Note: The GNU readline library is required for this project to build.

To build the programs, simply run "make".  Running rpcgen is not necessary, as it is run by the Makefile.

To delete all programs, build files, and virtual disk files, simply run "make clean".

To unregister an RPC service, you need to use "rpcinfo -p" to obtain the program number (program) and version number (vers), then use the command "sudo rpcinfo -d program vers" to unregister the service.

==================
Usage Instructions
==================
To run the server program, simply run "./server".  If your rpc binder is not running in insecure mode, you may need to run as "sudo ./server".

To run the client program, simply run "./client hostname", where hostname is the name of the host running the server, e.g., "./client localhost" if the server is running on the same host.

The client accepts the following commands: create, list, delete, write, read, copy.
create:  Requires a filename.  Create a file with given name.
list:  Lists all files owned by user.
delete: Requires a filename.  Deletes file with given name.
write: Requires a filename, offset, write length, and data.  Writes write length bytes of data to the file with given name at given offset.
read: Requires a filename, offset, and read length.  Reads read length bytes from file with given name from given offset.
copy: Requires a source filename and destination filename.  Copies data in source file to destination file.  Destination file must already exists.
