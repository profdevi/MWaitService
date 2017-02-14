/*    
    MWaitServices.exe : Waits for a Windows Service to reach a known state.
    Copyright (C) 2017  Comine.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

//v1.0 copyright Comine.com 20170214T1050
#include "MStdLib.h"
#include "MCommandArg.h"
#include "MWinServiceControl.h"
#include "MLicenseGPL.h"


//******************************************************
//* Module Elements
//******************************************************
static const char *GApplicationName="MWaitService";	// Used in Help
static const char *GApplicationVersion="1.0";		// Used in Help

////////////////////////////////////////////////////
static void GDisplayHelp(void);
static bool GServiceList(void);						// List out services
static int GServiceGetIndex(MWinServiceControl &servicecontrol,const char *name);			// get index of service
static bool GServiceWait(const char *name,unsigned int targetstate,int maxwaitsecs);		// Wait till service reaches state
static bool GServicePrintState(MWinServiceControl &servicecontrol,int index);				// Print Current State

////////////////////////////////////////////////////
int main(int argn,const char *argv[])
	{
	MCommandArg args(argn,argv);

	///////////////////////////////////////////////
	if(args.GetArgCount()<2)
		{
		GDisplayHelp();
		return 0;
		}

	if(args.CheckRemoveHelp()==true)
		{
		GDisplayHelp();
		return 0;
		}

	if(args.CheckRemoveArg("-gpl")==true)
		{
		MLicenseGPL gpl(true);
		gpl.Print();
		return 0;
		}

	if(args.CheckRemoveArg("-l")==true)
		{
		GServiceList();
		return 0;
		}

	if(args.GetArgCount()<3)
		{
		GDisplayHelp();
		return 0;
		}

	// Wait for no more than 30 seconds
	int maxwaitsecs=30;
	const char *waitflagarg=0;
	int waitflagindex=-1;
	if(args.GetNameValue("-w=",waitflagarg,waitflagindex)==true)
		{
		maxwaitsecs=MStdAToI(waitflagarg);
		if(maxwaitsecs<=0)
			{
			MStdPrintf("**Bad wait time argument");
			GDisplayHelp();
			return 1;
			}

		args.RemoveArg(waitflagindex);
		}
	
	if(args.CheckRemoveArg("-p")==true)
		{
		const char *servicename=args.GetArg(1);
		if(GServiceWait(servicename,SERVICE_PAUSED,maxwaitsecs)==false)
			{
			return 1;
			}

		MStdPrintf("Service Paused...\n");
		return 0;
		}
	else if(args.CheckRemoveArg("-r")==true)
		{
		const char *servicename=args.GetArg(1);
		if(GServiceWait(servicename,SERVICE_RUNNING,maxwaitsecs)==false)
			{
			return 1;
			}

		MStdPrintf("Service Running...\n");
		return 0;
		}
	else if(args.CheckRemoveArg("-s")==true)
		{
		const char *servicename=args.GetArg(1);
		if(GServiceWait(servicename,SERVICE_STOPPED,maxwaitsecs)==false)
			{
			return 1;
			}

		MStdPrintf("Service Stopped...\n");
		return 0;
		}

	return 0;
	}


////////////////////////////////////////////////////
static void GDisplayHelp(void)
	{
	MStdPrintf(	"\n"
				"   usage:  %s [-?|-gpl] [-w=<waittime>] <options>\n"
				"           v%s copyright Comine.com\n"
				"\n"
				"   This program comes with ABSOLUTELY NO WARRANTY; for details use -gpl.\n"
				"   This is free software, and you are welcome to redistribute it under\n"
				"   certain conditions.  This software has a Gnu Public License(GPL)\n"
				"\n"
				"   Program will wait for a service to reach a specified state of running,\n"
				"   stoped, or paused\n"
				"\n"
				"     -w=<waittime>       : Wait for a number of seconds\n"
				"     -s <service name>   : Wait till service reaches stop state\n"
				"     -r <service name>   : Wait till service reaches run state\n"
				"     -p <service name>   : Wait till service reaches pause state\n"
				"     -l                  : Show all services info\n"
				"     -gpl                : Show the Gnu Public License\n"
				"\n"
				,GApplicationName,GApplicationVersion);
	}


////////////////////////////////////////////////////////////////
static bool GServiceList(void)
	{
	MWinServiceControl servicecontrol;
	if(servicecontrol.Create()==false)
		{
		MStdPrintf("**Unable to init system\n");
		return false;
		}
	
	const int servicecount=servicecontrol.GetCount();
	for(int i=0;i<servicecount;++i)
		{
		MStdPrintf("%3d ",i+1);
		const char *shortname=servicecontrol.GetShortName(i);
		const char *longname=servicecontrol.GetLongName(i);

		MString info;
		if(servicecontrol.GetDescription(i,info)==false)
			{
			info.Create("");
			}

		GServicePrintState(servicecontrol,i);

		MStdPrintf("%-30s : %-30s\n",shortname,longname,info.Get() );
		}
	
	return true;
	}


////////////////////////////////////////////////////////////////
static int GServiceGetIndex(MWinServiceControl &servicecontrol,const char *name)
	{
	const int servicecount=servicecontrol.GetCount();
	for(int i=0;i<servicecount;++i)
		{
		const char *shortname=servicecontrol.GetShortName(i);
		MStdAssert(shortname!=0);

		if(MStdStrICmp(shortname,name)==0) { return i; }
		}

	return -1;
	}


////////////////////////////////////////////////////////////////
static bool GServiceWait(const char *name,unsigned int targetstate,int maxwaitsecs)
	{
	MWinServiceControl servicecontrol;
	if(servicecontrol.Create()==false)
		{
		MStdPrintf("**Unable to init system\n");
		return 1;
		}

	const int index=GServiceGetIndex(servicecontrol,name);
	if(index<0)
		{
		MStdPrintf("**No Short Service Name %s\n",name);
		return false;
		}
	
	for(int i=0;i<maxwaitsecs;++i)
		{
		const unsigned int state=servicecontrol.GetCurrentState(index);
		if(state==targetstate) { return true; }
		// Sleep for 1000ms
		MStdPrintf("Time: %ds  Service %s is ",i+1,name);
		GServicePrintState(servicecontrol,index);
		MStdPrintf("\n");
		MStdSleep(1000);
		}

	MStdPrintf("Wait Time Reached\n");
	return false;
	}


///////////////////////////////////////////////////////////////
static bool GServicePrintState(MWinServiceControl &servicecontrol,int index)
	{
	const unsigned int state=servicecontrol.GetCurrentState(index);
	const char *stateformat="%-15s";
	if(state==SERVICE_CONTINUE_PENDING)
		{
		MStdPrintf(stateformat,"Cont. Pending");
		}
	else if(state==SERVICE_PAUSE_PENDING)
		{
		MStdPrintf(stateformat,"Pause Pending");
		}
	else if(state==SERVICE_PAUSED)
		{
		MStdPrintf(stateformat,"Paused");
		}
	else if(state==SERVICE_RUNNING)
		{
		MStdPrintf(stateformat,"Running");
		}
	else if(state==SERVICE_START_PENDING)
		{
		MStdPrintf(stateformat,"Start Pending");
		}
	else if(state==SERVICE_STOP_PENDING)
		{
		MStdPrintf(stateformat,"Stop Pending");
		}
	else if(state==SERVICE_STOPPED)
		{
		MStdPrintf(stateformat,"Stopped");
		}
	else
		{
		MStdPrintf(stateformat,"UNKNOWN STATE");
		}
	
	return true;
	}


