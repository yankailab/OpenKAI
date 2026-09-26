#ifndef OpenKAI_src_Actuator_Articulated__xArm_H_
#define OpenKAI_src_Actuator_Articulated__xArm_H_

#include "../_ActuatorBase.h"
#include <xarm/wrapper/xarm_api.h>

namespace kai
{

	class _xArm : public _ActuatorBase
	{
	public:
		_xArm();
		~_xArm();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool start(void);
		virtual bool check(void);

		virtual bool power(bool bON);

	protected:
		virtual void updatePos(void);
		virtual void gotoPos(Vector3f &vP);
		virtual Vector3f getPtarget(void);
		virtual Vector3f getP(void);
		virtual Vector3f getAtarget(void);
		virtual Vector3f getA(void);
		virtual void readState(void);

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_xArm *)This)->update();
			return NULL;
		}

	protected:
		XArmAPI *m_pArm = NULL;

		string m_ip = "192.168.1.222";
		int m_mode = 0;
		int m_state = 0;
	};

}
#endif
