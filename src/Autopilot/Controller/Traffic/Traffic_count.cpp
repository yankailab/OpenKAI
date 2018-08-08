#include "Traffic_count.h"

namespace kai
{

Traffic_count::Traffic_count()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_roi.init();
	m_nCount = 0;
	m_bTouch = false;
	m_scaleBBox.x = 1.0;
	m_scaleBBox.y = 1.0;
}

Traffic_count::~Traffic_count()
{
}

bool Traffic_count::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("x", &m_roi.x));
	F_INFO(pK->v("y", &m_roi.y));
	F_INFO(pK->v("z", &m_roi.z));
	F_INFO(pK->v("w", &m_roi.w));

	//BBox scale
	F_INFO(pK->v("scaleBBox_x", &m_scaleBBox.x));
	F_INFO(pK->v("scaleBBox_y", &m_scaleBBox.y));


	return true;
}

bool Traffic_count::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;
	iName = "";
	pK->v("Traffic_base", &iName);
	m_pTB = (Traffic_base*) (pK->parent()->getChildInstByName(&iName));

	return true;
}

void Traffic_count::update(void)
{
	this->ActionBase::update();

	NULL_(m_pTB);
	_ObjectBase* pOB = m_pTB->m_pOB;
	NULL_(pOB);
	IF_(pOB->m_tStamp <= m_tStampOB);
	m_tStampOB = pOB->m_tStamp;

	OBJECT* pO;
	int i=0;
	bool bTouch = false;
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

		IF_CONT(!bOverlapped(&m_roi, &bbox));
		bTouch = true;
		break;
	}

	if(!m_bTouch && bTouch)
		m_nCount++;

	m_bTouch = bTouch;
}

bool Traffic_count::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "nCount = " + i2str(m_nCount);
	pWin->addMsg(&msg);

	Scalar colA = Scalar(128, 192, 192);
	Scalar colB = Scalar(192, 255, 255);
	Scalar col;
	vDouble4* pRoi;
	Rect r;

	col = colA;
	if(m_bTouch)col=colB;
	r.x = m_roi.x * pMat->cols;
	r.y = m_roi.y * pMat->rows;
	r.width = m_roi.z * pMat->cols - r.x;
	r.height = m_roi.w * pMat->rows - r.y;
	rectangle(*pMat, r, col, 2);
	putText(*pMat, i2str(m_nCount),
			Point(r.x + 15, r.y - 5),
			FONT_HERSHEY_SIMPLEX, 1.0, col, 2);

	return true;
}

}
