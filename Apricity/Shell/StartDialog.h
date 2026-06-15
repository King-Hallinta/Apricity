#pragma once

#include "../Project/RecentProjects.h"

#include <filesystem>
#include <vector>

#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/srchctrl.h>

enum class StartupAction
{
	None,
	NewProject,
	OpenProject
};

struct StartupResult
{
	StartupAction action = StartupAction::None;
	std::filesystem::path packageFile;
};

class StartDialog : public wxDialog
{
public:
	StartDialog(wxWindow *parent, const std::vector<RecentProjectEntry> &recentProjects);

	StartupResult GetResult() const;

private:
	void CreateControls();
	void RefreshRecentProjects();
	void HandleSearchChanged(wxCommandEvent &event);
	void HandleNewProject(wxCommandEvent &event);
	void HandleOpenProject(wxCommandEvent &event);
	void HandleRecentActivated(wxListEvent &event);

	std::vector<RecentProjectEntry> m_RecentProjects;
	std::vector<RecentProjectEntry> m_VisibleProjects;
	StartupResult m_Result;
	wxSearchCtrl *m_Search;
	wxListCtrl *m_RecentList;
};
