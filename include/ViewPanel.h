#pragma once

#include "wx/wx.h"

#include "Header.h"

class ViewPanel : public wxPanel {
public:
	ViewPanel(Frame *_frameParent, wxWindowID id);
	
	void SetWindowingInfo(int64_t windowCentre, int64_t windowWidth);
	
private:
	Frame *frameParent;
	
	void OnMouseModeChoice(wxCommandEvent &event);
	void OnResetWindowing(wxCommandEvent &event);
	void OnSwitchWindowingConstrained(wxCommandEvent &event);
	
	wxBoxSizer *mainSizer;
	
	wxBoxSizer *topSizer;
	wxStaticText *mouseModeLabel;
	wxChoice *mouseModeChoice;
	
	wxBoxSizer *bottomSizer;
	
	wxStaticBoxSizer *windowingSizer;
	wxStaticText *windowingTitleText;
	wxStaticText *windowingInfoText;
	wxCheckBox *windowingConstrainedBox;
	wxButton *windowingResetButton;
	
	wxStaticBoxSizer *placeholderSizer1;
	wxStaticBoxSizer *placeholderSizer2;
};
