#pragma once
#include "Common.h"

class DownloadThread
{
public:

	void operator()(Common& common);
	void SetUrl(std::string_view new_url);
private:
	/// </summary>
	std::string _download_url;
};

