#include "_AProver_KUfollowTag.h"

namespace kai
{

_AProver_KUfollowTag::_AProver_KUfollowTag()
{
    m_pAP = NULL;
    m_pD = NULL;
    m_pU = NULL;
    m_pPIDtagX = NULL;
    m_pPIDtagHdg = NULL;

    m_errX = 0.0;
    m_errHdg = 0.0;
    
    m_nSpd = 0.0;
    m_nStr = 0.0;
    m_tagTargetX = 0.5;
    m_iTagStop = -1;
}

_AProver_KUfollowTag::~_AProver_KUfollowTag()
{
}

bool _AProver_KUfollowTag::init ( void* pKiss )
{
    IF_F ( !this->_StateBase::init ( pKiss ) );
    NULL_F( m_pSC );

    Kiss* pK = ( Kiss* ) pKiss;
    pK->v ( "nSpd", &m_nSpd );
    pK->v ( "nStr", &m_nStr );
    pK->v ( "tagTargetX", &m_tagTargetX );
    pK->v ( "iTagStop", &m_iTagStop );

    string n;

    n = "";
    pK->v ( "_AP_base", &n );
    m_pAP = ( _AP_base* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pAP, n + ": not found" );

    n = "";
    pK->v ( "Drive", &n );
    m_pD = ( _Drive* ) ( pK->getInst ( n ) );
    IF_Fl ( !m_pD, n + ": not found" );

    n = "";
    pK->v ( "_Universe", &n );
    m_pU = ( _Universe* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pU, n + ": not found" );

    n = "";
    pK->v ( "PIDtagX", &n );
    m_pPIDtagX = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagX, n + " not found" );

    n = "";
    pK->v ( "PIDtagHdg", &n );
    m_pPIDtagHdg = ( PID* ) ( pK->getInst ( n ) );
    NULL_Fl ( m_pPIDtagHdg, n + " not found" );

    return true;
}

bool _AProver_KUfollowTag::start ( void )
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _AProver_KUfollowTag::check ( void )
{
    NULL__ ( m_pAP, -1 );
    NULL__ ( m_pD, -1 );
    NULL__ ( m_pU, -1 );
    NULL__ ( m_pPIDtagX, -1 );
    NULL__ ( m_pPIDtagHdg, -1 );

    return this->_StateBase::check();
}

void _AProver_KUfollowTag::update ( void )
{
    while(m_pT->bRun())
    {
        m_pT->autoFPSfrom();
        this->_StateBase::update();

        updateFollow();

        m_pT->autoFPSto();
    }
}

void _AProver_KUfollowTag::updateFollow ( void )
{
    IF_ ( check() <0 );
	IF_(!bActive());
    
    float dir = m_pD->getDirection();   //+/-1.0
    float nSpd = m_nSpd;
    
    _Object* pO = findTarget();
    if ( pO )
    {
        int iTag = pO->getTopClass();
        if(iTag == m_iTagStop)
        {
            nSpd = 0.0;
            m_errX = 0.0;
        }
        else
        {
            m_errX = dir * (pO->getX() - m_tagTargetX);
        }
        
        m_errHdg = dHdg<float>(0.0, pO->getRoll());
    }

    m_nStr = dir * m_pPIDtagX->update ( m_errX, 0.0, m_pT->getTfrom())
             + dir * m_pPIDtagHdg->update ( m_errHdg, 0.0, m_pT->getTfrom());
    m_pD->setSteering(m_nStr);
    m_pD->setSpeed(nSpd);

}

_Object* _AProver_KUfollowTag::findTarget ( void )
{
    IF_N ( check() <0 );

    _Object *pO;
    _Object *tO = NULL;
    float topY = FLT_MAX;
    int i = 0;
    while ( ( pO = m_pU->get ( i++ ) ) != NULL )
    {
        vFloat3 p = pO->getPos();
        IF_CONT ( p.y * m_pD->getDirection() > topY );

        tO = pO;
        topY = p.x;
    }

    return tO;
}

void _AProver_KUfollowTag::console ( void* pConsole )
{
    NULL_(pConsole);
    this->_StateBase::console(pConsole);
    msgActive(pConsole);

	_Console *pC = (_Console *)pConsole;
   	pC->addMsg("nSpd=" + f2str(m_nSpd) + ", nStr=" + f2str(m_nStr));
   	pC->addMsg("errX=" + f2str(m_errX) + ", errHdg=" + f2str(m_errHdg));
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
	"bON":1,
	"_WindowCV":"OKview",
	"_Console":"OK_Console",
	"cmdUnmount":"gio mount -s gphoto2",
}

*/

