/*
 * OCR.h
 *
 *  Created on: May 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector_OCR_H_
#define OpenKAI_src_Detector_OCR_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Script/Kiss.h"

#ifdef USE_OCR

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
	virtual string scan(Rect& r);

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
