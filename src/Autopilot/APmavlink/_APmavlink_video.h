#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_video_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_video_H_

#include "../../IO/_File.h"
#include "../../Net/_Uploader.h"
#include "_APmavlink_base.h"
#include "../../Utility/utilCV.h"

using namespace picojson;

namespace kai
{

	class _APmavlink_video : public _ModuleBase
	{
	public:
		_APmavlink_video();
		~_APmavlink_video();

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
			((_APmavlink_video *)This)->update();
			return NULL;
		}

	private:
		_APmavlink_base *m_pAP;
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
