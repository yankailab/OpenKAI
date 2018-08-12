#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_avrSpeed = 0.0;
	m_minArea = 0.0;
	m_maxArea = 1.0;
	m_drawVscale = 1.0;
	m_obj.reset();
	m_kSpeed = 1.0;

	m_vHdgLimit.x = -1.0;
	m_vHdgLimit.y = -1.0;
	m_vSpeedLimit.x = -1.0;
	m_vSpeedLimit.y = -1.0;
	m_vROI.clear();
}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK, minArea);
	KISSm(pK, maxArea);
	KISSm(pK, kSpeed);
	KISSm(pK, drawVscale);

	if(pK->v("hdgFrom", &m_vHdgLimit.x))
		m_vHdgLimit.x = Hdg(m_vHdgLimit.x);
	if(pK->v("hdgTo", &m_vHdgLimit.y))
		m_vHdgLimit.y = Hdg(m_vHdgLimit.y);

	pK->v("speedFrom", &m_vSpeedLimit.x);
	pK->v("speedTo", &m_vSpeedLimit.y);

	//ROI
	Kiss* pR = pK->o("ROI");
	NULL_T(pR);

	Kiss** ppP = pR->getChildItr();
	int i=0;
	while((pR = ppP[i++])!=NULL)
	{
		double x = 0;
		double y = 0;
		pR->v("x",&x);
		pR->v("y",&y);
		m_vROI.push_back(Point2f((float)x,(float)y));
	}

	return true;
}

bool Traffic_speed::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void Traffic_speed::update(void)
{
	this->ActionBase::update();

	NULL_(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_(pOB);
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int i=0;
	int nCount = 0;
	double speed = 0.0;
	while((pO = pOB->at(i++)) != NULL)
	{
		double a = pO->m_bb.area();
		IF_CONT(a < m_minArea);
		IF_CONT(a > m_maxArea);
		IF_CONT(!bInsideROI(pO->m_bb));

		m_obj.add(pO);

		//speed
		double s = pO->m_vTrack.len()*m_kSpeed;
		if(m_vSpeedLimit.x >= 0.0)
		{
			if(s < m_vSpeedLimit.x || s > m_vSpeedLimit.y)
				m_objSpeedAlert.add(pO);
		}
		nCount++;
		speed += s;

		//heading
		if(m_vHdgLimit.x >= 0.0 && s > 1.0)
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
}

bool Traffic_speed::bInsideROI(vDouble4& bb)
{
	if(m_vROI.empty())return true;

	double d = pointPolygonTest( m_vROI, Point2f((float)bb.midX(),(float)bb.midY()), false );
	if(d >= 0)return true;

	return false;
}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	NULL_F(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_F(pOB);
	NULL_F(pOB->m_pVision);

	string msg = "avrSpeed=" + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	Scalar col = Scalar(228, 228, 128);
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
		rectangle(*pMat, r, col, 1);

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
		rectangle(*pMat, r, colSpeedAlert, 1);
	}

	//heading alert
	i=0;
	while((pO = m_objHdgAlert.at(i++)) != NULL)
	{
		iBB = pO->iBBox(cSize);
		vInt42rect(iBB, r);
		rectangle(*pMat, r, colHdgAlert, 1);
	}

	//ROI
	int nP = m_vROI.size();
	for(i=0; i<m_vROI.size(); i++)
	{
		Point2f pA = m_vROI[i];
		pA.x *= cSize.x;
		pA.y *= cSize.y;

		Point2f pB = m_vROI[(i+1)%nP];
		pB.x *= cSize.x;
		pB.y *= cSize.y;

		line(*pMat, pA, pB, Scalar(255,255,200), 3, 8 );
	}

	return true;
}

}
