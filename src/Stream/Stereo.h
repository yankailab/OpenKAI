/*
 * Stereo.h
 *
 *  Created on: Aug 25, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMERA_STEREO_H_
#define SRC_CAMERA_STEREO_H_

#include "../Base/common.h"
#include "../Script/Kiss.h"
#include "../Stream/Frame.h"
#include "_StreamBase.h"

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

#ifdef USE_OPENCV3
    Ptr<cuda::StereoBM> m_pBM;
    Ptr<cuda::StereoBeliefPropagation> m_pBP;
    Ptr<cuda::StereoConstantSpaceBP> m_pCSBP;
#elif defined USE_OPENCV4TEGRA

#endif

};

}

#endif
