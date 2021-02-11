/*
 * PointCloudBase.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCviewer_H_
#define OpenKAI_src_PointCloud__PCviewer_H_

#include "../Base/_ModuleBase.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

struct PCVIEWER_GEO
{
	shared_ptr<PointCloud> m_spPC;
	pthread_mutex_t m_mutex;
	bool m_bAdded;
	bool m_bUpdated;

	void init(void)
	{
		m_spPC = shared_ptr<PointCloud>(new PointCloud);
		pthread_mutex_init(&m_mutex, NULL);
		m_bAdded = false;
		m_bUpdated = false;
	}

	void destroy(void)
	{
		pthread_mutex_destroy(&m_mutex);
	}

	void updateGeometry(PointCloud* pPC)
	{
		NULL_(pPC);

		pthread_mutex_lock(&m_mutex);
		*m_spPC = *pPC;
		pthread_mutex_unlock(&m_mutex);

		m_bUpdated = true;
	}

	void addToVisualizer(Visualizer* pV)
	{
		NULL_(pV);
		IF_(m_bAdded);

		pthread_mutex_lock(&m_mutex);
		pV->AddGeometry(m_spPC);
		pthread_mutex_unlock(&m_mutex);

		m_bAdded = true;
	}

	void updateVisualizer(Visualizer* pV)
	{
		NULL_(pV);
		IF_(!m_bAdded);
		IF_(!m_bUpdated);

		pthread_mutex_lock(&m_mutex);
		pV->UpdateGeometry(m_spPC);
		pthread_mutex_unlock(&m_mutex);

		m_bUpdated = false;
	}

};

class _PCviewer: public _ModuleBase
{
public:
	_PCviewer();
	virtual ~_PCviewer();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

	int addGeometry(void);
	void updateGeometry(int i, PointCloud* pPC);

private:
	void render(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_PCviewer *) This)->update();
		return NULL;
	}

public:
	Visualizer m_vis;
	vInt2 m_vWinSize;
	float m_fov;
	shared_ptr<TriangleMesh> m_pMcoordFrame;
    bool m_bCoordFrame;

	vector<PCVIEWER_GEO> m_vGeo;
};

}
#endif
#endif
