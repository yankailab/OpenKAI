#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_move_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_move_H_

#include "_APmavlink_base.h"

#define IGN_YAW (1 << 10)
#define IGN_YAW_RATE (1 << 11)

namespace kai
{

	class _APmavlink_move : public _ModuleBase
	{
	public:
		_APmavlink_move();
		~_APmavlink_move();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual void update(void);
		virtual void console(void *pConsole);
		virtual bool check(void);

		virtual void setHold(void);

		virtual void setHdg(float y = 0,
					float r = 0,
					bool bYaw = false,
					bool bYawRate = true,
					uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		virtual void setVlocal(const vFloat4 &vSpd,
					   bool bYaw = false,
					   bool bYawRate = true,
					   uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		virtual void setPlocal(const vFloat4 &vP,
					   bool bYaw = false,
					   bool bYawRate = true,
					   uint8_t frame = MAV_FRAME_BODY_OFFSET_NED);

		virtual void setPglobal(const vDouble4 &vP,
						bool bYaw = true,
						bool bYawRate = false,
						uint8_t frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT);

		virtual void doReposition(const vDouble4 &vP,	//Lat, Lon, Alt, Yaw
						float speed = -1,
						float radius = 0,
						uint8_t frame = MAV_FRAME_GLOBAL);

	private:
		static void *getUpdate(void *This)
		{
			((_APmavlink_move *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
	};

}
#endif
