/*
 * _GDimgUploader.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__GDimgUploader_H_
#define OpenKAI_src_Application__GDimgUploader_H_

#include "../../Vision/_VisionBase.h"
#include "../../Module/JSON.h"

namespace kai
{

	class _GDimgUploader : public _ModuleBase
	{
	public:
		_GDimgUploader(void);
		virtual ~_GDimgUploader();

		bool init(void *pKiss);
		bool start(void);
		int check(void);

	private:
		void updateUpload(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GDimgUploader *)This)->update();
			return NULL;
		}

	private:
		_VisionBase *m_pV;

		uint64_t m_tInterval;
		uint64_t m_tLastUpload;
		vector<int> m_vJPGquality;
		string m_tempDir;
		fstream m_fMeta;
		string m_gdUpload;
		string m_gdFolderID;
		string m_gdCredentials;
	};

}
#endif
