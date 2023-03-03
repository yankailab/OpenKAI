#ifndef OpenKAI_src_Data__GPhotoTake_H_
#define OpenKAI_src_Data__GPhotoTake_H_

#include "../../Vision/_GPhoto.h"
#include "../_DataBase.h"

namespace kai
{

	class _GPhotoTake : public _DataBase
	{
	public:
		_GPhotoTake();
		~_GPhotoTake();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual void update(void);
		virtual void console(void *pConsole);

		bool shutter(void);
		bool startAutoMode(int nTake = 0, int tInterval = 1000);
		void stopAutoMode(void);

	private:
		void updateTake(void);
		static void *getUpdate(void *This)
		{
			((_GPhotoTake *)This)->update();
			return NULL;
		}

	private:
		_GPhoto *m_pG;

		int m_iTake;
		uint64_t m_tDelay;

		string m_dir;
		string m_subDir;
		bool m_bFlipRGB;
		bool m_bFlipD;
		vector<int> m_compress;
		int m_quality;
	};

}
#endif
