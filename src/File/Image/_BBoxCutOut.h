/*
 * _BBoxCutOut.h
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_File_Image__BBoxCutOut_H_
#define OpenKAI_src_File_Image__BBoxCutOut_H_

#include "../../Base/_ModuleBase.h"

namespace kai
{

	class _BBoxCutOut : public _ModuleBase
	{
	public:
		_BBoxCutOut();
		~_BBoxCutOut();

		int init(void *pKiss);
		int start(void);

	private:
		void process();
		void update(void);
		static void *getUpdate(void *This)
		{
			((_BBoxCutOut *)This)->update();
			return NULL;
		}

	public:
		string m_dirIn;
		string m_dirOut;
		string m_extTxt;
		string m_extImgIn;
		string m_extImgOut;
	};

}
#endif
