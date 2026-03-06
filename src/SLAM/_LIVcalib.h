/*
 * _LIVcalib.h
 *
 *  Created on: Mar 6, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_SLAM__LIVcalib_H_
#define OpenKAI_src_SLAM__LIVcalib_H_

#include "../3D/PointCloud/_PCstream.h"
#include "../Vision/_VisionBase.h"
#include "../Sensor/_IMUbase.h"

namespace kai
{

	class _LIVcalib : public _PCstream
	{
	public:
		_LIVcalib();
		virtual ~_LIVcalib();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		//		virtual void draw(void *pFrame);

		virtual bool loadConfig(const string& fName);
		virtual bool saveConfig(const string& fName);

		bool pt2Pix(const vInt2 &vSizeImg, const vFloat3 &vPi, vInt2 &vPo);

		vDouble2 getCamFocal(void);
		vDouble2 getCamCenter(void);
		array<double, 5> getCamDistortion(void);
		array<double, 9> getCamR(void);
		array<double, 3> getCamT(void);

		void setCamFocal(const vDouble2& vF);
		void setCamCenter(const vDouble2& vC);
		void setCamDistortion(const array<double, 5>& aD);
		void setCamR(const array<double, 9>& aR);
		void getCamT(const array<double, 3>& aT);

		void updateMatrices(void);

	private:
		virtual void updateCalib(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_LIVcalib *)This)->update();
			return NULL;
		}

	protected:
		_PCstream *m_pPCin;
		_VisionBase *m_pV;
		_IMUbase *m_pIMU;

		string m_fName; // calib json save

		// ---------------Camera-----------------
		// intrinsics
		vInt2 m_vCsize;
		vDouble2 m_vCf;		// focal
		vDouble2 m_vCc;		// center
		array<double, 5> m_aCdistortion = {0.0, 0.0, 0.0, 0.0, 0.0}; // k1,k2,p1,p2,k3

		// extrinsics, Camera<-LiDAR: pCam = CLr * pLidar + CLt
		array<double, 9> m_aCr = {1, 0, 0,
								   0, 1, 0,
								   0, 0, 1};
		array<double, 3> m_aCt = {0.0, 0.0, 0.0};

		// ---------------IMU-----------------
		// extrinsics, IMU<-LiDAR: pImu = ILr * pLidar + ILt
		array<double, 9> m_aIr = {1, 0, 0,
								   0, 1, 0,
								   0, 0, 1};
		array<double, 3> m_aIt = {0, 0, 0};



	};

}
#endif
