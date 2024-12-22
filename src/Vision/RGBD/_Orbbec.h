/*
 * _Orbbec.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RGBD__Orbbec_H_
#define OpenKAI_src_Vision_RGBD__Orbbec_H_

#include "_RGBDbase.h"

namespace kai
{
	struct VzCtrl
	{
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

	class _Orbbec : public _RGBDbase
	{
	public:
		_Orbbec();
		virtual ~_Orbbec();

		virtual int init(void *pKiss);
		virtual int link(void);
		virtual int start(void);
		virtual int check(void);

		virtual bool open(void);
		virtual void close(void);

		VzCtrl getCamCtrl(void);
		bool setCamCtrl(const VzCtrl& camCtrl);
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
		bool updatePointCloud(const VzFrameReady& vfr);
		bool updateRGBD(const VzFrameReady& vfr);
		bool updateVzense(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Orbbec *)This)->update();
			return NULL;
		}

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Orbbec *)This)->updateTPP();
			return NULL;
		}

	private:
		VzCtrl m_vzCtrl;

		VzFrame m_vzfRGB;
		VzFrame m_vzfDepth;
        VzFrame m_vzfTransformedDepth;
        VzFrame m_vzfTransformedRGB;
		VzFrame m_vzfIR;

		VzVector3f *m_pVzVw; // world vector
	};

}
#endif
