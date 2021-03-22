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

		bool init(T dThold)
		{
			m_dThold = dThold;
			return true;
		}

		T* update(T* pV, T dT)
		{
			if(pV)
			{
				FilterBase<T>::m_v = *pV;
			}
			else if(dT >= m_dThold)
			{
				return NULL;
			}

			return FilterBase<T>::m_pV;
		}

	private:
		T m_dThold;
	};

}
#endif
