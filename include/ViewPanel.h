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
	
	std::shared_ptr<wxBoxSizer> mainSizer;
	
	std::shared_ptr<wxBoxSizer> topSizer;
	std::shared_ptr<wxStaticText> mouseModeLabel;
	std::shared_ptr<wxChoice> mouseModeChoice;
	
	std::shared_ptr<wxBoxSizer> bottomSizer;
	
	std::shared_ptr<wxStaticBoxSizer> windowingSizer;
	std::shared_ptr<wxStaticText> windowingTitleText;
	std::shared_ptr<wxStaticText> windowingInfoText;
	std::shared_ptr<wxCheckBox> windowingConstrainedBox;
	std::shared_ptr<wxButton> windowingResetButton;
	
	std::shared_ptr<wxStaticBoxSizer> placeholderSizer1;
	std::shared_ptr<wxStaticBoxSizer> placeholderSizer2;
};
