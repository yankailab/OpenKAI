#include "VEK_base.h"

namespace kai
{

VEK_base::VEK_base()
{
	m_pVEK = NULL;
	m_pCMD = NULL;

	m_pwmU = 2000;
	m_pwmN = 1500;
	m_pwmL = 1000;
	m_vL = 0.0;
	m_vR = 0.0;

	m_dT.init();
	m_dRot.init();
}

VEK_base::~VEK_base()
{
}

bool VEK_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,pwmU);
	KISSm(pK,pwmN);
	KISSm(pK,pwmL);

	Kiss* pI = pK->o("cmd");
	IF_T(pI->empty());
	m_pCMD = new _TCP();
	F_ERROR_F(m_pCMD->init(pI));

	return true;
}

bool VEK_base::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pVEK = (_IOBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void VEK_base::update(void)
{
	this->ActionBase::update();
	NULL_(m_pAM);
	NULL_(m_pCMD);

	string* pStateName = m_pAM->getCurrentStateName();
	if(*pStateName == "VEK_STANDBY")
	{
		m_vL = 0;
		m_vR = 0;
	}
	else
	{
		m_vL = 1.0;
		m_vR = 1.0;
	}

	cmd();
}

void VEK_base::cmd(void)
{
	if (!m_pCMD->isOpen())
	{
		m_pCMD->open();
		return;
	}

}

bool VEK_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName()
			+ ": vL=" + i2str(m_vL)
			+ ", vR=" + i2str(m_vR);
	pWin->addMsg(&msg);

	return true;
}



}
