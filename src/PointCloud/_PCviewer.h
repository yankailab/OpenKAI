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
		virtual void makeInitPC(PointCloud* pPC, int n, double l, int iAxis, Vector3d vCol);

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

		float m_fov;
		vFloat3 m_vCamCenter;
		vFloat3 m_vCamEye;
		vFloat3 m_vCamUp;
		vFloat3 m_vCamCoR;

		vector<_PCbase *> m_vpPCB;

		_Thread* m_pTui;
		shared_ptr<PCviewerUI> m_spWin;
	};

}
#endif
#endif
