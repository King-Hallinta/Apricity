#include "FieldSelectionDialog.h"

#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

FieldSelectionDialog::FieldSelectionDialog(wxWindow *parent, const RecordPanelConfig &config, const std::vector<std::size_t> &visibleFields)
	: wxDialog(parent, wxID_ANY, "Columns", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	  m_FieldList(nullptr)
{
	SetBackgroundColour(*wxWHITE);
	CreateControls(config, visibleFields);
}

std::vector<std::size_t> FieldSelectionDialog::GetVisibleFields() const
{
	std::vector<std::size_t> visibleFields;

	for (std::uint32_t index = 0; index < m_FieldList->GetCount(); ++index)
	{
		if (m_FieldList->IsChecked(index))
		{
			visibleFields.push_back(static_cast<std::size_t>(index));
		}
	}

	return visibleFields;
}

void FieldSelectionDialog::CreateControls(const RecordPanelConfig &config, const std::vector<std::size_t> &visibleFields)
{
	wxBoxSizer *rootSizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText *title = new wxStaticText(this, wxID_ANY, "Choose fields to show in the " + config.title.Lower() + " list");
	wxFont titleFont = title->GetFont();
	titleFont.SetPointSize(titleFont.GetPointSize() + 4);
	title->SetFont(titleFont);

	m_FieldList = new wxCheckListBox(this, wxID_ANY);
	m_FieldList->SetMinSize(wxSize(420, 260));

	for (std::size_t index = 0; index < config.fields.size(); ++index)
	{
		m_FieldList->Append(config.fields[index].label);
		m_FieldList->Check(static_cast<std::uint32_t>(index), IsVisibleField(index, visibleFields));
	}

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
	wxButton *applyButton = new wxButton(this, wxID_OK, "Apply");
	buttonSizer->AddStretchSpacer();
	buttonSizer->Add(cancelButton, 0, wxRIGHT, 8);
	buttonSizer->Add(applyButton, 0);

	rootSizer->Add(title, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 16);
	rootSizer->Add(m_FieldList, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 16);
	rootSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 16);
	SetSizer(rootSizer);
	rootSizer->SetSizeHints(this);
	Fit();
	CentreOnParent();

	applyButton->SetDefault();
	applyButton->Bind(wxEVT_BUTTON, &FieldSelectionDialog::HandleAccepted, this);
}

void FieldSelectionDialog::HandleAccepted(wxCommandEvent &event)
{
	event.Skip(false);

	if (GetVisibleFields().empty())
	{
		wxMessageBox("Select at least one field to show.", "Columns", wxOK | wxICON_WARNING, this);

		return;
	}

	EndModal(wxID_OK);
}

bool FieldSelectionDialog::IsVisibleField(std::size_t fieldIndex, const std::vector<std::size_t> &visibleFields) const
{
	for (const std::size_t visibleField : visibleFields)
	{
		if (visibleField == fieldIndex)
		{
			return true;
		}
	}

	return false;
}
