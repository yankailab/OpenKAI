/*
 * _FileBase.h
 *
 *  Created on: Oct 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_File__FileBase_H_
#define OpenKAI_src_File__FileBase_H_

#include "../Base/_ModuleBase.h"

#define D_TYPE_FILE 0x8
#define D_TYPE_FOLDER 0x4

namespace kai
{

	class _FileBase : public _ModuleBase
	{
	public:
		_FileBase();
		~_FileBase();

		virtual bool init(const json& j);
		virtual bool start(void);

		bool createDir(const string &dir);
		bool removeDir(const string &dir);
		string getFirstSubDir(const string &baseDir);
		bool getDirFileList(const string &dir, vector<string>* pvFile, vector<string>* pvExt = NULL);
		string getExtension(const string &fName);
		bool bExtension(const string &fName, const vector<string>& vExt);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_FileBase *)This)->update();
			return NULL;
		}

	protected:
		vector<string> m_vExt;
	};
}
#endif
