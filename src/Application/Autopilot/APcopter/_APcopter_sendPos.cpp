#include "_APcopter_sendPos.h"
#include "../../../Startup/Startup.h"

namespace kai
{

_APcopter_sendPos::_APcopter_sendPos()
{
	m_pAL = NULL;
	m_diff = 0.01;

	m_bbSize = 0.05;
	m_dBBsize = 0.025;
	m_vBBsize.x = 0.05;
	m_vBBsize.y = 0.25;
	m_vBB.init(-1.0);

	m_alt = 0.0;
	m_dAlt = 1.0;
	m_hdg = 0.0;
	m_dHdg = 1.0;

	m_ieSend.init(100000);
}

_APcopter_sendPos::~_APcopter_sendPos()
{
}

bool _APcopter_sendPos::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("dP", &m_diff);
	pK->v("dAlt", &m_dAlt);
	pK->v("tIntSend", &m_ieSend.m_tInterval);

	Startup* pS = (Startup*)pK->root()->o("APP")->m_pInst;
	pS->addKeyCallback(callbackKey,this);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_OKlinkAPcopter", &iName));
	m_pAL = (_APcopter_link*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pAL, iName+": not found");

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
	NULL__(m_pAL,-1);

	return this->_ActionBase::check();
}

void _APcopter_sendPos::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ActionBase::update();

		updateBB();
		updateAlt();
		updateHdg();

		this->autoFPSto();
	}
}

void _APcopter_sendPos::updateBB(void)
{
	IF_(check()<0);

	Window* pWin = (Window*) this->m_pWindow;
	NULL_(pWin);

	if(!pWin->bMouseButton(MOUSE_L))
	{
		m_vBB.init(-1.0);
		return;
	}

	vFloat4 vBB;
	vBB.x = pWin->m_vMouse.x - m_bbSize;
	vBB.y = pWin->m_vMouse.y - m_bbSize;
	vBB.z = pWin->m_vMouse.x + m_bbSize;
	vBB.w = pWin->m_vMouse.y + m_bbSize;

	IF_(!m_ieSend.update(m_tStamp) &&
			EAQ(vBB.x, m_vBB.x, m_diff) &&
			EAQ(vBB.y, m_vBB.y, m_diff) &&
			EAQ(vBB.z, m_vBB.z, m_diff) &&
			EAQ(vBB.w, m_vBB.w, m_diff)
			);

	m_pAL->setBB(vBB);
	m_vBB = vBB;
}

void _APcopter_sendPos::updateAlt(void)
{
	IF_(check()<0);

	IF_(m_alt > -m_dAlt);
	IF_(m_alt < m_dAlt);

	m_pAL->setAlt(m_alt);
	m_alt = 0.0;
}

void _APcopter_sendPos::updateHdg(void)
{
	IF_(check()<0);

	IF_(m_hdg > -m_dHdg);
	IF_(m_hdg < m_dHdg);

	m_pAL->setHdg(m_hdg);
	m_hdg = 0.0;
}

void _APcopter_sendPos::onKey(int key)
{
	switch (key)
	{
	case 'a':
		m_alt = -m_dAlt;
		break;
	case 'z':
		m_alt = m_dAlt;
		break;
	case 'q':
		m_hdg = -m_dHdg;
		break;
	case 'w':
		m_hdg = m_dHdg;
		break;
	case '1':
		m_bbSize = constrain(m_bbSize-m_dBBsize, m_vBBsize.x, m_vBBsize.y);
		break;
	case '2':
		m_bbSize = constrain(m_bbSize+m_dBBsize, m_vBBsize.x, m_vBBsize.y);
		break;
	}
}

bool _APcopter_sendPos::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	int w = pMat->cols * m_bbSize;
	int h = pMat->rows * m_bbSize;

//	pWin->addMsg("vP = (" + f2str(m_vPrevP.x) + ", "
//							   + f2str(m_vPrevP.y) + ", "
//					           + f2str(m_vPrevP.z) + ")");

	return true;
}

bool _APcopter_sendPos::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));

	string msg;
//	C_MSG("vP = (" + f2str(m_vPrevP.x) + ", "
//							   + f2str(m_vPrevP.y) + ", "
//					           + f2str(m_vPrevP.z) + ")");

	return true;
}

}
