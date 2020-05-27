/*
 * OCR.cpp
 *
 *  Created on: May 12, 2019
 *      Author: yankai
 */

#include "OCR.h"

#ifdef USE_OPENCV
#ifdef USE_OCR

namespace kai
{

OCR::OCR()
{
	m_pOCR = NULL;
	m_ocrDataDir = "";
	m_ocrLanguage = "eng";
	m_ocrMode = tesseract::OEM_LSTM_ONLY;
	m_ocrPageMode = tesseract::PSM_AUTO;
	m_locale = "C";

}

OCR::~OCR()
{
	if(m_pOCR)
	{
		m_pOCR->End();
		delete m_pOCR;
	}
}

bool OCR::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, ocrDataDir);
	KISSm(pK, ocrLanguage);
	KISSm(pK, ocrMode);
	KISSm(pK, ocrPageMode);
	KISSm(pK, locale);

	setlocale(LC_ALL, m_locale.c_str());
	m_pOCR = new tesseract::TessBaseAPI();
	m_pOCR->Init(m_ocrDataDir.c_str(), m_ocrLanguage.c_str(), (tesseract::OcrEngineMode)m_ocrMode);
	m_pOCR->SetPageSegMode((tesseract::PageSegMode)m_ocrPageMode);

	return true;
}

void OCR::update(void)
{
}

void OCR::setMat(Mat m)
{
	IF_(m.empty());

	m_fOCR.copy(m);
	if(m_fOCR.m()->channels()<3)
		m_fOCR.copy(m_fOCR.cvtColor(8));

	Mat mIn = *m_fOCR.m();
	m_pOCR->SetImage(mIn.data, mIn.cols, mIn.rows, 3, mIn.step);
}

void OCR::setFrame(Frame& f)
{
	IF_(f.bEmpty());

	m_fOCR.copy(f);
	if(m_fOCR.m()->channels()<3)
		m_fOCR.copy(m_fOCR.cvtColor(8));

	Mat mIn = *m_fOCR.m();
	m_pOCR->SetImage(mIn.data, mIn.cols, mIn.rows, 3, mIn.step);
}

string OCR::scan(Rect* pR)
{
	if(m_fOCR.bEmpty())return "";
	if(pR)
		m_pOCR->SetRectangle(pR->x, pR->y, pR->width, pR->height);

	string strO = string(m_pOCR->GetUTF8Text());

	return strO;
}

bool OCR::draw(void)
{
	return this->BASE::draw();
}

bool OCR::console(int& iY)
{
	return this->BASE::console(iY);
}

}
#endif
#endif
