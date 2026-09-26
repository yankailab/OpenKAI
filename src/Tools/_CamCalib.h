#ifndef OpenKAI_src_Tools__CamCalib_H_
#define OpenKAI_src_Tools__CamCalib_H_

#include "../Base/_ModuleBase.h"

namespace kai
{
	class _CamCalib : public _ModuleBase
	{
	public:
		_CamCalib();
		~_CamCalib();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool start(void);

		bool calibRGB(const char *pPath);
		Mat mC(void);
		Mat mD(void);

	protected:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_CamCalib *)This)->update();
			return NULL;
		}

	protected:
		string m_path = "";
		Vector2i m_vChessBoardSize = Vector2i::Zero(); // col, row
		float m_squareSize = 1.0;

		Mat m_mC;
		Mat m_mD;
	};

}
#endif
