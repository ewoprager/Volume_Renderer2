#pragma once

#include "wx/wx.h"

#include "Header.h"
#include "Data.h"

class ControlPanel : public wxScrolledWindow {
public:
	ControlPanel(Frame *_frameParent, wxWindowID id=wxID_ANY);
	
	void SetXRayInfo(const Data::XRay &xRay);
	
private:
	Frame *frameParent;
	
	void OnLoadXRay(wxCommandEvent &event);
	
	std::shared_ptr<wxBoxSizer> mySizer;
	
	std::shared_ptr<wxStaticBoxSizer> xRaySizer;
	std::shared_ptr<wxStaticText> xRayTitleText;
	std::shared_ptr<wxButton> loadXRayButton;
	std::shared_ptr<wxStaticText> xRayInfoText;
	
	std::shared_ptr<wxStaticBoxSizer> placeholderSizer1;
	std::shared_ptr<wxStaticBoxSizer> placeholderSizer2;
};
