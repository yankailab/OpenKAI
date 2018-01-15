#include "APcopter_distLidar.h"

namespace kai
{

APcopter_distLidar::APcopter_distLidar()
{
	m_pAP = NULL;
	m_nLidar = 0;
}

APcopter_distLidar::~APcopter_distLidar()
{
}

bool APcopter_distLidar::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_distLidar::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	Kiss** pItrDS = pK->getChildItr();
	m_nLidar = 0;

	while (pItrDS[m_nLidar])
	{
		Kiss* pKds = pItrDS[m_nLidar];
		IF_F(m_nLidar >= N_LIDAR);

		DIST_LIDAR* pLidar = &m_pLidar[m_nLidar];
		pLidar->init();
		F_ERROR_F(pKds->v("_DistSensorBase", &iName));
		pLidar->m_pDS = (_DistSensorBase*) (pK->root()->getChildInstByName(&iName));
		if(!pLidar->m_pDS)
		{
			LOG_E(iName << " not found");
			return false;
		}

		F_ERROR_F(pKds->v("orientation", (int*)&pLidar->m_orientation));
		m_nLidar++;
	}

	return true;
}

void APcopter_distLidar::update(void)
{
	this->ActionBase::update();

	updateMavlink();
}

void APcopter_distLidar::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	int i, j;
	for (i = 0; i < m_nLidar; i++)
	{
		DIST_LIDAR* pDS = &m_pLidar[i];
		double rMin = pDS->m_pDS->rMin() * 100;
		double rMax = pDS->m_pDS->rMax() * 100;

		//forward
		double degFrom = 360.0 - 22.5;
		double degTo = 360.0 + 22.5;
		double d = pDS->m_pDS->dMin(degFrom, degTo);
		if(d >= 0.0)
		{
			pMavlink->distanceSensor(
					0, //type
					0,	//orientation
					rMax, rMin,
					d * 100.0);
		}

		degFrom = 22.5;
		degTo = degFrom + 45.0;
		for (j = 0; j < 7; j++)
		{
			d = pDS->m_pDS->dMin(degFrom, degTo);
			if(d >= 0.0)
			{
				pMavlink->distanceSensor(
						0, //type
						j,	//orientation
						rMax, rMin,
						d * 100.0);
			}

			degFrom += 45.0;
			degTo = degFrom + 45.0;
		}
	}
}


bool APcopter_distLidar::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	return true;
}

}

