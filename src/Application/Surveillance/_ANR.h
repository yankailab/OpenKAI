/*
 * _ANR.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__ANR_H_
#define OpenKAI_src_Application__ANR_H_

#include "../../Base/_ThreadBase.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Detector/OCR.h"
#include "../../IO/_WebSocket.h"
#include "../../Script/JSON.h"

#ifdef USE_OPENCV

namespace kai
{

class _ANR: public _ThreadBase
{
public:
	_ANR(void);
	virtual ~_ANR();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	string char2Number(const char* pStr);
	bool cn(void);
	void lp(void);
	void lpO(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ANR *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pDcn;
	_DetectorBase* m_pDlp;
	_WebSocket* m_pWS;

	Frame m_fBGR;

	string m_cnPrefix;
	vector<string> m_vPrefixCandidate;
	float  m_cnPosMargin;
	string m_cn;
	int		m_nCNdigit;
	vFloat4 m_cnPrefixBB;
	vFloat4 m_cnBB;
	float	m_offsetRdigit;
	float	m_wRdigit;
	uint64_t m_tStampCNprefix;
	uint64_t m_tStampCN;
	uint64_t m_timeOut;

	string m_lp;
	vFloat4 m_lpBB;
	uint64_t m_tStampLP;

	vector<int> m_vJPGquality;
	string m_tempFile;
	string m_shFile;
	vFloat4 m_vAlprROI;

#ifdef USE_OCR
	bool m_bOCR;
	OCR* m_pOCR;
#endif

};

}
#endif
#endif
