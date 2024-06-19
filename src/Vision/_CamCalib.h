#ifndef OpenKAI_src_Application_CamCalib__CamCalib_H_
#define OpenKAI_src_Application_CamCalib__CamCalib_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	class _CamCalib : public _ModuleBase
	{
	public:
		_CamCalib();
		~_CamCalib();

		bool init(void *pKiss);
		bool start(void);

		bool calibRGB(const char *pPath);
		Mat mC(void);
		Mat mD(void);

	protected:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_CamCalib *)This)->update();
			return NULL;
		}

	public:
		string m_path;
		vInt2 m_vChessBoardSize;	//col, row
		float m_squareSize;

		Mat m_mC;
		Mat m_mD;
	};

}
#endif
