#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_avrSpeed = 0.0;
	m_drawVscale = 1.0;
	m_obj.reset();
	m_objSpeedAlert.reset();
	m_objHdgAlert.reset();
	m_kSpeed = 1.0;
	m_kSpeedX = 0;
	m_kSpeedY = 0;
	m_hdgMeasureMinSpeed = 3.0;
	m_vHdgLimit.x = -1.0;
	m_vHdgLimit.y = -1.0;
	m_vSpeedLimit.x = -1.0;
	m_vSpeedLimit.y = -1.0;

	m_nAlert = 25;
	m_nMed = 3;
}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK, kSpeed);
	KISSm(pK, kSpeedX);
	KISSm(pK, kSpeedY);
	KISSm(pK, nAlert);
	KISSm(pK, drawVscale);
	KISSm(pK, hdgMeasureMinSpeed);

	if(pK->v("hdgFrom", &m_vHdgLimit.x))
		m_vHdgLimit.x = Hdg(m_vHdgLimit.x);
	if(pK->v("hdgTo", &m_vHdgLimit.y))
		m_vHdgLimit.y = Hdg(m_vHdgLimit.y);

	pK->v("speedFrom", &m_vSpeedLimit.x);
	pK->v("speedTo", &m_vSpeedLimit.y);

	KISSm(pK,nMed);
	m_fNspeedAlert.init(m_nMed,3);

	//link
	string iName;

	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInst(iName));

	return true;
}

int Traffic_speed::check(void)
{
	NULL__(m_pTB,-1);
	NULL__(m_pTB->m_pOB,-1);

	return this->ActionBase::check();
}

void Traffic_speed::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	_ObjectBase* pOB = m_pTB->m_pOB;
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int i=0;
	int nCount = 0;
	double speed = 0.0;
	while((pO = pOB->at(i++)) != NULL)
	{
		IF_CONT(!m_pTB->bInsideROI(pO->m_bb));
		m_obj.add(pO);
		IF_CONT(pO->m_trackID <= 0);

		//speed
		double s = pO->m_vTrack.len();
		if(m_vSpeedLimit.x >= 0.0 && s >= 0.0)
		{
			vDouble2 pC = pO->m_bb.center();
			double sCam = s * (1.0 + pC.x * m_kSpeedX) * (1.0 + pC.y * m_kSpeedY) * m_kSpeed;

			if(sCam < m_vSpeedLimit.x || sCam > m_vSpeedLimit.y)
				m_objSpeedAlert.add(pO);
		}
		nCount++;
		speed += s;

		//heading
		if(m_vHdgLimit.x >= 0.0 && s > m_hdgMeasureMinSpeed)
		{
			double h = -atan2(pO->m_vTrack.x,
							  -pO->m_vTrack.y) * RAD_DEG;
			h = Hdg(h);
			if(m_vHdgLimit.x < m_vHdgLimit.y)
			{
				if(h < m_vHdgLimit.x || h > m_vHdgLimit.y)
					m_objHdgAlert.add(pO);
			}
			else
			{
				if(h < m_vHdgLimit.x && h > m_vHdgLimit.y)
					m_objHdgAlert.add(pO);
			}
		}
	}

	m_avrSpeed = speed / (double)nCount;
	m_obj.update();
	m_objSpeedAlert.update();
	m_objHdgAlert.update();

	m_fNspeedAlert.input(m_objSpeedAlert.size());
}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	string msg = "avrSpeed=" + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	Scalar col = Scalar(255, 255, 128);
	Scalar colSpeedAlert = Scalar(0, 255, 255);
	Scalar colHdgAlert = Scalar(0, 0, 255);

	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;

	//all
	vInt4 iBB;
	Point pC;
	Rect r;
	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		iBB = pO->iBBox(cSize);
		pC = Point(iBB.midX(), iBB.midY());

		//bbox
		vInt42rect(iBB, r);
		rectangle(*pMat, r, col, 2);

		//speed
		Point pV = Point(pO->m_vTrack.x * cSize.x * m_drawVscale,
						 pO->m_vTrack.y * cSize.y * m_drawVscale);
		line(*pMat, pC, pC - pV, col, 1);
	}

	//speed alert
	i=0;
	while((pO = m_objSpeedAlert.at(i++)) != NULL)
	{
		iBB = pO->iBBox(cSize);
		vInt42rect(iBB, r);
		rectangle(*pMat, r, colSpeedAlert, 2);
	}

	if(m_fNspeedAlert.v() > (double)m_nAlert)
	{
		msg = "CONGESTION";
		putText(*pMat, msg,
				Point(0.3 * cSize.x, 0.5 * cSize.y),
				FONT_HERSHEY_SIMPLEX, 5.0, colSpeedAlert, 10);
	}

	//heading alert
	i=0;
	while((pO = m_objHdgAlert.at(i++)) != NULL)
	{
		iBB = pO->iBBox(cSize);
		vInt42rect(iBB, r);
		rectangle(*pMat, r, colHdgAlert, 2);
	}

	return true;
}

bool Traffic_speed::cli(int& iY)
{
	IF_F(!this->ActionBase::cli(iY));

	string msg = "avrSpeed=" + f2str(m_avrSpeed);
	COL_MSG;
	iY++;
	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	int nAlert;
	COL_ERROR;

	//speed alert
	nAlert = m_objSpeedAlert.size();
	if(nAlert>0)
	{
		msg = "nSpeedAlert=" + i2str(nAlert);
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
	}

	//heading alert
	nAlert = m_objHdgAlert.size();
	if(nAlert>0)
	{
		msg = "nHeadingAlert=" + i2str(nAlert);
		iY++;
		mvaddstr(iY, CLI_X_MSG, msg.c_str());
	}

	return true;
}

}
