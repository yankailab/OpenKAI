/*
 * DetectorBase.cpp
 *
 *  Created on: Nov 27, 2015
 *      Author: root
 */

#include "DetectorBase.h"

namespace kai
{
DetectorBase::DetectorBase() {
	// TODO Auto-generated constructor stub

	m_pFrame = new CamFrame();
	m_pGray = new CamFrame();

}

DetectorBase::~DetectorBase() {
	// TODO Auto-generated destructor stub
}

}
