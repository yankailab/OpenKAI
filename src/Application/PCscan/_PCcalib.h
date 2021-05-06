/*
 * _PCcalib.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_PCcalib__PCcalib_H_
#define OpenKAI_src_Application_PCcalib__PCcalib_H_
#ifdef USE_OPEN3D
#include "../../PointCloud/_PCstream.h"
#include "../../PointCloud/_PCviewer.h"
#include "../../Utility/BitFlag.h"
#include "PCcalibUI.h"

namespace kai
{
	class _PCcalib : public _PCviewer
	{
	public:
		_PCcalib();
		virtual ~_PCcalib();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

	protected:
		void addUIpc(const PointCloud& pc);
		void updateUIpc(const PointCloud& pc);
		void removeUIpc(void);
		virtual void startScan(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCcalib *)This)->update();
			return NULL;
		}

		void updateCamProj(void);
		void updateCamPose(void);
		void camBound(const AxisAlignedBoundingBox& aabb);
		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCcalib *)This)->updateUI();
			return NULL;
		}

		AxisAlignedBoundingBox createDefaultAABB(void);

		static void OnLoadImgs(void *pPCV, void* pD);
		static void OnResetPC(void *pPCV, void* pD);

	protected:
		_PCstream* m_pPS;
		shared_ptr<visualizer::PCcalibUI> m_spWin;
		visualizer::UIState* m_pUIstate;
		string m_modelName;

		bool m_bFullScreen;
		int m_mouseMode;
		float m_rDummyDome;
		AxisAlignedBoundingBox m_aabb;

		//filter flags
		BIT_FLAG m_fProcess;
	};

}
#endif
#endif
