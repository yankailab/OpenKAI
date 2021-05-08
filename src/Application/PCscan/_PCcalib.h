/*
 * _PCcalib.h
 *
 *  Created on: May 6, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_PCcalib__PCcalib_H_
#define OpenKAI_src_Application_PCcalib__PCcalib_H_
#ifdef USE_OPEN3D
#include "../../PointCloud/_PCstream.h"
#include "../../PointCloud/_PCviewer.h"
#include "../../Vision/ImgFilter/_Remap.h"
#include "../../Utility/BitFlag.h"
#include "../CamCalib/_CamCalib.h"
#include "PCcalibUI.h"

using namespace open3d::visualization::visualizer;

namespace kai
{
	namespace
	{
		static const int pcfCalibReset = 0;
	}

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
		void updateProcess(void);
		virtual void startScan(void);
		virtual void updateScan(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCcalib *)This)->update();
			return NULL;
		}

		void updateRGBtransform(void);
		void updateRGB(void);
		static void *getUpdateRGB(void *This)
		{
			((_PCcalib *)This)->updateRGB();
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
		static void OnUpdateParams(void *pPCV, void* pD);

		bool calibRGB(const char* pPath);
		void updateParams(void);

	protected:
		_PCstream* m_pPS;
		shared_ptr<visualizer::PCcalibUI> m_spWin;
		visualizer::UIState* m_pUIstate;
		string m_modelName;
		_Thread *m_pTrgb;
		_Remap* m_pVremap;
		_CamCalib* m_pCC;

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
