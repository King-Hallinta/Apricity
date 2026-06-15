#include "RecentProjects.h"

#include <wx/config.h>

namespace
{
	constexpr std::int32_t MaximumRecentProjectCount = 16;

	wxString PathToText(const std::filesystem::path &path)
	{
		return wxString(path.wstring());
	}

	std::filesystem::path TextToPath(const wxString &text)
	{
		return std::filesystem::path(text.ToStdWstring());
	}

	wxString MakeFallbackTitle(const std::filesystem::path &path)
	{
		wxString title(path.stem().wstring());

		if (title.empty())
		{
			title = "Untitled Project";
		}

		return title;
	}
} // namespace

std::vector<RecentProjectEntry> RecentProjects::Load() const
{
	wxConfig config("Apricity");
	long count = 0;
	std::vector<RecentProjectEntry> entries;

	config.Read("/RecentProjects/Count", &count, 0);

	for (long index = 0; index < count and index < MaximumRecentProjectCount; ++index)
	{
		wxString pathText;
		wxString title;

		config.Read(wxString::Format("/RecentProjects/Path%ld", index), &pathText);
		config.Read(wxString::Format("/RecentProjects/Title%ld", index), &title);

		if (pathText.empty())
		{
			continue;
		}

		RecentProjectEntry entry;
		entry.packageFile = TextToPath(pathText);
		entry.title = title.empty() ? MakeFallbackTitle(entry.packageFile) : title;
		entries.push_back(entry);
	}

	return entries;
}

void RecentProjects::Remember(const RecentProjectEntry &entry) const
{
	std::vector<RecentProjectEntry> entries;
	entries.push_back(entry);

	for (const RecentProjectEntry &recentEntry : Load())
	{
		if (recentEntry.packageFile == entry.packageFile)
		{
			continue;
		}

		entries.push_back(recentEntry);

		if (entries.size() >= MaximumRecentProjectCount)
		{
			break;
		}
	}

	wxConfig config("Apricity");
	config.Write("/RecentProjects/Count", static_cast<long>(entries.size()));

	for (std::size_t index = 0; index < entries.size(); ++index)
	{
		const long configIndex = static_cast<long>(index);
		config.Write(wxString::Format("/RecentProjects/Path%ld", configIndex), PathToText(entries[index].packageFile));
		config.Write(wxString::Format("/RecentProjects/Title%ld", configIndex), entries[index].title);
	}

	config.Flush();
}
