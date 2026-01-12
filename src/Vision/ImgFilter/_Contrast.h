/*
 * _Contrast.h
 *
 *  Created on: March 12, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Contrast_H_
#define OpenKAI_src_Vision__Contrast_H_

#include "../_VisionBase.h"

namespace kai
{

	class _Contrast : public _VisionBase
	{
	public:
		_Contrast();
		virtual ~_Contrast();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);

	private:
		void filter(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Contrast *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		double m_alpha;
		double m_beta;
	};

}
#endif
