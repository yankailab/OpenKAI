#include "APcopter_depthVision.h"

namespace kai
{

APcopter_depthVision::APcopter_depthVision()
{
	m_pAP = NULL;
	m_pDV = NULL;
	m_nROI = 0;
}

APcopter_depthVision::~APcopter_depthVision()
{
}

bool APcopter_depthVision::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(iName));

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_DepthVisionBase*) (pK->root()->getChildInstByName(iName));
	IF_Fl(!m_pDV, iName + " not found");

	Kiss** pItrDS = pK->getChildItr();
	m_nROI = 0;

	while (pItrDS[m_nROI])
	{
		Kiss* pKs = pItrDS[m_nROI];
		IF_F(m_nROI >= N_DEPTH_ROI);

		DEPTH_ROI* pR = &m_pROI[m_nROI];
		pR->init();
		F_ERROR_F(pKs->v("orientation", (int*)&pR->m_orientation));
		F_ERROR_F(pKs->v("l", &pR->m_roi.x));
		F_ERROR_F(pKs->v("t", &pR->m_roi.y));
		F_ERROR_F(pKs->v("r", &pR->m_roi.z));
		F_ERROR_F(pKs->v("b", &pR->m_roi.w));
		F_INFO(pKs->v("dScale", &pR->m_dScale));

		m_nROI++;
	}

	return true;
}

void APcopter_depthVision::update(void)
{
	this->ActionBase::update();

	updateMavlink();
}

void APcopter_depthVision::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	NULL_(m_pDV);

	vDouble2 range = m_pDV->range();
	mavlink_distance_sensor_t D;

	for(int i=0; i<m_nROI; i++)
	{
		DEPTH_ROI* pR = &m_pROI[i];

		double d = m_pDV->d(&pR->m_roi,NULL) * pR->m_dScale;
		if(d < range.x)d = range.y;
		if(d > range.y)d = range.y;
		pR->m_minD = d;

		D.type = 0;
		D.max_distance = (uint16_t)(range.y*100);	//unit: centimeters
		D.min_distance = (uint16_t)(range.x*100);
		D.current_distance = (uint16_t)(pR->m_minD * 100);
		D.orientation = pR->m_orientation;
		D.covariance = 255;

		pMavlink->distanceSensor(D);
		LOG_I("orient: " + i2str(pR->m_orientation) + " minD: " + f2str(pR->m_minD));
	}
}

bool APcopter_depthVision::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());

	string msg;
	msg = *this->getName() + " nROI=" + i2str(m_nROI);
	pWin->addMsg(&msg);

	NULL_F(m_pDV);

	vInt2 mDim = m_pDV->matrixDim();
	int bW = pMat->cols / mDim.x;
	int bH = pMat->rows / mDim.y;

	for (int i = 0; i < m_nROI; i++)
	{
		DEPTH_ROI* pR = &m_pROI[i];
		vDouble4 roi = pR->m_roi;
		vInt2 pMin;
		int d = m_pDV->d(&roi,&pMin)*100;

		Rect r;
		r.x = roi.x * pMat->cols;
		r.y = roi.y * pMat->rows;
		r.width = roi.z * pMat->cols - r.x;
		r.height = roi.w * pMat->rows - r.y;
		rectangle(*pMat, r, Scalar(0, 255, 255), 1);

		circle(*pMat,
				Point((pMin.x + 0.5) * bW,
					  (pMin.y + 0.5) * bH),
				0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
	}

	return true;
}

}
