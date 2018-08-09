#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_scaleBBox.x = 1.0;
	m_scaleBBox.y = 1.0;
	m_dist = 0.0;
	m_speed = 0.0;
	m_tFrom = 0;
	m_tTo = 0;
	m_nInside = 0;

	m_barFrom.init();
	m_barTo.init();
}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK, dist);
	F_INFO(pK->v("scaleBBox_x", &m_scaleBBox.x));
	F_INFO(pK->v("scaleBBox_y", &m_scaleBBox.y));

	Kiss* pB = pK->o("barFrom");
	NULL_F(pB);
	F_INFO(pB->v("x", &m_barFrom.m_roi.x));
	F_INFO(pB->v("y", &m_barFrom.m_roi.y));
	F_INFO(pB->v("z", &m_barFrom.m_roi.z));
	F_INFO(pB->v("w", &m_barFrom.m_roi.w));

	pB = pK->o("barTo");
	NULL_F(pB);
	F_INFO(pB->v("x", &m_barTo.m_roi.x));
	F_INFO(pB->v("y", &m_barTo.m_roi.y));
	F_INFO(pB->v("z", &m_barTo.m_roi.z));
	F_INFO(pB->v("w", &m_barTo.m_roi.w));

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

	// bar touch
	OBJECT* pO;
	int i=0;
	bool bTouchFrom = false;
	bool bTouchTo = false;
	while((pO = pOB->at(i++)) != NULL)
	{
		vDouble4 bbox = pO->m_fBBox;

		double mX = bbox.midX();
		double dX = mX - bbox.x;
		dX *= m_scaleBBox.x;
		bbox.x = mX - dX;
		bbox.z = mX + dX;

		double mY = bbox.midY();
		double dY = mY - bbox.y;
		dY *= m_scaleBBox.y;
		bbox.y = mY - dY;
		bbox.w = mY + dY;

		if(!bTouchFrom)
			bTouchFrom = m_barFrom.bTouch(&bbox);

		if(!bTouchTo)
			bTouchTo = m_barTo.bTouch(&bbox);

		if(bTouchFrom && bTouchTo)
			break;
	}

	m_barFrom.count(bTouchFrom);
	m_barTo.count(bTouchTo);

	// speed

	//invalid count, reset
	if(m_barFrom.m_nCount < m_barTo.m_nCount)
	{
		m_barFrom.resetCount();
		m_barTo.resetCount();
		m_tFrom = 0;
		m_tTo = 0;
		return;
	}

	int nInside = m_barFrom.m_nCount - m_barTo.m_nCount;
	if(nInside > m_nInside)
	{
		m_tFrom = getTimeUsec();
		m_tTo = 0;
	}
	else if(nInside < m_nInside)
	{
		m_tTo = getTimeUsec();
		if(m_tFrom > 0)
			m_speed = (m_dist * USEC_1SEC) / (m_tTo - m_tFrom);

		m_tFrom = 0;
		m_tTo = 0;
	}

	m_nInside = nInside;
}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nBarFrom = " + i2str(m_barFrom.m_nCountTot)
				 + "nBarTo = " + i2str(m_barTo.m_nCountTot);
	pWin->addMsg(&msg);

	Scalar colA = Scalar(128, 192, 192);
	Scalar colB = Scalar(192, 255, 255);
	Scalar col;
	Rect r;

	col = colA;
	if(m_barFrom.m_bTouch)col=colB;
	vDouble4* pRoiFrom = &m_barFrom.m_roi;
	r.x = pRoiFrom->x * pMat->cols;
	r.y = pRoiFrom->y * pMat->rows;
	r.width = pRoiFrom->z * pMat->cols - r.x;
	r.height = pRoiFrom->w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);
	putText(*pMat, i2str(m_barFrom.m_nCountTot),
			Point(r.x + 15, r.y - 5),
			FONT_HERSHEY_SIMPLEX, 1.0, col, 2);

	col = colA;
	if(m_barTo.m_bTouch)col=colB;
	vDouble4* pRoiTo = &m_barTo.m_roi;
	r.x = pRoiTo->x * pMat->cols;
	r.y = pRoiTo->y * pMat->rows;
	r.width = pRoiTo->z * pMat->cols - r.x;
	r.height = pRoiTo->w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);
	putText(*pMat, i2str(m_barTo.m_nCountTot),
			Point(r.x + 15, r.y - 5),
			FONT_HERSHEY_SIMPLEX, 1.0, col, 2);

	putText(*pMat, "Speed: "+f2str(m_speed),
			Point((pRoiFrom->x + pRoiTo->x)*0.5*pMat->cols,
				  (pRoiFrom->y + pRoiTo->y)*0.5*pMat->rows - 5),
			FONT_HERSHEY_SIMPLEX, 1.0, colA, 2);

	return true;
}

}
