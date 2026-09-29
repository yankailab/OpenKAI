/*
 * UGLIDcellStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__UGLIDcellStream__H_
#define OpenKAI_src__DataStream__UGLIDcellStream__H_

#include "DataObjBase.h"

namespace kai
{
	class UGLIDcellStream : public DataObjBase
	{
	public:
		UGLIDcellStream();
		virtual ~UGLIDcellStream();
		void console(void *pConsole) override;


	protected:
	};

}
#endif
