#ifndef OpenKAI_src_Autopilot_ArduPilot__APmav_photo_H_
#define OpenKAI_src_Autopilot_ArduPilot__APmav_photo_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "../../Sensor/Distance/_DistSensorBase.h"
#include "_APmav_base.h"
#include "_APmav_move.h"

namespace kai
{

	class _APmav_photo : public _ModuleBase
	{
	public:
		_APmav_photo();
		~_APmav_photo();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual void update(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		void updatePhoto(void);
		static void *getUpdate(void *This)
		{
			((_APmav_photo *)This)->update();
			return NULL;
		}

	protected:
		_APmav_base *m_pAP = nullptr;
		int m_iTake = 0;
		string m_dir = "/home/";
		string m_subDir = "";
		string m_exifConfig = "";
	};

}
#endif
