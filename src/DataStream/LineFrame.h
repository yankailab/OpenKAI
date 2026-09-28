/*
 * LineFrame.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__LineFrame__H_
#define OpenKAI_src__DataStream__LineFrame__H_

#include "DataStreamBase.h"
#include <memory>

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

	class LineFrame : public DataStreamBase
	{
	public:
		struct Snapshot
		{
			vector<GEOMETRY_LINE> m_vLines;
			uint64_t m_tStamp = 0;
			uint64_t m_revision = 0; // zero means no frame has been published
		};
		using SnapshotPtr = shared_ptr<const Snapshot>;

		LineFrame();
		virtual ~LineFrame();
		void console(void *pConsole) override;

		// Readers retain immutable storage without copying or locking while processing.
		SnapshotPtr get(void) const;
		// Pass std::move(lines) to transfer a complete frame, including an empty frame.
		void set(vector<GEOMETRY_LINE> lines, uint64_t tStamp = 0);

	private:
		SnapshotPtr m_snapshot;
		mutable std::shared_mutex m_sMutex;
	};

}
#endif
