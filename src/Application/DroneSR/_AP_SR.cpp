#include "_AP_SR.h"

namespace kai
{

	_AP_SR::_AP_SR()
	{
		m_pV = NULL;
		m_pU = NULL;
		m_pAP = NULL;
		m_pCurl = NULL;

		m_fName = "";
		m_dir = "";

		m_iWP = INT_MAX;
		m_vWPrange.set(1,INT_MAX);

		m_vClass.clear();
	}

	_AP_SR::~_AP_SR()
	{
	}

	bool _AP_SR::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("fName", &m_fName);
		pK->v("dir", &m_dir);
		pK->v("vWPrange", &m_vWPrange);
		pK->a("vClass", &m_vClass);

		return true;
	}

	bool _AP_SR::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		NULL_Fl(m_pV, n + ": not found");

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->getInst(n));
		NULL_Fl(m_pU, n + ": not found");

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->getInst(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_Curl", &n);
		m_pCurl = (_Curl *)(pK->getInst(n));

		return true;
	}

	bool _AP_SR::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _AP_SR::check(void)
	{
		NULL__(m_pV, -1);
		NULL__(m_pU, -1);
		NULL__(m_pAP, -1);
		NULL__(m_pCurl, -1);

		return this->_ModuleBase::check();
	}

	void _AP_SR::update(void)
	{
		while (m_pT->bThread())
		{
			m_pT->autoFPSfrom();

			updateSR();

			m_pT->autoFPSto();
		}
	}

	void _AP_SR::updateSR(void)
	{
		IF_(check() < 0);


		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			IF_CONT(pO->getTopClass() != m_vClass);
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		NULL_F(tO);
		vFloat4 vTbb = tO->getBB2D();

		Frame* pRGB = m_pV->getFrameRGB();

	}

	void _AP_SR::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		pC->addMsg("fName = " + m_fName);
	}

}
