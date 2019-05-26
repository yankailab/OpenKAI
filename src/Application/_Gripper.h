/*
 * _Gripper.h
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__Gripper_H_
#define OpenKAI_src_Application__Gripper_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Detector/_DetectorBase.h"

namespace kai
{

class _Gripper: public _ThreadBase
{
public:
	_Gripper(void);
	virtual ~_Gripper();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Gripper *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pDet;
	Frame m_fBGR;

};

}
#endif
