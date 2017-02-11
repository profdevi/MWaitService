# MWaitService

This a Microsoft Windows Win32 console application that can be used to 
in batch files to wait for a Windows Service to reach a certain state
of Running, Paused, or Stopped.  This program is useful in backup procedures
were certain services have to be stopped, before a full backup of files can 
be done.


## Example Usage1:

The following example command line will wait till the MySQL server is stopped.  
If the MySQL service does not stop within 10s, the MWaitService returns with 
an not zero value, which can be used in batch files.

C:>  MWaitService  -s MySQL -w=10





