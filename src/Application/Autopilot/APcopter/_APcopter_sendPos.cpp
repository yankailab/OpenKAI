#include "_APcopter_sendPos.h"

namespace kai
{

_APcopter_sendPos::_APcopter_sendPos()
{
	m_pOK = NULL;

}

_APcopter_sendPos::~_APcopter_sendPos()
{
}

bool _APcopter_sendPos::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_OKlinkAPcopter", &iName));
	m_pOK = (_OKlinkAPcopter*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pOK, iName+": not found");

	return true;
}

bool _APcopter_sendPos::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: "<< retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _APcopter_sendPos::check(void)
{
	NULL__(m_pOK,-1);

	return this->_ActionBase::check();
}

void _APcopter_sendPos::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ActionBase::update();
		updatePos();

		this->autoFPSto();
	}
}

void _APcopter_sendPos::updatePos(void)
{
	IF_(check()<0);

	Window* pWin = (Window*) this->m_pWindow;
	NULL_(pWin);
	IF_(!pWin->bMouseButton(MOUSE_L));

	m_pOK->setPos(pWin->m_vMouse);

}

bool _APcopter_sendPos::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	return true;
}

bool _APcopter_sendPos::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));
	IF_F(check()<0);

	return true;
}

}
