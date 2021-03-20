#ifndef OpenKAI_src_PointCloud__PClattice_H_
#define OpenKAI_src_PointCloud__PClattice_H_

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

struct PCLATTICE_CELL
{
    vFloat3 m_vColor;
    uint16_t m_nP;          //nPoints inside the cell

    PCLATTICE_CELL* m_pSub;
    uint16_t m_dSub;        //sub cell dimension

	void init(void)
	{
        m_vColor.init();
        m_nP = 0;
        
        m_pSub = NULL;
        m_dSub = 0;
	}
};

class _PClattice: public _PCbase
{
public:
	_PClattice();
	virtual ~_PClattice();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual int check(void);
	virtual void draw(void);

protected:
	void render(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		(( _PClattice *) This)->update();
		return NULL;
	}

private:
    PCLATTICE_CELL* m_pCell;
    uint16_t m_dCell;        //top level cell dimension
    
    vFloat3 m_vCoverage;
    
    
};

}
#endif
#endif
