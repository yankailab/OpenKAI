/*
 * _Chilitags.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#include "_Chilitags.h"

#ifdef USE_OPENCV
#ifdef USE_CHILITAGS

namespace kai
{

    _Chilitags::_Chilitags()
    {
        m_persistence = 0;
        m_gain = 0.0;
        m_angleOffset = 0.0;
    }

    _Chilitags::~_Chilitags()
    {
    }

    bool _Chilitags::init(void *pKiss)
    {
        IF_F(!this->_DetectorBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("persistence", &m_persistence);
        pK->v("gain", &m_gain);
        pK->v("angleOffset", &m_angleOffset);

        m_chilitags.setFilter(m_persistence, m_gain);

        return true;
    }

    bool _Chilitags::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _Chilitags::check(void)
    {
        NULL__(m_pV, -1);
        NULL__(m_pU, -1);

        return this->_DetectorBase::check();
    }

    void _Chilitags::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();

            if (check() >= 0)
            {
                detect();

                if (m_pT->bGoSleep())
                    m_pU->m_pPrev->clear();
            }

            m_pT->autoFPSto();
        }
    }

    void _Chilitags::detect(void)
    {
        Mat m = *m_pV->BGR()->m();
        IF_(m.empty());

        // The resulting map associates tag ids (between 0 and 1023)
        // to four 2D points corresponding to the corners positions
        // in the picture.
        TagCornerMap tags = m_chilitags.find(m);

        _Object o;
        float dx, dy;
        float kx = 1.0 / (float)m.cols;
        float ky = 1.0 / (float)m.rows;

        for (const std::pair<int, chilitags::Quad> &tag : tags)
        {
            o.init();
            //        o.m_tStamp = m_pT->getTfrom();
            o.setTopClass(tag.first, 1.0);

            // We wrap the corner matrix into a datastructure that allows an easy access to the coordinates
            const cv::Mat_<cv::Point2f> corners(tag.second);

            // bbox
            vFloat2 pV[4];
            for (size_t i = 0; i < 4; i++)
            {
                pV[i].x = corners(i).x;
                pV[i].y = corners(i).y;
            }
            o.setVertices2D(pV, 4);
            o.scale(kx, ky);

            // distance
            if (m_pDV)
            {
                vFloat4 bb = o.getBB2D();
                o.setZ(m_pDV->d(&bb));
            }

            // center position
            dx = (float)(pV[0].x + pV[1].x + pV[2].x + pV[3].x) * 0.25;
            dy = (float)(pV[0].y + pV[1].y + pV[2].y + pV[3].y) * 0.25;
            o.setX(dx * kx);
            o.setY(dy * ky);

            // radius
            dx -= pV[0].x;
            dy -= pV[0].y;
            o.setRadius(sqrt(dx * dx + dy * dy));

            // angle in deg
            dx = pV[3].x - pV[0].x;
            dy = pV[3].y - pV[0].y;
            o.setRoll(Hdg(-atan2(dx, dy) * RAD_2_DEG + 180.0 + m_angleOffset));

            m_pU->add(o);
            LOG_I("ID: " + i2str(o.getTopClass()));
        }

        m_pU->updateObj();
    }

    void _Chilitags::console(void *pConsole)
    {
        NULL_(pConsole);
        IF_(check() < 0);
        this->_DetectorBase::console(pConsole);

        string msg = "| ";
        _Object *pO;
        int i = 0;
        while ((pO = m_pU->get(i++)) != NULL)
        {
            msg += i2str(pO->getTopClass()) + "(" + f2str(pO->getZ()) + ") | ";
        }
        ((_Console *)pConsole)->addMsg(msg, 1);
    }

    void _Chilitags::cvDraw(void *pWindow)
    {
        NULL_(pWindow);
        this->_DetectorBase::cvDraw(pWindow);
        IF_(check() < 0);

        _WindowCV *pWin = (_WindowCV *)pWindow;
        Frame *pF = pWin->getFrame();
        NULL_(pF);
        Mat *pM = pF->m();
        IF_(pM->empty());

        IF_(m_pU->size() <= 0);

        int i = 0;
        while ((pO = m_pU->get(i++)) != NULL)
        {
            Point pCenter = Point(pO->getX() * pM->cols,
                                  pO->getY() * pM->rows);
            circle(*pM, pCenter, pO->getRadius(), Scalar(255, 255, 0), 2);

            putText(*pM, "iTag=" + i2str(pO->getTopClass()) + ", angle=" + i2str(pO->getRoll()),
                    pCenter,
                    FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 0);

            double rad = -pO->getRoll() * DEG_2_RAD;
            Point pD = Point(pO->getRadius() * sin(rad), pO->getRadius() * cos(rad));
            line(*pM, pCenter + pD, pCenter - pD, Scalar(0, 0, 255), 2);
        }
    }

}
#endif
#endif
