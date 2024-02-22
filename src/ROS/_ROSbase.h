/*
 * _ROSbase.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_ROS__ROSbase_H_
#define OpenKAI_src_ROS__ROSbase_H_

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include "ROSnode.h"

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
		virtual bool start(void);
		virtual void console(void *pConsole);

	private:
		void updateROS(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ROSbase *)This)->update();
			return NULL;
		}

	protected:
		shared_ptr<ROSnode> m_pROSnode;
		string m_topic;

	};

}
#endif
