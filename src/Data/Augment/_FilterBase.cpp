/*
 * _FilterBase.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#include "_FilterBase.h"

namespace kai
{

_FilterBase::_FilterBase()
{
	m_nProduce = 1;
	m_progress = 0.0;
	m_dRand = 1.0;
	m_bComplete = false;

	m_noiseMean = 0;
	m_noiseDev = 0;
	m_noiseType = cv::RNG::NORMAL;
}

_FilterBase::~_FilterBase()
{
}

bool _FilterBase::init(void* pKiss)
{
	IF_F(!this->_DataBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dRand",&m_dRand);
	pK->v("nProduce",&m_nProduce);
	pK->v("noiseMean",&m_noiseMean);
	pK->v("noiseDev",&m_noiseDev);
	pK->v("noiseType",&m_noiseType);

	return true;
}

bool _FilterBase::start(void)
{
	m_bComplete = false;
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _FilterBase::update(void)
{
	srand(time(NULL));
}

bool _FilterBase::bComplete(void)
{
	return m_bComplete;
}

}

