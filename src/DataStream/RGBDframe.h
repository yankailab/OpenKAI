/*
 * RGBDframe.h
 *
 *  Created on: Sep 28, 2026
 *      Author: yankai
 */

#ifndef OpenKAI_src__DataStream__RGBDframe__H_
#define OpenKAI_src__DataStream__RGBDframe__H_

#include "DataStreamBase.h"
#include <memory>

namespace kai
{
	class RGBDframe : public DataStreamBase
	{
	public:
		struct Snapshot
		{
			Mat m_mRGB;
			Mat m_mD;
			uint64_t m_tStamp = 0;
			uint64_t m_revision = 0; // zero means no frame has been published
		};
		using SnapshotPtr = shared_ptr<const Snapshot>;

		RGBDframe();
		virtual ~RGBDframe();
		void console(void *pConsole) override;

		// Snapshot pixels are read-only; clone a Mat before modifying its pixels.
		SnapshotPtr get(void) const;
		// Clone borrowed SDK or reusable buffers before publishing owned pixels.
		void set(const Mat &rgb, const Mat &depth, uint64_t tStamp = 0);

	private:
		SnapshotPtr m_snapshot;
		mutable std::shared_mutex m_sMutex;
	};

}
#endif
