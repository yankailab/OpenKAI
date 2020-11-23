#include "_AProver_follow.h"

namespace kai
{

_AProver_follow::_AProver_follow()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pU = NULL;
    m_pXpid = NULL;

    m_nSpeed = 0.0;
    m_yaw = 0.0;
    m_vP.init ( 0.5,0.5,0.0 );
    m_vPtarget.init ( 0.5,0.5,0.0 );

    m_rcMode.init();
    m_rcDir.init();
}

_AProver_follow::~_AProver_follow()
{
}

bool _AProver_follow::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    Kiss* pK = ( Kiss* ) pKiss;

    pK->v ( "nSpeed", &m_nSpeed );
    pK->v ( "iRCmodeChan", &m_rcMode.m_iChan );
    pK->a ( "vRCmodeDiv", &m_rcMode.m_vDiv );
    m_rcMode.setup();
    pK->v ( "iRCdirChan", &m_rcDir.m_iChan );
    pK->a ( "vRCdirDiv", &m_rcDir.m_vDiv );
    m_rcDir.setup();

    IF_F ( !m_pMC );
    m_iMode.MANUAL = m_pMC->getMissionIdx ( "MANUAL" );
    m_iMode.HYBRID = m_pMC->getMissionIdx ( "HYBRID" );
    m_iMode.AUTO = m_pMC->getMissionIdx ( "AUTO" );
    IF_F ( !m_iMode.bValid() );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "_AProver_drive", &n );
    m_pD = ( _AProver_drive* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pD, n + ": not found" );

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
    NULL__ ( m_pMC, -1 );
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

        updateMode();
        updateDrive();

        this->autoFPSto();
    }
}

void _AProver_follow::updateMode ( void )
{
    IF_ ( check() <0 );

    uint32_t apMode = m_pAP->getApMode();
    uint16_t pwmMode = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcMode.m_iChan );
    IF_ ( pwmMode == UINT16_MAX );

    m_rcMode.pwm ( pwmMode );
    int iMode = m_rcMode.i();

    switch ( iMode )
    {
    case 0:
        m_pMC->transit ( m_iMode.MANUAL );
        break;
    case 1:
        m_pMC->transit ( m_iMode.HYBRID );
        break;
    case 2:
        m_pMC->transit ( m_iMode.AUTO );
        break;
    default:
        m_pMC->transit ( m_iMode.MANUAL );
        break;
    }

    if ( apMode != AP_ROVER_MANUAL )
    {
        m_pMC->transit ( m_iMode.MANUAL );
    }
}

void _AProver_follow::updateDrive ( void )
{
    IF_ ( check() <0 );
    IF_ ( !bActive() <0 );

    uint16_t pwmDir = m_pAP->m_pMav->m_rcChannels.getRC ( m_rcDir.m_iChan );
    float dir = 0.0;
    if ( pwmDir != UINT16_MAX )
    {
        m_rcDir.pwm ( pwmDir );
        dir = ( float ) m_rcDir.i();
        dir -= 1.0;
    }
    float nSpeed = dir * m_nSpeed;

    int iM = m_pMC->getMissionIdx();
    if ( iM == m_iMode.HYBRID )
    {
        m_yaw = 0.0;
        m_nSpeed = nSpeed;
    }
    else if ( iM == m_iMode.AUTO )
    {
        _Object* pO = findTarget();
        if(pO)
        {
            m_vP.x = pO->getX() * dir; //for backward we have to reverse the PID
            m_yaw = m_pXpid->update (m_vP.x, m_vPtarget.x, m_tStamp );
        }
        else
        {
            m_yaw = 0.0;
        }

        int iClass = pO->getTopClass();
        m_nSpeed = nSpeed;
    }
    else
    {
        m_yaw = 0.0;
        m_nSpeed = 0.0;
    }

    m_pD->setYaw ( m_yaw );
    m_pD->setSpeed ( m_nSpeed );
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
*/

