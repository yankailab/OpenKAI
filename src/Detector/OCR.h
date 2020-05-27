/*
 * OCR.h
 *
 *  Created on: May 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector_OCR_H_
#define OpenKAI_src_Detector_OCR_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OCR
#include "../Base/BASE.h"
#include "../Script/Kiss.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

namespace kai
{

class OCR: public BASE
{
public:
	OCR();
	virtual ~OCR();

	virtual bool init(void* pKiss);
	virtual void update(void);
	virtual bool draw(void);
	virtual bool console(int& iY);

	virtual void setFrame(Frame& f);
	virtual void setMat(Mat m);
	virtual string scan(Rect* pR);

public:
	Frame	m_fOCR;
	tesseract::TessBaseAPI* m_pOCR;
	string	m_ocrDataDir;
	string	m_ocrLanguage;
	string	m_locale;
	int		m_ocrMode;
	int		m_ocrPageMode;

};

}
#endif
#endif
#endif
