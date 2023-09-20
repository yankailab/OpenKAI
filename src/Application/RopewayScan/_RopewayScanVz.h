/*
 * _RopewayScanVz.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_RopewayScan__RopewayScanVz_H_
#define OpenKAI_src_Application_RopewayScan__RopewayScanVz_H_

#include "../../3D/PointCloud/_PCframe.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"
#include "../../LIDAR/_VzensePC.h"

namespace kai
{
    enum ROPEWAYSCAN_VZ_THREAD_BIT
	{
		rws_vz_reset,
		rws_vz_take,
		rws_vz_save,
	};

	class _RopewayScanVz : public _PCframe
	{
	public:
		_RopewayScanVz();
		virtual ~_RopewayScanVz();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

		virtual bool bBusy(void);
		virtual void reset(void);
		virtual void take(void);
		virtual void save(void);

	protected:
		// point cloud
		virtual void scanReset(void);
		virtual void scanTake(void);
		virtual void savePC(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_RopewayScanVz *)This)->update();
			return NULL;
		}

		// kinematic
		virtual void updateAttitude(void);
		virtual void updateK(void);
		static void *getUpdateK(void *This)
		{
			((_RopewayScanVz *)This)->updateK();
			return NULL;
		}

	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;
		vector<_GeometryBase *> m_vpGB;

		vector<PointCloud> m_vPC;	// original point cloud data frames
		int m_nP;
		int m_nPmax;
		float m_rVoxel;
		string m_baseDir;
		string m_dir;
		float m_rB;			// used buffer ratio
		int m_nTake;

		BIT_FLAG m_fProcess;

	};

}
#endif
