#ifndef OpenKAI_src_Utility_BIT_FLAG_H_
#define OpenKAI_src_Utility_BIT_FLAG_H_

#include "../Base/common.h"

namespace kai
{

	struct BIT_FLAG
	{
		uint64_t m_flag = 0;

		bool bClear(void)
		{
			return (m_flag == 0);
		}

		void clearAll(void)
		{
			m_flag = 0;
		}

		void set(int i)
		{
			m_flag |= (1 << i);
		}

		void clear(int i)
		{
			m_flag &= ~(1 << i);
		}

		bool b(int i, bool bClearAfterRead = false)
		{
			bool b = m_flag & (1 << i);
			if (bClearAfterRead)
				clear(i);

			return b;
		}
	};

}
#endif
