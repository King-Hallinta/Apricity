#pragma once

#include "../Data/StoryRepository.h"

#include <string>
#include <vector>

#include <wx/event.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/srchctrl.h>

class wxButton;
class ProjectPackage;

enum class FieldEditor
{
	SingleLine,
	MultiLine,
	DraftPath
};

enum class FieldChoiceSource
{
	None,
	Characters,
	Locations,
	StoryRecords,
	Scenes,
	SceneStatuses,
	RelationshipTypes,
	RelationshipStatuses,
	RelationshipStrengths,
	EntityTypes
};

struct FieldDefinition
{
	std::string columnName;
	wxString label;
	FieldEditor editor;
	bool searchable;
	FieldChoiceSource choiceSource = FieldChoiceSource::None;
};

struct RecordPanelConfig
{
	wxString title;
	wxString dialogTitle;
	wxString createTitle;
	wxString editTitle;
	std::string tableName;
	std::vector<FieldDefinition> fields;
	std::string primaryColumn;
	std::string secondaryColumn;
	std::string draftTitleColumn;
};

class RecordPanel : public wxPanel
{
public:
	RecordPanel(wxWindow *parent, const RecordPanelConfig &config);

	void SetContext(StoryRepository *newRepository, ProjectPackage *newPackage);
	void RefreshRecords();
	bool CreateRecord();
	bool EditSelectedRecord();
	void DeleteSelectedRecord();
	void FocusSearch();

private:
	void CreateControls();
	void HandleSearchChanged(wxCommandEvent &event);
	void HandleSelectionChanged(wxListEvent &event);
	void HandleNewRecord(wxCommandEvent &event);
	void HandleEditRecord(wxCommandEvent &event);
	void HandleRecordActivated(wxListEvent &event);
	void HandleDeleteRecord(wxCommandEvent &event);
	void HandleShowColumns(wxCommandEvent &event);
	void HandleRefreshRecords(wxCommandEvent &event);
	void HandleListContextMenu(wxListEvent &event);
	void HandleListContextRequest(wxContextMenuEvent &event);
	void HandleColumnClicked(wxListEvent &event);
	bool OpenRecordDialog(StoryRecord *record);
	std::vector<std::string> GetColumnNames() const;
	std::vector<std::string> GetSearchColumns() const;
	void InitializeVisibleFields();
	void ShowFieldSelection();
	void RebuildListColumns();
	void SortRecords();
	void PopulateList();
	std::string GetRecordFieldText(const StoryRecord &record, std::size_t fieldIndex) const;
	void UpdateActionButtons();
	void PresentListContextMenu();
	void ShowRepositoryError(const wxString &fallbackMessage, const wxString &errorMessage) const;
	void SetPanelEnabled(bool enabled);
	StoryRecord *GetSelectedRecord();

	RecordPanelConfig m_Config;
	StoryRepository *m_Repository;
	ProjectPackage *m_Package;
	std::vector<StoryRecord> m_Records;
	std::vector<std::size_t> m_VisibleFields;
	std::size_t m_SortFieldIndex;
	bool m_SortAscending;
	std::int32_t m_SelectedRowId;
	wxSearchCtrl *m_Search;
	wxListCtrl *m_List;
	wxButton *m_EditButton;
	wxButton *m_DeleteButton;
};
