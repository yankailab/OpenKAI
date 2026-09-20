/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry__GeometryBase_H_
#define OpenKAI_src_Universe_Geometry__GeometryBase_H_

#include "../_ReferenceFrame.h"

namespace kai
{
    enum GEOMETRY_TYPE
    {
        geometry_unknown = 0,
        geometry_pointCloud = 1,
        geometry_line = 2,
        geometry_mesh = 3,
    };

    struct GEOMETRY_POINT
    {
        Vector3f m_vP = Vector3f::Zero(); // pos
        Vector3f m_vC{0, 0, 0};           // color
        uint64_t m_tStamp;

        void clear(void)
        {
            m_vP.setZero();
            m_vC.setZero();
            m_tStamp = 0; // time stamp, 0: invalid, >= 1 valid
        }
    };

    struct GEOMETRY_LINE
    {
        Vector3f m_vPa = Vector3f::Zero(); // line from
        Vector3f m_vPb = Vector3f::Zero(); // line to
        Vector3f m_vC{0, 0, 0};            // color
        uint64_t m_tStamp;                 // time stamp, 0: invalid, >= 1 valid

        void clear(void)
        {
            m_vPa.setZero();
            m_vPb.setZero();
            m_vC.setZero();
            m_tStamp = 0;
        }
    };

    template <typename T>
    struct GEOMETRY_RINGBUF
    {
        T *m_pT = nullptr;
        int m_nT = 0;
        int m_iT = 0;

        bool alloc(int nP)
        {
            IF_F(nP <= 0);

            m_pT = new T[nP];
            NULL_F(m_pT);

            m_nT = nP;
            return true;
        }

        void release(void)
        {
            delete[] m_pT;
            m_pT = nullptr;

            m_nT = 0;
            m_iT = 0;
        }

        void clear(void)
        {
            NULL_(m_pT);
            IF_(m_nT <= 0);

            m_iT = 0;
            for (int i = 0; i < m_nT; i++)
                m_pT[i].clear();
        }

        void add(const T &p)
        {
            NULL_(m_pT);
            IF_(m_nT <= m_iT);

            m_pT[m_iT] = p;
            iInc();
        }

        void iInc(void)
        {
            if (++m_iT >= m_nT)
                m_iT = 0;
        }

        int iLastT(void)
        {
            IF__(m_iT <= 0, 0);
            return m_iT - 1;
        }

        int nT(void)
        {
            return m_nT;
        }

        int iDec(int i)
        {
            if (--i < 0)
                i = m_nT - 1;

            return i;
        }

        T *get(int i)
        {
            NULL_N(m_pT);
            IF_N(m_nT <= i);

            return &m_pT[i];
        }
    };

    class _GeometryBase : public _ReferenceFrame
    {
    public:
        _GeometryBase();
        virtual ~_GeometryBase();

        virtual bool init(const json &j);
        virtual bool link(const json &j, ModuleMgr *pM);
        virtual bool check(void);
        virtual void console(void *pConsole);

        virtual bool loadConfig(json *pJ = nullptr, string fName = "");
        virtual bool saveConfig(json &j, string fName = "");

        virtual GEOMETRY_TYPE getType(void);
        virtual void clear(void);

        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_POINT> *pOut, uint64_t tExpire = 0);
        virtual int get(GEOMETRY_RINGBUF<GEOMETRY_LINE> *pOut, uint64_t tExpire = 0);

        // Sources without framed point clouds return an empty frame.
        virtual int getLastFrame(vector<Vector3f> *pvP, vector<Vector3f> *pvC, uint64_t &tStamp);

    protected:
        GEOMETRY_TYPE m_type = geometry_unknown;

    };

}
#endif
