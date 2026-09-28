/*
 * PCLframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__PCLframe__H_
#define OpenKAI_src__DataStream__PCLframe__H_

#include "DataStreamBase.h"
#include <memory>

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

	class PCLframe : public DataStreamBase
	{
	public:
		struct Snapshot
		{
			vector<GEOMETRY_POINT> m_vPoints;
			uint64_t m_tStamp = 0;
			uint64_t m_revision = 0; // zero means no frame has been published
		};
		using SnapshotPtr = shared_ptr<const Snapshot>;

		PCLframe();
		virtual ~PCLframe();
		void console(void *pConsole) override;

		// Readers retain immutable storage without copying or locking while processing.
		SnapshotPtr get(void) const;
		// Pass std::move(points) to transfer a complete frame, including an empty frame.
		void set(vector<GEOMETRY_POINT> points, uint64_t tStamp = 0);

	private:
		SnapshotPtr m_snapshot;
		mutable std::shared_mutex m_sMutex;
	};

}
#endif
