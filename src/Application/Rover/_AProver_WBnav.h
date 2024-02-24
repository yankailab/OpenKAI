
#ifndef OpenKAI_src_Application_UTphenoRover__AProver_WBnav_H_
#define OpenKAI_src_Application_UTphenoRover__AProver_WBnav_H_

#include "../../Autopilot/ArduPilot/_AP_base.h"
#include "../../Universe/_Universe.h"
#include "../../Control/PID.h"
#include "../../Autopilot/Drive/_Drive.h"
#include "../../Utility/util.h"

namespace kai
{

	struct UTPR_MISSIOIN
	{
		int8_t DRIVE;
		int8_t DIVIDER;
		int8_t EXTRACT;
		int8_t FOLLOW;
		int8_t RECOVER;

		bool bValid(void)
		{
			IF_F(DRIVE < 0);
			IF_F(DIVIDER < 0);
			IF_F(EXTRACT < 0);
			IF_F(FOLLOW < 0);
			IF_F(RECOVER < 0);

			return true;
		}

		bool assign(_StateControl *pMC)
		{
			NULL_F(pMC);
			DRIVE = pMC->getStateIdxByName("DRIVE");
			DIVIDER = pMC->getStateIdxByName("DIVIDER");
			EXTRACT = pMC->getStateIdxByName("EXTRACT");
			FOLLOW = pMC->getStateIdxByName("FOLLOW");
			RECOVER = pMC->getStateIdxByName("RECOVER");

			return bValid();
		}
	};

	class _AProver_WBnav : public _ModuleBase
	{
	public:
		_AProver_WBnav();
		~_AProver_WBnav();

		bool init(void *pKiss);
		bool start(void);
		void update(void);
		int check(void);
		void console(void *pConsole);

	private:
		void updateFollow(void);
		static void *getUpdate(void *This)
		{
			((_AProver_WBnav *)This)->update();
			return NULL;
		}

	private:
		_AP_base *m_pAP;
		_Drive *m_pD;

		_Universe *m_pUside;
		PID *m_pPIDside;
		float m_errSide;
		float m_targetSide;
		vFloat2 m_vSide;

		_Universe *m_pUdivider;
		bool m_bDivider;

		_Universe *m_pUtag;
		int m_iTag;
		vFloat2 m_vTagY;
		vFloat2 m_vTagX;

		float m_nSpd;
		float m_nStr;

		UTPR_MISSIOIN m_iState;
	};

}
#endif
