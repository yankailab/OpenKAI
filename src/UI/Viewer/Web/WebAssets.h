#ifndef OpenKAI_src_UI_Viewer_Web_WebAssets_H_
#define OpenKAI_src_UI_Viewer_Web_WebAssets_H_

#include "../../../Net/HttpServer.h"
#include <filesystem>

namespace kai::webviewer
{
	// Viewers live under html/viewer/<name>; share only html/common at /common.
	// Custom standalone web roots may still contain all of their own assets.
	inline HttpServer::Mounts commonAssets(const std::string &webRoot)
	{
		const auto common = std::filesystem::path(webRoot) / "../../common";
		std::error_code error;
		if (std::filesystem::is_directory(common, error))
			return {{"/common", common.string()}};
		return {};
	}
}
#endif
