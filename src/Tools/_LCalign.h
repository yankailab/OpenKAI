/*
 * _LCalign.h
 *
 *  Created on: Mar 6, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Tools__LCalign_H_
#define OpenKAI_src_Tools__LCalign_H_

#include "../Universe/Geometry/PointCloud/_PointCloud.h"
#include "../Vision/_VisionBase.h"
#include "../Sensor/_IMUbase.h"
#include "../Protocol/_JSONbase.h"

namespace kai
{

	class _LCalign : public _PointCloud
	{
	public:
		_LCalign();
		virtual ~_LCalign();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);
		//		virtual void draw(void *pMat);

		virtual bool saveConfig(void) override;

		void updateMatrices(void);
		bool L2C(const Vector2i &vSizeImg, const Vector3f &vPi, Vector2i &vPo);
		//		bool pt2Pix(const Vector2i &vSizeImg, const Vector3f &vPi, Vector2i &vPo);

		Vector2d getCamFocal(void);
		Vector2d getCamCenter(void);
		array<double, 5> getCamDistortion(void);
		array<double, 9> getCamR(void);
		array<double, 3> getCamT(void);

		void setCamFocal(const Vector2d &vF);
		void setCamCenter(const Vector2d &vC);
		void setCamDistortion(const array<double, 5> &aD);
		void setCamR(const array<double, 9> &aR);
		void getCamT(const array<double, 3> &aT);

	private:
		virtual void updateCalib(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_LCalign *)This)->update();
			return NULL;
		}

	protected:
		_PointCloud *m_pPCin = nullptr;
		_VisionBase *m_pV = nullptr;
		_IMUbase *m_pIMU = nullptr;

		// Camera
		// intrinsics
		Vector2i m_vCsize = Vector2i::Zero();
		Vector2d m_vCf = Vector2d::Zero();										   // focal
		Vector2d m_vCc = Vector2d::Zero();										   // center
		array<double, 5> m_aCdist = {0.0, 0.0, 0.0, 0.0, 0.0}; // k1,k2,p1,p2,k3

		// extrinsics, Camera <- LiDAR: pCam = m_aCr * pLidar + m_aCt
		array<double, 9> m_aCr = {1, 0, 0,
								  0, 1, 0,
								  0, 0, 1};
		array<double, 3> m_aCt = {0.0, 0.0, 0.0};

		// prebuilt matrices for pLC
		Mat m_mCam;
		Mat m_mDistCoeffs;
		Mat m_mRot;
		Mat m_mvR;
		Mat m_mvT;
		pthread_mutex_t m_mtxMat;

		// IMU
		// extrinsics, IMU<-LiDAR: pImu = m_aIr * pLidar + m_aIt
		array<double, 9>
			m_aIr = {1, 0, 0,
					 0, 1, 0,
					 0, 0, 1};
		array<double, 3> m_aIt = {0, 0, 0};
	};

}
#endif
