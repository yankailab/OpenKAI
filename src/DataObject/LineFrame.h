/*
 * LineFrame.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__LineFrame__H_
#define OpenKAI_src__DataStream__LineFrame__H_

#include "DataObjBase.h"

namespace kai
{
    struct GEOMETRY_LINE
    {
        Vector3f m_vPa = Vector3f::Zero(); // line from
        Vector3f m_vPb = Vector3f::Zero(); // line to
        Vector3f m_vC{0, 0, 0};            // color
        uint64_t m_tStamp = 0;                 // nanosecond timestamp, 0: invalid, >= 1 valid

        void clear(void)
        {
            m_vPa.setZero();
            m_vPb.setZero();
            m_vC.setZero();
            m_tStamp = 0;
        }
    };

	class LineFrame : public DataObjBase
	{
	public:
		LineFrame();
		virtual ~LineFrame();
		void console(void *pConsole) override;

		void set(const vector<GEOMETRY_LINE> &src, uint64_t tStamp = 0);
		// Copy data and return its timestamp under the same lock.
		uint64_t get(vector<GEOMETRY_LINE> &dest);

	private:
		vector<GEOMETRY_LINE> m_vLines;
		std::shared_mutex m_sMutex;
	};

}
#endif
