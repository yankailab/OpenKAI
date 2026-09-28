/*
 * _DetectorBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__DetectorBase_H_
#define OpenKAI_src_Detector__DetectorBase_H_

#include "../Universe/Surface/_SurfaceBase.h"
#include "../DataStream/RGBframe.h"
#include "../Utility/utilCV.h"

namespace kai
{

	class _DetectorBase : public _ModuleBase
	{
	public:
		_DetectorBase();
		virtual ~_DetectorBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool check(void);
		virtual void console(void *pConsole);

		virtual bool loadModel(void);
		virtual int getClassIdx(string &className);
		virtual string getClassName(int iClass);
		virtual _SurfaceBase *getCanvas(void);

	protected:
		virtual void onPause(void);

	protected:
		// input
		RGBframe *m_pRGBin = nullptr;

		// data
		_SurfaceBase *m_pCanvas = nullptr;

		// model
		string m_fModel = "";
		string m_fWeight = "";
		string m_fMean = "";
		string m_fClass = "";
		vector<string> m_vClass;
	};

}
#endif
