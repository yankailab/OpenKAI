/*
 * _DNNtext.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__DNNtext_H_
#define OpenKAI_src_DNN_Darknet__DNNtext_H_

#include "_DNNdetect.h"

namespace kai
{

struct DNNTEXT_ROI
{
	vFloat4 m_roi;
	bool m_bInvert;

	void init(void)
	{
		m_roi.init();
		m_bInvert = false;
	}
};

class _DNNtext: public _DNNdetect
{
public:
	_DNNtext();
	~_DNNtext();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	int check(void);

private:
	void decode(const Mat& mScores, const Mat& mGeometry, float scoreThresh,
	        std::vector<RotatedRect>& vDetections, std::vector<float>& vConfidences);
	bool detect(Frame* pFrame, int iImg);
	void ocr(Frame* pFrame, int iImg);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNtext*) This)->update();
		return NULL;
	}

public:
	Frame	m_fBGRinv;
	vDouble3 m_vMean;
	bool	m_bDetect;
	vector<DNNTEXT_ROI>	m_vROI;

#ifdef USE_OCR
	bool	m_bOCR;
	tesseract::TessBaseAPI* m_pOCR;
	string	m_ocrDataDir;
	string	m_ocrLanguage;
	int		m_ocrMode;
	int		m_ocrPageMode;
#endif

};

}
#endif
