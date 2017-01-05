#include "APMcopter_avoid.h"

namespace kai
{

APMcopter_avoid::APMcopter_avoid()
{
	ActionBase();

	m_pSF40 = NULL;
	m_pAPM = NULL;
	m_pZED = NULL;
}

APMcopter_avoid::~APMcopter_avoid()
{
}

bool APMcopter_avoid::init(void* pKiss)
{
	CHECK_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	return true;
}

bool APMcopter_avoid::link(void)
{
	CHECK_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";

	F_INFO(pK->v("APMcopter_base", &iName));
	m_pAPM = (APMcopter_base*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	F_INFO(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	//Add GPS

	return true;
}

void APMcopter_avoid::update(void)
{
	this->ActionBase::update();

	NULL_(m_pAPM);

	updateDistanceSensor();
}

void APMcopter_avoid::updateDistanceSensor(void)
{
	NULL_(m_pAPM->m_pMavlink);

	double dist = -1;

	if(m_pSF40)
	{
		//TODO: update forward value from SF40
	}

	if(m_pZED)
	{
		Object* pObj = m_pZED->getObject();
		if(pObj)
		{
			uint64_t frameID = get_time_usec()-1000000;

			//get the closest object
			for(int i=0;i<pObj->size();i++)
			{
				OBJECT* pO = pObj->get(i,frameID);
				if(!pO)continue;

				if(pO->m_dist>0.0)
				{
					if(pO->m_dist < dist)
						dist = pO->m_dist;
				}
			}
		}
	}

	CHECK_(dist>0.0);

	m_DS.m_distance = dist;
	m_DS.m_maxDistance = m_pZED->m_zedMaxDist*0.1;
	m_DS.m_minDistance = m_pZED->m_zedMinDist*0.1;
	m_DS.m_orientation = m_pZED->m_orientation;
	m_DS.m_type = 0;
	m_pAPM->updateDistanceSensor(&m_DS);
}

bool APMcopter_avoid::draw(void)
{
	CHECK_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();

	if(m_pAPM)
	{
		putText(*pMat, *this->getName()+" Dist="+i2str(m_DS.m_distance),
				*pWin->getTextPos(), FONT_HERSHEY_SIMPLEX, 0.5,
				Scalar(0, 255, 0), 1);
		pWin->lineNext();
	}

	return true;
}


}
