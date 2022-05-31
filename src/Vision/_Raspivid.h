/*
 * _Raspivid.h
 *
 *  Created on: May 6, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Raspivid_H_
#define OpenKAI_src_Vision__Raspivid_H_

#include "_VisionBase.h"

namespace kai
{

	enum RASPIVID_MODE
	{
		raspivid_rgb,
		raspivid_y,
	};

	class _Raspivid : public _VisionBase
	{
	public:
		_Raspivid();
		virtual ~_Raspivid();

		bool init(void *pKiss);
		bool start(void);

	private:
		bool open(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Raspivid *)This)->update();
			return NULL;
		}

	public:
		string m_cmd;
		string m_option;
		RASPIVID_MODE m_cMode;
		FILE *m_pFr;
		int m_iFr;

		int m_nFB;
		uint8_t *m_pFB;
	};

}
#endif
