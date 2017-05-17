/*
 * _DNNSLAM.h
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_SLAM__DNNSLAM_H_
#define OPENKAI_SRC_SLAM__DNNSLAM_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../DNN/_ImageNet.h"

namespace kai
{

class _DNNSLAM: public _ThreadBase
{
public:
	_DNNSLAM(void);
	virtual ~_DNNSLAM();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNSLAM *) This)->update();
		return NULL;
	}

public:
	_ImageNet* m_pIN;

	vDouble3	m_attiRad;	//yaw, pitch, roll, heading

	uint64_t	m_tNow;
};

}

#endif
