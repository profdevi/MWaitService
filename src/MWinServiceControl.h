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
#ifndef MWinServiceControl_h
#define MWinServiceControl_h

/////////////////////////////////////////////////////
#include <windows.h>
#include "MBuffer.h"
#include "MString.h"

//******************************************************
//**  MWinServiceControl class
//******************************************************
class MWinServiceControl
	{
	////////////////////////////////////////////////
	MBuffer mBuffer;								// Buffer Space
	int mServiceCount;								// Total Number of Services
	
	////////////////////////////////////////////////
	void ClearObject(void);
	
	////////////////////////////////////////////////
	public:
	MWinServiceControl(void);
	~MWinServiceControl(void);
	bool Create(void);
	bool Destroy(void);
	int GetCount(void);								// Services
	const char *GetShortName(int index);
	const char *GetLongName(int index);
	bool GetDescription(int index,MString &description);
	bool GetImagePath(int index,MString &imagepath);

	// returns the current state of service
	//=0(Failure) | SERVICE_CONTINUE_PENDING|SERVICE_PAUSE_PENDING|SERVICE_PAUSED
	//|SERVICE_RUNNING|SERVICE_START_PENDING|SERVICE_STOP_PENDING
	//|SERVICE_STOPPED
	unsigned int GetCurrentState(int index);
	
	//=-1(Failure)|=0(Disabled),=1(Manual Start),=2(AutoStart)
	int GetStartState(int index);

	//state=0(Disabled),=1(Manual Start),=2(AutoStart)
	bool SetStartState(int index,int state);

	// Get the service index given shortname
	int GetServiceIndex(const char *shortname);

	// Start Service
	bool StartService(int index,int argn,const char *argv[]);
	bool StopService(int index);
	bool PauseService(int index);
	bool ContinueService(int index);
	};


#endif // MWinServiceControl_h

