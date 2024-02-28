/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud__PCframe_H_
#define OpenKAI_src_3D_PointCloud__PCframe_H_

#include "../_GeometryBase.h"

namespace kai
{
	class _PCframe : public _GeometryBase
	{
	public:
		_PCframe();
		virtual ~_PCframe();

		virtual bool init(void *pKiss);
		virtual int check(void);

		virtual bool initBuffer(void);
		virtual void swapBuffer(void);
		virtual PointCloud* getBuffer(void);
		virtual PointCloud* getNextBuffer(void);
        virtual void clear(void);

        virtual void getPCstream(void* p, const uint64_t& tExpire);
        virtual void getPCframe(void* p);
        virtual void getPCgrid(void* p);

   		virtual void copyTo(PointCloud *pPC);
        virtual int nP(void);
        virtual int nPnext(void);

	protected:
		// frame buf
		int m_nPresv;		// max number of reserved point buf
		int m_nPresvNext;
		tSwap<PointCloud> m_sPC;
		uint64_t m_tStamp;
	};

}
#endif
