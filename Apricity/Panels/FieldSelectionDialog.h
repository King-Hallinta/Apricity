#pragma once

#include "RecordPanel.h"

#include <vector>

#include <wx/dialog.h>

class wxCheckListBox;

class FieldSelectionDialog : public wxDialog
{
public:
	FieldSelectionDialog(wxWindow *parent, const RecordPanelConfig &config, const std::vector<std::size_t> &visibleFields);

	std::vector<std::size_t> GetVisibleFields() const;

private:
	void CreateControls(const RecordPanelConfig &config, const std::vector<std::size_t> &visibleFields);
	void HandleAccepted(wxCommandEvent &event);
	bool IsVisibleField(std::size_t fieldIndex, const std::vector<std::size_t> &visibleFields) const;

	wxCheckListBox *m_FieldList;
};
