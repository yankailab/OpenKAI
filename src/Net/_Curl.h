/*
 * _Curl.h
 *
 *  Created on: May 30, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Net__Curl_H_
#define OpenKAI_src_Net__Curl_H_

#include "../../Base/_ModuleBase.h"
#include "../../Script/JSON.h"

namespace kai
{

	class _Curl : public _ModuleBase
	{
	public:
		_Curl(void);
		virtual ~_Curl();

		bool init(void *pKiss);
		bool start(void);
		void draw(void *pFrame);
		int check(void);

	private:
		void updateCurl(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_Curl *)This)->update();
			return NULL;
		}

	private:
		string m_dir;
	};

}
#endif
