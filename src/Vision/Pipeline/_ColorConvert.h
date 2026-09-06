/*
 * _ColorConvert.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__ColorConvert_H_
#define OpenKAI_src_Vision__ColorConvert_H_

#include "../_VisionBase.h"

namespace kai
{

	class _ColorConvert : public _VisionBase
	{
	public:
		_ColorConvert();
		virtual ~_ColorConvert();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);

	private:
		void filter(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ColorConvert *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		int m_code;
	};

}
#endif
