#pragma once
#pragma once

#include "../Data/StoryRepository.h"
#include "../Panels/RecordPanel.h"
#include "../Project/ProjectPackage.h"
#include "../Project/RecentProjects.h"

#include <filesystem>
#include <memory>
#include <vector>

#include <wx/frame.h>
#include <wx/notebook.h>

class MainFrame : public wxFrame
{
public:
	MainFrame();
	void PresentStartDialog();

private:
	void CreateMenu();
	void CreatePanels();
	void HandleStart(wxCommandEvent &event);
	void HandleNewProject(wxCommandEvent &event);
	void HandleOpenProject(wxCommandEvent &event);
	void HandleExportProject(wxCommandEvent &event);
	void HandleCreateRecord(wxCommandEvent &event);
	void HandleNavigate(wxCommandEvent &event);
	void HandleNewRecord(wxCommandEvent &event);
	void HandleEditRecord(wxCommandEvent &event);
	void HandleDeleteRecord(wxCommandEvent &event);
	void HandleRefreshCurrent(wxCommandEvent &event);
	void HandleRefreshAll(wxCommandEvent &event);
	void HandleFind(wxCommandEvent &event);
	void HandleExportSelection(wxCommandEvent &event);
	void HandleOpenPackageFolder(wxCommandEvent &event);
	void HandleBuildInfo(wxCommandEvent &event);
	void HandleExit(wxCommandEvent &event);
	void SetProjectContext();
	bool RequireOpenProject();
	RecordPanel *GetCurrentPanel() const;
	void SelectPanel(std::size_t panelIndex);
	void RefreshAllPanels();
	bool RequestNewProject();
	bool RequestOpenProject();
	bool OpenProjectPackage(const std::filesystem::path &packageFile);
	bool CreateProjectPackage(const std::filesystem::path &packageFile, const wxString &title);
	void RememberOpenProject();

	RecentProjects m_RecentProjects;
	ProjectPackage m_Package;
	std::unique_ptr<StoryRepository> m_Repository;
	wxNotebook *m_Notebook;
	std::vector<RecordPanel *> m_Panels;
};
