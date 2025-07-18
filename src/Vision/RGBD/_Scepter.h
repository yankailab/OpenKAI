/*
 * _Scepter.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Scepter_H_
#define OpenKAI_src_Vision_RGBD__Scepter_H_

#include "_RGBDbase.h"
#include <Scepter_api.h>

namespace kai
{
	struct ScCtrl
	{
		int m_tScan = 3000; // scan time
		int m_pixelFormat = SC_PIXEL_FORMAT_BGR_888;

		bool m_bAutoExposureToF = true;
		int m_tExposureToF = 4000;

		bool m_bAutoExposureRGB = true;
		int m_tExposureRGB = 4000;

		bool m_bFilTime = false;
		int m_filTime = 0;

		bool m_bFilConfidence = true;
		int m_filConfidence = 1;

		bool m_bFilFlyingPix = false;
		int m_filFlyingPix = 0;

		bool m_bFillHole = false;
		bool m_bSpatialFilter = false;
		bool m_bHDR = false;
	};

	class _Scepter : public _RGBDbase
	{
	public:
		_Scepter();
		virtual ~_Scepter();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		virtual bool open(void);
		virtual void close(void);

#ifdef WITH_3D
		virtual int getPointCloud(_PCframe* pPCframe, int nPmax = INT_MAX);
#endif

		ScCtrl getCamCtrl(void);
		bool setCamCtrl(const ScCtrl& camCtrl);
		bool setToFexposureControlMode(bool bAuto);
		bool setToFexposureTime(bool bAuto, int tExposure);
		bool setRGBexposureControlMode(bool bAuto);
		bool setRGBexposureTime(bool bAuto, int tExposure);
		bool setTimeFilter(bool bON, int thr);
		bool setConfidenceFilter(bool bON, int thr);
		bool setFlyingPixelFilter(bool bON, int thr);
		bool setFillHole(bool bON);
		bool setSpatialFilter(bool bON);
		bool setHDR(bool bON);

	private:
		bool updatePointCloud(void);
		bool updateScRGBD(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Scepter *)This)->update();
			return NULL;
		}

	protected:
		uint32_t m_nDevice;
		ScDeviceInfo *m_pScDevListInfo;
		ScDeviceHandle m_scDevHandle;
		ScSensorIntrinsicParameters m_scCamParams;
		ScCtrl m_scCtrl;

		ScFrame m_scfRGB;
		ScFrame m_scfDepth;
        ScFrame m_scfTransformedDepth;
        ScFrame m_scfTransformedRGB;
		ScFrame m_scfIR;

		ScVector3f *m_pScVw; // world vector
	};

}
#endif
