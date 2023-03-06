#ifndef OpenKAI_src_File__PhotoTake_H_
#define OpenKAI_src_File__PhotoTake_H_

#include "../../Vision/_VisionBase.h"
#include "../../Utility/BitFlag.h"
#include "../../Utility/utilEvent.h"
#include "../_FileBase.h"

namespace kai
{
    enum PHOTOTAKE_ACTION
	{
		pt_shutter,
	};

	class _PhotoTake : public _FileBase
	{
	public:
		_PhotoTake();
		~_PhotoTake();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

		void setPos(const vDouble3& vPos);
		void setDelay(const uint64_t& tDelay);
		bool shutter(void);
		void setInterval(const uint64_t& tInterval);
		bool startAutoMode(int nTake = 0);
		void stopAutoMode(void);

	private:
		bool take(void);
		void updateTake(void);
		static void *getUpdate(void *This)
		{
			((_PhotoTake *)This)->update();
			return NULL;
		}

	private:
		_VisionBase *m_pV;

		BIT_FLAG m_bfProcess;
		int m_iTake;
		int m_nTake;
		vDouble3 m_vPos;
		uint64_t m_tDelay;
		bool m_bAuto;
		INTERVAL_EVENT m_ieShutter;

		string m_dir;
		string m_subDir;
		bool m_bFlip;
		vector<int> m_compress;
		int m_quality;
	};

}
#endif
