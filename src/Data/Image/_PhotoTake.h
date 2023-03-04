#ifndef OpenKAI_src_Data__PhotoTake_H_
#define OpenKAI_src_Data__PhotoTake_H_

#include "../../Vision/_VisionBase.h"
#include "../../Utility/BitFlag.h"
#include "../../Utility/utilEvent.h"
#include "../_DataBase.h"

namespace kai
{
    enum PHOTOTAKE_ACTION
	{
		pt_ScanReset,
	};

	class _PhotoTake : public _DataBase
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
		bool shutter(void);
		bool startAutoMode(int nTake = 0, int tInterval = 1000);
		void stopAutoMode(void);

	private:
		void updateTake(void);
		static void *getUpdate(void *This)
		{
			((_PhotoTake *)This)->update();
			return NULL;
		}

	private:
		_VisionBase *m_pV;

		BIT_FLAG m_fProcess;
		INTERVAL_EVENT m_ieShutter;
		vDouble3 m_vPos;


		int m_iTake;
		uint64_t m_tDelay;


		string m_dir;
		string m_subDir;
		bool m_bFlip;
		vector<int> m_compress;
		int m_quality;
	};

}
#endif
