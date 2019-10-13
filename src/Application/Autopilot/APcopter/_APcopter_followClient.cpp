#include "../../../Startup/Startup.h"
#include "_APcopter_followClient.h"

namespace kai
{

_APcopter_followClient::_APcopter_followClient()
{
	m_pAL = NULL;
	m_iSetState = APFOLLOW_OFF;
	m_diff = 0.01;

	m_bbSize = 0.05;
	m_dBBsize = 0.01;
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

bool _APcopter_followClient::init(void *pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("diff", &m_diff);
	pK->v("dAlt", &m_dAlt);
	pK->v("dHdg", &m_dHdg);
	pK->v("bbSize", &m_bbSize);
	pK->v("dBBsize", &m_dBBsize);
	pK->v("tIntSend", &m_ieSend.m_tInterval);

	Startup *pS = (Startup*) pK->root()->o("APP")->m_pInst;
	pS->addKeyCallback(callbackKey, this);

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_APcopter_link", &iName));
	m_pAL = (_APcopter_link*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pAL, iName + ": not found");

	return true;
}

bool _APcopter_followClient::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG(ERROR) << "Return code: " << retCode;
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _APcopter_followClient::check(void)
{
	NULL__(m_pAL, -1);

	return this->_AutopilotBase::check();
}

void _APcopter_followClient::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_AutopilotBase::update();

		updateBB();
		updateState();
		updateAlt();
		updateHdg();

		this->autoFPSto();
	}
}

void _APcopter_followClient::updateState(void)
{
	IF_(check() < 0);
	IF_(m_pAL->m_iState == m_iSetState);

	m_pAL->state(m_iSetState);
}

void _APcopter_followClient::updateBB(void)
{
	IF_(check() < 0);

	Window *pWin = (Window*) this->m_pWindow;
	NULL_(pWin);

	if (!pWin->bMouseButton(MOUSE_L))
	{
		m_vBB.init(-1.0);
		return;
	}

	m_iSetState = APFOLLOW_ON;

	vFloat4 vBB;
	vBB.x = pWin->m_vMouse.x - m_bbSize;
	vBB.y = pWin->m_vMouse.y - m_bbSize;
	vBB.z = pWin->m_vMouse.x + m_bbSize;
	vBB.w = pWin->m_vMouse.y + m_bbSize;

	IF_(
			!m_ieSend.update(m_tStamp) && EAQ(vBB.x, m_vBB.x, m_diff) && EAQ(vBB.y, m_vBB.y, m_diff) && EAQ(vBB.z, m_vBB.z, m_diff) && EAQ(vBB.w, m_vBB.w, m_diff));

	m_pAL->setBB(vBB);
	m_vBB = vBB;
}

void _APcopter_followClient::updateAlt(void)
{
	IF_(check() < 0);
	IF_(EAQ(m_alt,0.0,0.01));

	m_pAL->setAlt(m_alt);
	m_alt = 0.0;
}

void _APcopter_followClient::updateHdg(void)
{
	IF_(check() < 0);
	IF_(EAQ(m_hdg,0.0,0.01));

	m_pAL->setHdg(m_hdg);
	m_hdg = 0.0;
}

void _APcopter_followClient::onKey(int key)
{
	switch (key)
	{
	case 13:
		m_iSetState = APFOLLOW_OFF;
		break;
	case KEY_ARROW_UP:
		m_alt = -m_dAlt;
		break;
	case KEY_ARROW_DOWN:
		m_alt = m_dAlt;
		break;
	case KEY_ARROW_LEFT:
		m_hdg = -m_dHdg;
		break;
	case KEY_ARROW_RIGHT:
		m_hdg = m_dHdg;
		break;
	case 44:	//<
		m_bbSize = constrain(m_bbSize - m_dBBsize, m_vBBsize.x, m_vBBsize.y);
		break;
	case 46:	//>
		m_bbSize = constrain(m_bbSize + m_dBBsize, m_vBBsize.x, m_vBBsize.y);
		break;
	}
}

void _APcopter_followClient::draw(void)
{
	this->_AutopilotBase::draw();

	addMsg("iState = " + i2str(m_pAL->m_iState), 1);
	addMsg(	"detector = (" + f2str(m_vBB.midX()) + ", " + f2str(m_vBB.midY())
					+ ")", 1);
	addMsg(	"track = (" + f2str(m_pAL->m_vBB.midX()) + ", "
					+ f2str(m_pAL->m_vBB.midY()) + ")", 1);
	addMsg(	"target = (" + f2str(m_pAL->m_vTargetBB.midX()) + ", "
					+ f2str(m_pAL->m_vTargetBB.midY()) + ")", 1);
	addMsg("alt = " + f2str(m_alt) + ", hdg = " + f2str(m_hdg), 1);

	IF_(!checkWindow());

	Window *pWin = (Window*) this->m_pWindow;
	Mat *pMat = pWin->getFrame()->m();

	int w = pMat->cols * m_bbSize;
	int h = pMat->rows * m_bbSize;

	//center bb
	rectangle(*pMat,
			Rect(pMat->cols * 0.5 - w, pMat->rows * 0.5 - h, w * 2, h * 2),
			Scalar(255, 255, 255), 1);

	//mouse bb
	if (m_vBB.x >= 0.0)
	{
		rectangle(*pMat,
				Rect(m_vBB.midX() * pMat->cols - w,
						m_vBB.midY() * pMat->rows - h, w * 2, h * 2),
				Scalar(0, 255, 0), 1);
	}

	if (m_tStamp - m_pAL->m_tBB < USEC_1SEC)
	{
		//tracking bb
		rectangle(*pMat,
				Rect(m_pAL->m_vBB.x * pMat->cols, m_pAL->m_vBB.y * pMat->rows,
						m_pAL->m_vBB.width() * pMat->cols,
						m_pAL->m_vBB.height() * pMat->rows),
				Scalar(0, 255, 255), 1);

		line(*pMat,
				Point(m_pAL->m_vTargetBB.midX() * pMat->cols,
						m_pAL->m_vTargetBB.midY() * pMat->rows),
				Point(m_pAL->m_vBB.midX() * pMat->cols,
						m_pAL->m_vBB.midY() * pMat->rows), Scalar(0, 0, 255));
	}

	if (m_tStamp - m_pAL->m_tTargetBB < USEC_1SEC)
	{
		//target bb
		rectangle(*pMat,
				Rect(m_pAL->m_vTargetBB.x * pMat->cols,
						m_pAL->m_vTargetBB.y * pMat->rows,
						m_pAL->m_vTargetBB.width() * pMat->cols,
						m_pAL->m_vTargetBB.height() * pMat->rows),
				Scalar(0, 0, 255), 1);

		line(*pMat, Point(pMat->cols * 0.5, pMat->rows * 0.5),
				Point(m_pAL->m_vTargetBB.midX() * pMat->cols,
						m_pAL->m_vTargetBB.midY() * pMat->rows),
				Scalar(255, 255, 255));
	}
}

}
