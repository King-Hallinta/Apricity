#pragma once

#include "RecordPanel.h"

#include <filesystem>
#include <map>
#include <string>

#include <wx/dialog.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>

class wxSizer;
class ProjectPackage;

class RecordDialog : public wxDialog
{
public:
	RecordDialog(wxWindow *parent, const RecordPanelConfig &config, StoryRepository *repository, ProjectPackage *package, const StoryRecord *record);

	std::map<std::string, std::string> GetValues() const;

private:
	void CreateControls();
	void CreateFieldEditors(wxWindow *parent, wxSizer *sizer);
	void HandleAccepted(wxCommandEvent &event);
	void HandleBrowseDraft(wxCommandEvent &event);
	void HandleMakeInternalDraft(wxCommandEvent &event);
	void HandleOpenDraft(wxCommandEvent &event);
	void LoadRecord();
	void LoadDraftText();
	std::vector<wxString> LoadChoices(const FieldDefinition &field) const;
	bool SaveDraftText(wxString *errorMessage) const;
	wxString GetEditorValue(const std::string &columnName) const;
	void SetEditorValue(const std::string &columnName, const wxString &value);
	std::filesystem::path ResolveDraftPath() const;

	RecordPanelConfig m_Config;
	StoryRepository *m_Repository;
	ProjectPackage *m_Package;
	const StoryRecord *m_Record;
	wxStyledTextCtrl *m_DraftEditor;
	std::map<std::string, wxWindow *> m_Editors;
};
