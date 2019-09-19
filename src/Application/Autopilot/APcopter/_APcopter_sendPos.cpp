#include "_APcopter_sendPos.h"
#include "../../../Startup/Startup.h"

namespace kai
{

_APcopter_sendPos::_APcopter_sendPos()
{
	m_pOK = NULL;
	m_dP = 0.01;
	m_vPrevP.init();
	m_tLastSent = 0;
	m_timeOut = 300000;
	m_dAlt = 0.0;
	m_key = -1;

}

_APcopter_sendPos::~_APcopter_sendPos()
{
}

bool _APcopter_sendPos::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dP", &m_dP);
	pK->v("timeOut", &m_timeOut);
	pK->v("dAlt", &m_dAlt);

	Startup* pS = (Startup*)pK->root()->o("APP")->m_pInst;
	pS->addKeyCallback(callbackBtn,this);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_OKlinkAPcopter", &iName));
	m_pOK = (_APcopter_link*) (pK->root()->getChildInst(iName));
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

	float dA = 0.0;
	if(m_key == 'a')dA = -m_dAlt;
	else if(m_key == 'z')dA = m_dAlt;

	vFloat3 vP;
	vP.x = pWin->m_vMouse.x;
	vP.y = pWin->m_vMouse.y;
	vP.z = dA;

	uint64_t dT = m_tStamp - m_tLastSent;

	IF_(dT < m_timeOut &&
			EAQ(vP.x, m_vPrevP.x, m_dP) &&
			EAQ(vP.y, m_vPrevP.y, m_dP) &&
			EAQ(vP.z, m_vPrevP.z, m_dP));

	m_pOK->setPos(vP);

	m_vPrevP = vP;
	m_tLastSent = m_tStamp;

}

void _APcopter_sendPos::onBtn(int key)
{
	m_key = key;
}

bool _APcopter_sendPos::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	pWin->addMsg("vP = (" + f2str(m_vPrevP.x) + ", "
							   + f2str(m_vPrevP.y) + ", "
					           + f2str(m_vPrevP.z) + ")");

	return true;
}

bool _APcopter_sendPos::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));

	string msg;
	C_MSG("vP = (" + f2str(m_vPrevP.x) + ", "
							   + f2str(m_vPrevP.y) + ", "
					           + f2str(m_vPrevP.z) + ")");

	return true;
}

}
