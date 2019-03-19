/*
 * _ANR.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__ANR_H_
#define OpenKAI_src_Application__ANR_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Detector/_DetectorBase.h"

namespace kai
{

#define N_PREFIX_CANDIDATE 128

class _ANR: public _ThreadBase
{
public:
	_ANR(void);
	virtual ~_ANR();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void cn(void);
	void lp(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ANR *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pDcn;
	_DetectorBase* m_pDlp;

	string m_cnPrefix;
	vector<string> m_vPrefixCandidate;
	float  m_cnPosMargin;
	string m_cn;
	int		m_nCNdigit;
	vFloat4 m_cnPrefixBB;
	vFloat4 m_cnBB;
	uint64_t m_tStampCNprefix;
	uint64_t m_tStampCN;
	uint64_t m_timeOut;

	string m_lp;
	vFloat4 m_lpBB;
	uint64_t m_tStampLP;


};

}
#endif
