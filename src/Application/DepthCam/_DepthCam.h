#ifndef OpenKAI_src_Application_DepthCam__DepthCam_H_
#define OpenKAI_src_Application_DepthCam__DepthCam_H_

#include "../../Protocol/_JSONbase.h"
#include "../../Vision/_RealSense.h"
#include "../../Vision/ImgFilter/_InRange.h"

namespace kai
{

	class _DepthCam : public _JSONbase
	{
	public:
		_DepthCam();
		~_DepthCam();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual void console(void *pConsole);

	protected:
		void send(void);
		void heartbeat(void);

		//msg handlers
		void handleMsg(string &str);
		void detectPos(picojson::object &o);
		void setSensorOption(picojson::object &o);
		void getSensorOption(picojson::object &o);
		void setRangeFilter(picojson::object &o);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_DepthCam *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_DepthCam *)This)->updateR();
			return NULL;
		}

	public:
		_Thread *m_Tr;
		_RealSense *m_pRS;
		vFloat4 m_vBBhb;
		_InRange* m_pIR;

		bool m_bHeartbeat;
		vFloat3 m_ofsRobot;
	};

}
#endif
