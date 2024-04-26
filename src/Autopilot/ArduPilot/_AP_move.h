#ifndef OpenKAI_src_Autopilot_AP__AP_move_H_
#define OpenKAI_src_Autopilot_AP__AP_move_H_

#include "_AP_base.h"

#define IGN_YAW (1 << 10)
#define IGN_YAW_RATE (1 << 11)

namespace kai
{

	class _AP_move : public _ModuleBase
	{
	public:
		_AP_move();
		~_AP_move();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual int check(void);

		void stop(void);

		void setHdg(float y = 0,
					float r = 0,
					bool bYaw = false,
					bool bYawRate = true,
					uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		void setVlocal(const vFloat4 &vSpd,
					   bool bYaw = false,
					   bool bYawRate = true,
					   uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		void setPlocal(const vDouble4 &vP,
					   bool bYaw = false,
					   bool bYawRate = true,
					   uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		void setPglobal(const vDouble4 &vP,
						bool bYaw = true,
						bool bYawRate = false,
						uint8_t frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT);

		void doReposition(const vDouble4 &vP,	//Lat, Lon, Alt, Yaw
						float speed = -1,
						float radius = 0,
						uint8_t frame = MAV_FRAME_GLOBAL);

	private:
		static void *getUpdate(void *This)
		{
			((_AP_move *)This)->update();
			return NULL;
		}

	protected:
		_AP_base *m_pAP;
	};

}
#endif
