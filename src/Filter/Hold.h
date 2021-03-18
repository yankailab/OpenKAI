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

		bool init(T dThold, T m_vTover)
		{
			m_dThold = dThold;
			return true;
		}

		T input(T v, bool b, T dT)
		{
			if(!b)
			{
				IF__(dT < m_dThold, FilterBase<T>::m_v);
				return m_vTover;
			}

			FilterBase<T>::m_v = v;
			return FilterBase<T>::m_v;
		}

	private:
		T m_dThold;
		T m_vTover;
	};

}
#endif
