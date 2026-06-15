#pragma once

#include <filesystem>
#include <vector>

#include <wx/string.h>

struct RecentProjectEntry
{
	wxString title;
	std::filesystem::path packageFile;
};

class RecentProjects
{
public:
	std::vector<RecentProjectEntry> Load() const;
	void Remember(const RecentProjectEntry &entry) const;
};
