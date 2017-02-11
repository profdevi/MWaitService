# MWaitService

This a Microsoft Windows Win32 console application that can be used to 
in batch files to wait for a Windows Service to reach a certain state
of Running, Paused, or Stopped.  This program is useful in backup procedures
were certain services have to be stopped, before a full backup of files can 
be done.


## Command Information

```
   usage:  MWaitService [-?] [-w=<waittime>] <options>

   Program will wait for a service to reach a specified state of running,
   stoped, or paused

     -w=<waittime>       : Wait for a number of seconds
     -s <service name>   : Wait till service reaches stop state
     -r <service name>   : Wait till service reaches run state
     -p <service name>   : Wait till service reaches pause state
     -l                  : Show all services info
```

## Example Usage 1:

The following example command line will wait till the MySQL server is stopped.  
If the MySQL service does not stop within 10s, the MWaitService returns with
an not zero value, which can be used in batch files.

C:>  MWaitService  -s MySQL -w=10


## Example Usage 2:

Show list of all the services on the windows machine and show the state of the
service.  Both short and long names of the service is displayed.

C:>  MWaitService -l






