/*
 * _RSdepth.cpp
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#include "_RSdepth.h"

namespace kai
{

_RSdepth::_RSdepth()
{
	m_type = vision_RSdepth;
	m_pV = NULL;
}

_RSdepth::~_RSdepth()
{
	close();
}

bool _RSdepth::init(void* pKiss)
{
	IF_F(!_VisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n;
	n = "";
	pK->v("_RealSense", &n );
	m_pV = (_RealSense*) (pK->getInst( n ));
	IF_Fl(!m_pV, n + ": not found");

	return true;
}

bool _RSdepth::open(void)
{
	NULL_F(m_pV);
	m_bOpen = m_pV->isOpened();

	return m_bOpen;
}

void _RSdepth::close(void)
{
	this->_VisionBase::close();
}

bool _RSdepth::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _RSdepth::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_bOpen)
			open();

		m_pT->autoFPSfrom();

		if(m_bOpen)
		{
			if(m_fIn.tStamp() < m_pV->Depth()->tStamp())
				filter();
		}

		m_pT->autoFPSto();
	}
}

void _RSdepth::filter(void)
{
	IF_(m_pV->DepthShow()->bEmpty());

	Mat mD = *m_pV->DepthShow()->m();
	m_fBGR.copy(mD);
}

}
