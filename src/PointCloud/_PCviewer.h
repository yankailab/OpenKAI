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
		float m_fov;
		float m_aspect;		
		vFloat2 m_vNF;		//near far plane
		uint8_t m_fovType;

		void init(void)
		{
			m_fov = 70.0;
			m_aspect = 16.0/9.0;
			m_vNF.init(0, FLT_MAX);
			m_fovType = 0;
		}
	};

	class _PCviewer : public _PCframe
	{
	public:
		_PCviewer();
		virtual ~_PCviewer();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void draw(void);

	protected:
		virtual void readAllPC(void);
		virtual void makeInitPC(PointCloud *pPC, int n, double l, int iAxis, Vector3d vCol);

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
		vFloat3 m_vCamCenter;
		vFloat3 m_vCamEye;
		vFloat3 m_vCamUp;
		vFloat3 m_vCamCoR;

		vector<_PCbase *> m_vpPCB;

		_Thread *m_pTui;
		shared_ptr<PCviewerUI> m_spWin;
	};

}
#endif
#endif
