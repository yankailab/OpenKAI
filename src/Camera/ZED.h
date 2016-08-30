/*
 * ZED.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef SRC_ZED_H_
#define SRC_ZED_H_

#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/Frame.h"
#include "CamBase.h"

namespace kai
{

class ZED : public CamBase
{
public:
	ZED();
	virtual ~ZED();

	bool setup(JSON* pJson, string name);
	bool openCamera(void);
	GpuMat* readFrame(void);
	GpuMat* getDepthFrame(void);
	void release(void);

public:
	sl::zed::Camera* m_pZed;
	sl::zed::SENSING_MODE m_zedMode;
	int m_zedResolution;
	double m_zedMinDist;
	int m_zedFPS;

};

} /* namespace kai */

#endif /* SRC_CAMINPUT_H_ */
