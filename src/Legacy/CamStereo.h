/*
 * CamStereo.h
 *
 *  Created on: Aug 25, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMSTEREO_H_
#define SRC_CAMSTEREO_H_

#include "../Base/common.h"
#include "../Stream/Frame.h"

namespace kai
{

class CamStereo
{
public:
	CamStereo();
	virtual ~CamStereo();


	bool init(int disparity);
	void detect(Frame* pLeft, Frame* pRight, Frame* pDepth);
	void detect(Frame* pLRsbs, Frame* pDepth);

public:
    int		m_disparity;

    Ptr<cuda::StereoBM> m_pBM;
    Ptr<cuda::StereoBeliefPropagation> m_pBP;
    Ptr<cuda::StereoConstantSpaceBP> m_pCSBP;

};

} /* namespace kai */

#endif /* SRC_CAMSTEREO_H_ */
