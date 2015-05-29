/**
 * \file Matlab.h
 * \version 1.0
 *
 * \section COPYRIGHT
 *
 * Copyright 2012-2013 The Iris Project Developers. See the
 * COPYRIGHT file at the top-level directory of this distribution
 * and at http://www.softwareradiosystems.com/iris/copyright.html.
 *
 * \section LICENSE
 *
 * This file is part of the Iris Project.
 *
 * Iris is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Iris is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * A copy of the GNU Lesser General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 * \section DESCRIPTION
 *
 * This Class wraps the Matlab engine - mainly used in the matlab component.
 */

#ifndef MATLAB_H_
#define MATLAB_H_

#include "engine.h"

class Matlab 
{
public:
	Matlab()
	{
	  pEng_=NULL;
	}
	
	virtual ~Matlab()
	{
		if (pEng_!=NULL)
			close();
	}
	
	bool open(const char* StartCmd)
	{
	  pEng_=engOpen(StartCmd);
		return (pEng_ != NULL);
	}
	
	int close()
	{
		int Result=engClose(pEng_);
		if (Result==0)	
		  pEng_=NULL;

		return Result;
	}
	
	int evalString(const char* string)
	{
		return (engEvalString(pEng_, string));
	}
	
	mxArray* getVariable(const char* name)
	{
		return (engGetVariable(pEng_, name));
	}
	
	int getVisible(bool* value)
	{
		return (engGetVisible(pEng_, value));
	}
	
	void openSingleUse(const char *startcmd, void *dcom, int *retstatus)
	{
	  pEng_=engOpenSingleUse(startcmd, dcom, retstatus);
	}
	
	int outputBuffer(char *p, int n)
	{
		return (engOutputBuffer(pEng_, p, n));
	}
	
	int setVisible(bool value)
	{
		return (engSetVisible(pEng_, value));
	}
	
	int putVariable(const char *name, const mxArray *mp)
	{
		return (engPutVariable(pEng_, name, mp));
	}

protected:
	Engine* pEng_;
	
}; /* class Matlab */

#endif /* MATLAB_H_ */
