#ifndef OpenKAI_src_Autopilot_AP__AProver_field_H_
#define OpenKAI_src_Autopilot_AP__AProver_field_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "_AP_base.h"
#include "_AProver_drive.h"

namespace kai
{

class _AProver_field: public _AutopilotBase
{
public:
	_AProver_field();
	~_AProver_field();

	virtual	bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void update(void);
	virtual void draw(void);

protected:
	void findBlockBoarder(void);
	void findSideBoarder(void);
	bool updateDrive(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_field *) This)->update();
		return NULL;
	}

public:
	_AP_base* 	m_pAP;
	_AProver_drive* m_pDrive;
	_DetectorBase* m_pDetBB;
	_DetectorBase* m_pDetSB;
	PIDctrl* m_pPIDhdg;

	uint16_t m_rcMode;
	float	m_nSpeed;

	bool m_bBlockBorder;
	float m_sideBorder;
	float m_sideBorderTarget;
	vFloat2 m_vSideBorderRange;
	float m_dHdg;

	uint8_t m_iPinLED;
	uint8_t m_iPinShutter;

};

}
#endif
