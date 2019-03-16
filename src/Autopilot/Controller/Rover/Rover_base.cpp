#include "Rover_base.h"

namespace kai
{

Rover_base::Rover_base()
{
	m_pCMD = NULL;
	m_hdg = 0.0;
	m_speed = 0.0;
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
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_RoverCMD*) (pK->root()->getChildInst(iName));

	return true;
}

int Rover_base::check(void)
{
	NULL__(m_pCMD, -1);

	return 0;
}

void Rover_base::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

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
			+ ": hdg=" + f2str(m_hdg)
			+ ", speed=" + f2str(m_speed);
	pWin->addMsg(msg);

	return true;
}

}
