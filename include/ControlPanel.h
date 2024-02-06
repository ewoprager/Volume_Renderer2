#pragma once

#include "wx/wx.h"

class ControlPanel : public wxScrolledWindow {
public:
	ControlPanel(wxWindow *parent, wxWindowID id=wxID_ANY);
	
private:
	std::shared_ptr<wxBoxSizer> mySizer;
};
