#pragma once

#include <filesystem>

#include <wx/dialog.h>
#include <wx/string.h>

class wxStaticText;
class wxTextCtrl;

class NewProjectDialog : public wxDialog
{
public:
	explicit NewProjectDialog(wxWindow *parent);

	wxString GetProjectTitle() const;
	std::filesystem::path GetPackageFile() const;

private:
	void CreateControls();
	void HandleBrowse(wxCommandEvent &event);
	void HandleAccepted(wxCommandEvent &event);
	void HandleTextChanged(wxCommandEvent &event);
	void RefreshPreview();
	wxString MakeFolderName(const wxString &title) const;

	wxTextCtrl *m_NameText;
	wxTextCtrl *m_LocationText;
	wxStaticText *m_PreviewText;
};
