#include "_SortingImgClient.h"

namespace kai
{
_SortingImgClient::_SortingImgClient()
{
}

_SortingImgClient::~_SortingImgClient()
{
}

bool _SortingImgClient::init(void *pKiss)
{
	IF_F(!this->_OKlink::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	return true;
}

bool _SortingImgClient::start(void)
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

void _SortingImgClient::update(void)
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

		this->autoFPSto();
	}
}

void _SortingImgClient::handleCMD(void)
{
	IF_(m_recvMsg.m_pBuf[1] != OKLINK_BB);

	m_COO.m_id = unpack_uint32(&m_recvMsg.m_pBuf[3], false);
	m_COO.m_topClass = unpack_uint16(&m_recvMsg.m_pBuf[7], false);
	m_COO.m_bb.x = ((float) unpack_uint16(&m_recvMsg.m_pBuf[9], false)) * 0.001;
	m_COO.m_bb.y = ((float) unpack_uint16(&m_recvMsg.m_pBuf[11], false)) * 0.001;
	m_COO.m_bb.z = ((float) unpack_uint16(&m_recvMsg.m_pBuf[13], false)) * 0.001;
	m_COO.m_bb.w = ((float) unpack_uint16(&m_recvMsg.m_pBuf[15], false)) * 0.001;

	m_recvMsg.reset();
}

void _SortingImgClient::handleBtn(int iBtn, int state)
{
	IF_(state != BTN_DOWN);

	m_COO.m_topClass = iBtn;
//TODO
	this->sendBB(m_COO.m_id, m_COO.m_topClass, m_COO.m_bb);
}

bool _SortingImgClient::draw(void)
{
	IF_F(!this->_OKlink::draw());
	Window *pWin = (Window*) this->m_pWindow;

	string msg;
	pWin->tabNext();
	pWin->tabPrev();

	return true;
}

bool _SortingImgClient::console(int &iY)
{
	IF_F(!this->_OKlink::console(iY));
	string msg;

	return true;
}

}
