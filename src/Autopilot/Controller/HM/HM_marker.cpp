#include "HM_marker.h"

namespace kai
{

HM_marker::HM_marker()
{
	m_pHM = NULL;
	m_pDet = NULL;
	m_iMarkerClass = -1;
	m_minSize = 0.0;
	m_maxSize = 0.5;
	m_rpmSteer = 0;
	m_obj.init();

}

HM_marker::~HM_marker()
{
}

bool HM_marker::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,rpmSteer);
	KISSm(pK,iMarkerClass);
	KISSm(pK,minSize);
	KISSm(pK,maxSize);

	return true;
}

bool HM_marker::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("HM_base", &iName));
	m_pHM = (HM_base*) (pK->parent()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void HM_marker::update(void)
{
	this->ActionBase::update();

	NULL_(m_pHM);
	NULL_(m_pDet);
	IF_(!isActive());
	IF_(m_myPriority < m_pHM->m_priority);

	//standby until Detector is ready
	if(!m_pDet->bReady())
	{
		m_pHM->m_rpmL = 0;
		m_pHM->m_rpmR = 0;
		return;
	}

	//TOOD: if turned angle > desired angle

	//if found new marker, set angle and start to turn
	m_obj.init();
	for(int i=0; i<m_pDet->size(); i++)
	{
		OBJECT* pO = m_pDet->at(i);
		IF_CONT(!pO->bClass(m_iMarkerClass));
		IF_CONT(pO->m_fBBox.area() < m_minSize);
		IF_CONT(pO->m_fBBox.area() > m_maxSize);

		m_obj = *pO;
		m_rpmSteer = abs(m_rpmSteer);
		m_pHM->m_rpmL = m_rpmSteer;
		m_pHM->m_rpmR = -m_rpmSteer;
		m_pHM->m_bSpeaker = true;
		return;
	}

}

bool HM_marker::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	//draw messages
	string msg;
	if (isActive())
		msg = "* ";
	else
		msg = "- ";
	msg += *this->getName();
	pWin->addMsg(&msg);

	IF_T(!m_obj.bClass(m_iMarkerClass));

	Rect r;
	vInt42rect(&m_obj.m_bbox, &r);
	rectangle(*pMat, r, Scalar(0, 255, 255), 10);

	return true;
}

}
