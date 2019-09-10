#include "_SortingCtrlClient.h"

namespace kai
{
_SortingCtrlClient::_SortingCtrlClient()
{
	m_bDrag = false;
	m_vDragFrom.init();
	m_vDragTo.init();

	m_bbWin.init();
	m_bbWin.z = 1.0;
	m_bbWin.w = 1.0;

	m_COO.init();

	m_bStop = false;
	m_tLastSent = 0;
	m_tSendInt = 300000;
}

_SortingCtrlClient::~_SortingCtrlClient()
{
}

bool _SortingCtrlClient::init(void *pKiss)
{
	IF_F(!this->_OKlink::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("tSendInt",&m_tSendInt);
	pK->v("bbWin",&m_bbWin);

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

int _SortingCtrlClient::check(void)
{
	return 0;
}

void _SortingCtrlClient::update(void)
{
	while (m_bThreadON)
	{
		if (!m_pIO)
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		if (!m_pIO->isOpen())
		{
			this->sleepTime(USEC_1SEC);
			continue;
		}

		this->autoFPSfrom();

		while (readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		if(m_COO.m_id >= 0)
		{
			if(m_tStamp - m_tLastSent > m_tSendInt)
				this->sendBB(m_COO.m_id, m_COO.m_topClass, m_COO.m_bb);
		}

		this->autoFPSto();
	}
}

void _SortingCtrlClient::handleCMD(void)
{
	IF_(m_recvMsg.m_pBuf[1] != OKLINK_BB);

	int id = unpack_uint32(&m_recvMsg.m_pBuf[3], false);
	if(id != m_COO.m_id)
	{
		m_COO.m_id = id;
		m_COO.m_topClass = unpack_int16(&m_recvMsg.m_pBuf[7], false);
		m_COO.m_bb.x = ((float) unpack_uint16(&m_recvMsg.m_pBuf[9], false)) * 0.001;
		m_COO.m_bb.y = ((float) unpack_uint16(&m_recvMsg.m_pBuf[11], false)) * 0.001;
		m_COO.m_bb.z = ((float) unpack_uint16(&m_recvMsg.m_pBuf[13], false)) * 0.001;
		m_COO.m_bb.w = ((float) unpack_uint16(&m_recvMsg.m_pBuf[15], false)) * 0.001;

		updateWindow();
	}

	m_recvMsg.reset();
}

void _SortingCtrlClient::onBtn(int id, int state)
{
	IF_(state != BTN_DOWN);

	m_COO.m_topClass = id;

	updateWindow();
	this->sendBB(m_COO.m_id, m_COO.m_topClass, m_COO.m_bb);
}

void _SortingCtrlClient::updateWindow(void)
{
	Window *pWin = (Window*) this->m_pWindow;

	pWin->resetAllBtn();
	WINDOW_BUTTON* pB;

	pB = pWin->getBtn(m_COO.m_topClass);
	if(pB)
		pB->setShownDown(true);

	if(m_bStop)
	{
		pB = pWin->getBtn(7);
		if(pB)
			pB->setEnable(true);

		pB = pWin->getBtn(8);
		if(pB)
			pB->setEnable(false);

	}
	else
	{
		pB = pWin->getBtn(7);
		if(pB)
			pB->setEnable(true);

		pB = pWin->getBtn(8);
		if(pB)
			pB->setEnable(false);

	}
}

void _SortingCtrlClient::onMouse(int event, float x, float y)
{
	IF_(x < m_bbWin.x);
	IF_(y < m_bbWin.y);
	IF_(x > m_bbWin.z);
	IF_(y > m_bbWin.w);

	m_vDragTo.x = x;
	m_vDragTo.y = y;

	if(event == EVENT_MOUSEMOVE)
	{
		IF_(!m_bDrag);
	}
	else if(event == EVENT_LBUTTONDOWN)
	{
		m_vDragFrom.x = x;
		m_vDragFrom.y = y;
		m_vDragTo = m_vDragFrom;
		m_bDrag = true;
	}
	else if(event == EVENT_LBUTTONUP)
	{
		m_bDrag = false;

		//call send
	}

	vFloat4 bb;
	m_COO.m_bb.x = small(m_vDragFrom.x, m_vDragTo.x);
	m_COO.m_bb.y = small(m_vDragFrom.y, m_vDragTo.y);
	m_COO.m_bb.z = big(m_vDragFrom.x, m_vDragTo.x);
	m_COO.m_bb.w = big(m_vDragFrom.y, m_vDragTo.y);

	this->sendBB(m_COO.m_id, m_COO.m_topClass, m_COO.m_bb);
}

bool _SortingCtrlClient::draw(void)
{
	IF_F(!this->_OKlink::draw());
	Window *pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Rect r = convertBB<vInt4>(convertBB(m_COO.m_bb, cs));
	rectangle(*pMat, r, Scalar(0,255,0), 5);

	putText(*pMat, i2str(m_COO.m_topClass), Point(cs.x/2, 50), FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,255,0), 2);

	string msg;
	pWin->tabNext();
	pWin->tabPrev();

	return true;
}

bool _SortingCtrlClient::console(int &iY)
{
	IF_F(!this->_OKlink::console(iY));
	string msg;

	return true;
}

}
