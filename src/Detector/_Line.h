/*
 * _Line.h
 *
 *  Created on: April 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__Line_H_
#define OpenKAI_src_Detector__Line_H_

#include "_DetectorBase.h"

namespace kai
{

	class _Line : public _DetectorBase
	{
	public:
		_Line();
		virtual ~_Line();

		virtual bool init(const json& j);
		virtual bool link(const json& j, ModuleMgr* pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	private:
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Line *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV;
		float m_wSlide;
		float m_minPixLine;
		float m_line;
		Mat m_mIn;
	};

}
#endif
