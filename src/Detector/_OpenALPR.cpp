/*
 *  Created on: March 19, 2019
 *      Author: yankai
 */
#include "_OpenALPR.h"

#ifdef USE_OPENALPR
namespace kai
{

_OpenALPR::_OpenALPR()
{
	m_pAlpr = NULL;
	m_region = "us";
	m_config = "";
	m_runtime = "";
}

_OpenALPR::~_OpenALPR()
{
}

bool _OpenALPR::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, region);
	KISSm(pK, config);
	KISSm(pK, runtime);

	// Initialize the library using United States-style license plates.
	// You can use other countries/regions as well (for example: "eu", "au", or "kr").
	m_pAlpr = new alpr::Alpr(m_region,
							m_config,
							m_runtime);

	// Optionally, you can specify the top N possible plates to return (with confidences). The default is ten.
	m_pAlpr->setTopN(10);

	// Optionally, you can provide the library with a region for pattern matching. This improves accuracy by
	// comparing the plate text with the regional pattern.
//	m_pAlpr->setDefaultRegion("md");

	// Make sure the library loads before continuing.
	// For example, it could fail if the config/runtime_data is not found.
	IF_Fl(!m_pAlpr->isLoaded(), "Error loading OpenALPR");

	return true;
}

bool _OpenALPR::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _OpenALPR::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		IF_CONT(!detect());

		m_obj.update();

		if (m_bGoSleep)
		{
			m_obj.m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _OpenALPR::check(void)
{
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);
	NULL__(m_pAlpr,-1);

	return 0;
}

bool _OpenALPR::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	vector<alpr::AlprRegionOfInterest> vROI;

	alpr::AlprResults results = m_pAlpr->recognize(mIn.data, mIn.elemSize(), mIn.cols, mIn.rows, vROI);

	// Carefully observe the results. There may be multiple plates in an image,
	// and each plate returns the top N candidates.
	for (int i = 0; i < results.plates.size(); i++)
	{
	  alpr::AlprPlateResult plate = results.plates[i];
	  std::cout << "plate" << i << ": " << plate.topNPlates.size() << " results" << std::endl;

	    for (int k = 0; k < plate.topNPlates.size(); k++)
	    {
	      alpr::AlprPlate candidate = plate.topNPlates[k];
	      std::cout << "    - " << candidate.characters << "\t confidence: " << candidate.overall_confidence;
	      std::cout << "\t pattern_match: " << candidate.matches_template << std::endl;
	    }
	}

	return true;
}

bool _OpenALPR::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0,0,255);

	OBJECT* pO;
	int i=0;
	while((pO = m_obj.at(i++)) != NULL)
	{
		int iClass = pO->m_topClass;
		IF_CONT(iClass >= m_nClass);
		IF_CONT(iClass < 0);

		Rect r = pO->getRect(cs);
		rectangle(*pMat, r, col, 1);

		string oName = m_vClass[iClass].m_name;
		if (oName.length()>0)
		{
			putText(*pMat, oName,
					Point(r.x + 15, r.y + 25),
					FONT_HERSHEY_SIMPLEX, 1.0, col, 2);
		}
	}

	return true;
}

}
#endif
