/*
 * _RopewayScanLivox.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_RopewayScan__RopewayScanLivox_H_
#define OpenKAI_src_Application_RopewayScan__RopewayScanLivox_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../LIDAR/Livox/_Livox.h"
#include "../../Navigation/_NavBase.h"
#include "../../Utility/BitFlag.h"

namespace kai
{
    enum ROPEWAYSCAN_LV_THREAD_BIT
	{
		rws_lv_reset,
		rws_lv_take,
		rws_lv_start,
		rws_lv_stop,
		rws_lv_save,
	};

	class _RopewayScanLivox : public _PCstream
	{
	public:
		_RopewayScanLivox();
		virtual ~_RopewayScanLivox();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

		virtual bool bBusy(void);
		virtual void resetScan(void);
		virtual void startScan(void);
		virtual void stopScan(void);
		virtual void takeScan(void);
		virtual void save(void);

	protected:
		//point cloud
		virtual void scanReset(void);
		virtual void scanTake(void);
		virtual void scanStart(void);
		virtual void scanStop(void);
		virtual void savePC(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_RopewayScanLivox *)This)->update();
			return NULL;
		}

		//attitude
		virtual void updateAttitude(void);
		virtual void updateK(void);
		static void *getUpdateK(void *This)
		{
			((_RopewayScanLivox *)This)->updateK();
			return NULL;
		}

	protected:
		_NavBase *m_pNav;
		_Thread *m_pTk;
		vector<_GeometryBase *> m_vpGB;

		vector<PointCloud> m_vPC;
		int m_nP;
		int m_nPmax;
		float m_rVoxel;
		string m_baseDir;
		string m_dir;
		int m_tWait;
		float m_rB;			// used buffer ratio

		LivoxCtrl m_livoxCtrl;
		BIT_FLAG m_fProcess;

	};

}
#endif
