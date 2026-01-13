/*
 * _DetectNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_JetsonInference__DetectNet_H_
#define OpenKAI_src_DNN_JetsonInference__DetectNet_H_

#include "detectNet.h"
#include "../../Vision/_VisionBase.h"
#include "../../Detector/_DetectorBase.h"

namespace kai
{

	enum DETECTNET_TYPE
	{
		detectNet_uff,
		detectNet_caffe,
	};

	class _DetectNet : public _DetectorBase
	{
	public:
		_DetectNet();
		~_DetectNet();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual bool check(void);

	private:
		bool open(void);
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_DetectNet *)This)->update();
			return NULL;
		}

	protected:
		detectNet *m_pDN;
		int m_nBox;
		uint32_t m_nClass;
		DETECTNET_TYPE m_type;
		float m_thr;
		string m_layerIn;
		string m_layerOut;
		string m_layerNboxOut;
		vInt3 m_vDimIn;
		int m_nMaxBatch;
		int m_precision;
		int m_device;
		bool m_bAllowGPUfallback;

		Frame *m_pRGBA;
		Frame *m_pRGBAf;
		bool m_bSwapRB;
		vFloat3 m_vMean;
	};

}
#endif
