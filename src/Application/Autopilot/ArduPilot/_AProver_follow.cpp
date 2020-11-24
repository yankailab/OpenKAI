#include "_AProver_follow.h"

namespace kai
{

_AProver_follow::_AProver_follow()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pU = NULL;
    m_pXpid = NULL;

    m_iClass = -1;
    m_vP.init ( 0.5, 0.5, 0.0 );
    m_vPtarget.init ( 0.5, 0.5, 0.0 );

    m_nSpd = 1.0;
    m_dir = 1.0;
    m_yaw = 0.0;
    
}

_AProver_follow::~_AProver_follow()
{
}

bool _AProver_follow::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    IF_F ( !m_pMC );

    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "dir", &m_dir );
    pK->v ( "yaw", &m_yaw );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    n = "";
    pK->v ( "_Universe", &n );
    m_pU = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pU, n + ": not found" );

    n = "";
    pK->v ( "PIDx", &n );
    m_pXpid = ( PIDctrl* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pXpid, n + " not found" );

    return true;
}

bool _AProver_follow::start ( void )
{
    m_bThreadON = true;
    int retCode = pthread_create ( &m_threadID, 0, getUpdateThread, this );
    if ( retCode != 0 )
    {
        LOG ( ERROR ) << "Return code: " << retCode;
        m_bThreadON = false;
        return false;
    }

    return true;
}

int _AProver_follow::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pU, -1 );
    NULL__ ( m_pXpid, -1 );

    return this->_MissionBase::check();
}

void _AProver_follow::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateDrive();

        this->autoFPSto();
    }
}

void _AProver_follow::updateDrive ( void )
{
    IF_ ( check() <0 );
    IF_ ( !bActive() <0 );

    _Object* pO = findTarget();
    if ( pO )
    {
        m_vP.x = pO->getX();
        m_yaw = m_pXpid->update ( m_vP.x, m_vPtarget.x, m_tStamp );
    }
    else
    {
        m_yaw = 0.0;
    }

    int iClass = pO->getTopClass();

    m_pD->setDrive(m_nSpd, m_dir, m_yaw);
}

_Object* _AProver_follow::findTarget ( void )
{
    IF_N ( check() <0 );

    _Object *pO;
    _Object *tO = NULL;
    float minD = FLT_MAX;
    int i = 0;
    while ( ( pO = m_pU->get ( i++ ) ) != NULL )
    {
        vFloat3 p = pO->getPos();
        float x = p.x - m_vPtarget.x;
        float y = p.y - m_vPtarget.y;
        float r = x*x + y*y;
        IF_CONT ( r > minD );

        tO = pO;
        minD = r;
    }

    return tO;
}

void _AProver_follow::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

}

}

/*
 * 	string cmd;
	cmd = "mkdir /media/usb";
	system(cmd.c_str());
	cmd = "mount /dev/sda1 /media/usb";
	system(cmd.c_str());
	cmd = "mkdir " + m_subDir;
	system(cmd.c_str());

	vDouble4 vP;
	vP.init();
	vP = m_pAP->getGlobalPos();
	string lat = lf2str(vP.x, 7);
	string lon = lf2str(vP.y, 7);
	string alt = lf2str(vP.z, 3);

	string fName;

	if(m_pV)
	{
		//rgb
		Frame fBGR = *m_pV->BGR();
		if(m_bFlipRGB)fBGR = fBGR.flip(-1);
		Mat mBGR;
		fBGR.m()->copyTo(mBGR);
		IF_(mBGR.empty());

		fName = m_subDir + i2str(m_iTake) + "_tag" + i2str(m_iTake) + "_rgb.jpg";
		cv::imwrite(fName, mBGR, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("RGB: " + fName);
	}

	if(m_pDV)
	{
		//depth
		Frame fD = *m_pDV->Depth();
		if(m_bFlipD)fD = fD.flip(-1);
		Mat mD;
		fD.m()->copyTo(mD);
		IF_(mD.empty());

		Mat mDscale;
		mD.convertTo(mDscale, CV_8UC1, 100);

		fName = m_subDir + i2str(m_iTake)  + "_tag" + i2str(m_iTake) +  "_d.jpg";
		cv::imwrite(fName, mDscale, m_compress);
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("Depth: " + fName);
	}

	if(m_pG)
	{
		//gphoto
		m_pG->open();

		fName = m_subDir + i2str(m_iTake)  + "_tag" + i2str(m_iTake) +  ".jpg";
		cmd = "gphoto2 --capture-image-and-download --filename " + fName;
		system(cmd.c_str());
		cmd = "exiftool -overwrite_original -GPSLongitude=\"" + lon + "\" -GPSLatitude=\"" + lat + "\" " + fName;
		system(cmd.c_str());

		LOG_I("GPhoto: " + fName);

        //gphoto2 --capture-image-and-download --usbid 001,009 --filename test.jpg
	}

	LOG_I("Take: " + i2str(m_iTake) + ", Tag: " + i2str(m_iTake));
	m_iTake++;

    {
	"name":"gphoto",
	"class":"_GPhoto",
	"FPS":30,
	"bInst":1,
	"Window":"OKview",
	"Console":"OKconsole",
	"cmdUnmount":"gio mount -s gphoto2",
}

*/

