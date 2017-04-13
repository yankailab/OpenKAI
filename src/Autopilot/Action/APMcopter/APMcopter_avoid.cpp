#include "APMcopter_avoid.h"

namespace kai
{

APMcopter_avoid::APMcopter_avoid()
{
	m_pAPM = NULL;
	m_nDS = 0;
}

APMcopter_avoid::~APMcopter_avoid()
{
}

bool APMcopter_avoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	for(int i=0;i<N_DIR_MAV;i++)
	{
		m_dd[i].init();
	}

	return true;
}

bool APMcopter_avoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->parent()->getChildInstByName(&iName));

	Kiss** pItr = pK->getChildItr();
	m_nDS = 0;
	while (pItr[m_nDS])
	{
		Kiss* pS = pItr[m_nDS];
		IF_F(m_nDS >= N_DISTANCE_SENSOR);

		iName = "";
		F_INFO(pS->v("distSensor", &iName));
		m_pDS[m_nDS] = (_Obstacle*) (pK->root()->getChildInstByName(&iName));

		m_nDS++;
	}

	return true;
}

void APMcopter_avoid::update(void)
{
	this->ActionBase::update();

	updateSensor();
	updateMavlink();
}

void APMcopter_avoid::updateMavlink(void)
{
	NULL_(m_pAPM);
	NULL_(m_pAPM->m_pMavlink);
	_Mavlink* pMavlink = m_pAPM->m_pMavlink;

	int i;
	for(i=0;i<N_DIR_MAV;i++)
	{
		DIR_DIST* pDD = &m_dd[i];
		IF_CONT(pDD->m_dist<0.0);

		pMavlink->distance_sensor(0, //type
				i,	//orientation
				pDD->m_rMax,
				pDD->m_rMin,
				pDD->m_dist);
	}

}

void APMcopter_avoid::updateSensor(void)
{
	for(int i=0; i<m_nDS; i++)
	{
		DistanceSensorBase* pDS = m_pDS[i];
		DIR_DIST* pDD = &m_dd[pDS->orientation()];

		double d = pDS->d() * 100;
		IF_CONT(d<0.0);
		pDD->m_dist = d;

		vDouble2 r = pDS->range() * 100;
		pDD->m_rMin = r.x;
		pDD->m_rMax = r.y;
	}
}

bool APMcopter_avoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	string msg;

	msg = *this->getName() + " Forward obs dist=" + i2str(m_dd[MAV_SENSOR_ROTATION_NONE].m_dist);
	pWin->addMsg(&msg);

	return true;
}

}
