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

//v2.0 copyright Comine.com 20150812W0852
#include <windows.h>
#include "MBuffer.h"
#include "MString.h"
#include "MWinRegKey.h"
#include "MWinServiceControl.h"


//******************************************************
//**  Module Elements
//******************************************************
static const int GMaxServiceCount=1000;


//******************************************************
//**  MWinServiceControl class
//******************************************************
void MWinServiceControl::ClearObject(void)
	{
	mServiceCount=0;
	}


////////////////////////////////////////////////
MWinServiceControl::MWinServiceControl(void)
	{  ClearObject();  }


////////////////////////////////////////////////
MWinServiceControl::~MWinServiceControl(void)
	{  Destroy();  }


////////////////////////////////////////////////
bool MWinServiceControl::Create(void)
	{
	Destroy();
	
	// Allocate space for buffer
	if(mBuffer.Create(sizeof(ENUM_SERVICE_STATUS)*GMaxServiceCount)==false)
		{
		Destroy();  return false;
		}

	// Open Service Control Manager
	SC_HANDLE hmanager=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ENUMERATE_SERVICE);
	if(hmanager==NULL)
		{
		Destroy();  return false;
		}

	// Enumerate all the services
	DWORD bytesextra=0;
	DWORD resumehandle=0;
	BOOL ret=EnumServicesStatusA(hmanager,SERVICE_WIN32,SERVICE_STATE_ALL
			,(LPENUM_SERVICE_STATUSA)mBuffer.GetBuffer(),mBuffer.GetSize()
			,&bytesextra,(DWORD *)&mServiceCount,&resumehandle);
	if(ret==FALSE)
		{
		Destroy();  return false;
		}

	if(CloseServiceHandle(hmanager)==FALSE)
		{
		Destroy();  return false;
		}
	
	return true;
	}


////////////////////////////////////////////////
bool MWinServiceControl::Destroy(void)
	{
	mBuffer.Destroy();
	ClearObject();
	return true;
	}


////////////////////////////////////////////////
int MWinServiceControl::GetCount(void)
	{
	return mServiceCount;
	}


////////////////////////////////////////////////
const char *MWinServiceControl::GetShortName(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return "";
		}

	ENUM_SERVICE_STATUSA *base=(ENUM_SERVICE_STATUSA *)mBuffer.GetBuffer();
	return base[index].lpServiceName;
	}


////////////////////////////////////////////////
const char *MWinServiceControl::GetLongName(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return "";
		}

	ENUM_SERVICE_STATUSA *base=(ENUM_SERVICE_STATUSA *)mBuffer.GetBuffer();
	return base[index].lpDisplayName;
	}


////////////////////////////////////////////////
bool MWinServiceControl::GetDescription(int index,MString &output)
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}

	ENUM_SERVICE_STATUSA *base=(ENUM_SERVICE_STATUSA *)mBuffer.GetBuffer();
	const char *servicename=base[index].lpServiceName;

	//=We Now have the service name in servicename

	// Open the Service key
	MWinRegKey servicekey;
	if(servicekey.Create(false,"System\\CurrentControlSet\\Services",HKEY_LOCAL_MACHINE
			,false, KEY_QUERY_VALUE |KEY_ENUMERATE_SUB_KEYS)==false)
		{
		//=Unable to open registry HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services
		return false;
		}
	
	// Open the service key
	MWinRegKey controlkey;
	if(controlkey.Create(false,servicename,servicekey.GetKey(),false,KEY_READ)==false)
		{
		//=Unable to open service subkey
		return false;
		}

	// Check if desciption name/value exists
	if(controlkey.DoesNameExist("Description")==false)
		{  return false;  }

	// Get the description
	if(controlkey.GetValueData("Description",output)==false)
		{
		return false;
		}

	controlkey.Destroy();
	servicekey.Destroy();
	
	return true;
	}


////////////////////////////////////////////////////////////
bool MWinServiceControl::GetImagePath(int index,MString &imagepath)
	{
	if(index<0 || index>=mServiceCount)
		{
		return 0;
		}

	ENUM_SERVICE_STATUSA *base=(ENUM_SERVICE_STATUSA *)mBuffer.GetBuffer();
	const char *servicename=base[index].lpServiceName;

	//=We Now have the service name in servicename

	// Open the Service key
	MWinRegKey servicekey;
	if(servicekey.Create(false,"System\\CurrentControlSet\\Services",HKEY_LOCAL_MACHINE
			,false, KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)==false)
		{
		//=Unable to open registry HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services
		return false;
		}
	
	// Open the service key
	MWinRegKey controlkey;
	if(controlkey.Create(false,servicename,servicekey.GetKey(),false
			, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS )==false)
		{
		//=Unable to open service subkey
		return false;
		}

	// Check if desciption name/value exists
	if(controlkey.DoesNameExist("ImagePath")==false)
		{  return false;  }

	// Get the description
	if(controlkey.GetValueData("ImagePath",imagepath)==false)
		{
		return false;
		}

	controlkey.Destroy();
	servicekey.Destroy();
	
	return true;
	}

////////////////////////////////////////////////////////////
unsigned int MWinServiceControl::GetCurrentState(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return 0;
		}

	// Open Service Manager
	SC_HANDLE hscm=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_CONNECT);
	if(hscm==NULL)
		{
		return 0; 
		}

	// Get Service Name
	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		CloseServiceHandle(hscm);
		return 0;
		}

	SC_HANDLE hservice=OpenServiceA(hscm,servicename,SERVICE_QUERY_STATUS);
	if(hservice==NULL)
		{  
		CloseServiceHandle(hscm);
		return 0;
		}

	//Get current service status
	SERVICE_STATUS currentstatus;
	if(QueryServiceStatus(hservice,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return 0;
		}

	CloseServiceHandle(hservice);
	CloseServiceHandle(hscm);

	ENUM_SERVICE_STATUS *base=(ENUM_SERVICE_STATUS *)mBuffer.GetBuffer();
	unsigned int state=currentstatus.dwCurrentState;

	//Update info about service
	base[index].ServiceStatus.dwCurrentState=state;

	if(state==SERVICE_CONTINUE_PENDING) { return state; }
	else if(state==SERVICE_PAUSE_PENDING) { return state; }
	else if(state==SERVICE_PAUSED) { return state; }
	else if(state==SERVICE_RUNNING) { return state; }
	else if(state==SERVICE_START_PENDING) { return state; }
	else if(state==SERVICE_STOP_PENDING) { return state; }
	else if(state==SERVICE_STOPPED) { return state; }
	
	return 0;
	}

//////////////////////////////////////////////////////////////////////
//=-1(Failure)|=0(Disabled),=1(Manual Start),=2(AutoStart)
int MWinServiceControl::GetStartState(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return -1;
		}

	MWinRegKey servicekey;
	if(servicekey.Create(false,"System\\CurrentControlSet\\Services",HKEY_LOCAL_MACHINE
			,false, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS )==false)
		{
		//=Unable to open registry HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services
		return -1;
		}

	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		return -1;
		}

	MWinRegKey controlkey;
	if(controlkey.Create(false,servicename,servicekey.GetKey()
			,false,KEY_QUERY_VALUE |KEY_ENUMERATE_SUB_KEYS)==false)
		{
		//=Unable to open service subkey
		return -1;
		}
	
	if(controlkey.DoesNameExist("Start")==false)
		{
		return -1;
		}

	DWORD startvalue=0;
	if(controlkey.GetValueData("Start",startvalue)==false)
		{
		return -1;
		}

	// startvalue 4(Disabled),3(Manual),2(Automatic)
	if(startvalue==4) { return 0; }
	else if(startvalue==3) { return 1; }
	else if(startvalue==2) { return 2; }

	return -1;
	}


/////////////////////////////////////////////////////////////////////
//state=0(Disabled),=1(Manual Start),=2(AutoStart)
bool MWinServiceControl::SetStartState(int index,int state)
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}
	
	if(state<0 || state>2)
		{
		return false;
		}

	MWinRegKey servicekey;
	if(servicekey.Create(false,"System\\CurrentControlSet\\Services",HKEY_LOCAL_MACHINE
			,false, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS)==false)
		{
		//=Unable to open registry HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services
		return false;
		}

	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		return false;
		}

	MWinRegKey controlkey;
	if(controlkey.Create(false,servicename,servicekey.GetKey())==false)
		{
		//=Unable to open service subkey
		return false;
		}
	
	if(controlkey.DoesNameExist("Start")==false)
		{
		return false;
		}

	int startstate=0;
	if(state==0)
		{ startstate=4;  }
	else if(state==1)
		{  startstate=3; }
	else if(state==2)
		{  startstate=2; }
	else
		{
		// Dead Code
		return false;
		}
	
	if(controlkey.SetValueData("Start",startstate)==false)
		{
		return false;
		}

	return true;
	}


//////////////////////////////////////////////////////////
int MWinServiceControl::GetServiceIndex(const char *shortname)
	{
	// Perform Search for name
	int i;
	for(i=0;i<mServiceCount;++i)
		{
		const char *servicename=GetShortName(i);
		if(MString::ICompare(servicename,shortname)==0) { return i; }
		}

	return -1;
	}


//////////////////////////////////////////////////////////////
bool MWinServiceControl::StartService(int index,int argn,const char *argv[])
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}

	// Open Service Manager
	SC_HANDLE hscm=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_CONNECT);
	if(hscm==NULL)
		{
		return false;
		}

	// Get Service Name
	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		CloseServiceHandle(hscm);
		return false;
		}

	SC_HANDLE hservice=OpenServiceA(hscm,servicename,SERVICE_START|SERVICE_QUERY_STATUS);
	if(hservice==NULL)
		{  
		CloseServiceHandle(hscm);
		return false; 
		}

	// Check is service already starting
	SERVICE_STATUS currentstatus;
	if(QueryServiceStatus(hservice,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	//if started return;
	if(currentstatus.dwCurrentState==SERVICE_RUNNING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//If Pending too
	if(currentstatus.dwCurrentState==SERVICE_START_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//service currently paused
	if(currentstatus.dwCurrentState==SERVICE_PAUSED)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	// start the service
	if(::StartServiceA(hservice,argn,argv)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	CloseServiceHandle(hservice);  // Close Service Handle
	CloseServiceHandle(hscm);	// Close Service Control Manager	
	return true;
	}


//////////////////////////////////////////////////////////////
bool MWinServiceControl::StopService(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}


	// Open Service Manager
	SC_HANDLE hscm=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_CONNECT);
	if(hscm==NULL)
		{
		return false; 
		}

	// Get Service Name
	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		CloseServiceHandle(hscm);
		return false;
		}

	SC_HANDLE hservice=OpenServiceA(hscm,servicename,SERVICE_STOP|SERVICE_QUERY_STATUS);
	if(hservice==NULL)
		{  
		CloseServiceHandle(hscm);
		return false;
		}

	//Check if service is stopped already
	SERVICE_STATUS currentstatus;
	if(QueryServiceStatus(hservice,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	//if stopped return;
	if(currentstatus.dwCurrentState==SERVICE_STOPPED)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//If Pending too
	if(currentstatus.dwCurrentState==SERVICE_STOP_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}


	//if stopped return;
	if(currentstatus.dwCurrentState==SERVICE_PAUSED)
		{

		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}


	//stop service
	if(ControlService(hservice,SERVICE_CONTROL_STOP,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	CloseServiceHandle(hservice);  // Close Service Handle
	CloseServiceHandle(hscm);	// Close Service Control Manager	

	return true;
	}


//////////////////////////////////////////////////////////////
bool MWinServiceControl::PauseService(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}

	// Open Service Manager
	SC_HANDLE hscm=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_CONNECT);
	if(hscm==NULL)
		{
		return false; 
		}

	// Get Service Name
	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		CloseServiceHandle(hscm);
		return false;
		}

	SC_HANDLE hservice=OpenServiceA(hscm,servicename
			,SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
	if(hservice==NULL)
		{  
		CloseServiceHandle(hscm);
		return false;
		}

	//Get current service status
	SERVICE_STATUS currentstatus;
	if(QueryServiceStatus(hservice,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	//if stopped return;
	if(currentstatus.dwCurrentState==SERVICE_STOPPED)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//if stop pending return;
	if(currentstatus.dwCurrentState==SERVICE_STOP_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//If Pending too
	if(currentstatus.dwCurrentState==SERVICE_PAUSE_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}


	//If Pending too
	if(currentstatus.dwCurrentState==SERVICE_PAUSED)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//pause the service
	if(ControlService(hservice,SERVICE_CONTROL_PAUSE,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	CloseServiceHandle(hservice);  // Close Service Handle
	CloseServiceHandle(hscm);	// Close Service Control Manager	

	return true;
	}


//////////////////////////////////////////////////////////////
bool MWinServiceControl::ContinueService(int index)
	{
	if(index<0 || index>=mServiceCount)
		{
		return false;
		}

			// Open Service Manager
	SC_HANDLE hscm=OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_CONNECT);
	if(hscm==NULL)
		{
		return false;
		}

	// Get Service Name
	const char *servicename=GetShortName(index);
	if(servicename==NULL)
		{
		CloseServiceHandle(hscm);
		return false;
		}

	SC_HANDLE hservice=OpenServiceA(hscm,servicename
			,SERVICE_PAUSE_CONTINUE|SERVICE_QUERY_STATUS);
	if(hservice==NULL)
		{  
		CloseServiceHandle(hscm);
		return false;
		}

	//Get current service status
	SERVICE_STATUS currentstatus;
	if(QueryServiceStatus(hservice,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	//if stopped return;
	if(currentstatus.dwCurrentState==SERVICE_STOPPED)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}


	//Service is stopping
	if(currentstatus.dwCurrentState==SERVICE_STOP_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	//Service is continue pending
	if(currentstatus.dwCurrentState==SERVICE_CONTINUE_PENDING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	//Service is continue pending
	if(currentstatus.dwCurrentState==SERVICE_RUNNING)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return true;
		}

	// continue process
	if(ControlService(hservice,SERVICE_CONTROL_CONTINUE,&currentstatus)==FALSE)
		{
		CloseServiceHandle(hservice);
		CloseServiceHandle(hscm);
		return false;
		}

	CloseServiceHandle(hservice);  // Close Service Handle
	CloseServiceHandle(hscm);	// Close Service Control Manager	
	return true;
	}

