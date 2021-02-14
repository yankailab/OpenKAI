#include "_SortingCtrlClient.h"

#ifdef USE_OPENCV

namespace kai
{

_SortingCtrlClient::_SortingCtrlClient()
{
	m_bMouse = false;
	m_vMouse.init();
	m_vBB.x = 0.05;
	m_vBB.y = 0.05;
	m_COO.init();
	m_vROI.init();
	m_vROI.z = 1.0;
	m_vROI.w = 1.0;

	m_tLastSent = 0;
	m_tSendInt = 300000;

	m_iState = SORT_STATE_OFF;
	m_iSetState = SORT_STATE_OFF;
}

_SortingCtrlClient::~_SortingCtrlClient()
{
}

bool _SortingCtrlClient::init(void *pKiss)
{
	IF_F(!this->_ProtocolBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("tSendInt",&m_tSendInt);
	pK->v("vROI",&m_vROI);

	Window *pWin = (Window*) this->m_pWindow;
	pWin->addCallbackMouse(callbackMouse, this);

	WINDOW_BUTTON* pBtn;
	for(int i=0; i<pWin->m_vBtn.size(); i++)
	{
		pBtn = pWin->getBtn(i);
		IF_CONT(!pBtn);

		pBtn->setBtnCallback(callbackBtn, this);
	}

	return true;
}

bool _SortingCtrlClient::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _SortingCtrlClient::check(void)
{
	return 0;
}

void _SortingCtrlClient::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_pIO)
		{
			m_pT->sleepT (USEC_1SEC);
			continue;
		}

		if (!m_pIO->isOpen())
		{
			m_pT->sleepT (USEC_1SEC);
			continue;
		}

		m_pT->autoFPSfrom();

		while (readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

//		if(m_COO.m_id >= 0)
//		{
//			if(m_tStamp - m_tLastSent > m_tSendInt)
//				this->sendBB(m_COO.m_id, m_COO.getTopClass(), m_COO.m_bb);
//		}

		m_pT->autoFPSto();
	}
}

void _SortingCtrlClient::handleCMD(void)
{
//	if(m_recvMsg.m_pBuf[1] == PROTOCOL_STATE)
//	{
//		m_iState = unpack_int32(&m_recvMsg.m_pBuf[3], false);
//	}

	m_recvMsg.reset();
}

void _SortingCtrlClient::onBtn(int id, int state)
{
	IF_(state != BTN_DOWN);

	if(id >= 0 && id <= 4)
	{
		m_COO.setTopClass(id, 1.0);
//		this->sendBB(m_COO.m_id, m_COO.getTopClass(), m_COO.m_bb);
	}
	else if(id == 5)
	{
		m_iSetState = SORT_STATE_OFF;

//		this->sendState(m_iSetState);
	}
	else if(id == 6)
	{
		m_iSetState = SORT_STATE_ON;

//		this->sendState(m_iSetState);
	}

	updateWindow();
}

void _SortingCtrlClient::updateWindow(void)
{
	Window *pWin = (Window*) this->m_pWindow;

	pWin->resetAllBtn();
	WINDOW_BUTTON* pB;

	pB = pWin->getBtn(m_COO.getTopClass());
	if(pB)
		pB->setShownDown(true);
}

void _SortingCtrlClient::onMouse(int event, float x, float y)
{
	m_vMouse.x = x;
	m_vMouse.y = y;

	if(event == EVENT_MOUSEMOVE)
	{
		IF_(!m_bMouse);
	}
	else if(event == EVENT_LBUTTONDOWN)
	{
		m_bMouse = true;
	}
	else if(event == EVENT_LBUTTONUP)
	{
		m_bMouse = false;
	}

	if(m_bMouse)
	{
		if(m_vMouse.x > m_vROI.x &&
			m_vMouse.x < m_vROI.z &&
			m_vMouse.y > m_vROI.y &&
			m_vMouse.y < m_vROI.w)
		{
			m_COO.setX(m_vMouse.x);
			m_COO.setY(m_vMouse.y);
			m_COO.setWidth(m_vBB.x * 2);
			m_COO.setHeight(m_vBB.y * 2);
		}
	}

//	this->sendBB(m_COO.m_id, m_COO.getTopClass(), m_COO.m_bb);
}

void _SortingCtrlClient::draw(void)
{
	this->_ProtocolBase::draw();

	IF_(!checkWindow());
	Mat* pM = ((Window*) this->m_pWindow)->getFrame()->m();

	circle(*pM, Point(m_COO.getX() * pM->cols, m_COO.getY() * pM->rows), 15, Scalar(0,255,0), 5);

	IF_(!m_bDebug);
}

}
#endif
