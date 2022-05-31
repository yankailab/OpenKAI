/*
 * _VideoFile.h
 *
 *  Created on: Aug 5, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__VideoFile_H_
#define OpenKAI_src_Vision__VideoFile_H_

#include "_VisionBase.h"

namespace kai
{

	class _VideoFile : public _VisionBase
	{
	public:
		_VideoFile();
		virtual ~_VideoFile();

		bool init(void *pKiss);
		bool start(void);
		void close(void);

	private:
		bool open(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_VideoFile *)This)->update();
			return NULL;
		}

	public:
		VideoCapture m_vc;
		string m_videoFile;
	};

}
#endif
