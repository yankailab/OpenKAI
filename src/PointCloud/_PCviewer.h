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
#include <open3d/visualization/gui/Native.h>
#include "../UI/WindowO3D.h"

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
		virtual void update(void);
		virtual void updateGUI(void);

		static void *getUpdate(void *This)
		{
			((_PCviewer *)This)->update();
			return NULL;
		}

		static void *getUpdateGUI(void *This)
		{
			((_PCviewer *)This)->updateGUI();
			return NULL;
		}

	protected:
		string m_pathRes;
		Visualizer m_vis;
		vInt2 m_vWinSize;
		float m_fov;
		string m_device;

		vector<_PCbase *> m_vpPCB;
		shared_ptr<PointCloud> m_spPC;

		_Thread* m_pTgui;
		shared_ptr<WindowO3D> m_spWin;
	};

}
#endif
#endif
