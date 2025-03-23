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

		// BASE
		virtual int init(void *pKiss);
        virtual int start(void);
		virtual int check(void);

        // _GeometryBase
		virtual int initGeometry(void);
        virtual void clear(void);
        virtual void addPCstream(void* p, const uint64_t& tExpire);
        virtual void addPCframe(void* p);
        virtual void addPCgrid(void* p);

        virtual void writeSharedMem(void);
        virtual void readSharedMem(void);

        // _PCframe
		virtual void swapBuffer(void);
		virtual PointCloud* getBuffer(void);
		virtual PointCloud* getNextBuffer(void);

   		virtual void copyTo(PointCloud *pPC);
        virtual int nP(void);
        virtual int nPnext(void);

    private:
        void updatePCframe(void);
        void update(void);
        static void *getUpdate(void *This)
        {
            ((_PCframe *)This)->update();
            return NULL;
        }

	protected:
		// frame buf
		int m_nPresv;		// max number of reserved point buf
		int m_nPresvNext;
		tSwap<PointCloud> m_sPC;
		uint64_t m_tStamp;

        GEOMETRY_POINT *m_pGpSM;
//        int m_nGpSM;
	};

}
#endif
