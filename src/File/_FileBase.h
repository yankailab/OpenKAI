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

		bool init(void *pKiss);
		bool start(void);

		bool createDir(const string &dir);
		string getFirstSubDir(const string &baseDir);
		void getDirFileList(const string &dir);
		string getExtension(const string &fName);


		bool verifyExtension(const string &fName);
		void setFileList(vector<string> vFileIn);

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			((_FileBase *)This)->update();
			return NULL;
		}

	public:
		string m_dirIn;
		vector<string> m_vExtIn;
		vector<string> m_vFileIn;
	};
}
#endif
