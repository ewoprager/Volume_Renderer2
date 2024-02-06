#pragma once

#include "wx/wx.h"

#include "MainPanel.h"

class ViewPanel : public wxPanel {
public:
	ViewPanel(MainPanel *_mainPanel, wxWindow *parent, wxWindowID id);
	
private:
	MainPanel *mainPanel;
	
	void OnMouseModeChoice(wxCommandEvent &event);
	
	std::shared_ptr<wxBoxSizer> mainSizer;
	std::shared_ptr<wxStaticText> mouseModeLabel;
	std::shared_ptr<wxChoice> mouseModeChoice;
};
