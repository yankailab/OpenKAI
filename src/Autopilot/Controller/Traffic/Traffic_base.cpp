#include "Traffic_base.h"

namespace kai
{

Traffic_base::Traffic_base()
{
	m_pOB = NULL;
	m_vROI.clear();
}

Traffic_base::~Traffic_base()
{
}

bool Traffic_base::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

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

	//link
	string iName;
	iName = "";
	pK->v("_DetectorBase", &iName);
	m_pOB = (_DetectorBase*) (pK->root()->getChildInst(iName));

	return true;
}

void Traffic_base::update(void)
{
	this->ActionBase::update();

}

bool Traffic_base::bInsideROI(vDouble4& bb)
{
	IF_T(m_vROI.empty());

	double d = pointPolygonTest( m_vROI, Point2f((float)bb.midX(),(float)bb.midY()), false );
	if(d >= 0)return true;

	return false;
}

bool Traffic_base::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	vInt2 cSize;
	cSize.x = pMat->cols;
	cSize.y = pMat->rows;

	//ROI
	int nP = m_vROI.size();
	for(unsigned int i=0; i<m_vROI.size(); i++)
	{
		Point2f pA = m_vROI[i];
		pA.x *= cSize.x;
		pA.y *= cSize.y;

		Point2f pB = m_vROI[(i+1)%nP];
		pB.x *= cSize.x;
		pB.y *= cSize.y;

		line(*pMat, pA, pB, Scalar(0,255,0), 3, 10);
	}

	return true;
}

}
