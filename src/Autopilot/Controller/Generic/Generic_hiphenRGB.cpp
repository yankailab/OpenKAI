#include "Generic_hiphenRGB.h"

namespace kai
{

Generic_hiphenRGB::Generic_hiphenRGB()
{
	m_pV = NULL;
	m_pIO = NULL;

	m_dir = "/home/";
	m_subDir = "";

	m_tInterval = 1;
	m_tLastTake = 0;
	m_quality = 100;
	m_iTake = 0;
	m_bAuto = true;
	m_bFlipRGB = false;
	m_bFlipD = false;
}

Generic_hiphenRGB::~Generic_hiphenRGB()
{
}

bool Generic_hiphenRGB::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,quality);
	KISSm(pK,dir);
	KISSm(pK,subDir);
	KISSm(pK,bAuto);
	KISSm(pK,tInterval);
	KISSm(pK,bFlipRGB);
	KISSm(pK,bFlipD);

	if(m_subDir.empty())
	{
		m_subDir = m_dir + tFormat() + "/";
	}
	else
	{
		m_subDir = m_dir + m_subDir;
	}

	string cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	m_compress.push_back(IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pV, iName + " not found");

	iName = "";
	F_INFO(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIO, iName + " not found");

	return true;
}

int Generic_hiphenRGB::check(void)
{
	NULL__(m_pV,-1);

	return this->ActionBase::check();
}

void Generic_hiphenRGB::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	//Auto take
	if(m_bAuto)
	{
		uint64_t tInt = USEC_1SEC / m_tInterval;
		IF_(m_tStamp - m_tLastTake < tInt);
		m_tLastTake = m_tStamp;
		take();
	}
}

void Generic_hiphenRGB::take(void)
{
	IF_(check()<0);

	LL_POS pLL;
	pLL.init();

	string lat = lf2str(pLL.m_lat, 7);
	string lon = lf2str(pLL.m_lng, 7);
	string alt = lf2str(pLL.m_altRel, 3);
	string hnd = lf2str(pLL.m_hdg, 2);

	Frame fBGR = *m_pV->BGR();
	if(m_bFlipRGB)fBGR = fBGR.flip(-1);
	Mat mBGR;
	fBGR.m()->copyTo(mBGR);
	IF_(mBGR.empty());

	string fName;
	string cmd;

	LOG_I("Take: " + i2str(m_iTake));

	//rgb
	fName = m_subDir + i2str(m_iTake) + "_rgb" + ".jpg";
	cv::imwrite(fName, mBGR, m_compress);
	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	LOG_I("RGB: " + fName);

	m_iTake++;
}

bool Generic_hiphenRGB::draw(void)
{
	IF_F(!this->ActionBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	IF_F(pMat->empty());
	IF_F(check()<0);

	string msg = *this->getName() + ": ";

	if(!bActive())
	{
		pWin->addMsg("Inactive");
	}

	if(m_bAuto)
	{
		pWin->addMsg("AUTO INTERVAL");
	}
	else
	{
		pWin->addMsg("MANUAL");
	}

	pWin->addMsg("Inteval = " + i2str(m_tInterval) + " Hz");
	pWin->addMsg("iTake = " + i2str(m_iTake));
	pWin->addMsg("Dir = " + m_subDir);

	return true;
}

bool Generic_hiphenRGB::console(int& iY)
{
	IF_F(!this->ActionBase::console(iY));
	IF_F(check()<0);

	string msg;

	if(!bActive())
	{
		C_MSG("Inactive");
	}

	if(m_bAuto)
	{
		C_MSG("AUTO INTERVAL");
	}
	else
	{
		C_MSG("MANUAL");
	}

	C_MSG("Inteval = " + i2str(m_tInterval) + " Hz");
	C_MSG("iTake = " + i2str(m_iTake));
	C_MSG("Dir = " + m_subDir);

	return true;
}

}
