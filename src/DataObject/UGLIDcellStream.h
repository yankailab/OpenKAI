/*
 * UGLIDcellStream.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__UGLIDcellStream__H_
#define OpenKAI_src__DataStream__UGLIDcellStream__H_

#include "DataObjBase.h"
#include "../Primitive/UUID128.h"

namespace kai
{
	struct UGLID_CELL_T
	{
		/*
		cell ID format:
		128 bit width from MSB to LSB
		[2 bit] 0
		[3 bit][3bit]... each 3-bit fragment correspondent to its cell index at the Level from 0 to 39, 40 levels at most (3 bit x 40 = 120 bit)
		[6 bit] cell depth (0 = root, 40 = deepest); unused path segments are zero
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

	class UGLIDcellStream : public DataObjBase
	{
	public:
		UGLIDcellStream();
		virtual ~UGLIDcellStream();
		void console(void *pConsole) override;

		bool loadConfig(void);
		bool saveConfig(bool bExport = false);

		bool clear(size_t nBuf);
		void add(const vector<UGLID_CELL_T> &vSrc, uint64_t tStamp = 0);
		uint64_t get(vector<UGLID_CELL_T> &vDest, uint64_t tStampFrom = 0);

	protected:
		uint32_t m_nBuf = 1000;
		size_t m_iBset = 0;

		vector<UGLID_CELL_T> m_vCell;

		std::shared_mutex m_sMutex;
	};

}
#endif
