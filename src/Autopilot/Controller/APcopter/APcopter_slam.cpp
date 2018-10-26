#include "APcopter_slam.h"

namespace kai
{

APcopter_slam::APcopter_slam()
{
	m_pAP = NULL;
	m_pIO = NULL;
	m_iCmd = 0;

	m_gpsID = 0;
	m_iFixType = 3;
	m_nSat = 10;
	m_hdop = 0;
	m_vdop = 0;

	m_zTop = 50.0;
	m_vGPSorigin.init();
	m_vSlamOrigin.init();
	m_vSlamPos.init();
}

APcopter_slam::~APcopter_slam()
{
}

bool APcopter_slam::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,zTop);
	pK->v("lat", &m_vGPSorigin.m_lat);
	pK->v("lng", &m_vGPSorigin.m_lng);
	m_vSlamOrigin.init();
	m_vSlamPos.init();
	m_GPS.update(m_vGPSorigin);

	KISSm(pK,gpsID);
	KISSm(pK,iFixType);
	KISSm(pK,nSat);
	KISSm(pK,hdop);
	KISSm(pK,vdop);

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
			m_vSlamOrigin = m_vSlamPos;
		}
	}

	m_GPS.m_UTM.m_hdg = ((double)m_pAP->m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
	m_GPS.m_UTM.m_altRel = ((double)m_pAP->m_pMavlink->m_msg.global_position_int.relative_alt) * 0.01;

	vDouble3 dPos = m_vSlamPos - m_vSlamOrigin;
	UTM_POS pUTM = m_GPS.getPos(dPos);
	LL_POS pLL = m_GPS.UTM2LL(pUTM);

	mavlink_gps_input_t D;
	D.lat = pLL.m_lat * 1e7;
	D.lon = pLL.m_lng * 1e7;
	D.alt = (float)pLL.m_altRel;
	D.gps_id = m_gpsID;
	D.fix_type = m_iFixType;
	D.satellites_visible = m_nSat;
	D.hdop = (float)m_hdop;
	D.vdop = (float)m_vdop;
	D.ignore_flags = 0b11111111;
	m_pAP->m_pMavlink->gpsInput(D);

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
			m_fX.input(((double)makeINT32(&m_pCmd[1], false)) * 0.001);
			m_fY.input(((double)makeINT32(&m_pCmd[5], false)) * 0.001);
			m_fHdg.input(((double)makeINT32(&m_pCmd[13], false)) * 0.001);
			m_iCmd = 0;

			m_vSlamPos.x = m_fY.v();
			m_vSlamPos.y = -m_fX.v();
			m_vSlamPos.z = 0;
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
		pWin->addMsg(&msg);
		msg = "";
	}

	circle(*pMat, Point(m_vSlamPos.x * pMat->cols,
						0.5 * pMat->rows),
						pMat->cols * pMat->rows * 0.00002, Scalar(0, 0, 255), 2);

	msg += "SLAM pos = (" +
			f2str(m_vSlamPos.x) + ", " +
			f2str(m_vSlamPos.y) + ", " +
			f2str(m_vSlamPos.z) + ")";

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
		COL_MSG;
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
		msg = "";
	}

	msg += "SLAM pos = (" +
			f2str(m_vSlamPos.x) + ", " +
			f2str(m_vSlamPos.y) + ", " +
			f2str(m_vSlamPos.z) + ")";

	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
