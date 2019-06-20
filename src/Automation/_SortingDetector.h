/*
 * _SortingDetector.h
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Automation__SortingDetector_H_
#define OpenKAI_src_Automation__SortingDetector_H_

#include "../Base/common.h"
#include "../Detector/_DetectorBase.h"

namespace kai
{

class _SortingDetector: public _DetectorBase
{
public:
	_SortingDetector(void);
	virtual ~_SortingDetector();

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
		((_SortingDetector *) This)->update();
		return NULL;
	}

public:
	deque<OBJECT> m_vTarget;

	float m_cSpeed; //conveyer speed
	float m_cLen;	//conveyer length
	float m_minOverlap;
};

}
#endif
