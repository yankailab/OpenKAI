/*
 * _ImgFile.h
 *
 *  Created on: Aug 5, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__ImgFile_H_
#define OpenKAI_src_Vision__ImgFile_H_

#include "_VisionBase.h"

namespace kai
{

	class _ImgFile : public _VisionBase
	{
	public:
		_ImgFile();
		virtual ~_ImgFile();

		int init(void *pKiss);
		int start(void);

	private:
		bool open(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ImgFile *)This)->update();
			return NULL;
		}

	public:
		string m_file;
	};

}
#endif
