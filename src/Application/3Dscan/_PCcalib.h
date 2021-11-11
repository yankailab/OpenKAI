/*
 * _PCcalib.h
 *
 *  Created on: May 6, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_PCcalib__PCcalib_H_
#define OpenKAI_src_Application_PCcalib__PCcalib_H_

#include "../../IO/_File.h"
#include "../CamCalib/_CamCalib.h"
#include "_PCscan.h"
#include "PCcalibUI.h"

namespace kai
{
	class _PCcalib : public _PCscan
	{
	public:
		_PCcalib();
		virtual ~_PCcalib();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
	
	    void importParams(const char* pPath);
	    void exportParams(const char* pPath);

	protected:
		// point cloud
		virtual void updateProcess(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCcalib *)This)->update();
			return NULL;
		}

		// Kinetics
		static void *getUpdateKinematics(void *This)
		{
			((_PCcalib *)This)->updateKinematics();
			return NULL;
		}

		// UI
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCcalib *)This)->updateUI();
			return NULL;
		}

		// handlers
		static void OnLoadImgs(void *pPCV, void* pD);
		static void OnRefreshCol(void *pPCV, void* pD);
		static void OnUpdateParams(void *pPCV, void* pD);
		static void OnImportParams(void *pPCV, void* pD);
		static void OnExportParams(void *pPCV, void* pD);

		bool calibRGB(const char* pPath);
		void updateParams(void);

	protected:
		_CamCalib* m_pCC;
	};

}
#endif
