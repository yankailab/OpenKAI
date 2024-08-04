#ifndef OpenKAI_src_Autopilot_MavAP__MavAPcopter_photo_H_
#define OpenKAI_src_Autopilot_MavAP__MavAPcopter_photo_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "../../Vision/_GPhoto.h"
#include "../../Sensor/Distance/_DistSensorBase.h"
#include "_MavAP_base.h"
#include "_MavAP_move.h"

namespace kai
{

	class _MavAPcopter_photo : public _ModuleBase
	{
	public:
		_MavAPcopter_photo();
		~_MavAPcopter_photo();

		int init(void *pKiss);
		int start(void);
		void update(void);
		int check(void);
		void console(void *pConsole);

	private:
		void autoMode(void);
		void manualMode(void);
		void shutter(void);
		static void *getUpdate(void *This)
		{
			((_MavAPcopter_photo *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		_MavAP_move *m_pPC;
		_DistSensorBase *m_pDS;
		int m_iDiv;
		float m_speed;

		_VisionBase *m_pV;
		_RGBDbase *m_pDV;
		_GPhoto *m_pG;

		int m_iRCshutter;
		STATE_CHANGE m_scShutter;

		float m_alt;
		float m_dAlt;
		float m_lastAlt;
		int m_iRelayShutter;

		int m_iTake;
		uint64_t m_tDelay;

		string m_dir;
		string m_subDir;
		bool m_bFlipRGB;
		bool m_bFlipD;
		vector<int> m_compress;
		int m_quality;
	};

}
#endif
