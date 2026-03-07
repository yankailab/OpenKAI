/*
 * _LCalign.h
 *
 *  Created on: Mar 6, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__LCalign_H_
#define OpenKAI_src_SLAM__LCalign_H_

#include "../3D/PointCloud/_PCstream.h"
#include "../Vision/_VisionBase.h"
#include "../Sensor/_IMUbase.h"

namespace kai
{

	class _LCalign : public _PCstream
	{
	public:
		_LCalign();
		virtual ~_LCalign();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual bool console(const json &j, json *pJout = nullptr);
		//		virtual void draw(void *pFrame);

		virtual bool loadConfig(const string &fName);
		virtual bool saveConfig(const string &fName);

		void updateMatrices(void);
		bool L2C(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo);
//		bool pt2Pix(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo);

		vDouble2 getCamFocal(void);
		vDouble2 getCamCenter(void);
		array<double, 5> getCamDistortion(void);
		array<double, 9> getCamR(void);
		array<double, 3> getCamT(void);

		void setCamFocal(const vDouble2 &vF);
		void setCamCenter(const vDouble2 &vC);
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
		_PCstream *m_pPCin;
		_VisionBase *m_pV;
		_IMUbase *m_pIMU;

		string m_fName; // calib json save

		// Camera
		// intrinsics
		vInt2 m_vCsize;
		vDouble2 m_vCf;												 // focal
		vDouble2 m_vCc;												 // center
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
