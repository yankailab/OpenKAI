#ifndef OpenKAI_src_Autopilot_AP__AP_move_H_
#define OpenKAI_src_Autopilot_AP__AP_move_H_

#include "../../Control/PID.h"
#include "_AP_base.h"

namespace kai
{

	class _AP_move : public _StateBase
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

		void setPglobal(const vFloat4 &vP,
						bool bYaw = true,
						bool bYawRate = false,
						uint8_t frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT);

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
