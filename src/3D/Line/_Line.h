/*
 * _Line.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_Line__Line_H_
#define OpenKAI_src_3D_Line__Line_H_

#include "../_GeometryBase.h"
#include "../../Protocol/_JSONbase.h"

namespace kai
{
    class _Line : public _GeometryBase
    {
    public:
        _Line();
        virtual ~_Line();

        // BASE
        virtual bool init(const json &j);
        virtual bool start(void);
        virtual bool check(void);
        virtual void console(void *pConsole);
        virtual void console(const json &j, void *pJSONbase);

        // _GeometryBase
        virtual void clear(void);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLout, uint64_t dTexpire = 0);

        // data io
        virtual void add(const Vector3d &vPa, const Vector3d &vPb, const Vector3f &vC, uint64_t tStamp = 1);
        virtual void add(const vFloat3 &vPa, const vFloat3 &vPb, const vFloat3 &vC, uint64_t tStamp = 1);
        virtual int add(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pGrLIn, uint64_t dTexpire = 0);

        virtual GEOMETRY_RINGBUF<GEOMETRY_LINE>* getRingBuf(void);

    private:
        void updateLine(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_Line *)This)->update();
            return NULL;
        }

    protected:
        GEOMETRY_RINGBUF<GEOMETRY_LINE> m_grLn;
    };

}
#endif
