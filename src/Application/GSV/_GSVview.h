/*
 * _GSVview.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_GSVview__GSVview_H_
#define OpenKAI_src_Application_GSVview__GSVview_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../3D/_GeometryViewer.h"
#include "../../Utility/BitFlag.h"

namespace kai
{
	class _GSVview : public _GeometryViewer
	{
	public:
		_GSVview();
		virtual ~_GSVview();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

	protected:
		//point cloud
		virtual void scanReset(void);
		virtual void scanTake(void);
		virtual void savePC(void);

		virtual void updateCamCtrl(void);
		virtual void updateCamAuto(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GSVview *)This)->update();
			return NULL;
		}

		// UI
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_GSVview *)This)->updateUI();
			return NULL;
		}

		// handlers
		static void OnScanReset(void *pPCV, void* pD);
		static void OnScanTake(void *pPCV, void* pD);
		static void OnSavePC(void *pPCV, void* pD);
		static void OnCamSet(void *pPCV, void* pD);
		static void OnCamCtrl(void *pPCV, void* pD);

	protected:
		
		float m_rVoxel;
		string m_fNameSavePC;

		BIT_FLAG m_fProcess;
	};

}
#endif
