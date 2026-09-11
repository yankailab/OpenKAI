/*
 * _Frame2Img.h
 *
 *  Created on: June 11, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Frame2Img_H_
#define OpenKAI_src_Vision__Frame2Img_H_

#include "_VisionBase.h"

namespace kai
{

	class _Frame2Img : public _VisionBase
	{
	public:
		_Frame2Img();
		virtual ~_Frame2Img();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void console(const json &j, void *pJSONbase);

	private:
		void recFrame(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Frame2Img *)This)->update();
			return NULL;
		}

	protected:
		_VisionBase *m_pV = nullptr;

		string m_dir = "";
		string m_dirRec = "";
		string m_tRec = "";
		int m_iFrame = 0;
		int m_pngCompression = 3;
		float m_vOffset = 50.0;
		float m_vScale = 64.0;
	};

}
#endif
