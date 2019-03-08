#include "Traffic_videoOut.h"

namespace kai
{

Traffic_videoOut::Traffic_videoOut()
{
	m_pTB = NULL;

	m_tStampOB = 0;
	m_tDraw = 0;
}

Traffic_videoOut::~Traffic_videoOut()
{
}

bool Traffic_videoOut::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	//link
	string iName;
	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInst(iName));

	return true;
}

int Traffic_videoOut::check(void)
{
	NULL__(m_pTB,-1);
	NULL__(m_pTB->m_pOB,-1);
	NULL__(m_pTB->m_pOB->m_pVision,-1);

	return this->ActionBase::check();
}

void Traffic_videoOut::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	_VisionBase* pV = m_pTB->m_pOB->m_pVision;
	pV->wakeUp();
	pV->goSleep();
	while(!pV->bSleeping());

	_DetectorBase* pOB = m_pTB->m_pOB;
	while(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;


}

bool Traffic_videoOut::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	IF_F(check()<0);

	//block until everything's complete for the frame
	while(m_tDraw >= m_tStamp);
	m_tDraw = m_tStamp;

	return true;
}

}
