#include "APcopter_takePhoto.h"

#ifdef USE_REALSENSE

namespace kai
{

APcopter_takePhoto::APcopter_takePhoto()
{
	m_pAP = NULL;
	m_pV = NULL;
	m_pDV = NULL;
	m_pIO = NULL;

	m_dir = "/home/";
	m_subDir = "";

	m_tInterval = 1;
	m_tLastTake = 0;
	m_quality = 100;
	m_iTake = 0;
	m_bAuto = false;
}

APcopter_takePhoto::~APcopter_takePhoto()
{
}

bool APcopter_takePhoto::init(void* pKiss)
{
	IF_F(!this->ActionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK,quality);
	KISSm(pK,dir);
	KISSm(pK,subDir);
	KISSm(pK,bAuto);
	KISSm(pK,tInterval);

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

	m_compress.push_back(CV_IMWRITE_JPEG_QUALITY);
	m_compress.push_back(m_quality);

	//link
	string iName;

	iName = "";
	F_INFO(pK->v("APcopter_base", &iName));
	m_pAP = (APcopter_base*) (pK->parent()->getChildInst(iName));

	iName = "";
	F_INFO(pK->v("_VisionBase", &iName));
	m_pV = (_VisionBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pV, iName + " not found");

	iName = "";
	F_INFO(pK->v("_DepthVisionBase", &iName));
	m_pDV = (_RealSense*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDV, iName + " not found");

	iName = "";
	F_INFO(pK->v("_IOBase", &iName));
	m_pIO = (_IOBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pIO, iName + " not found");

	return true;
}

int APcopter_takePhoto::check(void)
{
	NULL__(m_pAP,-1);
	NULL__(m_pAP->m_pMavlink,-1);
	NULL__(m_pV,-1);
	NULL__(m_pDV,-1);

	return this->ActionBase::check();
}

void APcopter_takePhoto::update(void)
{
	this->ActionBase::update();
	IF_(check()<0);

	//receive cmd to take one shot
	cmd();

	//Auto take
	if(m_bAuto)
	{
		uint64_t tInt = USEC_1SEC / m_tInterval;
		IF_(m_tStamp - m_tLastTake < tInt);
		m_tLastTake = m_tStamp;
		take();
	}
}

void APcopter_takePhoto::cmd(void)
{
	char buf;
	string msg;

	while(m_pIO->read((uint8_t*) &buf, 1) > 0)
	{

		switch (buf)
		{
		case 't':
			take();
			msg = "Photo take: " + i2str(m_iTake);
			m_pIO->write((unsigned char*)msg.c_str(), msg.length());
			break;
		case 'a':
			m_bAuto = true;
			msg = "Auto interval started in " + i2str(m_tInterval) + " Hz";
			m_pIO->write((unsigned char*)msg.c_str(), msg.length());
			break;
		case 's':
			m_bAuto = false;
			msg = "Auto interval stopped, " + i2str(m_iTake) + " photos taken in total.";
			m_pIO->write((unsigned char*)msg.c_str(), msg.length());
			break;
		default:
			if(buf >= '1' && buf <= '9')
			{
				m_tInterval = (uint64_t)(buf-'0');
				msg = "Set interval to " + i2str(m_tInterval) +" Hz";
				m_pIO->write((unsigned char*)msg.c_str(), msg.length());
			}
			break;
		}
	}
}

void APcopter_takePhoto::take(void)
{
	IF_(check()<0);

	//RGB
	m_pV->open();
	m_pV->close();

	//Depth
	m_pDV->wakeUp();
	m_pDV->goSleep();
	while(!m_pDV->bSleeping());
	while(!m_pDV->m_pTPP->bSleeping());

	string lat = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lat * 0.0000001, 7);
	string lon = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.lon * 0.0000001, 7);
	string alt = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.alt * 0.001, 3);
	string hnd = f2str(m_pAP->m_pMavlink->m_msg.global_position_int.hdg * 0.01, 2);

	Mat mRGB;
	m_pV->BGR()->m()->copyTo(mRGB);
	IF_(mRGB.empty());

	Mat mD;
	m_pDV->Depth()->m()->copyTo(mD);
	IF_(mD.empty());

	Mat mDscale;
	mD.convertTo(mDscale,CV_8UC1,100);

	string fName;
	string cmd;

	LOG_I("Take: " + i2str(m_iTake));

	//rgb
	fName = m_subDir + i2str(m_iTake) + "_rgb" + ".jpg";
	cv::imwrite(fName, mRGB, m_compress);
	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	LOG_I("RGB: " + fName);

	//depth
	fName = m_subDir + i2str(m_iTake) + "_d" + ".jpg";
	cv::imwrite(fName, mDscale, m_compress);
	cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
	system(cmd.c_str());

	LOG_I("Depth: " + fName);

	m_iTake++;
}

bool APcopter_takePhoto::draw(void)
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

bool APcopter_takePhoto::console(int& iY)
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
#endif
