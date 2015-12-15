/*
 * DetectorBase.h
 *
 *  Created on: Nov 27, 2015
 *      Author: root
 */

#ifndef DETECTOR_DETECTORBASE_H_
#define DETECTOR_DETECTORBASE_H_

#include "../Camera/CamFrame.h"

namespace kai
{

class DetectorBase
{
public:
	DetectorBase();
	virtual ~DetectorBase();


public:
	CamFrame*	m_pFrame;
	CamFrame*	m_pGray;


};



}

#endif /* DETECTOR_DETECTORBASE_H_ */
