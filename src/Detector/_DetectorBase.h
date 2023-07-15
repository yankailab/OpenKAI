/*
 * _DetectorBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__DetectorBase_H_
#define OpenKAI_src_Base__DetectorBase_H_

#include "../Universe/_Universe.h"
#include "../Vision/_VisionBase.h"
#include "../Utility/utilCV.h"

namespace kai
{

	class _DetectorBase : public _ModuleBase
	{
	public:
		_DetectorBase();
		virtual ~_DetectorBase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual int getClassIdx(string &className);
		virtual string getClassName(int iClass);

	public:
		// input
		_VisionBase *m_pV;
		_DetectorBase *m_pDB;

		// data
		Frame m_fBGR;
		_Universe *m_pU;

		// model
		string m_fModel;
		string m_fWeight;
		string m_fMean;
		string m_fClass;
		int m_nClass;
		vector<OBJ_CLASS> m_vClass;
	};

}
#endif
