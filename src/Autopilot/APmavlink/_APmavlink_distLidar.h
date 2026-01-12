
#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_distLidar_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_distLidar_H_

#include "../../Sensor/Distance/_DistSensorBase.h"
#include "_APmavlink_base.h"

#define N_LIDAR_SECTION 10

namespace kai
{

	struct DIST_LIDAR_SECTION
	{
		uint8_t m_orientation;
		double m_degFrom;
		double m_degTo;
		double m_minD;
		double m_sensorScale;

		void init(void)
		{
			m_orientation = 0;
			m_degFrom = 360 - 22.5;
			m_degTo = 360 + 22.5;
			m_minD = 0.0;
			m_sensorScale = 1.0;
		}
	};

	class _APmavlink_distLidar : public _ModuleBase
	{
	public:
		_APmavlink_distLidar();
		~_APmavlink_distLidar();

		bool init(const json& j);
		void update(void);
		void draw(void *pFrame);

	private:
		void updateMavlink(void);

		_APmavlink_base *m_pAP;
		_DistSensorBase *m_pDS;

		int m_nSection;
		DIST_LIDAR_SECTION m_pSection[N_LIDAR_SECTION];
	};

}
#endif
