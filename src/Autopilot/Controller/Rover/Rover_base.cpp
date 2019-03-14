#include "Rover_base.h"

namespace kai
{

Rover_base::Rover_base()
{
	m_pA = NULL;
	m_hdg = 0.0;
	m_speed = 0.0;
	m_rMode = rover_unknown;
}

Rover_base::~Rover_base()
{
}

bool Rover_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	//link
	string iName;
	iName = "";
	F_ERROR_F(pK->v("_Arduino", &iName));
	m_pA = (_Arduino*) (pK->root()->getChildInst(iName));

	return true;
}

int Rover_base::check(void)
{
	NULL__(m_pA, -1);

	return 0;
}

void Rover_base::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	m_rMode = (ROVER_MODE)m_pA->m_pState[0];


}

void Rover_base::cmd(void)
{
}

bool Rover_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	NULL_F(pMat);
	IF_F(pMat->empty());

	string msg = *this->getName()
			+ ": mode=" + i2str(m_rMode)
			+ ", hdg=" + f2str(m_hdg)
			+ ", speed=" + f2str(m_speed);
	pWin->addMsg(msg);

	return true;
}

}
