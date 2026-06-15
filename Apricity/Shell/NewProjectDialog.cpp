#include "NewProjectDialog.h"

#include <cwctype>

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/stdpaths.h>
#include <wx/textctrl.h>

namespace
{
	constexpr std::int32_t BrowseCommand = wxID_HIGHEST + 101;
}

NewProjectDialog::NewProjectDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, "New Project", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	  m_NameText(nullptr),
	  m_LocationText(nullptr),
	  m_PreviewText(nullptr)
{
	SetBackgroundColour(*wxWHITE);
	CreateControls();
	RefreshPreview();
}

wxString NewProjectDialog::GetProjectTitle() const
{
	return m_NameText->GetValue();
}

std::filesystem::path NewProjectDialog::GetPackageFile() const
{
	const wxString folderName = MakeFolderName(GetProjectTitle());
	const std::filesystem::path parentFolder(m_LocationText->GetValue().ToStdWstring());
	const std::filesystem::path projectFolder = parentFolder / folderName.ToStdWstring();

	return projectFolder / (folderName.ToStdWstring() + L".apy");
}

void NewProjectDialog::CreateControls()
{
	wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *title = new wxStaticText(this, wxID_ANY, "Configure your new story project");
	wxFont titleFont = title->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 5);
	title->SetFont(titleFont);

	wxStaticText *nameLabel = new wxStaticText(this, wxID_ANY, "Project name");
	m_NameText = new wxTextCtrl(this, wxID_ANY, "Untitled Project");
	m_NameText->SetMinSize(wxSize(620, -1));

	wxStaticText *locationLabel = new wxStaticText(this, wxID_ANY, "Project location");
	wxBoxSizer *locationSizer = new wxBoxSizer(wxHORIZONTAL);
	m_LocationText = new wxTextCtrl(this, wxID_ANY, wxStandardPaths::Get().GetDocumentsDir());
	m_LocationText->SetMinSize(wxSize(620, -1));
	wxButton *browseButton = new wxButton(this, BrowseCommand, "...");
	locationSizer->Add(m_LocationText, 1, wxRIGHT, 8);
	locationSizer->Add(browseButton, 0);

	wxStaticText *summaryLabel = new wxStaticText(this, wxID_ANY, "Summary");
	m_PreviewText = new wxStaticText(this, wxID_ANY, "");

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Back");
	wxButton *createButton = new wxButton(this, wxID_OK, "Create");
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(cancelButton, 0, wxRIGHT, 8);
	buttonSizer->Add(createButton, 0);

	rootSizer->Add(title, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 18);
	rootSizer->Add(nameLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 18);
	rootSizer->Add(m_NameText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 6);
	rootSizer->Add(locationLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 18);
	rootSizer->Add(locationSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 6);
	rootSizer->Add(summaryLabel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 18);
	rootSizer->Add(m_PreviewText, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 6);
	rootSizer->AddStretchSpacer();
	rootSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 16);

	SetSizer(rootSizer);
	rootSizer->SetSizeHints(this);
	Fit();
	CentreOnParent();
	createButton->SetDefault();

	browseButton->Bind(wxEVT_BUTTON, &NewProjectDialog::HandleBrowse, this);
	createButton->Bind(wxEVT_BUTTON, &NewProjectDialog::HandleAccepted, this);
	m_NameText->Bind(wxEVT_TEXT, &NewProjectDialog::HandleTextChanged, this);
	m_LocationText->Bind(wxEVT_TEXT, &NewProjectDialog::HandleTextChanged, this);
}

void NewProjectDialog::HandleBrowse(wxCommandEvent &event)
{
	event.Skip();

	wxDirDialog dialog(this, "Choose parent folder for the project", m_LocationText->GetValue());

	if (dialog.ShowModal() == wxID_OK)
	{
		m_LocationText->SetValue(dialog.GetPath());
		RefreshPreview();
	}
}

void NewProjectDialog::HandleAccepted(wxCommandEvent &event)
{
	event.Skip(false);

	const wxString title = GetProjectTitle();

	if (title.empty())
	{
		wxMessageBox("Project name cannot be empty.", "New Project", wxOK | wxICON_WARNING, this);

		return;
	}

	if (MakeFolderName(title).empty())
	{
		wxMessageBox("Project name must contain at least one letter or number.", "New Project", wxOK | wxICON_WARNING, this);

		return;
	}

	if (m_LocationText->GetValue().empty())
	{
		wxMessageBox("Project location cannot be empty.", "New Project", wxOK | wxICON_WARNING, this);

		return;
	}

	EndModal(wxID_OK);
}

void NewProjectDialog::HandleTextChanged(wxCommandEvent &event)
{
	event.Skip();
	RefreshPreview();
}

void NewProjectDialog::RefreshPreview()
{
	const wxString title = GetProjectTitle();

	if (title.empty())
	{
		m_PreviewText->SetLabel("Project name cannot be empty.");
		m_PreviewText->Wrap(620);
		Layout();
		Fit();

		return;
	}

	if (MakeFolderName(title).empty())
	{
		m_PreviewText->SetLabel("Project name must contain at least one letter or number.");
		m_PreviewText->Wrap(620);
		Layout();
		Fit();

		return;
	}

	m_PreviewText->SetLabel("Project will be created in \"" + wxString(GetPackageFile().parent_path().wstring()) + "\"");
	m_PreviewText->Wrap(620);
	Layout();
	Fit();
}

wxString NewProjectDialog::MakeFolderName(const wxString &title) const
{
	wxString result;

	for (const wxUniChar character : title)
	{
		const wchar_t value = static_cast<wchar_t>(character.GetValue());

		if (std::iswalnum(value) != 0)
		{
			result += value;
		}
		else if (character == ' ' or character == '-' or character == '_')
		{
			if (result.empty() or result.Last() != '_')
			{
				result += '_';
			}
		}
	}

	return result;
}
