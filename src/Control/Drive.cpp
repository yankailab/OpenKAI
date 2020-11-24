#include "Drive.h"

namespace kai
{

Drive::Drive()
{
	m_nSpd = 0.0;
	m_nDir = 1.0;
	m_nStr = 0.0;    
}

Drive::~Drive()
{
}

bool Drive::init(void* pKiss)
{
	IF_F(!this->ControlBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nSpd",&m_nSpd);
	pK->v("nDir",&m_nDir);
	pK->v("nStr",&m_nStr);
    
    Kiss *pKM = pK->child("motor");
	NULL_Fl(pKM, "motor not found");

	int i = 0;
	while (1)
	{
		Kiss *pM = pKM->child(i++);
		if (pM->empty())
			break;

		DRIVE_MOTOR m;
		m.init();
		pM->v("kSpd", &m.m_kSpd);
		pM->v("kDir", &m.m_kDir);
		pM->v("kStr", &m.m_kStr);
		pM->v("vSpd", &m.m_vSpd);
		pM->v("vStr", &m.m_vStr);

		m_vM.push_back(m);
	}
    
 	return true;
}

void Drive::update(void)
{
    for(int i=0; i<m_vM.size(); i++)
    {
        DRIVE_MOTOR* pM = &m_vM[i];
        pM->update(m_nSpd, m_nDir, m_nStr);
    }
}

void Drive::setDrive(float nSpd, float nDir, float nStr)
{
    m_nSpd = nSpd;
    m_nDir = nDir;
    m_nStr = nStr;
}

float Drive::getSpeed(int iM)
{
    IF__(iM >= m_vM.size(), 0.0);

    return m_vM[iM].m_spd;
}

float Drive::getSteering(int iM)
{
    IF__(iM >= m_vM.size(), 0.0);

    return m_vM[iM].m_str;
}

void Drive::draw(void)
{
	this->ControlBase::draw();

	addMsg("nSpd=" + f2str(m_nSpd) + ", nDir=" + f2str(m_nDir) + ", nStr=" + f2str(m_nStr));
    for(int i=0; i<m_vM.size(); i++)
    {
        DRIVE_MOTOR* pM = &m_vM[i];
        addMsg("iMotor" + i2str(i) + ": spd=" + f2str(pM->m_spd) + ", str=" + f2str(pM->m_str));
    }

}

}
