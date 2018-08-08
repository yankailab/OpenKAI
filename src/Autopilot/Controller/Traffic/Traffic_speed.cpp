#include "Traffic_speed.h"

namespace kai
{

Traffic_speed::Traffic_speed()
{
	m_pTB = NULL;
	m_tStampOB = 0;
	m_dist = 0.0;
	m_avrSpeed = 0.0;
}

Traffic_speed::~Traffic_speed()
{
}

bool Traffic_speed::init(void* pKiss)
{
	IF_F(this->ActionBase::init(pKiss)==false);
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	KISSm(pK,dist);

	m_barFrom.init();
	m_barTo.init();

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

	OBJECT* pO;
	int nCount = 0;
	double s = 0.0;
	int i=0;
	while((pO = pOB->at(i++)) != NULL)
	{
		IF_CONT(pO->m_trackID <= 0);

		m_barFrom.detect(pO->m_fBBox, pO->m_trackID);

		int r = m_barTo.detect(pO->m_fBBox, pO->m_trackID);
		IF_CONT(r < 0);

		uint64_t tFrom = m_barFrom.tTouch(pO->m_trackID);
		uint64_t tTo = m_barTo.tTouch(pO->m_trackID);

		if(tFrom > 0 && tTo > 0)
		{
			pO->m_speed = (m_dist * USEC_1SEC)/(double)(tTo - tFrom);
			s += pO->m_speed;
			nCount++;
		}

		m_barFrom.erase(pO->m_trackID);
		m_barTo.erase(pO->m_trackID);
	}

	if(nCount>0)
		m_avrSpeed = s/nCount;

}

bool Traffic_speed::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	string msg = "Avr Speed = " + f2str(m_avrSpeed);
	pWin->addMsg(&msg);

	vDouble4* pRoi;
	Rect r;

	pRoi = &m_barFrom.m_roi;
	r.x = pRoi->x * pMat->cols;
	r.y = pRoi->y * pMat->rows;
	r.width = pRoi->z * pMat->cols - r.x;
	r.height = pRoi->w * pMat->rows - r.y;
	rectangle(*pMat, r, Scalar(128, 128, 255), 2);

	pRoi = &m_barTo.m_roi;
	r.x = pRoi->x * pMat->cols;
	r.y = pRoi->y * pMat->rows;
	r.width = pRoi->z * pMat->cols - r.x;
	r.height = pRoi->w * pMat->rows - r.y;
	rectangle(*pMat, r, Scalar(128, 128, 255), 2);

	return true;
}

}
