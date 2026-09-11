#ifndef OpenKAI_src_Autopilot_APmavlink__APmavlink_videoStream_H_
#define OpenKAI_src_Autopilot_APmavlink__APmavlink_videoStream_H_

#include "../../IO/_File.h"
#include "../../Net/_Uploader.h"
#include "_APmavlink_base.h"
#include "../../Utility/util.h"

namespace kai
{

	class _APmavlink_videoStream : public _ModuleBase
	{
	public:
		_APmavlink_videoStream();
		~_APmavlink_videoStream();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual void update(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		void updateStream(void);
		bool openStream(void);
		void closeStream(void);
		static void *getUpdate(void *This)
		{
			((_APmavlink_videoStream *)This)->update();
			return NULL;
		}

	protected:
		_APmavlink_base *m_pAP = NULL;
		_Uploader* m_pCurl = NULL;
		string m_fName = "";
		string m_process = "";
		string m_dir = "";

		int m_iWP = INT_MAX;
		vector<int> m_vWP;

		int m_gstPID = 0;
		uint64_t m_tVidInt = 10;
		uint64_t m_tRecStart = 0;
	};

}
#endif
