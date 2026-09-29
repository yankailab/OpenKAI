/*
 * PCLframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__PCLframe__H_
#define OpenKAI_src__DataStream__PCLframe__H_

#include "DataObjBase.h"

namespace kai
{
    struct GEOMETRY_POINT
    {
        Vector3f m_vP = Vector3f::Zero();	// pos
        Vector3f m_vC = Vector3f::Zero();	// color
        uint64_t m_tStamp = 0;				// nanosecond timestamp, 0: invalid, >= 1 valid

        void clear(void)
        {
            m_vP.setZero();
            m_vC.setZero();
            m_tStamp = 0;
        }
    };

	class PCLframe : public DataObjBase
	{
	public:
		PCLframe();
		virtual ~PCLframe();
		void console(void *pConsole) override;

		void set(const vector<GEOMETRY_POINT> &src, uint64_t tStamp = 0);
		uint64_t get(vector<GEOMETRY_POINT> &dest);

	private:
		vector<GEOMETRY_POINT> m_vPoints;
		std::shared_mutex m_sMutex;
	};

}
#endif
