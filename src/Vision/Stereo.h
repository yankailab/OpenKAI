/*
 * Stereo.h
 *
 *  Created on: Aug 25, 2015
 *      Author: yankai
 */

#ifndef SRC_VISION_STEREO_H_
#define SRC_VISION_STEREO_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "Frame.h"
#include "_VisionBase.h"

#ifdef USE_CUDASTEREO

namespace kai
{

class Stereo
{
public:
	Stereo();
	virtual ~Stereo();

	bool init(int disparity);
	void detect(Frame* pLeft, Frame* pRight, Frame* pDepth);
	void detect(Frame* pLRsbs, Frame* pDepth);

public:
    int		m_disparity;

#ifndef USE_OPENCV4TEGRA
    Ptr<cuda::StereoBM> m_pBM;
    Ptr<cuda::StereoBeliefPropagation> m_pBP;
    Ptr<cuda::StereoConstantSpaceBP> m_pCSBP;
#else

#endif

};

}

#endif
#endif
