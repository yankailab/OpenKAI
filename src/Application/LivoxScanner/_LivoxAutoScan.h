/*
 * _LivoxAutoScan.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_LidarScanner__LivoxAutoScan_H_
#define OpenKAI_src_Application_LidarScanner__LivoxAutoScan_H_

#include "../../3D/PointCloud/_PCstream.h"
#include "../../LIDAR/Livox/_Livox.h"
#include "../../Utility/BitFlag.h"
#include "../../Actuator/_ActuatorBase.h"

namespace kai
{
	struct LivoxAutoScanActuator
	{
		// config
		vFloat2 m_vRange;
		float m_dV = 10;
		_ActuatorBase* m_pAct = NULL;

		// dynamic
		float m_v = 0.0;

		void reset(void)
		{
			m_v = m_vRange.x;
		}

		bool bComplete(void)
		{
			IF_T(m_v > m_vRange.y);

			return false;
		}

		float update(void)
		{
			if(m_v <= m_vRange.y)
				m_v += m_dV;

			return m_v;
		}
	};

	struct LivoxAutoScanConfig
	{
		vFloat2 m_vRangeH;
		float m_dH;
		vFloat2 m_vRangeV;
		float m_dV;

		vDouble3 m_vOffset;
	};

    enum LidarAutoScan_threadBit
	{
		lvScanner_reset,
		lvScanner_start,
		lvScanner_stop,
		lvScanner_save,
	};

	class _LivoxAutoScan : public _PCstream
	{
	public:
		_LivoxAutoScan();
		virtual ~_LivoxAutoScan();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);

		virtual void center(void);
		virtual void reset(void);
		virtual void startAuto(void);
		virtual void stop(void);
		virtual void save(void);
		virtual void setConfig(const LivoxAutoScanConfig& c);
		virtual LivoxAutoScanConfig getConfig(void);

		virtual bool bScanning(void);

		float getBufferCap(void);

	protected:
		//point cloud
		virtual void savePC(void);
		virtual void scanUpdate(void);
		virtual void scanStop(void);
		virtual void scanStart(void);
		virtual void scanReset(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_LivoxAutoScan *)This)->update();
			return NULL;
		}

	protected:
		vector<_GeometryBase *> m_vpGB;

		vector<PointCloud> m_vPC;
		int m_nP;
		int m_nPmax;
		float m_rVoxel;
		string m_baseDir;
		string m_dir;
		int m_tWaitSec;

		LivoxCtrl m_livoxCtrl;
		BIT_FLAG m_fProcess;
		float m_rB;

		bool m_bScanning;
		int m_nTake;
		LivoxAutoScanActuator m_actH;
		LivoxAutoScanActuator m_actV;
		vDouble3 m_vOffset;


	};

}
#endif
