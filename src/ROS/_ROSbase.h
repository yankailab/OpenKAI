/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS_ROSbase_H_
#define OpenKAI_src_ROS_ROSbase_H_

#include "../../IPC/_SharedMem.h"
#include "../../UI/_Console.h"

namespace kai
{
	class _ROSbase : public _ModuleBase
	{
	public:
		_ROSbase();
		virtual ~_ROSbase();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual int check(void);
		virtual void console(void *pConsole);
		virtual void draw(void *pFrame);

		virtual bool open(void);
		virtual void close(void);

	protected:
		// post processing thread
		_Thread *m_pTPP;


	};

}
#endif
