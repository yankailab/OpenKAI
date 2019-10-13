/*
 * _DepthSegment.h
 *
 *  Created on: May 29, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__DepthSegment_H_
#define OpenKAI_src_Detector__DepthSegment_H_

#include "../Base/common.h"
#include "_DetectorBase.h"
#include "../Vision/_DepthVisionBase.h"

namespace kai
{

class _DepthSegment : public _DetectorBase
{
public:
	_DepthSegment();
	virtual ~_DepthSegment();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DepthSegment*) This)->update();
		return NULL;
	}

public:
	Mat		m_mR;
	float	m_rL;
	float	m_rH;
	float	m_rD;
	float	m_rArea;

};
}

#endif
