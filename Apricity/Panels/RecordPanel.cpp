#include "RecordPanel.h"

#include "../Core/TextEncoding.h"
#include "FieldSelectionDialog.h"
#include "RecordDialog.h"

#include <algorithm>
#include <map>

#include <wx/button.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>

namespace
{
	constexpr std::int32_t ContextNewCommand = wxID_HIGHEST + 701;
	constexpr std::int32_t ContextEditCommand = wxID_HIGHEST + 702;
	constexpr std::int32_t ContextDeleteCommand = wxID_HIGHEST + 703;
	constexpr std::int32_t ContextColumnsCommand = wxID_HIGHEST + 704;
	constexpr std::int32_t ContextRefreshCommand = wxID_HIGHEST + 705;
} // namespace

RecordPanel::RecordPanel(wxWindow *parent, const RecordPanelConfig &requestedConfig)
	: wxPanel(parent),
	  m_Config(requestedConfig),
	  m_Repository(nullptr),
	  m_Package(nullptr),
	  m_SortFieldIndex(0),
	  m_SortAscending(true),
	  m_SelectedRowId(0),
	  m_Search(nullptr),
	  m_List(nullptr),
	  m_EditButton(nullptr),
	  m_DeleteButton(nullptr)
{
	SetBackgroundColour(*wxWHITE);
	InitializeVisibleFields();
	CreateControls();
	SetPanelEnabled(false);
}

void RecordPanel::SetContext(StoryRepository *newRepository, ProjectPackage *newPackage)
{
	m_Repository = newRepository;
	m_Package = newPackage;
	m_SelectedRowId = 0;
	SetPanelEnabled(m_Repository != nullptr and m_Repository->IsOpen());
	RefreshRecords();
}

void RecordPanel::RefreshRecords()
{
	m_List->DeleteAllItems();
	m_Records.clear();
	m_SelectedRowId = 0;
	UpdateActionButtons();
	RebuildListColumns();

	if (m_Repository == nullptr or not m_Repository->IsOpen())
	{
		return;
	}

	wxString errorMessage;
	m_Records = m_Repository->LoadRecords(
		m_Config.tableName,
		GetColumnNames(),
		GetSearchColumns(),
		m_Search->GetValue(),
		&errorMessage);

	if (not errorMessage.empty())
	{
		ShowRepositoryError("Could not refresh records.", errorMessage);

		return;
	}

	SortRecords();
	PopulateList();
}

bool RecordPanel::CreateRecord()
{
	if (m_Repository == nullptr or not m_Repository->IsOpen())
	{
		return false;
	}

	return OpenRecordDialog(nullptr);
}

bool RecordPanel::EditSelectedRecord()
{
	if (m_Repository == nullptr or not m_Repository->IsOpen() or m_SelectedRowId == 0)
	{
		return false;
	}

	return OpenRecordDialog(GetSelectedRecord());
}

void RecordPanel::DeleteSelectedRecord()
{
	if (m_Repository == nullptr or m_SelectedRowId == 0)
	{
		return;
	}

	if (wxMessageBox("Delete the selected record?", m_Config.title, wxYES_NO | wxICON_WARNING, this) != wxYES)
	{
		return;
	}

	wxString errorMessage;

	if (not m_Repository->DeleteRecord(m_Config.tableName, m_SelectedRowId, &errorMessage))
	{
		ShowRepositoryError("Could not delete the record.", errorMessage);

		return;
	}

	RefreshRecords();
}

void RecordPanel::FocusSearch()
{
	m_Search->SetFocus();
	m_Search->SelectAll();
}

void RecordPanel::CreateControls()
{
	wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *toolbarSizer = new wxBoxSizer(wxHORIZONTAL);

	m_Search = new wxSearchCtrl(this, wxID_ANY);
	m_Search->ShowSearchButton(true);
	m_Search->ShowCancelButton(true);

	wxButton *newButton = new wxButton(this, wxID_ANY, "New...");
	m_EditButton = new wxButton(this, wxID_ANY, "Edit...");
	m_DeleteButton = new wxButton(this, wxID_ANY, "Delete");
	wxButton *columnsButton = new wxButton(this, wxID_ANY, "Columns...");

	toolbarSizer->Add(m_Search, 1, wxEXPAND | wxRIGHT, 6);
	toolbarSizer->Add(newButton, 0, wxRIGHT, 6);
	toolbarSizer->Add(m_EditButton, 0, wxRIGHT, 6);
	toolbarSizer->Add(m_DeleteButton, 0, wxRIGHT, 6);
	toolbarSizer->Add(columnsButton, 0);

	m_List = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	m_List->SetBackgroundColour(*wxWHITE);
	RebuildListColumns();

	rootSizer->Add(toolbarSizer, 0, wxEXPAND | wxALL, 8);
	rootSizer->Add(m_List, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
	SetSizer(rootSizer);

	m_Search->Bind(wxEVT_TEXT, &RecordPanel::HandleSearchChanged, this);
	m_List->Bind(wxEVT_LIST_ITEM_SELECTED, &RecordPanel::HandleSelectionChanged, this);
	m_List->Bind(wxEVT_LIST_ITEM_ACTIVATED, &RecordPanel::HandleRecordActivated, this);
	m_List->Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &RecordPanel::HandleListContextMenu, this);
	m_List->Bind(wxEVT_CONTEXT_MENU, &RecordPanel::HandleListContextRequest, this);
	m_List->Bind(wxEVT_LIST_COL_CLICK, &RecordPanel::HandleColumnClicked, this);
	newButton->Bind(wxEVT_BUTTON, &RecordPanel::HandleNewRecord, this);
	m_EditButton->Bind(wxEVT_BUTTON, &RecordPanel::HandleEditRecord, this);
	m_DeleteButton->Bind(wxEVT_BUTTON, &RecordPanel::HandleDeleteRecord, this);
	columnsButton->Bind(wxEVT_BUTTON, &RecordPanel::HandleShowColumns, this);
	Bind(wxEVT_MENU, &RecordPanel::HandleNewRecord, this, ContextNewCommand);
	Bind(wxEVT_MENU, &RecordPanel::HandleEditRecord, this, ContextEditCommand);
	Bind(wxEVT_MENU, &RecordPanel::HandleDeleteRecord, this, ContextDeleteCommand);
	Bind(wxEVT_MENU, &RecordPanel::HandleShowColumns, this, ContextColumnsCommand);
	Bind(wxEVT_MENU, &RecordPanel::HandleRefreshRecords, this, ContextRefreshCommand);
}

void RecordPanel::HandleSearchChanged(wxCommandEvent &event)
{
	event.Skip();
	RefreshRecords();
}

void RecordPanel::HandleSelectionChanged(wxListEvent &event)
{
	const long recordIndex = m_List->GetItemData(event.GetIndex());

	if (recordIndex < 0 or recordIndex >= static_cast<long>(m_Records.size()))
	{
		m_SelectedRowId = 0;
		UpdateActionButtons();

		return;
	}

	m_SelectedRowId = m_Records[static_cast<std::size_t>(recordIndex)].rowId;
	UpdateActionButtons();
}

void RecordPanel::HandleNewRecord(wxCommandEvent &event)
{
	event.Skip();
	CreateRecord();
}

void RecordPanel::HandleEditRecord(wxCommandEvent &event)
{
	event.Skip();
	EditSelectedRecord();
}

void RecordPanel::HandleRecordActivated(wxListEvent &event)
{
	event.Skip();
	OpenRecordDialog(GetSelectedRecord());
}

void RecordPanel::HandleDeleteRecord(wxCommandEvent &event)
{
	event.Skip();
	DeleteSelectedRecord();
}

void RecordPanel::HandleShowColumns(wxCommandEvent &event)
{
	event.Skip();
	ShowFieldSelection();
}

void RecordPanel::HandleRefreshRecords(wxCommandEvent &event)
{
	event.Skip();
	RefreshRecords();
}

void RecordPanel::HandleListContextMenu(wxListEvent &event)
{
	event.Skip(false);

	const long itemIndex = event.GetIndex();

	if (itemIndex >= 0)
	{
		m_List->SetItemState(itemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

		const long recordIndex = m_List->GetItemData(itemIndex);

		if (recordIndex >= 0 and recordIndex < static_cast<long>(m_Records.size()))
		{
			m_SelectedRowId = m_Records[static_cast<std::size_t>(recordIndex)].rowId;
		}
	}

	UpdateActionButtons();
	PresentListContextMenu();
}

void RecordPanel::HandleListContextRequest(wxContextMenuEvent &event)
{
	event.Skip(false);
	UpdateActionButtons();
	PresentListContextMenu();
}

void RecordPanel::HandleColumnClicked(wxListEvent &event)
{
	event.Skip(false);

	const std::int32_t columnIndex = event.GetColumn();

	if (columnIndex < 0 or columnIndex >= static_cast<std::int32_t>(m_VisibleFields.size()))
	{
		return;
	}

	const std::size_t requestedSortField = m_VisibleFields[static_cast<std::size_t>(columnIndex)];

	if (m_SortFieldIndex == requestedSortField)
	{
		m_SortAscending = not m_SortAscending;
	}
	else
	{
		m_SortFieldIndex = requestedSortField;
		m_SortAscending = true;
	}

	RebuildListColumns();
	SortRecords();
	PopulateList();
}

void RecordPanel::PresentListContextMenu()
{
	const bool hasProject = m_Repository != nullptr and m_Repository->IsOpen();

	wxMenu menu;
	menu.Append(ContextNewCommand, "New...");
	menu.Append(ContextEditCommand, "Edit...");
	menu.Append(ContextDeleteCommand, "Delete");
	menu.AppendSeparator();
	menu.Append(ContextColumnsCommand, "Columns...");
	menu.Append(ContextRefreshCommand, "Refresh");
	menu.Enable(ContextNewCommand, hasProject);
	menu.Enable(ContextEditCommand, hasProject and m_SelectedRowId != 0);
	menu.Enable(ContextDeleteCommand, hasProject and m_SelectedRowId != 0);
	menu.Enable(ContextRefreshCommand, hasProject);
	PopupMenu(&menu);
}

bool RecordPanel::OpenRecordDialog(StoryRecord *record)
{
	if (m_Repository == nullptr or not m_Repository->IsOpen())
	{
		return false;
	}

	RecordDialog dialog(this, m_Config, m_Repository, m_Package, record);

	if (dialog.ShowModal() != wxID_OK)
	{
		return false;
	}

	wxString errorMessage;
	const std::map<std::string, std::string> values = dialog.GetValues();
	bool succeeded = false;

	if (record == nullptr)
	{
		succeeded = m_Repository->InsertRecord(m_Config.tableName, values, &errorMessage);
	}
	else
	{
		succeeded = m_Repository->UpdateRecord(m_Config.tableName, record->rowId, values, &errorMessage);
	}

	if (not succeeded)
	{
		ShowRepositoryError("Could not save the record.", errorMessage);

		return false;
	}

	RefreshRecords();

	return true;
}

std::vector<std::string> RecordPanel::GetColumnNames() const
{
	std::vector<std::string> columnNames;

	for (const FieldDefinition &field : m_Config.fields)
	{
		columnNames.push_back(field.columnName);
	}

	return columnNames;
}

std::vector<std::string> RecordPanel::GetSearchColumns() const
{
	std::vector<std::string> columnNames;

	for (const FieldDefinition &field : m_Config.fields)
	{
		if (field.searchable)
		{
			columnNames.push_back(field.columnName);
		}
	}

	return columnNames;
}

void RecordPanel::InitializeVisibleFields()
{
	m_VisibleFields.clear();

	for (std::size_t index = 0; index < m_Config.fields.size(); ++index)
	{
		if (m_Config.fields[index].columnName == m_Config.primaryColumn or m_Config.fields[index].columnName == m_Config.secondaryColumn)
		{
			m_VisibleFields.push_back(index);
		}
	}

	if (m_VisibleFields.empty() and not m_Config.fields.empty())
	{
		m_VisibleFields.push_back(0);
	}

	if (not m_VisibleFields.empty())
	{
		m_SortFieldIndex = m_VisibleFields.front();
	}
}

void RecordPanel::ShowFieldSelection()
{
	FieldSelectionDialog dialog(this, m_Config, m_VisibleFields);

	if (dialog.ShowModal() != wxID_OK)
	{
		return;
	}

	m_VisibleFields = dialog.GetVisibleFields();

	bool foundSortField = false;

	for (const std::size_t visibleField : m_VisibleFields)
	{
		if (visibleField == m_SortFieldIndex)
		{
			foundSortField = true;

			break;
		}
	}

	if (not foundSortField)
	{
		m_SortFieldIndex = m_VisibleFields.front();
		m_SortAscending = true;
	}

	RefreshRecords();
}

void RecordPanel::RebuildListColumns()
{
	while (m_List->GetColumnCount() > 0)
	{
		m_List->DeleteColumn(0);
	}

	for (const std::size_t fieldIndex : m_VisibleFields)
	{
		wxString label = m_Config.fields[fieldIndex].label;

		if (fieldIndex == m_SortFieldIndex)
		{
			label += m_SortAscending ? " ^" : " v";
		}

		m_List->AppendColumn(label, wxLIST_FORMAT_LEFT, 220);
	}
}

void RecordPanel::SortRecords()
{
	if (m_SortFieldIndex >= m_Config.fields.size())
	{
		return;
	}

	std::sort(
		m_Records.begin(),
		m_Records.end(),
		[this](const StoryRecord &left, const StoryRecord &right)
	{
		const std::string leftText = GetRecordFieldText(left, m_SortFieldIndex);
		const std::string rightText = GetRecordFieldText(right, m_SortFieldIndex);

		if (m_SortAscending)
		{
			return leftText < rightText;
		}

		return rightText < leftText;
	});
}

void RecordPanel::PopulateList()
{
	m_List->DeleteAllItems();

	if (m_VisibleFields.empty())
	{
		return;
	}

	for (std::size_t index = 0; index < m_Records.size(); ++index)
	{
		const StoryRecord &record = m_Records[index];
		wxString firstText = FromUtf8(GetRecordFieldText(record, m_VisibleFields.front()));

		if (firstText.empty())
		{
			firstText = "(blank)";
		}

		const long itemIndex = m_List->InsertItem(static_cast<long>(index), firstText);

		for (std::size_t fieldIndex = 1; fieldIndex < m_VisibleFields.size(); ++fieldIndex)
		{
			const std::size_t visibleField = m_VisibleFields[fieldIndex];
			const wxString text = FromUtf8(GetRecordFieldText(record, visibleField));
			m_List->SetItem(itemIndex, static_cast<std::int32_t>(fieldIndex), text);
		}

		m_List->SetItemData(itemIndex, static_cast<long>(index));

		if (record.rowId == m_SelectedRowId)
		{
			m_List->SetItemState(itemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

std::string RecordPanel::GetRecordFieldText(const StoryRecord &record, std::size_t fieldIndex) const
{
	if (fieldIndex >= m_Config.fields.size())
	{
		return "";
	}

	const auto field = record.fields.find(m_Config.fields[fieldIndex].columnName);

	if (field == record.fields.end())
	{
		return "";
	}

	return field->second;
}

void RecordPanel::UpdateActionButtons()
{
	const bool hasSelection = m_SelectedRowId != 0;
	m_EditButton->Enable(hasSelection);
	m_DeleteButton->Enable(hasSelection);
}

void RecordPanel::ShowRepositoryError(const wxString &fallbackMessage, const wxString &errorMessage) const
{
	wxString message = fallbackMessage;

	if (not errorMessage.empty())
	{
		message = errorMessage;
	}

	wxMessageBox(message, m_Config.title, wxOK | wxICON_ERROR, const_cast<RecordPanel *>(this));
}

void RecordPanel::SetPanelEnabled(bool enabled)
{
	m_Search->Enable(enabled);
	m_List->Enable(enabled);
	m_SelectedRowId = 0;
	UpdateActionButtons();
}

StoryRecord *RecordPanel::GetSelectedRecord()
{
	if (m_SelectedRowId == 0)
	{
		return nullptr;
	}

	for (StoryRecord &record : m_Records)
	{
		if (record.rowId == m_SelectedRowId)
		{
			return &record;
		}
	}

	return nullptr;
}
