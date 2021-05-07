/*
 * _PCviewer.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCviewer_H_
#define OpenKAI_src_PointCloud__PCviewer_H_
#ifdef USE_OPEN3D
#include "_PCframe.h"
#include "PCviewerUI.h"

namespace kai
{

	struct CAM_PROJ
	{
		float m_fov = 70.0;
		vFloat2 m_vNF = {0, FLT_MAX};	//near far plane
		uint8_t m_fovType = 0;
	};

	struct PC_CAM
	{
		vFloat3 m_vLookAt = {0,0,0};
		vFloat3 m_vEye = {0,0,1};
		vFloat3 m_vUp = {0,1,0};
	};

	class _PCviewer : public _PCframe
	{
	public:
		_PCviewer();
		virtual ~_PCviewer();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);

		virtual void resetCamPose(void);

	protected:
		virtual void readAllPC(void);
		virtual void addDummyDome(PointCloud* pPC, int n, float r, Vector3d vCol = {0,0,0});

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCviewer *)This)->update();
			return NULL;
		}

		virtual void updateUI(void);
		static void *getUpdateUI(void *This)
		{
			((_PCviewer *)This)->updateUI();
			return NULL;
		}

	protected:
		string m_pathRes;
		Visualizer m_vis;
		vInt2 m_vWinSize;
		string m_device;

		CAM_PROJ m_camProj;
		PC_CAM m_cam;
		PC_CAM m_camDefault;
		PC_CAM m_camAuto;
		vFloat3 m_vCoR;

		vector<_PCbase *> m_vpPCB;

		_Thread *m_pTui;
		shared_ptr<PCviewerUI> m_spWin;
	};

}
#endif
#endif
