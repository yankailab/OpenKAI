#ifndef OpenKAI_src_Autopilot_MavAP__MavAP_photo_H_
#define OpenKAI_src_Autopilot_MavAP__MavAP_photo_H_

#include "../../IO/_File.h"
#include "../../Net/_Uploader.h"
#include "_MavAP_base.h"
#include "../../Utility/util.h"

using namespace picojson;

namespace kai
{

	class _MavAP_videoStream : public _ModuleBase
	{
	public:
		_MavAP_videoStream();
		~_MavAP_videoStream();

		int init(void *pKiss);
		int link(void);
		int start(void);
		void update(void);
		int check(void);
		void console(void *pConsole);

	private:
		void updateStream(void);
		bool openStream(void);
		void closeStream(void);
		static void *getUpdate(void *This)
		{
			((_MavAP_videoStream *)This)->update();
			return NULL;
		}

	private:
		_MavAP_base *m_pAP;
		_Uploader* m_pCurl;
		string m_fName;
		string m_process;
		string m_dir;

		int m_iWP;
		vector<int> m_vWP;

		int m_gstPID;
		uint64_t m_tVidInt;
		uint64_t m_tRecStart;
	};

}
#endif
