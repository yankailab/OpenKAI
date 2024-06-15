#include "_AP_SR.h"

namespace kai
{

	_AP_SR::_AP_SR()
	{
		m_pV = NULL;
		m_pU = NULL;
		m_pAP = NULL;
		m_pCurl = NULL;

		m_iWP = 0;
		m_vWPrange.set(1, INT_MAX);
		m_vClass.clear();
		m_bRTL = false;
		m_modeRTL = 84148224;
		/*
		// Mode values for pf2
		// setMode(100925440); // landing
		// setMode(50593792); // Guided
		// setMode(84148224); // RTL
		*/

		m_dir = "/home/";
		m_quality = 100;
	}

	_AP_SR::~_AP_SR()
	{
	}

	bool _AP_SR::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("vWPrange", &m_vWPrange);
		pK->a("vClass", &m_vClass);
		pK->v("dir", &m_dir);
		pK->v("bRTL", &m_bRTL);
		pK->v("modeRTL", &m_modeRTL);

		m_compress.push_back(IMWRITE_JPEG_QUALITY);
		m_compress.push_back(m_quality);

		return true;
	}

	bool _AP_SR::link(void)
	{
		IF_F(!this->_ModuleBase::link());

		Kiss *pK = (Kiss *)m_pKiss;
		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->findModule(n));
		NULL_Fl(m_pV, n + ": not found");

		n = "";
		pK->v("_Universe", &n);
		m_pU = (_Universe *)(pK->findModule(n));
		NULL_Fl(m_pU, n + ": not found");

		n = "";
		pK->v("_AP_base", &n);
		m_pAP = (_AP_base *)(pK->findModule(n));
		NULL_Fl(m_pAP, n + ": not found");

		n = "";
		pK->v("_Uploader", &n);
		m_pCurl = (_Uploader *)(pK->findModule(n));

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

		return this->_ModuleBase::check();
	}

	void _AP_SR::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			updateSR();

			m_pT->autoFPSto();
		}
	}

	void _AP_SR::updateSR(void)
	{
		IF_(check() < 0);

		// verify WP range
		m_iWP = m_pAP->getWPseq();
		IF_(!m_vWPrange.bContain(m_iWP));

		// find target
		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			IF_CONT(std::find(m_vClass.begin(), m_vClass.end(), pO->getTopClass()) == m_vClass.end());
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		//		NULL_(tO);

	static uint64_t tLast = 0;
	uint64_t tNow = getApproxTbootUs();
	if (tNow - tLast < USEC_1SEC)
	{
		NULL_(tO);
	}
	tLast = getApproxTbootUs();

		// record current pos
		vDouble4 vP;
		vP.clear();
		vP = m_pAP->getGlobalPos();
		string lat = lf2str(vP.x, 7);
		string lon = lf2str(vP.y, 7);
		string alt = lf2str(vP.z, 3);

		Frame fBGR = *m_pV->getFrameRGB();
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_(mBGR.empty());

		// bb
		// Rect r = bb2Rect<vFloat4>(tO->getBB2Dscaled(mBGR.cols, mBGR.rows));
		// rectangle(mBGR, r, Scalar(255, 200, 200), 1);
	if (tO)
	{
		Rect r = bb2Rect<vFloat4>(tO->getBB2Dscaled(mBGR.cols, mBGR.rows));
		rectangle(mBGR, r, Scalar(255, 200, 200), 1);
	}

		// save file
		string fName = m_dir + tFormat() + ".jpg";
		cv::imwrite(fName, mBGR, m_compress);
		string cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("File saved: " + fName);

		// send to cloud
		if (m_pCurl)
			m_pCurl->addFile(fName);

		// RTL
		IF_(!m_bRTL);
//		m_pAP->setMode(m_modeRTL);
//		m_pAP->m_pMav->clNavRTL();
	}

	void _AP_SR::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
		IF_(check() < 0);

		_Console *pC = (_Console *)pConsole;
		//		pC->addMsg("fName = " + m_fName);
	}

}
