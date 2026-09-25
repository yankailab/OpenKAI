#ifndef OpenKAI_src_Autopilot__AutopilotBase_H_
#define OpenKAI_src_Autopilot__AutopilotBase_H_

#include "../Universe/_ReferenceFrame.h"
#include "../State/_StateControl.h"
#include "../Protocol/_JSONbase.h"
#include "../Utility/utilEvent.h"
#include "../Utility/utilVar.h"

namespace kai
{
	enum AP_TYPE
	{
		ardupilot_unknown = 0,
		ardupilot_rover = 1,
		ardupilot_copter = 2,
		ardupilot_plane = 3,
		ardupilot_boat = 4,
	};

	enum AP_MODE
	{
		apMode_unknown = 0,
		apMode_manual = 1,
		apMode_fc = 2,
		apMode_ok = 3,
	};

	enum AP_ARM
	{
		apArm_unknown = 0,
		apArm_disarm = 1,
		apArm_arm = 2,
	};

	enum AP_GPS_FIX_TYPE
	{
		apGPS_unknown = 0,
	};

	class _AutopilotBase : public _ReferenceFrame
	{
	public:
		_AutopilotBase();
		~_AutopilotBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);
		virtual void console(const json &j, void *pJSONbase);

		// general
		virtual AP_TYPE getType(void);
		virtual bool setMode(AP_MODE m);
		virtual bool setArm(AP_ARM a);

		virtual AP_MODE getMode(void);
		virtual AP_ARM getArm(void);

		// status
		virtual float getRelativeAlt(void);
		virtual const Vector4d& getHomePos(void);
		virtual const Vector3f& getVelocity(void);

		virtual float getBattery(void);

	private:
		void updateAutopilot(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_AutopilotBase *)This)->update();
			return NULL;
		}

	protected:
		AP_TYPE m_type = ardupilot_unknown;

		// state control
		AP_MODE m_mode = apMode_unknown;
		AP_ARM m_arm = apArm_unknown;

		// read only
		float m_rAlt;
		Vector3f m_vVelocity = Vector3f::Zero();
		Vector4d m_vHomePos = Vector4d::Zero();
		// Reference frame
        // Vector3d m_vPos;    // position in Lat, Lon, Alt
        // Quaterniond m_vOrt; // orientation quaternion
        // Vector3d m_vAngle;  // euler angles (radian) in roll, pitch, yaw (North 0) order



		int8_t m_battery = -1;	// remaining percentage in integer, -1:unknown

	};

}
#endif
