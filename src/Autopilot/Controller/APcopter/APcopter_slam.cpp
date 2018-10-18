#include "APcopter_slam.h"

namespace kai
{

APcopter_slam::APcopter_slam()
{
	m_pAP = NULL;
	m_pIO = NULL;
	m_iCmd = 0;

	m_zTop = 50.0;
	m_vGPSorigin.init();
	m_vOrigin.init();
	m_vPos.init();
}

APcopter_slam::~APcopter_slam()
{
}

bool APcopter_slam::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,zTop);
	pK->v("originX", &m_vGPSorigin.x);
	pK->v("originY", &m_vGPSorigin.y);
	pK->v("originZ", &m_vGPSorigin.z);
	m_vOrigin.init();
	m_vPos = m_vGPSorigin;

	int n = 3;
	pK->v("nMedian", &n);
	m_fX.init(n, 0);
	m_fY.init(n, 0);
	m_fHdg.init(n, 0);

	//link
	string iName;

	iName = "";
	F_ERROR_F(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));
	IF_Fl(!m_pAP, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIO, iName + ": not found");

	return true;
}

int APcopter_slam::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pIO,-1);

	return 0;
}

void APcopter_slam::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	updatePos();

	uint32_t apMode = m_pAP->apMode();
	if(apMode == LOITER || apMode == AUTO)
	{
		if(m_pAP->bApModeChanged())
		{
			m_vOrigin = m_vPos;
		}
	}

//	vDouble4 vPos = m_vGPSorigin + (m_vPos - m_vOrigin);
	//TODO: convert LL and UTM

//	m_LL.m_hdg = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
//	setLL(&m_LL);
//
//	mavlink_gps_input_t D;
//	D.lat = m_LL.m_lat * 1e7;
//	D.lon = m_LL.m_lng * 1e7;
//	D.alt = m_LL.m_alt;
//	D.gps_id = 0;
//	D.fix_type = 3;
//	D.satellites_visible = m_nSat;
//	D.ignore_flags = 0b11111111;
//	m_pMavlink->gpsInput(D);

}

void APcopter_slam::updatePos(void)
{
	uint8_t	pBuf[N_IO_BUF];
	int nRead = m_pIO->read(pBuf, N_IO_BUF);
	IF_(nRead <= 0);

	for(int i=0; i<nRead; i++)
	{
		if(m_iCmd == 0)
		{
			IF_CONT(pBuf[i] != 0xff);
		}

		m_pCmd[m_iCmd] = pBuf[i];
		m_iCmd++;

		if(m_iCmd >= MG_PACKET_N)
		{
			//decode one packet
			m_fX.input((double)makeINT32(&m_pCmd[1]) * 0.001);
			m_fY.input((double)makeINT32(&m_pCmd[5]) * 0.001);
			m_fHdg.input((double)makeINT32(&m_pCmd[13]) * 0.001);
			m_iCmd = 0;
		}
	}
}

bool APcopter_slam::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!isActive())
	{
		msg += "Inactive";
	}
	else
	{
		circle(*pMat, Point(m_vPos.x * pMat->cols,
							0.5 * pMat->rows),
							pMat->cols * pMat->rows * 0.00002, Scalar(0, 0, 255), 2);

		msg += "Edge pos = (" +
				f2str(m_vPos.x) + ", " +
				f2str(m_vPos.y) + ", " +
				f2str(m_vPos.z) + ", " +
				f2str(m_vPos.w) + ")";
	}

	pWin->addMsg(&msg);

	return true;
}

bool APcopter_slam::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));
	IF_F(check()<0);

	string msg;

	if(!isActive())
	{
		msg = "Inactive";
	}
	else
	{
		msg = "Edge pos = (" +
				f2str(m_vPos.x) + ", " +
				f2str(m_vPos.y) + ", " +
				f2str(m_vPos.z) + ", " +
				f2str(m_vPos.w) + ")";
	}

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
