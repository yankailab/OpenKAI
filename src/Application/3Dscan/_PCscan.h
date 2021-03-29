#ifndef OpenKAI_src_Application_3Dscan__PCscan_H_
#define OpenKAI_src_Application_3Dscan__PCscan_H_

#ifdef USE_OPEN3D
#include "../../PointCloud/_PCbase.h"
#include "../../SLAM/_SlamBase.h"

namespace kai
{

class _PCscan : public _ModuleBase
{
public:
	_PCscan();
	~_PCscan();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

private:
	void updateIMU ( void );
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PCscan*) This)->update();
		return NULL;
	}

	void updateUI(void);
	static void* getUpdateUI(void* This)
	{
		(( _PCscan*) This)->updateUI();
		return NULL;
	}

public:
    _Thread* m_pTui;
    
    vector<_PCbase*> m_vPCB;
    _SlamBase* m_pSB;
};

}
#endif
#endif
