#include "_GSVcollisionAvoid.h"

namespace kai
{

    _GSVcollisionAvoid::_GSVcollisionAvoid()
    {
    }

    _GSVcollisionAvoid::~_GSVcollisionAvoid()
    {
    }

    bool _GSVcollisionAvoid::init(void *pKiss)
    {
        IF_F(!this->_ModuleBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

//        pK->v("ID", &m_ID);

        return true;
    }

	bool _GSVcollisionAvoid::link(void)
	{
		IF_F(!this->_ModuleBase::link());
		Kiss *pK = (Kiss *)m_pKiss;

        string n;
        // n = "";
        // pK->v("_Modbus", &n);
        // m_pMB = (_Modbus *)(pK->getInst(n));
        // IF_Fl(!m_pMB, n + ": not found");

		return true;
	}

    bool _GSVcollisionAvoid::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    int _GSVcollisionAvoid::check(void)
    {
//        NULL__(m_pMB, -1);
//        IF__(!m_pMB->bOpen(), -1);

        return this->_ModuleBase::check();
    }

    void _GSVcollisionAvoid::update(void)
    {
        while (m_pT->bRun())
        {
            m_pT->autoFPSfrom();
            // this->_ModuleBase::update();
            // this->_ModuleBase::updateGCS();

            updateGSV();

            m_pT->autoFPSto();
        }
    }

    void _GSVcollisionAvoid::updateGSV(void)
    {
        IF_(check() < 0);

    }

	void _GSVcollisionAvoid::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("""), 0);
	}

    void _GSVcollisionAvoid::draw(void* pFrame)
	{
		NULL_(pFrame);
		this->_ModuleBase::draw(pFrame);
		IF_(check() < 0);
//		IF_(m_fRGB.bEmpty());

		Frame *pF = (Frame*)pFrame;
//		pF->copy(m_fDepth);

		// if (m_bDebug)
		// {
		// 	Mat *pM = pF->m();
		// 	IF_(pM->empty());

		// 	vFloat4 vRoi(0.4, 0.4, 0.6, 0.6);

		// 	vFloat4 bb;
		// 	bb.x = vRoi.x * pM->cols;
		// 	bb.y = vRoi.y * pM->rows;
		// 	bb.z = vRoi.z * pM->cols;
		// 	bb.w = vRoi.w * pM->rows;
		// 	Rect r = bb2Rect(bb);
		// 	rectangle(*pM, r, Scalar(0, 255, 0), 1);

		// 	putText(*pM, f2str(d(vRoi)),
		// 			Point(r.x + 15, r.y + 25),
		// 			FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 1);
		// }
	}

}
