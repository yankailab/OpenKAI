/*
 * _ADIO_EBYTE.h
 *
 *  Created on: Jan 7, 2024
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__ADIO_EBYTE_H_
#define OpenKAI_src_IO__ADIO_EBYTE_H_

#include "_ADIObase.h"
#include "../Protocol/_Modbus.h"

namespace kai
{

	class _ADIO_EBYTE : public _ADIObase
	{
	public:
		_ADIO_EBYTE();
		virtual ~_ADIO_EBYTE();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
		virtual void close(void);
		virtual void console(void *pConsole);

		virtual bool open(void);

	protected:
		virtual void updateW(void);
		virtual void updateR(void);

		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_ADIO_EBYTE *)This)->update();
			return NULL;
		}

	protected:
		_Modbus *m_pMB;
		int m_iID;
	};

}
#endif
