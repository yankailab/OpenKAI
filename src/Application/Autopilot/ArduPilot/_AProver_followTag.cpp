#include "_AProver_followTag.h"

namespace kai
{

_AProver_followTag::_AProver_followTag()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pU = NULL;
    m_pPIDtag = NULL;
    m_pPIDhdg = NULL;

    m_nSpd = 0.0;
    m_tagTargetX = 0.5;
    m_tagTargetHdg = 0.0;
    m_tagPointingHdg = 0.0;
    m_targetHdg = -1.0;
    m_dHdg = 0.0;
    m_nStr = 0.0;

    m_iClass = -1;
}

_AProver_followTag::~_AProver_followTag()
{
}

bool _AProver_followTag::init ( void* pKiss )
{
    IF_F ( !this->_MissionBase::init ( pKiss ) );
    NULL_F( m_pMC );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "tagTargetX", &m_tagTargetX );
    pK->v ( "nStr", &m_nStr );

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
    pK->v ( "PIDhdg", &n );
    m_pPIDhdg = ( PIDctrl* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDhdg, n + " not found" );

    n = "";
    pK->v ( "PIDtag", &n );
    m_pPIDtag = ( PIDctrl* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtag, n + " not found" );

    return true;
}

bool _AProver_followTag::start ( void )
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

int _AProver_followTag::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pU, -1 );
    NULL__ ( m_pPIDhdg, -1 );
    NULL__ ( m_pPIDtag, -1 );

    return this->_MissionBase::check();
}

void _AProver_followTag::update ( void )
{
    while ( m_bThreadON )
    {
        this->autoFPSfrom();
        this->_MissionBase::update();

        updateFollow();

        this->autoFPSto();
    }
}

void _AProver_followTag::updateFollow ( void )
{
    IF_ ( check() <0 );
	IF_(!bActive());
    
    m_pD->setSpeed(m_nSpd);

    float apHdg = m_pAP->getApHdg();
    IF_(apHdg < 0.0);
    
    if(m_targetHdg < 0.0 || bMissionChanged())
        m_targetHdg = apHdg;

    float dir = m_pD->getDirection();   //+/-1.0
    _Object* pO = findTarget();
    if ( pO )
    {
        float tagX = pO->getX();
        m_tagTargetHdg = dir * m_pPIDtag->update ( tagX, m_tagTargetX, m_tStamp );
        m_tagPointingHdg = 0.0;//pO->getRoll();
        m_targetHdg = Hdg(apHdg + m_tagPointingHdg + m_tagTargetHdg);
    }
    else
    {
        m_tagTargetHdg = 0.0;
        m_tagPointingHdg = 0.0;
    }

    m_dHdg = dHdg(apHdg, m_targetHdg);
    float tHdg = apHdg + m_dHdg;
    m_nStr = dir * m_pPIDhdg->update ( apHdg, tHdg, m_tStamp );

    m_pD->setSteering(m_nStr);

//    int iClass = pO->getTopClass();

}

_Object* _AProver_followTag::findTarget ( void )
{
    IF_N ( check() <0 );

    _Object *pO;
    _Object *tO = NULL;
    float topY = FLT_MAX;
    int i = 0;
    while ( ( pO = m_pU->get ( i++ ) ) != NULL )
    {
        vFloat3 p = pO->getPos();
        IF_CONT ( p.y > topY );

        tO = pO;
        topY = p.x;
    }

    return tO;
}

void _AProver_followTag::draw ( void )
{
    this->_MissionBase::draw();
    drawActive();

   	addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	addMsg("tagTargetHdg=" + f2str(m_tagTargetHdg));
   	addMsg("tagPointingHdg=" + f2str(m_tagPointingHdg));
   	addMsg("targetHdg=" + f2str(m_targetHdg));
   	addMsg("dHdg=" + f2str(m_dHdg));
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

