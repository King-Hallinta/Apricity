#include "MainFrame.h"

#include "../App/ApplicationVersion.h"
#include "NewProjectDialog.h"
#include "StartDialog.h"

#include <filesystem>

#include <wx/filedlg.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/utils.h>
#include <wx/version.h>

namespace
{
	constexpr std::int32_t NewProjectCommand = wxID_HIGHEST + 1;
	constexpr std::int32_t StartCommand = wxID_HIGHEST + 2;
	constexpr std::int32_t ExportProjectCommand = wxID_HIGHEST + 3;
	constexpr std::int32_t CreateCharacterCommand = wxID_HIGHEST + 10;
	constexpr std::int32_t CreateLocationCommand = wxID_HIGHEST + 11;
	constexpr std::int32_t CreateSceneCommand = wxID_HIGHEST + 12;
	constexpr std::int32_t CreateRelationshipCommand = wxID_HIGHEST + 13;
	constexpr std::int32_t CreateTimelineCommand = wxID_HIGHEST + 14;
	constexpr std::int32_t CreateWorldCommand = wxID_HIGHEST + 15;
	constexpr std::int32_t CreateContinuityCommand = wxID_HIGHEST + 16;
	constexpr std::int32_t NavigateCharactersCommand = wxID_HIGHEST + 20;
	constexpr std::int32_t NavigateLocationsCommand = wxID_HIGHEST + 21;
	constexpr std::int32_t NavigateScenesCommand = wxID_HIGHEST + 22;
	constexpr std::int32_t NavigateRelationshipsCommand = wxID_HIGHEST + 23;
	constexpr std::int32_t NavigateTimelineCommand = wxID_HIGHEST + 24;
	constexpr std::int32_t NavigateWorldCommand = wxID_HIGHEST + 25;
	constexpr std::int32_t NavigateContinuityCommand = wxID_HIGHEST + 26;
	constexpr std::int32_t ExportCharactersCommand = wxID_HIGHEST + 30;
	constexpr std::int32_t ExportLocationsCommand = wxID_HIGHEST + 31;
	constexpr std::int32_t ExportScenesCommand = wxID_HIGHEST + 32;
	constexpr std::int32_t ExportTimelineCommand = wxID_HIGHEST + 33;
	constexpr std::int32_t ExportRelationshipsCommand = wxID_HIGHEST + 34;
	constexpr std::int32_t NewRecordCommand = wxID_HIGHEST + 40;
	constexpr std::int32_t EditRecordCommand = wxID_HIGHEST + 41;
	constexpr std::int32_t DeleteRecordCommand = wxID_HIGHEST + 42;
	constexpr std::int32_t RefreshCurrentCommand = wxID_HIGHEST + 43;
	constexpr std::int32_t RefreshAllCommand = wxID_HIGHEST + 44;
	constexpr std::int32_t FindCommand = wxID_HIGHEST + 45;
	constexpr std::int32_t OpenProjectFolderCommand = wxID_HIGHEST + 50;
	constexpr std::int32_t OpenAssetsFolderCommand = wxID_HIGHEST + 51;
	constexpr std::int32_t OpenDraftsFolderCommand = wxID_HIGHEST + 52;
	constexpr std::int32_t OpenExportsFolderCommand = wxID_HIGHEST + 53;

	constexpr std::size_t CharactersPage = 0;
	constexpr std::size_t LocationsPage = 1;
	constexpr std::size_t ScenesPage = 2;
	constexpr std::size_t RelationshipsPage = 3;
	constexpr std::size_t TimelinePage = 4;
	constexpr std::size_t WorldPage = 5;
	constexpr std::size_t ContinuityPage = 6;

	FieldDefinition Field(const char *columnName, const wxString &label, FieldEditor editor, bool searchable, FieldChoiceSource choiceSource = FieldChoiceSource::None)
	{
		return {columnName, label, editor, searchable, choiceSource};
	}

	RecordPanelConfig CharactersConfig()
	{
		return {
			"Characters",
			"Character",
			"Create a new character",
			"Edit character",
			"characters",
			{Field("display_name", "Name", FieldEditor::SingleLine, true),
				Field("aliases", "Aliases", FieldEditor::MultiLine, true),
				Field("traits", "Traits", FieldEditor::MultiLine, true),
				Field("goals", "Goals", FieldEditor::MultiLine, true),
				Field("secrets", "Secrets", FieldEditor::MultiLine, true),
				Field("notes", "Notes", FieldEditor::MultiLine, true),
				Field("first_appearance", "First appearance", FieldEditor::SingleLine, true, FieldChoiceSource::Scenes),
				Field("last_appearance", "Last appearance", FieldEditor::SingleLine, true, FieldChoiceSource::Scenes)},
			"display_name",
			"first_appearance",
			""};
	}

	RecordPanelConfig LocationsConfig()
	{
		return {
			"Locations",
			"Location",
			"Create a new location",
			"Edit location",
			"locations",
			{Field("location_name", "Name", FieldEditor::SingleLine, true),
				Field("description", "Description", FieldEditor::MultiLine, true),
				Field("linked_characters", "Linked characters", FieldEditor::MultiLine, true),
				Field("important_events", "Important events", FieldEditor::MultiLine, true),
				Field("mood_notes", "Mood and reference notes", FieldEditor::MultiLine, true)},
			"location_name",
			"linked_characters",
			""};
	}

	RecordPanelConfig ScenesConfig()
	{
		return {
			"Scenes",
			"Scene",
			"Create a new scene",
			"Edit scene",
			"scenes",
			{Field("scene_title", "Title", FieldEditor::SingleLine, true),
				Field("summary", "Summary", FieldEditor::MultiLine, true),
				Field("pov_character", "POV character", FieldEditor::SingleLine, true, FieldChoiceSource::Characters),
				Field("location_name", "Location", FieldEditor::SingleLine, true, FieldChoiceSource::Locations),
				Field("story_time", "Story date/time", FieldEditor::SingleLine, true),
				Field("status", "Status", FieldEditor::SingleLine, true, FieldChoiceSource::SceneStatuses),
				Field("tags", "Tags", FieldEditor::SingleLine, true),
				Field("draft_path", "Draft file", FieldEditor::DraftPath, true)},
			"scene_title",
			"status",
			"scene_title"};
	}

	RecordPanelConfig RelationshipsConfig()
	{
		return {
			"Relationships",
			"Relationship",
			"Create a new relationship",
			"Edit relationship",
			"relationships",
			{Field("source_entity", "Source entity", FieldEditor::SingleLine, true, FieldChoiceSource::StoryRecords),
				Field("target_entity", "Target entity", FieldEditor::SingleLine, true, FieldChoiceSource::StoryRecords),
				Field("statement", "Statement", FieldEditor::MultiLine, true),
				Field("relation_type", "Relation type", FieldEditor::SingleLine, true, FieldChoiceSource::RelationshipTypes),
				Field("status", "Status", FieldEditor::SingleLine, true, FieldChoiceSource::RelationshipStatuses),
				Field("strength", "Strength", FieldEditor::SingleLine, true, FieldChoiceSource::RelationshipStrengths),
				Field("notes", "Notes", FieldEditor::MultiLine, true)},
			"source_entity",
			"target_entity",
			""};
	}

	RecordPanelConfig TimelineConfig()
	{
		return {
			"Timeline",
			"Timeline Event",
			"Create a new timeline event",
			"Edit timeline event",
			"timeline_events",
			{Field("event_title", "Event", FieldEditor::SingleLine, true),
				Field("story_order", "Story order", FieldEditor::SingleLine, true),
				Field("manuscript_order", "Manuscript order", FieldEditor::SingleLine, true),
				Field("story_time", "Story date/time", FieldEditor::SingleLine, true),
				Field("summary", "Summary", FieldEditor::MultiLine, true),
				Field("entities", "Characters, items, organizations", FieldEditor::MultiLine, true),
				Field("location_name", "Location", FieldEditor::SingleLine, true, FieldChoiceSource::Locations),
				Field("notes", "Notes", FieldEditor::MultiLine, true)},
			"event_title",
			"story_time",
			""};
	}

	RecordPanelConfig WorldEntitiesConfig()
	{
		return {
			"World",
			"World Entity",
			"Create a new world entity",
			"Edit world entity",
			"entities",
			{Field("entity_name", "Name", FieldEditor::SingleLine, true),
				Field("entity_type", "Type", FieldEditor::SingleLine, true, FieldChoiceSource::EntityTypes),
				Field("description", "Description", FieldEditor::MultiLine, true),
				Field("aliases", "Aliases", FieldEditor::MultiLine, true),
				Field("rules", "Rules and laws", FieldEditor::MultiLine, true),
				Field("notes", "Notes", FieldEditor::MultiLine, true)},
			"entity_name",
			"entity_type",
			""};
	}

	RecordPanelConfig StoryFactsConfig()
	{
		return {
			"Continuity",
			"Continuity Fact",
			"Create a new continuity fact",
			"Edit continuity fact",
			"continuity_facts",
			{Field("fact_title", "Fact", FieldEditor::SingleLine, true),
				Field("fact_text", "What must remain true", FieldEditor::MultiLine, true),
				Field("scope_record", "Scope", FieldEditor::SingleLine, true, FieldChoiceSource::StoryRecords),
				Field("known_by", "Known by", FieldEditor::MultiLine, true),
				Field("starts_in_scene", "Starts in scene", FieldEditor::SingleLine, true, FieldChoiceSource::Scenes),
				Field("ends_in_scene", "Ends in scene", FieldEditor::SingleLine, true, FieldChoiceSource::Scenes),
				Field("notes", "Notes", FieldEditor::MultiLine, true)},
			"fact_title",
			"starts_in_scene",
			""};
	}
} // namespace

MainFrame::MainFrame()
	: wxFrame(nullptr, wxID_ANY, "Apricity", wxDefaultPosition, wxSize(1200, 760)),
	  m_Repository(std::make_unique<StoryRepository>()),
	  m_Notebook(nullptr)
{
	SetBackgroundColour(*wxWHITE);

#if defined(__WXMSW__)
	SetIcon(wxICON(IDI_LOGO));
#endif

	CreateMenu();
	CreatePanels();
	CreateStatusBar();
	SetStatusText("Create or open an Apricity project.");
}

void MainFrame::PresentStartDialog()
{
	StartDialog dialog(this, m_RecentProjects.Load());

	if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	const StartupResult result = dialog.GetResult();

	if (result.action == StartupAction::NewProject)
	{
		RequestNewProject();

		return;
	}

	if (result.action == StartupAction::OpenProject)
	{
		if (result.packageFile.empty())
		{
			RequestOpenProject();

			return;
		}

		OpenProjectPackage(result.packageFile);
	}
}

void MainFrame::CreateMenu()
{
	wxMenu *fileMenu = new wxMenu();
	fileMenu->Append(StartCommand, "Start...\tCtrl+Shift+S");
	fileMenu->AppendSeparator();
	fileMenu->Append(NewProjectCommand, "New Project...\tCtrl+Shift+N");
	fileMenu->Append(wxID_OPEN, "Open Project...\tCtrl+O");
	fileMenu->AppendSeparator();
	fileMenu->Append(ExportProjectCommand, "Export All Summaries");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "Exit\tAlt+F4");

	wxMenu *editMenu = new wxMenu();
	editMenu->Append(NewRecordCommand, "New Record...\tCtrl+N");
	editMenu->Append(EditRecordCommand, "Edit Selected Record...\tCtrl+E");
	editMenu->Append(DeleteRecordCommand, "Delete Selected Record\tDel");
	editMenu->AppendSeparator();
	editMenu->Append(FindCommand, "Find in Current List\tCtrl+F");
	editMenu->AppendSeparator();
	editMenu->Append(RefreshCurrentCommand, "Refresh Current List\tF5");
	editMenu->Append(RefreshAllCommand, "Refresh All Lists\tCtrl+F5");

	wxMenu *createMenu = new wxMenu();
	createMenu->Append(CreateCharacterCommand, "Character...");
	createMenu->Append(CreateLocationCommand, "Location...");
	createMenu->Append(CreateSceneCommand, "Scene...");
	createMenu->Append(CreateRelationshipCommand, "Relationship...");
	createMenu->Append(CreateTimelineCommand, "Timeline Event...");
	createMenu->Append(CreateWorldCommand, "World Entity...");
	createMenu->Append(CreateContinuityCommand, "Continuity Fact...");

	wxMenu *navigateMenu = new wxMenu();
	navigateMenu->Append(NavigateCharactersCommand, "Characters\tCtrl+1");
	navigateMenu->Append(NavigateLocationsCommand, "Locations\tCtrl+2");
	navigateMenu->Append(NavigateScenesCommand, "Scenes\tCtrl+3");
	navigateMenu->Append(NavigateRelationshipsCommand, "Relationships\tCtrl+4");
	navigateMenu->Append(NavigateTimelineCommand, "Timeline\tCtrl+5");
	navigateMenu->Append(NavigateWorldCommand, "World\tCtrl+6");
	navigateMenu->Append(NavigateContinuityCommand, "Continuity\tCtrl+7");

	wxMenu *exportMenu = new wxMenu();
	exportMenu->Append(ExportProjectCommand, "All Summaries");
	exportMenu->AppendSeparator();
	exportMenu->Append(ExportCharactersCommand, "Character Bible");
	exportMenu->Append(ExportLocationsCommand, "Location List");
	exportMenu->Append(ExportScenesCommand, "Scene Outline");
	exportMenu->Append(ExportTimelineCommand, "Timeline");
	exportMenu->Append(ExportRelationshipsCommand, "Relationship Summary");

	wxMenu *toolsMenu = new wxMenu();
	toolsMenu->Append(RefreshAllCommand, "Refresh All Lists");
	toolsMenu->AppendSeparator();
	toolsMenu->Append(OpenProjectFolderCommand, "Project Package Folder");
	toolsMenu->Append(OpenAssetsFolderCommand, "Assets Folder");
	toolsMenu->Append(OpenDraftsFolderCommand, "Drafts Folder");
	toolsMenu->Append(OpenExportsFolderCommand, "Exports Folder");

	wxMenu *helpMenu = new wxMenu();
	helpMenu->Append(wxID_ABOUT, "Build Info and Notices");

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(editMenu, "Edit");
	menuBar->Append(createMenu, "Create");
	menuBar->Append(navigateMenu, "Navigate");
	menuBar->Append(exportMenu, "Export");
	menuBar->Append(toolsMenu, "Tools");
	menuBar->Append(helpMenu, "Help");
	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &MainFrame::HandleStart, this, StartCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleNewProject, this, NewProjectCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleOpenProject, this, wxID_OPEN);
	Bind(wxEVT_MENU, &MainFrame::HandleExportProject, this, ExportProjectCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleCreateRecord, this, CreateCharacterCommand, CreateContinuityCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleNavigate, this, NavigateCharactersCommand, NavigateContinuityCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleNewRecord, this, NewRecordCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleEditRecord, this, EditRecordCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleDeleteRecord, this, DeleteRecordCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleRefreshCurrent, this, RefreshCurrentCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleRefreshAll, this, RefreshAllCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleFind, this, FindCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleExportSelection, this, ExportCharactersCommand, ExportRelationshipsCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleOpenPackageFolder, this, OpenProjectFolderCommand, OpenExportsFolderCommand);
	Bind(wxEVT_MENU, &MainFrame::HandleBuildInfo, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::HandleExit, this, wxID_EXIT);
}

void MainFrame::CreatePanels()
{
	m_Notebook = new wxNotebook(this, wxID_ANY);
	m_Notebook->SetBackgroundColour(*wxWHITE);

	const std::vector<RecordPanelConfig> configs =
		{
			CharactersConfig(),
			LocationsConfig(),
			ScenesConfig(),
			RelationshipsConfig(),
			TimelineConfig(),
			WorldEntitiesConfig(),
			StoryFactsConfig()};

	for (const RecordPanelConfig &config : configs)
	{
		RecordPanel *panel = new RecordPanel(m_Notebook, config);
		m_Notebook->AddPage(panel, config.title);
		m_Panels.push_back(panel);
	}
}

void MainFrame::HandleStart(wxCommandEvent &event)
{
	event.Skip();
	PresentStartDialog();
}

void MainFrame::HandleNewProject(wxCommandEvent &event)
{
	event.Skip();
	RequestNewProject();
}

void MainFrame::HandleOpenProject(wxCommandEvent &event)
{
	event.Skip();
	RequestOpenProject();
}

bool MainFrame::RequestNewProject()
{
	NewProjectDialog dialog(this);

	if (dialog.ShowModal() != wxID_OK)
	{
		return false;
	}

	if (dialog.GetProjectTitle().empty())
	{
		wxMessageBox("Project title cannot be empty.", "Apricity", wxOK | wxICON_WARNING, this);

		return false;
	}

	return CreateProjectPackage(dialog.GetPackageFile(), dialog.GetProjectTitle());
}

bool MainFrame::RequestOpenProject()
{
	wxFileDialog dialog(
		this,
		"Open Apricity Project",
		"",
		"",
		"Apricity projects (*.apy)|*.apy|All files (*.*)|*.*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dialog.ShowModal() != wxID_OK)
	{
		return false;
	}

	return OpenProjectPackage(std::filesystem::path(dialog.GetPath().ToStdWstring()));
}

void MainFrame::HandleExportProject(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	wxString errorMessage;

	if (not m_Repository->WriteExports(m_Package, &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return;
	}

	wxMessageBox("Exports were written to the project Exports folder.", "Apricity", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::HandleCreateRecord(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	std::size_t panelIndex = CharactersPage;

	switch (event.GetId())
	{
		case CreateCharacterCommand:
		{
			panelIndex = CharactersPage;
			break;
		}
		case CreateLocationCommand:
		{
			panelIndex = LocationsPage;
			break;
		}
		case CreateSceneCommand:
		{
			panelIndex = ScenesPage;
			break;
		}
		case CreateRelationshipCommand:
		{
			panelIndex = RelationshipsPage;
			break;
		}
		case CreateTimelineCommand:
		{
			panelIndex = TimelinePage;
			break;
		}
		case CreateWorldCommand:
		{
			panelIndex = WorldPage;
			break;
		}
		case CreateContinuityCommand:
		{
			panelIndex = ContinuityPage;
			break;
		}
		default:
		{
			return;
		}
	}

	SelectPanel(panelIndex);
	m_Panels[panelIndex]->CreateRecord();
}

void MainFrame::HandleNavigate(wxCommandEvent &event)
{
	event.Skip();

	switch (event.GetId())
	{
		case NavigateCharactersCommand:
		{
			SelectPanel(CharactersPage);
			break;
		}
		case NavigateLocationsCommand:
		{
			SelectPanel(LocationsPage);
			break;
		}
		case NavigateScenesCommand:
		{
			SelectPanel(ScenesPage);
			break;
		}
		case NavigateRelationshipsCommand:
		{
			SelectPanel(RelationshipsPage);
			break;
		}
		case NavigateTimelineCommand:
		{
			SelectPanel(TimelinePage);
			break;
		}
		case NavigateWorldCommand:
		{
			SelectPanel(WorldPage);
			break;
		}
		case NavigateContinuityCommand:
		{
			SelectPanel(ContinuityPage);
			break;
		}
		default:
		{
			break;
		}
	}
}

void MainFrame::HandleNewRecord(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RecordPanel *panel = GetCurrentPanel();

	if (panel != nullptr)
	{
		panel->CreateRecord();
	}
}

void MainFrame::HandleEditRecord(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RecordPanel *panel = GetCurrentPanel();

	if (panel != nullptr)
	{
		panel->EditSelectedRecord();
	}
}

void MainFrame::HandleDeleteRecord(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RecordPanel *panel = GetCurrentPanel();

	if (panel != nullptr)
	{
		panel->DeleteSelectedRecord();
	}
}

void MainFrame::HandleRefreshCurrent(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RecordPanel *panel = GetCurrentPanel();

	if (panel != nullptr)
	{
		panel->RefreshRecords();
		SetStatusText("Current list refreshed.");
	}
}

void MainFrame::HandleRefreshAll(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RefreshAllPanels();
	SetStatusText("All lists refreshed.");
}

void MainFrame::HandleFind(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	RecordPanel *panel = GetCurrentPanel();

	if (panel != nullptr)
	{
		panel->FocusSearch();
	}
}

void MainFrame::HandleExportSelection(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	StoryExport exportTarget = StoryExport::Characters;

	switch (event.GetId())
	{
		case ExportCharactersCommand:
		{
			exportTarget = StoryExport::Characters;
			break;
		}
		case ExportLocationsCommand:
		{
			exportTarget = StoryExport::Locations;
			break;
		}
		case ExportScenesCommand:
		{
			exportTarget = StoryExport::SceneOutline;
			break;
		}
		case ExportTimelineCommand:
		{
			exportTarget = StoryExport::Timeline;
			break;
		}
		case ExportRelationshipsCommand:
		{
			exportTarget = StoryExport::Relationships;
			break;
		}
		default:
		{
			return;
		}
	}

	wxString errorMessage;

	if (not m_Repository->WriteExport(m_Package, exportTarget, &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return;
	}

	wxMessageBox("Export file was written to the project Exports folder.", "Apricity", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::HandleOpenPackageFolder(wxCommandEvent &event)
{
	event.Skip();

	if (not RequireOpenProject())
	{
		return;
	}

	std::filesystem::path folder = m_Package.GetRootFolder();

	switch (event.GetId())
	{
		case OpenProjectFolderCommand:
		{
			folder = m_Package.GetRootFolder();
			break;
		}
		case OpenAssetsFolderCommand:
		{
			folder = m_Package.GetAssetsFolder();
			break;
		}
		case OpenDraftsFolderCommand:
		{
			folder = m_Package.GetDraftsFolder();
			break;
		}
		case OpenExportsFolderCommand:
		{
			folder = m_Package.GetExportsFolder();
			break;
		}
		default:
		{
			return;
		}
	}

	wxLaunchDefaultApplication(wxString(folder.wstring()));
}

void MainFrame::HandleBuildInfo(wxCommandEvent &event)
{
	event.Skip();

	wxString message =
		"Apricity\n\n"
		"Build information\n";
	message += "Version: ";
	message += ApplicationVersion;
	message += "\n";

#if defined(_DEBUG)
	message += "Configuration: Debug\n";
#else
	message += "Configuration: Release\n";
#endif

#if defined(_WIN64)
	message += "Architecture: x64\n";
#else
	message += "Architecture: x86\n";
#endif

	message += "wxWidgets: ";
	message += wxVERSION_STRING;
	message += "\n";
	message += wxString::Format("C++ standard value: %ld\n", static_cast<long>(__cplusplus));
	message += "Build timestamp: " __DATE__ " " __TIME__ "\n\n";
	message +=
		"Copyright\n"
		"Copyright (c) 2026 Korrikada. All rights reserved.\n\n"
		"Notices\n"
		"Apricity uses wxWidgets for the desktop application framework.\n"
		"wxWidgets is distributed under the wxWindows Library Licence.\n"
		"Apricity uses SQLite for local project storage.\n"
		"SQLite is in the public domain.";

	wxMessageBox(
		message,
		"Build Info and Notices",
		wxOK | wxICON_INFORMATION,
		this);
}

void MainFrame::HandleExit(wxCommandEvent &event)
{
	event.Skip();
	Close(true);
}

void MainFrame::SetProjectContext()
{
	for (RecordPanel *panel : m_Panels)
	{
		panel->SetContext(m_Repository.get(), &m_Package);
	}

	SetStatusText(m_Package.GetTitle());
	SetTitle("Apricity - " + m_Package.GetTitle());
}

bool MainFrame::RequireOpenProject()
{
	if (m_Repository != nullptr and m_Repository->IsOpen() and m_Package.IsOpen())
	{
		return true;
	}

	wxMessageBox("Open a project first.", "Apricity", wxOK | wxICON_INFORMATION, this);

	return false;
}

RecordPanel *MainFrame::GetCurrentPanel() const
{
	if (m_Notebook == nullptr)
	{
		return nullptr;
	}

	const std::int32_t selection = m_Notebook->GetSelection();

	if (selection < 0 or selection >= static_cast<std::int32_t>(m_Panels.size()))
	{
		return nullptr;
	}

	return m_Panels[static_cast<std::size_t>(selection)];
}

void MainFrame::SelectPanel(std::size_t panelIndex)
{
	if (m_Notebook == nullptr or panelIndex >= m_Panels.size())
	{
		return;
	}

	m_Notebook->SetSelection(panelIndex);
}

void MainFrame::RefreshAllPanels()
{
	for (RecordPanel *panel : m_Panels)
	{
		panel->RefreshRecords();
	}
}

bool MainFrame::OpenProjectPackage(const std::filesystem::path &packageFile)
{
	wxString errorMessage;

	if (not m_Package.Open(packageFile, &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	if (not m_Repository->Open(m_Package.GetDatabaseFile(), &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	if (not m_Repository->Initialize(m_Package.GetTitle(), m_Package.GetProjectKey(), &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	SetProjectContext();
	RememberOpenProject();

	return true;
}

bool MainFrame::CreateProjectPackage(const std::filesystem::path &packageFile, const wxString &title)
{
	wxString errorMessage;

	if (not m_Package.Create(packageFile, title, &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	if (not m_Repository->Open(m_Package.GetDatabaseFile(), &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	if (not m_Repository->Initialize(m_Package.GetTitle(), m_Package.GetProjectKey(), &errorMessage))
	{
		wxMessageBox(errorMessage, "Apricity", wxOK | wxICON_ERROR, this);

		return false;
	}

	SetProjectContext();
	RememberOpenProject();

	return true;
}

void MainFrame::RememberOpenProject()
{
	RecentProjectEntry entry;
	entry.title = m_Package.GetTitle();
	entry.packageFile = m_Package.GetPackageFile();
	m_RecentProjects.Remember(entry);
}
