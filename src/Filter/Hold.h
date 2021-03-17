/*
 * Hold.h
 *
 *  Created on: Mar 18, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Hold_H_
#define OpenKAI_src_Filter_Hold_H_

#include "FilterBase.h"

namespace kai
{

	template <class T>
	class Hold : public FilterBase<T>
	{
	public:
		Hold<T>()
		{
		}
		virtual ~Hold()
		{			
		}

		bool init(uint64_t tHold, T m_vTover)
		{
			m_tHold = tHold;
			m_tLastUpdate = 0;
			return true;
		}

		T input(T v, bool b, uint64_t t)
		{
			if(!b)
			{
				uint64_t dT = t - m_tLastUpdate;
				IF__(dT < m_tHold, FilterBase<T>::m_v);
				return m_vTover;
			}

			FilterBase<T>::m_v = v;
			m_tLastUpdate = t;
			return FilterBase<T>::m_v;
		}

	private:
		uint64_t m_tHold;
		uint64_t m_tLastUpdate;
		T m_vTover;
	};

}
#endif
