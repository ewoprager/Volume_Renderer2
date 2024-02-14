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
	void OnXRayGaussianSlider(wxCommandEvent &event);
	void OnXRayGaussianEnter(wxCommandEvent &event);
	void OnXRayGaussianDefault(wxCommandEvent &event);
	
	wxBoxSizer *mySizer;
	
	wxStaticBoxSizer *xRaySizer;
	wxStaticText *xRayTitleText;
	wxButton *loadXRayButton;
	wxStaticText *xRayInfoText;
	wxBoxSizer *xRayGaussianSizer;
	wxSlider *xRayGaussianSlider;
	wxTextCtrl *xRayGaussianTextCtrl;
	wxButton *xRayGaussianDefaultButton;
	
	wxStaticBoxSizer *placeholderSizer1;
	wxStaticBoxSizer *placeholderSizer2;
};
