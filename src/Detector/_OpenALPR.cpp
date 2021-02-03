/*
 *  Created on: March 19, 2019
 *      Author: yankai
 */
#include "_OpenALPR.h"

#ifdef USE_OPENCV
#ifdef USE_OPENALPR
namespace kai
{

_OpenALPR::_OpenALPR()
{
	m_pAlpr = NULL;
	m_region = "us";
	m_config = "";
	m_runtime = "";
	m_nTop = 10;
	m_locale = "C";
}

_OpenALPR::~_OpenALPR()
{
}

bool _OpenALPR::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("region", &m_region);
	pK->v("config", &m_config);
	pK->v("runtime", &m_runtime);
	pK->v("nTop", &m_nTop);
	pK->v("locale", &m_locale);

//	char *old_ctype = strdup(setlocale(LC_ALL, NULL));
	setlocale(LC_ALL, m_locale.c_str());
//	tesseract::TessBaseAPI api;
//	api.InitForAnalysePage();
//	setlocale(LC_ALL, old_ctype);
//	free(old_ctype);

	// Initialize the library using United States-style license plates.
	// You can use other countries/regions as well (for example: "eu", "au", or "kr").
	m_pAlpr = new alpr::Alpr(m_region, m_config, m_runtime);

	// Optionally, you can specify the top N possible plates to return (with confidences). The default is ten.
	m_pAlpr->setTopN(m_nTop);

	// Optionally, you can provide the library with a region for pattern matching. This improves accuracy by
	// comparing the plate text with the regional pattern.
	//	m_pAlpr->setDefaultRegion("md");

	IF_Fl(!m_pAlpr->isLoaded(), "Error loading OpenALPR");

	return true;
}

bool _OpenALPR::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
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
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		IF_CONT(!detect());

		updateObj();

		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		m_pT->autoFPSto();
	}
}

int _OpenALPR::check(void)
{
	NULL__(m_pU,-1);
	NULL__(m_pVision, -1);
	Frame* pBGR = m_pVision->BGR();
	NULL__(pBGR, -1);
	IF__(pBGR->bEmpty(), -1);
	IF__(pBGR->tStamp() <= m_fBGR.tStamp(), -1);
	NULL__(m_pAlpr, -1);

	return 0;
}

bool _OpenALPR::detect(void)
{
	IF_F(check() < 0);

	Frame* pBGR = m_pVision->BGR();
	m_fBGR.copy(*pBGR);
	Mat mIn = *m_fBGR.m();

	vector<alpr::AlprRegionOfInterest> vROI;
	alpr::AlprResults results = m_pAlpr->recognize(mIn.data, mIn.elemSize(),
			mIn.cols, mIn.rows, vROI);

	vInt2 cs;
	cs.x = mIn.cols;
	cs.y = mIn.rows;

	for (unsigned int i=0; i < results.plates.size(); i++)
	{
		alpr::AlprPlateResult plate = results.plates[i];
		LOG_I("plate" + i2str(i) + ": " + i2str(plate.topNPlates.size()) + " results");

		if(m_bLog)
		{
			for (unsigned int k = 0; k < plate.topNPlates.size(); k++)
			{
				alpr::AlprPlate candidate = plate.topNPlates[k];
				LOG_I("    - " + candidate.characters + " confidence: " + f2str(candidate.overall_confidence));
				LOG_I("pattern_match: " + i2str(candidate.matches_template));
			}
		}

		OBJECT o;
		o.init();
		o.m_tStamp = m_tStamp;
		alpr::AlprPlate content = plate.topNPlates[0];
		o.setTopClass(0, content.overall_confidence);

		int n = constrain((int)content.characters.length(), 0, OBJ_N_CHAR-1);
		strncpy(o.m_pText, content.characters.c_str(), n);
		o.m_pText[n] = 0;

		Point2f pV[4];
		for (int p = 0; p < 4; p++)
		{
			o.m_pV[p].x = plate.plate_points[p].x;
			o.m_pV[p].y = plate.plate_points[p].y;
		}
		o.m_nV = 4;
		o.bbScale(cs);
		//TODO

		this->add(&o);
	}

	return true;
}

bool _OpenALPR::draw(void)
{
	IF_F(!this->_ModuleBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	vInt2 cs;
	cs.x = pMat->cols;
	cs.y = pMat->rows;
	Scalar col = Scalar(0, 0, 255);

	OBJECT* pO;
	int i = 0;
	while ((pO = at(i++)) != NULL)
	{
		Rect r = convertBB<vInt4>(convertBB(pO->m_bb, cs));

		rectangle(*pMat, r, col, 1);

		putText(*pMat, string(pO->m_pText),
				Point(pO->m_bb.x * pMat->cols,
					  pO->m_bb.w * pMat->rows),
				FONT_HERSHEY_SIMPLEX, 1.5, col, 2);
	}

	return true;
}

}
#endif
#endif
