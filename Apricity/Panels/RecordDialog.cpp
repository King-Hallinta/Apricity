#include "RecordDialog.h"

#include "../Core/TextEncoding.h"
#include "../Project/ProjectPackage.h"

#include <fstream>
#include <system_error>

#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/utils.h>

namespace
{
	void ConfigureTextEditor(wxStyledTextCtrl *editor)
	{
		editor->SetWrapMode(wxSTC_WRAP_WORD);
		editor->SetMarginWidth(0, 0);
		editor->SetUseTabs(false);
		editor->SetTabWidth(4);
	}

	void AddChoiceIfMissing(std::vector<wxString> *choices, const wxString &value)
	{
		for (const wxString &choice : *choices)
		{
			if (choice == value)
			{
				return;
			}
		}

		choices->push_back(value);
	}
} // namespace

RecordDialog::RecordDialog(wxWindow *parent, const RecordPanelConfig &requestedConfig, StoryRepository *requestedRepository, ProjectPackage *requestedPackage, const StoryRecord *requestedRecord)
	: wxDialog(parent, wxID_ANY, requestedConfig.dialogTitle, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	  m_Config(requestedConfig),
	  m_Repository(requestedRepository),
	  m_Package(requestedPackage),
	  m_Record(requestedRecord),
	  m_DraftEditor(nullptr)
{
	SetBackgroundColour(*wxWHITE);
	CreateControls();
	LoadRecord();
	LoadDraftText();
}

std::map<std::string, std::string> RecordDialog::GetValues() const
{
	std::map<std::string, std::string> values;

	for (const FieldDefinition &field : m_Config.fields)
	{
		values[field.columnName] = ToUtf8(GetEditorValue(field.columnName));
	}

	return values;
}

void RecordDialog::CreateControls()
{
	wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
	const wxString titleText = m_Record == nullptr ? m_Config.createTitle : m_Config.editTitle;
	wxStaticText *title = new wxStaticText(this, wxID_ANY, titleText);
	wxFont titleFont = title->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 4);
	title->SetFont(titleFont);

	wxPanel *editorPanel = new wxPanel(this, wxID_ANY);
	editorPanel->SetBackgroundColour(*wxWHITE);
	wxBoxSizer *editorSizer = new wxBoxSizer(wxVERTICAL);
	editorPanel->SetSizer(editorSizer);

	editorSizer->AddSpacer(10);
	CreateFieldEditors(editorPanel, editorSizer);
	editorSizer->AddSpacer(10);

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
	wxButton *saveButton = new wxButton(this, wxID_OK, "Save");
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(cancelButton, 0, wxRIGHT, 8);
	buttonSizer->Add(saveButton, 0);

	rootSizer->Add(title, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 16);
	rootSizer->Add(editorPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 16);
	rootSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 16);
	SetSizer(rootSizer);
	rootSizer->SetSizeHints(this);
	Fit();
	CentreOnParent();

	saveButton->SetDefault();
	saveButton->Bind(wxEVT_BUTTON, &RecordDialog::HandleAccepted, this);
}

void RecordDialog::CreateFieldEditors(wxWindow *parent, wxSizer *sizer)
{
	wxBoxSizer *columnSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *rightSizer = new wxBoxSizer(wxVERTICAL);
	const std::size_t rightColumnStart = (m_Config.fields.size() + 1) / 2;

	leftSizer->SetMinSize(wxSize(360, -1));
	rightSizer->SetMinSize(wxSize(360, -1));

	columnSizer->Add(leftSizer, 1, wxEXPAND | wxRIGHT, 14);
	columnSizer->Add(rightSizer, 1, wxEXPAND);
	sizer->Add(columnSizer, 0, wxEXPAND);

	for (std::size_t index = 0; index < m_Config.fields.size(); ++index)
	{
		const FieldDefinition &field = m_Config.fields[index];
		wxSizer *fieldSizer = index < rightColumnStart ? leftSizer : rightSizer;
		wxStaticText *label = new wxStaticText(parent, wxID_ANY, field.label);
		fieldSizer->Add(label, 0, wxEXPAND | wxBOTTOM, 3);

		if (field.editor == FieldEditor::SingleLine)
		{
			if (field.choiceSource == FieldChoiceSource::None)
			{
				wxTextCtrl *editor = new wxTextCtrl(parent, wxID_ANY);
				m_Editors[field.columnName] = editor;
				fieldSizer->Add(editor, 0, wxEXPAND | wxBOTTOM, 8);
			}
			else
			{
				wxComboBox *editor = new wxComboBox(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, 0, nullptr, wxCB_DROPDOWN);

				for (const wxString &choice : LoadChoices(field))
				{
					editor->Append(choice);
				}

				m_Editors[field.columnName] = editor;
				fieldSizer->Add(editor, 0, wxEXPAND | wxBOTTOM, 8);
			}
		}
		else if (field.editor == FieldEditor::MultiLine)
		{
			wxStyledTextCtrl *editor = new wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 86));
			ConfigureTextEditor(editor);
			m_Editors[field.columnName] = editor;
			fieldSizer->Add(editor, 0, wxEXPAND | wxBOTTOM, 8);
		}
		else
		{
			wxBoxSizer *draftPathSizer = new wxBoxSizer(wxHORIZONTAL);
			wxTextCtrl *editor = new wxTextCtrl(parent, wxID_ANY);
			wxButton *browseButton = new wxButton(parent, wxID_ANY, "Browse...");
			wxButton *internalButton = new wxButton(parent, wxID_ANY, "Use Internal");
			wxButton *openButton = new wxButton(parent, wxID_ANY, "Open Externally");

			m_Editors[field.columnName] = editor;
			draftPathSizer->Add(editor, 1, wxRIGHT, 6);
			draftPathSizer->Add(browseButton, 0, wxRIGHT, 6);
			draftPathSizer->Add(internalButton, 0, wxRIGHT, 6);
			draftPathSizer->Add(openButton, 0);
			fieldSizer->Add(draftPathSizer, 0, wxEXPAND | wxBOTTOM, 8);

			wxStaticText *draftLabel = new wxStaticText(parent, wxID_ANY, "Draft text");
			m_DraftEditor = new wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxSize(-1, 160));
			ConfigureTextEditor(m_DraftEditor);
			fieldSizer->Add(draftLabel, 0, wxEXPAND | wxBOTTOM, 3);
			fieldSizer->Add(m_DraftEditor, 0, wxEXPAND | wxBOTTOM, 8);

			browseButton->Bind(wxEVT_BUTTON, &RecordDialog::HandleBrowseDraft, this);
			internalButton->Bind(wxEVT_BUTTON, &RecordDialog::HandleMakeInternalDraft, this);
			openButton->Bind(wxEVT_BUTTON, &RecordDialog::HandleOpenDraft, this);
		}
	}
}

void RecordDialog::HandleAccepted(wxCommandEvent &event)
{
	event.Skip(false);

	wxString errorMessage;

	if (not SaveDraftText(&errorMessage))
	{
		wxMessageBox(errorMessage, m_Config.title, wxOK | wxICON_ERROR, this);

		return;
	}

	EndModal(wxID_OK);
}

void RecordDialog::HandleBrowseDraft(wxCommandEvent &event)
{
	event.Skip();

	wxFileDialog dialog(
		this,
		"Choose draft file",
		"",
		"",
		"Draft files (*.md;*.txt)|*.md;*.txt|All files (*.*)|*.*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dialog.ShowModal() == wxID_OK)
	{
		SetEditorValue("draft_path", dialog.GetPath());
		LoadDraftText();
	}
}

void RecordDialog::HandleMakeInternalDraft(wxCommandEvent &event)
{
	event.Skip();

	if (m_Package == nullptr)
	{
		return;
	}

	wxString title = GetEditorValue(m_Config.draftTitleColumn);

	if (title.empty())
	{
		title = "draft";
	}

	SetEditorValue("draft_path", m_Package->MakeInternalDraftPath(title));
	LoadDraftText();
}

void RecordDialog::HandleOpenDraft(wxCommandEvent &event)
{
	event.Skip();

	const std::filesystem::path draftPath = ResolveDraftPath();

	if (draftPath.empty())
	{
		return;
	}

	wxLaunchDefaultApplication(wxString(draftPath.wstring()));
}

void RecordDialog::LoadRecord()
{
	if (m_Record == nullptr)
	{
		return;
	}

	for (const FieldDefinition &field : m_Config.fields)
	{
		const auto iterator = m_Record->fields.find(field.columnName);
		wxString value;

		if (iterator != m_Record->fields.end())
		{
			value = FromUtf8(iterator->second);
		}

		SetEditorValue(field.columnName, value);
	}
}

void RecordDialog::LoadDraftText()
{
	if (m_DraftEditor == nullptr)
	{
		return;
	}

	m_DraftEditor->SetText("");

	const std::filesystem::path draftPath = ResolveDraftPath();

	if (draftPath.empty() or not std::filesystem::exists(draftPath))
	{
		return;
	}

	std::ifstream stream(draftPath, std::ios::binary);

	if (not stream)
	{
		return;
	}

	const std::string text((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
	m_DraftEditor->SetText(wxString::FromUTF8(text.c_str()));
}

std::vector<wxString> RecordDialog::LoadChoices(const FieldDefinition &field) const
{
	std::vector<wxString> choices;
	wxString errorMessage;

	if (m_Repository == nullptr)
	{
		return choices;
	}

	if (field.choiceSource == FieldChoiceSource::Characters)
	{
		return m_Repository->LoadDistinctColumnValues("characters", "display_name", &errorMessage);
	}

	if (field.choiceSource == FieldChoiceSource::Locations)
	{
		return m_Repository->LoadDistinctColumnValues("locations", "location_name", &errorMessage);
	}

	if (field.choiceSource == FieldChoiceSource::Scenes)
	{
		return m_Repository->LoadDistinctColumnValues("scenes", "scene_title", &errorMessage);
	}

	if (field.choiceSource == FieldChoiceSource::StoryRecords)
	{
		for (const wxString &value : m_Repository->LoadDistinctColumnValues("characters", "display_name", &errorMessage))
		{
			AddChoiceIfMissing(&choices, value);
		}

		for (const wxString &value : m_Repository->LoadDistinctColumnValues("locations", "location_name", &errorMessage))
		{
			AddChoiceIfMissing(&choices, value);
		}

		for (const wxString &value : m_Repository->LoadDistinctColumnValues("entities", "entity_name", &errorMessage))
		{
			AddChoiceIfMissing(&choices, value);
		}

		return choices;
	}

	if (field.choiceSource == FieldChoiceSource::SceneStatuses)
	{
		return {"Outline", "Draft", "In revision", "Needs work", "Complete", "Final"};
	}

	if (field.choiceSource == FieldChoiceSource::RelationshipTypes)
	{
		return {"ally", "enemy", "family", "romantic", "works for", "owns", "member of", "controls", "knows", "hates", "trusts"};
	}

	if (field.choiceSource == FieldChoiceSource::RelationshipStatuses)
	{
		return {"planned", "active", "hidden", "broken", "resolved"};
	}

	if (field.choiceSource == FieldChoiceSource::RelationshipStrengths)
	{
		return {"weak", "moderate", "strong", "defining"};
	}

	if (field.choiceSource == FieldChoiceSource::EntityTypes)
	{
		return {"item", "organization", "concept", "faction", "artifact", "law", "species", "technology", "magic system"};
	}

	return choices;
}

bool RecordDialog::SaveDraftText(wxString *errorMessage) const
{
	if (m_DraftEditor == nullptr)
	{
		return true;
	}

	const std::filesystem::path draftPath = ResolveDraftPath();

	if (draftPath.empty())
	{
		return true;
	}

	std::error_code errorCode;
	std::filesystem::create_directories(draftPath.parent_path(), errorCode);

	if (errorCode)
	{
		*errorMessage = "Could not create draft folder: " + wxString::FromUTF8(errorCode.message().c_str());

		return false;
	}

	std::ofstream stream(draftPath, std::ios::binary);

	if (not stream)
	{
		*errorMessage = "Could not open the draft file for writing.";

		return false;
	}

	const std::string text = ToUtf8(m_DraftEditor->GetText());
	stream << text;

	return true;
}

wxString RecordDialog::GetEditorValue(const std::string &columnName) const
{
	const auto iterator = m_Editors.find(columnName);

	if (iterator == m_Editors.end())
	{
		return "";
	}

	if (wxTextCtrl *text = wxDynamicCast(iterator->second, wxTextCtrl))
	{
		return text->GetValue();
	}

	if (wxComboBox *combo = wxDynamicCast(iterator->second, wxComboBox))
	{
		return combo->GetValue();
	}

	if (wxStyledTextCtrl *styledText = wxDynamicCast(iterator->second, wxStyledTextCtrl))
	{
		return styledText->GetText();
	}

	return "";
}

void RecordDialog::SetEditorValue(const std::string &columnName, const wxString &value)
{
	const auto iterator = m_Editors.find(columnName);

	if (iterator == m_Editors.end())
	{
		return;
	}

	if (wxTextCtrl *text = wxDynamicCast(iterator->second, wxTextCtrl))
	{
		text->SetValue(value);

		return;
	}

	if (wxComboBox *combo = wxDynamicCast(iterator->second, wxComboBox))
	{
		combo->SetValue(value);

		return;
	}

	if (wxStyledTextCtrl *styledText = wxDynamicCast(iterator->second, wxStyledTextCtrl))
	{
		styledText->SetText(value);
	}
}

std::filesystem::path RecordDialog::ResolveDraftPath() const
{
	const wxString draftPath = GetEditorValue("draft_path");

	if (draftPath.empty())
	{
		return {};
	}

	if (m_Package == nullptr)
	{
		return std::filesystem::path(draftPath.ToStdWstring());
	}

	return m_Package->ResolveStoredPath(draftPath);
}
