#include "APcopter_slam.h"

namespace kai
{

APcopter_slam::APcopter_slam()
{
	m_pAP = NULL;
	m_pIOr = NULL;
	m_pIOw = NULL;
	m_iCmd = 0;
	m_iSeq = 0;
	m_bAxisXY = true;
	m_kX = 1.0;
	m_kY = 1.0;

	m_gpsID = 0;
	m_iFixType = 3;
	m_nSat = 10;
	m_hdop = 0;
	m_vdop = 0;
	m_yawOffset = 0.0;
	m_utmPos.init();
	m_vVelo.init();

	m_bUseApOrigin = true;
	m_llGPSorigin.init();
	m_vSlamPos.init();
}

APcopter_slam::~APcopter_slam()
{
}

bool APcopter_slam::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("lat", &m_llGPSorigin.m_lat);
	pK->v("lng", &m_llGPSorigin.m_lng);
	m_utmGPSorigin = m_GPS.LL2UTM(m_llGPSorigin);
	m_vSlamPos.init();

	pK->v("gpsID",&m_gpsID);
	pK->v("iFixType",&m_iFixType);
	pK->v("nSat",&m_nSat);
	pK->v("hdop",&m_hdop);
	pK->v("vdop",&m_vdop);
	pK->v("yawOffset",&m_yawOffset);
	pK->v("bAxisXY",&m_bAxisXY);
	pK->v("kX",&m_kX);
	pK->v("kY",&m_kY);

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
	F_ERROR_F(pK->v("_IOBaseR", &iName));
	m_pIOr = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIOr, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_IOBaseW", &iName));
	m_pIOw = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIOw, iName + ": not found");

	return true;
}

int APcopter_slam::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pIOr,-1);
	NULL__(m_pIOw,-1);

	return 0;
}

void APcopter_slam::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	updatePos();
	sendState();

	if(m_bUseApOrigin)
	{
		vDouble3 vHome;
		IF_(!m_pAP->getHomePos(&vHome));

		m_llGPSorigin.m_lat = vHome.x;
		m_llGPSorigin.m_lng = vHome.y;
		m_utmGPSorigin = m_GPS.LL2UTM(m_llGPSorigin);
	}

	m_utmGPSorigin.m_hdg = m_yawOffset;
	//	m_GPS.m_UTM.m_hdg = ((double)m_pAP->m_pMavlink->m_msg.global_position_int.hdg) * 0.01;
	//	m_GPS.m_UTM.m_hdg += m_yawOffset;
	m_utmGPSorigin.m_altRel = ((double)m_pAP->m_pMavlink->m_msg.global_position_int.relative_alt) * 0.01;

	UTM_POS pUTM = m_GPS.offset(m_utmGPSorigin, m_vSlamPos);
	LL_POS pLL = m_GPS.UTM2LL(pUTM);

	double tBase = (double)USEC_1SEC/(double)m_dTime;
	m_vVelo.x = (pUTM.m_northing - m_utmPos.m_northing)*tBase;
	m_vVelo.y = (pUTM.m_easting - m_utmPos.m_easting)*tBase;
	m_vVelo.z = (pUTM.m_altRel - m_utmPos.m_altRel)*tBase;
	m_utmPos = pUTM;

	mavlink_gps_input_t D;
	D.lat = pLL.m_lat * 1e7;
	D.lon = pLL.m_lng * 1e7;
	D.alt = (float)pLL.m_altRel;
	D.vn = (float)m_vVelo.x;
	D.ve = (float)m_vVelo.y;
	D.vd = (float)m_vVelo.z;
	D.gps_id = m_gpsID;
	D.fix_type = m_iFixType;
	D.satellites_visible = m_nSat;
	D.hdop = (float)m_hdop;
	D.vdop = (float)m_vdop;
	D.ignore_flags = 0b11110111;
	m_pAP->m_pMavlink->gpsInput(D);

}

void APcopter_slam::updatePos(void)
{
	uint8_t pBufR[N_IO_BUF];
	int nRead = m_pIOr->read(pBufR, N_IO_BUF);
	IF_(nRead <= 0);

	for(int i=0; i<nRead; i++)
	{
		if(m_iCmd == 0)
		{
			IF_CONT(pBufR[i] != MG_CMD_START);
		}

		m_pCmd[m_iCmd] = pBufR[i];
		m_iCmd++;
		IF_CONT(m_iCmd < 12);
		m_iCmd = 0;

		//decode one command
		IF_CONT(m_pCmd[2] != MG_CMD_POS);
		m_iSeq = m_pCmd[1];
		m_fX.input(((double)unpack_int16(&m_pCmd[3])) * 0.001);
		m_fY.input(((double)unpack_int16(&m_pCmd[5])) * 0.001);
		m_fHdg.input(((double)unpack_int16(&m_pCmd[9])) * 0.001);

		if(m_bAxisXY)
		{
			m_vSlamPos.x = m_kX * m_fX.v();
			m_vSlamPos.y = m_kY * m_fY.v();
		}
		else
		{
			m_vSlamPos.x = m_kX * m_fY.v();
			m_vSlamPos.y = m_kY * m_fX.v();
		}

		m_vSlamPos.z = m_fHdg.v();

		LOG_I("iSeq=" + i2str((uint32_t)m_iSeq));
	}
}

void APcopter_slam::sendState(void)
{
	uint8_t pBufW[N_IO_BUF];

	pBufW[0] = MG_CMD_START;
	pBufW[1] = 0;
	pBufW[2] = MG_CMD_ATTITUDE;
	pack_int16(&pBufW[3], (int16_t)(m_pAP->m_pMavlink->m_msg.attitude.roll*1000));
	pack_int16(&pBufW[5], (int16_t)(m_pAP->m_pMavlink->m_msg.attitude.pitch*1000));
	pack_int16(&pBufW[7], (int16_t)((m_pAP->m_pMavlink->m_msg.attitude.yaw + CV_PI)*1000));
	m_pIOw->write(pBufW, 9);

	pBufW[0] = MG_CMD_START;
	pBufW[1] = 0;
	pBufW[2] = MG_CMD_RAW_IMU;
	pack_int16(&pBufW[3], m_pAP->m_pMavlink->m_msg.raw_imu.xacc);
	pack_int16(&pBufW[5], m_pAP->m_pMavlink->m_msg.raw_imu.yacc);
	pack_int16(&pBufW[7], m_pAP->m_pMavlink->m_msg.raw_imu.zacc);
	pack_int16(&pBufW[9], m_pAP->m_pMavlink->m_msg.raw_imu.xgyro);
	pack_int16(&pBufW[11], m_pAP->m_pMavlink->m_msg.raw_imu.ygyro);
	pack_int16(&pBufW[13], m_pAP->m_pMavlink->m_msg.raw_imu.zgyro);
	pack_int16(&pBufW[15], m_pAP->m_pMavlink->m_msg.raw_imu.xmag);
	pack_int16(&pBufW[17], m_pAP->m_pMavlink->m_msg.raw_imu.ymag);
	pack_int16(&pBufW[19], m_pAP->m_pMavlink->m_msg.raw_imu.zmag);
	m_pIOw->write(pBufW, 21);
}

bool APcopter_slam::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	circle(*pMat, Point(m_vSlamPos.x * pMat->cols,
						0.5 * pMat->rows),
						pMat->cols * pMat->rows * 0.00002, Scalar(0, 0, 255), 2);

	pWin->addMsg(*this->getName());

	pWin->addMsg("pos = (" +
			f2str(m_vSlamPos.x) + ", " +
			f2str(m_vSlamPos.y) + ", " +
			f2str(m_vSlamPos.z) + ")");

	pWin->addMsg("yawOffset = " + f2str(m_yawOffset));

	pWin->addMsg("velocity = (" +
			f2str(m_vVelo.x) + ", " +
			f2str(m_vVelo.y) + ", " +
			f2str(m_vVelo.z) + ")");

	return true;
}

bool APcopter_slam::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	C_MSG("pos = (" +
			f2str(m_vSlamPos.x) + ", " +
			f2str(m_vSlamPos.y) + ", " +
			f2str(m_vSlamPos.z) + ")");

	C_MSG("yawOffset = " + f2str(m_yawOffset));

	C_MSG("velocity = (" +
			f2str(m_vVelo.x) + ", " +
			f2str(m_vVelo.y) + ", " +
			f2str(m_vVelo.z) + ")");

	return true;
}

}
