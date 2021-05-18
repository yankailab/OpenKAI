#ifndef OpenKAI_src_Application_AgileX__AgileX_H_
#define OpenKAI_src_Application_AgileX__AgileX_H_
#include "../../IO/_IOBase.h"

namespace kai
{
	class _AgileX : public _ModuleBase
	{
	public:
		_AgileX();
		~_AgileX();

		bool init(void *pKiss);
		bool start(void);
		int check(void);

	protected:
		void updateMove(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_AgileX *)This)->update();
			return NULL;
		}

		void move(float spd, float steer);
		void setMode(uint8_t iMode);
	    uint8_t checksum(uint8_t *data, uint8_t len);

	public:
		_IOBase* m_pIO;
		uint8_t m_iFrame;
		float m_spd;
		float m_steer;
	};

}
#endif
