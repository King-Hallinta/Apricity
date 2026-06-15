#include "StartDialog.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace
{
	constexpr std::int32_t NewProjectCommand = wxID_HIGHEST + 201;
	constexpr std::int32_t OpenProjectCommand = wxID_HIGHEST + 202;

	bool MatchesFilter(const RecentProjectEntry &entry, const wxString &filterText)
	{
		if (filterText.empty())
		{
			return true;
		}

		const wxString loweredFilter = filterText.Lower();

		return entry.title.Lower().Contains(loweredFilter) or wxString(entry.packageFile.wstring()).Lower().Contains(loweredFilter);
	}
} // namespace

StartDialog::StartDialog(wxWindow *parent, const std::vector<RecentProjectEntry> &requestedRecentProjects)
	: wxDialog(parent, wxID_ANY, "Apricity", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	  m_RecentProjects(requestedRecentProjects),
	  m_Search(nullptr),
	  m_RecentList(nullptr)
{
	SetBackgroundColour(*wxWHITE);
	CreateControls();
	RefreshRecentProjects();
}

StartupResult StartDialog::GetResult() const
{
	return m_Result;
}

void StartDialog::CreateControls()
{
	wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *title = new wxStaticText(this, wxID_ANY, "What would you like to do?");
	wxFont titleFont = title->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 4);
	title->SetFont(titleFont);

	wxBoxSizer *contentSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *recentSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *actionSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *recentTitle = new wxStaticText(this, wxID_ANY, "Recent projects");
	wxStaticText *actionTitle = new wxStaticText(this, wxID_ANY, "Project");

	m_Search = new wxSearchCtrl(this, wxID_ANY);
	m_Search->ShowSearchButton(true);
	m_Search->ShowCancelButton(true);

	m_RecentList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(520, 340), wxLC_REPORT | wxLC_SINGLE_SEL);
	m_RecentList->SetBackgroundColour(*wxWHITE);
	m_RecentList->AppendColumn("Project", wxLIST_FORMAT_LEFT, 190);
	m_RecentList->AppendColumn("Location", wxLIST_FORMAT_LEFT, 310);

	wxButton *newProjectButton = new wxButton(this, NewProjectCommand, "Create a new project");
	wxButton *openProjectButton = new wxButton(this, OpenProjectCommand, "Open a project");

	recentSizer->Add(recentTitle, 0, wxEXPAND | wxBOTTOM, 8);
	recentSizer->Add(m_Search, 0, wxEXPAND | wxBOTTOM, 8);
	recentSizer->Add(m_RecentList, 1, wxEXPAND);

	actionSizer->Add(actionTitle, 0, wxEXPAND | wxBOTTOM, 8);
	actionSizer->Add(newProjectButton, 0, wxEXPAND | wxBOTTOM, 8);
	actionSizer->Add(openProjectButton, 0, wxEXPAND);
	actionSizer->AddStretchSpacer();

	contentSizer->Add(recentSizer, 1, wxEXPAND | wxRIGHT, 16);
	contentSizer->Add(actionSizer, 0, wxEXPAND);

	rootSizer->Add(title, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 16);
	rootSizer->Add(contentSizer, 1, wxEXPAND | wxALL, 16);

	SetSizer(rootSizer);
	rootSizer->SetSizeHints(this);
	Fit();
	CentreOnParent();

	m_Search->Bind(wxEVT_TEXT, &StartDialog::HandleSearchChanged, this);
	newProjectButton->Bind(wxEVT_BUTTON, &StartDialog::HandleNewProject, this);
	openProjectButton->Bind(wxEVT_BUTTON, &StartDialog::HandleOpenProject, this);
	m_RecentList->Bind(wxEVT_LIST_ITEM_ACTIVATED, &StartDialog::HandleRecentActivated, this);
}

void StartDialog::RefreshRecentProjects()
{
	m_RecentList->DeleteAllItems();
	m_VisibleProjects.clear();

	for (const RecentProjectEntry &entry : m_RecentProjects)
	{
		if (not MatchesFilter(entry, m_Search->GetValue()))
		{
			continue;
		}

		m_VisibleProjects.push_back(entry);
		const long itemIndex = m_RecentList->InsertItem(m_RecentList->GetItemCount(), entry.title);
		m_RecentList->SetItem(itemIndex, 1, wxString(entry.packageFile.parent_path().wstring()));
		m_RecentList->SetItemData(itemIndex, static_cast<long>(m_VisibleProjects.size() - 1));
	}
}

void StartDialog::HandleSearchChanged(wxCommandEvent &event)
{
	event.Skip();
	RefreshRecentProjects();
}

void StartDialog::HandleNewProject(wxCommandEvent &event)
{
	event.Skip();
	m_Result.action = StartupAction::NewProject;
	EndModal(wxID_OK);
}

void StartDialog::HandleOpenProject(wxCommandEvent &event)
{
	event.Skip();

	wxFileDialog dialog(
		this,
		"Open Apricity Project",
		"",
		"",
		"Apricity projects (*.apy)|*.apy|All files (*.*)|*.*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	m_Result.action = StartupAction::OpenProject;
	m_Result.packageFile = std::filesystem::path(dialog.GetPath().ToStdWstring());
	EndModal(wxID_OK);
}

void StartDialog::HandleRecentActivated(wxListEvent &event)
{
	const long recentIndex = m_RecentList->GetItemData(event.GetIndex());

	if (recentIndex < 0 or recentIndex >= static_cast<long>(m_VisibleProjects.size()))
	{
		return;
	}

	m_Result.action = StartupAction::OpenProject;
	m_Result.packageFile = m_VisibleProjects[static_cast<std::size_t>(recentIndex)].packageFile;
	EndModal(wxID_OK);
}
