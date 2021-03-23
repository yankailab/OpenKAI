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

		bool init(uint64_t dThold)
		{
			m_dThold = dThold;
			return true;
		}

		T *update(T *pV, uint64_t t)
		{
			if (pV)
			{
				FilterBase<T>::m_v = *pV;
				m_tLast = t;
				return FilterBase<T>::m_pV;
			}

			uint64_t dT = t - m_tLast;
			IF_N(dT >= m_dThold);

			return FilterBase<T>::m_pV;
		}

		void reset(void)
		{
			FilterBase<T>::reset();
			m_tLast = 0;
		}

	private:
		uint64_t m_dThold;
		uint64_t m_tLast;
	};

}
#endif
