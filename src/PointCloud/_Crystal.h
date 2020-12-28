#ifndef OpenKAI_src_PointCloud__Crystal_H_
#define OpenKAI_src_PointCloud__Crystal_H_

#include "../Base/_ThreadBase.h"

#ifdef USE_OPEN3D
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

struct CRYSTAL_CELL
{
    vFloat3 m_vColor;
    uint16_t m_nP;          //nPoints inside the cell

    CRYSTAL_CELL* m_pSub;
    uint16_t m_dSub;        //sub cell dimension

	void init(void)
	{
        m_vColor.init();
        m_nP = 0;
        
        m_pSub = NULL;
        m_dSub = 0;
	}
};

class _Crystal: public _ThreadBase
{
public:
	_Crystal();
	virtual ~_Crystal();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	void render(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Crystal *) This)->update();
		return NULL;
	}

private:
    CRYSTAL_CELL* m_pCell;
    uint16_t m_dCell;        //top level cell dimension
    
    vFloat3 m_vCoverage;
    
    
};

}
#endif
#endif
