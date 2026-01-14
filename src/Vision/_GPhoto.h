/*
 * _GPhoto.h
 *
 *  Created on: Feb 20, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__GPhoto_H_
#define OpenKAI_src_Vision__GPhoto_H_

#include "_VisionBase.h"

namespace kai
{

	class _GPhoto : public _VisionBase
	{
	public:
		_GPhoto();
		virtual ~_GPhoto();

		virtual bool init(const json &j);
		virtual bool start(void);

		bool open(void);
		void close(void);
		bool shutter(string &fName);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GPhoto *)This)->update();
			return NULL;
		}

	protected:
		string m_cmdUnmount;
	};

}
#endif
