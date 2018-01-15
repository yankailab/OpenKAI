#include "APcopter_distZED.h"

namespace kai
{

APcopter_distZED::APcopter_distZED()
{
	m_pAP = NULL;
	m_nZED = 0;
}

APcopter_distZED::~APcopter_distZED()
{
}

bool APcopter_distZED::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool APcopter_distZED::link(void)
{
	IF_F(!this->ActionBase::link());
	Kiss* pK = (Kiss*) m_pKiss;
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInstByName(&iName));

	Kiss* pDS = pK->o("distZED");
	IF_F(pDS->empty());

	Kiss** pItrDS = pDS->getChildItr();
	m_nZED = 0;

	while (pItrDS[m_nZED])
	{
		Kiss* pKds = pItrDS[m_nZED];
		IF_F(m_nZED >= N_DIST_ZED);

		DIST_ZED* pDS = &m_pZED[m_nZED];
		pDS->init();
		F_ERROR_F(pKds->v("_ZEDdistance", &iName));
		pDS->m_pZED = (_ZEDdistance*) (pK->root()->getChildInstByName(&iName));
		if(!pDS->m_pZED)
		{
			LOG_E(iName << " not found");
			return false;
		}

		vInt2 mDim = ((_ZEDdistance*) pDS->m_pZED)->matrixDim();
		Kiss** pItrSeg = pKds->getChildItr();
		pDS->m_nROI = 0;

		while (pItrSeg[pDS->m_nROI])
		{
			Kiss* pKseg = pItrSeg[pDS->m_nROI];
			IF_F(pDS->m_nROI >= N_DIST_ZED_ROI);

			DIST_ROI* pDseg = &pDS->m_pROI[pDS->m_nROI];

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

			pDS->m_nROI++;
		}

		m_nZED++;
	}

	return true;
}

void APcopter_distZED::update(void)
{
	this->ActionBase::update();

	updateMavlink();
}

void APcopter_distZED::updateMavlink(void)
{
	NULL_(m_pAP);
	NULL_(m_pAP->m_pMavlink);
	_Mavlink* pMavlink = m_pAP->m_pMavlink;

	int i, j;
	for (i = 0; i < m_nZED; i++)
	{
		DIST_ZED* pDS = &m_pZED[i];
		double rMin = pDS->m_pZED->rMin() * 100;
		double rMax = pDS->m_pZED->rMax() * 100;

		for (j = 0; j < pDS->m_nROI; j++)
		{
			DIST_ROI* pSeg = &pDS->m_pROI[j];

			pMavlink->distanceSensor(0, //type
					pSeg->m_orient,	//orientation
					rMax, rMin, pDS->m_pZED->d(&pSeg->m_roi, NULL)*100);
		}
	}
}

bool APcopter_distZED::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	IF_F(pMat->empty());

	string msg;
	msg = *this->getName() + " nZED=" + i2str(m_nZED);
	pWin->addMsg(&msg);

	int i, j;
	Rect r;

	for (i = 0; i < m_nZED; i++)
	{
		DIST_ZED* pDS = &m_pZED[i];
		vInt2 mDim = ((_ZEDdistance*) pDS->m_pZED)->matrixDim();

		int bW = pMat->cols / mDim.x;
		int bH = pMat->rows / mDim.y;

		for (j = 0; j < pDS->m_nROI; j++)
		{
			DIST_ROI* pSeg = &pDS->m_pROI[j];
			vInt4 roi = pSeg->m_roi;
			vInt2 pMin;
			int d = pDS->m_pZED->d(&roi,&pMin)*100;

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
