/*
 * UGLIDcellStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__UGLIDcellStream__H_
#define OpenKAI_src__DataStream__UGLIDcellStream__H_

#include "DataObjStream.h"
#include "../Primitive/UUID128.h"

namespace kai
{
	struct UGLID_CELL_T
	{
		/*
		cell ID format:
		128 bit width from MSB to LSB
		[127:126] 2 reserved bits, zero
		[125:120] 6-bit cell depth (0 = root, 40 = deepest)
		[119:0] forty 3-bit child indices, from level 0 at bits 119:117
		        to level 39 at bits 2:0; unused path segments are zero
		*/
		UUID128 m_UGLID = 0;
		uint64_t m_tStamp = 0;

		void clear(void)
		{
			m_UGLID = 0;
			m_tStamp = 0;
		}

		void set(const UUID128 &id, uint64_t tStamp = 0)
		{
			m_UGLID = id;
			if (tStamp == 0)
				m_tStamp = getTns();
			else
				m_tStamp = tStamp;
		}
	};

	class UGLIDcellStream : public DataObjStream<UGLID_CELL_T>
	{
	public:
		UGLIDcellStream();
		virtual ~UGLIDcellStream();
		void console(void *pConsole) override;

		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		bool clear(size_t nBuf);

	protected:
		uint32_t m_nBuf = 1000;
	};

}
#endif
