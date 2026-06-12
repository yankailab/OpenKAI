/*
 * _Img2Frame.h
 *
 *  Created on: June 12, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Img2Frame_H_
#define OpenKAI_src_Vision__Img2Frame_H_

#include "_VisionBase.h"

namespace kai
{

	class _Img2Frame : public _VisionBase
	{
	public:
		struct ImgFile
		{
			string file;
			string time;
			int index;
			bool bParsed;
		};

		_Img2Frame();
		virtual ~_Img2Frame();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual void close(void);
		virtual void console(const json &j, void *pJSONbase);

	private:
		bool open(void);
		bool loadFrameBuffer(void);
		void updateFrame(void);
		vector<ImgFile> getImgFiles(void);

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Img2Frame *)This)->update();
			return NULL;
		}

	protected:
		string m_dir;
		vector<Mat> m_frameBuffer;
		int m_iFrame;
		float m_vOffset;
		float m_vScale;
		bool m_bLoop;
	};

}
#endif
