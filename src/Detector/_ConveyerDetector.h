/*
 * _ConveyerDetector.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__ConveyerDetector_H_
#define OpenKAI_src_Automation__ConveyerDetector_H_

#include "../Base/common.h"
#include "../Detector/_DetectorBase.h"

namespace kai
{

class _ConveyerDetector: public _DetectorBase
{
public:
	_ConveyerDetector(void);
	virtual ~_ConveyerDetector();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void updateTarget(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ConveyerDetector *) This)->update();
		return NULL;
	}

public:
	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length
	float m_minOverlap;
};

}
#endif
