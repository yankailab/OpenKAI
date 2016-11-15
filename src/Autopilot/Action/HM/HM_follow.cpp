#include "HM_follow.h"

namespace kai
{

HM_follow::HM_follow()
{
	ActionBase();

	m_pHM = NULL;
	m_pAM = NULL;
	m_pUniv = NULL;

}

HM_follow::~HM_follow()
{
}

bool HM_follow::init(Config* pConfig)
{
	CHECK_F(this->ActionBase::init(pConfig) == false)
	pConfig->m_pInst = this;

	return true;
}

bool HM_follow::link(void)
{
	NULL_F(m_pConfig);
	string iName = "";

	F_INFO(m_pConfig->v("HM_base", &iName));
	m_pHM = (HM_base*) (m_pConfig->parent()->getChildInstByName(&iName));

	F_INFO(m_pConfig->v("_Automaton", &iName));
	m_pAM = (_Automaton*) (m_pConfig->root()->getChildInstByName(&iName));

	F_INFO(m_pConfig->v("_Universe", &iName));
	m_pUniv = (_Universe*) (m_pConfig->root()->getChildInstByName(&iName));

	return true;
}

void HM_follow::update(void)
{
	this->ActionBase::update();

	updateMotion();
}

void HM_follow::updateMotion(void)
{
	NULL_(m_pHM);

	//TODO: get visual and decide motion



	m_pHM->updateCAN();
}


bool HM_follow::draw(Frame* pFrame, vInt4* pTextPos)
{
	NULL_F(pFrame);
	Mat* pMat = pFrame->getCMat();

	putText(*pMat, "HM", cv::Point(pTextPos->m_x, pTextPos->m_y),
			FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);

	pTextPos->m_y += pTextPos->m_w;

	return true;
}

}
