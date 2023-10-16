#ifndef OpenKAI_src_Autopilot_AP__AP_photo_H_
#define OpenKAI_src_Autopilot_AP__AP_photo_H_

#include "../../IO/_File.h"
#include "../../Net/_Curl.h"
#include "_AP_base.h"
#include "../../Utility/util.h"

using namespace picojson;

namespace kai
{

	class _AP_videoStream : public _StateBase
	{
	public:
		_AP_videoStream();
		~_AP_videoStream();

		bool init(void *pKiss);
		bool link(void);
		bool start(void);
		void update(void);
		int check(void);
		void console(void *pConsole);

	private:
		void updateStream(void);
		bool openStream(void);
		void closeStream(void);
		static void *getUpdate(void *This)
		{
			((_AP_videoStream *)This)->update();
			return NULL;
		}

	private:
		_AP_base *m_pAP;
		_Curl* m_pCurl;
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
