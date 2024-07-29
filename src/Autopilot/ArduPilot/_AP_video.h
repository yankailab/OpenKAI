#ifndef OpenKAI_src_Autopilot_AP__AP_video_H_
#define OpenKAI_src_Autopilot_AP__AP_video_H_

#include "../../IO/_File.h"
#include "../../Net/_Uploader.h"
#include "_AP_base.h"
#include "../../Utility/utilCV.h"

using namespace picojson;

namespace kai
{

	class _AP_video : public _ModuleBase
	{
	public:
		_AP_video();
		~_AP_video();

		int init(void *pKiss);
		int link(void);
		int start(void);
		void update(void);
		int check(void);
		void console(void *pConsole);

	private:
		void onResume(void);
		void onPause(void);

		bool openStream(void);
		void closeStream(void);
		void writeStream(void);
		static void *getUpdate(void *This)
		{
			((_AP_video *)This)->update();
			return NULL;
		}

	private:
		_AP_base *m_pAP;
		_Uploader* m_pCurl;
		string m_fName;

		string m_process;
		FILE* m_pFvid;
		uint64_t m_tRecStart;

		bool m_bMeta;
		_File* m_pFmeta;
		string m_fCalib;
		Mat m_mC;
		Mat m_mD;
		string m_dir;
	};

}
#endif
