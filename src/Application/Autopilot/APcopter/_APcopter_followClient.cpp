#include "../../../Startup/Startup.h"
#include "_APcopter_followClient.h"

namespace kai
{

_APcopter_followClient::_APcopter_followClient()
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

_APcopter_followClient::~_APcopter_followClient()
{
}

bool _APcopter_followClient::init(void* pKiss)
{
	IF_F(!this->_ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("diff", &m_diff);
	pK->v("dAlt", &m_dAlt);
	pK->v("dHdg", &m_dHdg);
	pK->v("bbSize", &m_bbSize);

	pK->v("tIntSend", &m_ieSend.m_tInterval);

	Startup* pS = (Startup*)pK->root()->o("APP")->m_pInst;
	pS->addKeyCallback(callbackKey,this);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_APcopter_link", &iName));
	m_pAL = (_APcopter_link*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pAL, iName+": not found");

	return true;
}

bool _APcopter_followClient::start(void)
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

int _APcopter_followClient::check(void)
{
	NULL__(m_pAL,-1);

	return this->_ActionBase::check();
}

void _APcopter_followClient::update(void)
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

void _APcopter_followClient::updateBB(void)
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

void _APcopter_followClient::updateAlt(void)
{
	IF_(check()<0);

	IF_(m_alt > -m_dAlt);
	IF_(m_alt < m_dAlt);

	m_pAL->setAlt(m_alt);
	m_alt = 0.0;
}

void _APcopter_followClient::updateHdg(void)
{
	IF_(check()<0);

	IF_(m_hdg > -m_dHdg);
	IF_(m_hdg < m_dHdg);

	m_pAL->setHdg(m_hdg);
	m_hdg = 0.0;
}

void _APcopter_followClient::onKey(int key)
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

bool _APcopter_followClient::draw(void)
{
	IF_F(!this->_ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	int w = pMat->cols * m_bbSize;
	int h = pMat->rows * m_bbSize;

	//center bb indicator
	cv::rectangle(*pMat,
					Rect(pMat->cols/2-w,
						 pMat->rows/2-h,
						 w*2,
						 h*2),
					Scalar(0,255,255),
					1,
					LINE_4);

	//pointing bb indicator
	if(m_vBB.x >= 0.0)
	{
		cv::rectangle(*pMat,
						Rect(m_vBB.midX()*pMat->cols-w,
							 m_vBB.midY()*pMat->rows-h,
							 w*2,
							 h*2),
						Scalar(0,255,0),
						1,
						LINE_8);
	}


	pWin->tabNext();
	pWin->addMsg("bbPos = (" + f2str(m_vBB.midX()) + ", " + f2str(m_vBB.midY()) + ")");
	pWin->addMsg("alt = " + f2str(m_alt) + ", hdg = " + f2str(m_hdg));
	pWin->tabPrev();

	return true;
}

bool _APcopter_followClient::console(int& iY)
{
	IF_F(!this->_ActionBase::console(iY));

	string msg;
	C_MSG("bbPos = (" + f2str(m_vBB.midX()) + ", " + f2str(m_vBB.midY()) + ")");
	C_MSG("alt = " + f2str(m_alt) + ", hdg = " + f2str(m_hdg));

	return true;
}

}
