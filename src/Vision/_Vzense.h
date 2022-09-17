/*
 * _Vzense.h
 *
 *  Created on: Sept 7, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_Vzense_H_
#define OpenKAI_src_Vision_Vzense_H_

#include "_DepthVisionBase.h"
#include "../Utility/util.h"
#include <VzenseDS_api.h>

namespace kai
{

	class _Vzense : public _DepthVisionBase
	{
	public:
		_Vzense();
		virtual ~_Vzense();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		bool open(void);
		void close(void);

	private:
		void update(void);
		bool updateVzense(void);
		void getDepthMat(int height, int width, uint8_t*pData, cv::Mat& dispImg);
		static void *getUpdate(void *This)
		{
			((_Vzense *)This)->update();
			return NULL;
		}

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Vzense *)This)->updateTPP();
			return NULL;
		}

	public:
		VzDeviceInfo *m_pDeviceListInfo;
		VzDeviceHandle m_deviceHandle;
		uint32_t m_nDevice;
        VzSensorIntrinsicParameters m_cameraParameters;
		uint32_t m_slope;

        VzFrame m_vzfDepth;
        VzFrame m_vzfIR;
        VzFrame m_vzfRGB;
        VzFrame m_vzfTransformedDepth;
        VzFrame m_vzfTransformedRGB;



		string m_rsSN;

		float m_fConfidenceThreshold;
		float m_fDigitalGain;
		float m_fPostProcessingSharpening;
		float m_fFilterManitude;
		float m_fHolesFill;
		float m_fEmitter;
		float m_fLaserPower;

		float m_fDefault;
		float m_fBrightness;
		float m_fContrast;
		float m_fGain;
		float m_fExposure;
		float m_fHue;
		float m_fSaturation;
		float m_fSharpness;
		float m_fWhiteBalance;

		int m_rsFPS;
		int m_rsDFPS;
		bool m_bRsRGB;

		//depth filter processing thread
		_Thread *m_pTPP;
	};

}
#endif
