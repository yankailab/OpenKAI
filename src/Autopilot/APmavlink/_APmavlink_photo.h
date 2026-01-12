#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_photo_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_photo_H_

#include "../../Vision/RGBD/_RGBDbase.h"
#include "../../Vision/_GPhoto.h"
#include "../../Sensor/Distance/_DistSensorBase.h"
#include "_APmavlink_base.h"
#include "_APmavlink_move.h"

namespace kai
{

	class _APmavlink_photo : public _ModuleBase
	{
	public:
		_APmavlink_photo();
		~_APmavlink_photo();

		bool init(const json& j);
		bool link(const json& j, ModuleMgr* pM);
		bool start(void);
		void update(void);
		bool check(void);
		void console(void *pConsole);

	private:
		void updatePhoto(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_photo *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP;
		int m_iTake;
		string m_dir;
		string m_subDir;
		string m_exifConfig;
	};

}
#endif
