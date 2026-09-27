/*
 * _Line.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_Line__Line_H_
#define OpenKAI_src_Universe_Geometry_Line__Line_H_

#include "../_GeometryBase.h"
#include "../../../Protocol/_JSONbase.h"
#include <mutex>

namespace kai
{
    class _Line : public _GeometryBase
    {
    public:
        _Line();
        virtual ~_Line();

        // BASE
        virtual bool loadConfig(void) override;
        bool saveConfig(bool bExport) override;
        virtual bool start(void);
        virtual bool check(void);
        virtual void console(void *pConsole);
        virtual void console(const json &j, void *pJSONbase);

        // _GeometryBase
        virtual void clear(void);
        virtual int get(RingBuffer<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);

        // data io
        virtual void add(const Vector3f &vPa, const Vector3f &vPb, const Vector3f &vC, uint64_t tStamp = 1);

    protected:
        virtual int copy(RingBuffer<GEOMETRY_LINE> *pIn, RingBuffer<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);
        virtual RingBuffer<GEOMETRY_LINE>* getRingBuf(void);

    private:
        void updateLine(void);
        virtual void update(void);
        static void *getUpdate(void *This)
        {
            ((_Line *)This)->update();
            return NULL;
        }

    protected:
        RingBuffer<GEOMETRY_LINE> m_rLn;
        std::mutex m_mtxLn;
    };

}
#endif
