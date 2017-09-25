#include "../../Controller/APcopter/APcopter_sensorAvoid.h"

namespace kai
{

APcopter_sensorAvoid::APcopter_sensorAvoid()
{
	m_pAP = NULL;
	m_nDS = 0;
}

APcopter_sensorAvoid::~APcopter_sensorAvoid()
{
}

bool APcopter_sensorAvoid::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_sensorAvoid::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	//dist sensor
	Kiss* pDS = pK->o("distSensor");
	IF_F(pDS->empty());

	Kiss** pItrDS = pDS->getChildItr();
	m_nDS = 0;

	while (pItrDS[m_nDS])
	{
		Kiss* pKds = pItrDS[m_nDS];
		IF_F(m_nDS >= N_DIST_SENSOR);

		DIST_SENSOR* pDS = &m_pDS[m_nDS];
		F_ERROR_F(pKds->v("name", &iName));
		pDS->m_pSensor = NULL;
		pDS->m_pSensor = (_ZEDobstacle*) (pK->root()->getChildInstByName(&iName));
		NULL_F(pDS->m_pSensor);

		vInt2 mDim = ((_ZEDobstacle*) pDS->m_pSensor)->matrixDim();
		Kiss** pItrSeg = pKds->getChildItr();
		pDS->m_nSeg = 0;

		while (pItrSeg[pDS->m_nSeg])
		{
			Kiss* pKseg = pItrSeg[pDS->m_nSeg];
			IF_F(pDS->m_nSeg >= N_DS_SEGMENT);

			DIST_SEGMENT* pDseg = &pDS->m_pSeg[pDS->m_nSeg];

			vDouble4 fRoi;
			F_INFO(pKseg->v("l", &fRoi.x));
			F_INFO(pKseg->v("t", &fRoi.y));
			F_INFO(pKseg->v("r", &fRoi.z));
			F_INFO(pKseg->v("b", &fRoi.w));

			vInt4* pRoi = &pDseg->m_roi;
			pRoi->x = fRoi.x * mDim.x;
			pRoi->y = fRoi.y * mDim.y;
			pRoi->z = fRoi.z * mDim.x;
			pRoi->w = fRoi.w * mDim.y;

			if (pRoi->x < 0)
				pRoi->x = 0;
			if (pRoi->y < 0)
				pRoi->y = 0;
			if (pRoi->z >= mDim.x)
				pRoi->z = mDim.x - 1;
			if (pRoi->w >= mDim.y)
				pRoi->w = mDim.y - 1;

			pDS->m_nSeg++;
		}

		m_nDS++;
	}

	return true;
}

void APcopter_sensorAvoid::update(void)
{
	this->ActionBase::update();

	updateMavlink();
}

void APcopter_sensorAvoid::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	int i, j;
	for (i = 0; i < m_nDS; i++)
	{
		DIST_SENSOR* pDS = &m_pDS[i];
		vDouble2 range = pDS->m_pSensor->range() * 100;

		for (j = 0; j < pDS->m_nSeg; j++)
		{
			DIST_SEGMENT* pSeg = &pDS->m_pSeg[j];

			pMavlink->distanceSensor(0, //type
					pSeg->m_orient,	//orientation
					range.y, range.x, pDS->m_pSensor->d(&pSeg->m_roi, NULL)*100);
		}
	}
}

bool APcopter_sensorAvoid::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	string msg;
	msg = *this->getName() + " nSensor=" + i2str(m_nDS);
	pWin->addMsg(&msg);

	int i, j;
	Rect r;

	for (i = 0; i < m_nDS; i++)
	{
		DIST_SENSOR* pDS = &m_pDS[i];
		vInt2 mDim = ((_ZEDobstacle*) pDS->m_pSensor)->matrixDim();

		int bW = pMat->cols / mDim.x;
		int bH = pMat->rows / mDim.y;

		for (j = 0; j < pDS->m_nSeg; j++)
		{
			DIST_SEGMENT* pSeg = &pDS->m_pSeg[j];
			vInt4 roi = pSeg->m_roi;
			vInt2 pMin;
			int d = pDS->m_pSensor->d(&roi,&pMin)*100;

			r.x = roi.x * bW;
			r.y = roi.y * bH;
			r.width = roi.z * bW - r.x;
			r.height = roi.w * bH - r.y;
			rectangle(*pMat, r, Scalar(0, 255, 255), 1);

			circle(*pMat,
					Point((pMin.x + 0.5) * bW,
							(pMin.y + 0.5) * bH),
					0.000025 * pMat->cols * pMat->rows, Scalar(0, 255, 255), 2);
		}
	}

	return true;
}

}
