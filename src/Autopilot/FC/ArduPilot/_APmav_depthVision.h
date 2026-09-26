
#ifndef OpenKAI_src_Autopilot_FC_ArduPilot__APmav_depthVision_H_
#define OpenKAI_src_Autopilot_FC_ArduPilot__APmav_depthVision_H_

#include "../../../Vision/RGBD/_RGBDbase.h"
#include "_APmav_base.h"

#define N_DEPTH_ROI 16

namespace kai
{

	struct DEPTH_ROI
	{
		string m_configKey;
		uint8_t m_orientation;
		Vector4f m_roi = Vector4f::Zero();
		float m_minD;

		void init(void)
		{
			m_minD = 0.0;
			m_orientation = 0;
			m_roi.setZero();
		}
	};

	class _APmav_depthVision : public _ModuleBase
	{
	public:
		_APmav_depthVision();
		~_APmav_depthVision();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual void update(void);
		virtual void draw(void *pMat);

	protected:
		_APmav_base *m_pAP = nullptr;
		_RGBDbase *m_pDV = nullptr;

		int m_nROI = 0;
		DEPTH_ROI m_pROI[N_DEPTH_ROI];
	};

}
#endif
