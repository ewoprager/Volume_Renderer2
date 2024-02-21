#include <strstream>
//#include <format> // waiting for Xcode 15.3 which supports std::format

#include "Frame.h"

#include "ControlPanel.h"

ControlPanel::ControlPanel(Frame *_frameParent, wxWindowID id) : wxScrolledWindow(_frameParent, id, wxDefaultPosition, wxDefaultSize, wxVSCROLL){
	
	SetScrollRate(-1, 10);
	SetMinSize(wxSize{400, 300});
	
	frameParent = _frameParent;
	
	mySizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mySizer);
	
	// xRay panel
	xRaySizer = new wxStaticBoxSizer(wxVERTICAL, this);
	mySizer->Add(xRaySizer, 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, UI_SPACING);
	
	xRayTitleText = new wxStaticText(this, wxID_ANY, "X-Ray", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	xRaySizer->Add(xRayTitleText, 0, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	loadXRayButton = new wxButton(this, wxID_ANY, "Load X-Ray DICOM");
	xRaySizer->Add(loadXRayButton, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_BUTTON, &ControlPanel::OnLoadXRay, this, loadXRayButton->GetId());
	
	xRayInfoText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	xRaySizer->Add(xRayInfoText, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	xRayGaussianSizer = new wxBoxSizer(wxHORIZONTAL);
	xRaySizer->Add(xRayGaussianSizer, 1, wxEXPAND);
	
	xRayGaussianSlider = new wxSlider(this, wxID_ANY, frameParent->GetXRayGaussianManager().IntDefault(), 0, frameParent->GetXRayGaussianManager().precision);
	xRayGaussianSizer->Add(xRayGaussianSlider, 5, wxALL | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_SLIDER, &ControlPanel::OnXRayGaussianSlider, this, xRayGaussianSlider->GetId());
	
	char str[10];
	snprintf(str, 10, "%.2f", frameParent->GetXRayGaussianManager().dfault);
	xRayGaussianTextCtrl = new wxTextCtrl(this, wxID_ANY, wxString(str), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	xRayGaussianSizer->Add(xRayGaussianTextCtrl, 1, wxALL | wxFIXED_MINSIZE | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_TEXT_ENTER, &ControlPanel::OnXRayGaussianEnter, this, xRayGaussianTextCtrl->GetId());
	
	xRayGaussianDefaultButton = new wxButton(this, wxID_ANY, "Reset sigma to default");
	xRaySizer->Add(xRayGaussianDefaultButton, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_BUTTON, &ControlPanel::OnXRayGaussianDefault, this, xRayGaussianDefaultButton->GetId());
	
	// CT panel
	ctSizer = new wxStaticBoxSizer(wxVERTICAL, this);
	mySizer->Add(ctSizer, 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, UI_SPACING);
	
	ctTitleText = new wxStaticText(this, wxID_ANY, "CT", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	ctSizer->Add(ctTitleText, 0, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	loadCTButton = new wxButton(this, wxID_ANY, "Load CT DICOM");
	ctSizer->Add(loadCTButton, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_BUTTON, &ControlPanel::OnLoadCT, this, loadCTButton->GetId());
	
	ctInfoText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	ctSizer->Add(ctInfoText, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	// placeholders
	placeholderSizer = new wxStaticBoxSizer(wxVERTICAL, this);
	mySizer->Add(placeholderSizer, 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, UI_SPACING);
}

void ControlPanel::SetXRayInfo(const Data::XRay &xRay){
	std::stringstream stream {};
	stream << "Size: [" << xRay.size.x << " x " << xRay.size.y << "]\nPixel spacing: " << xRay.pixelSpacing.x << ", " << xRay.pixelSpacing.y << " mm\nSource distance: " << xRay.sourceDistance << " mm\nImage depth: " << xRay.imageDepth << " bytes";
	xRayInfoText->SetLabel(stream.str());
	frameParent->Layout();
}
void ControlPanel::SetCTInfo(const Data::CT &ct){
	std::stringstream stream {};
	stream << "Size: [" << ct.size.x << " x " << ct.size.y << " x " << ct.size.z << "]\nPixel spacing: " << ct.pixelSpacing.x << ", " << ct.pixelSpacing.y << ", " << ct.pixelSpacing.z;
	ctInfoText->SetLabel(stream.str());
	frameParent->Layout();
}

void ControlPanel::OnLoadXRay(wxCommandEvent &event){
	frameParent->OnOpenXray(event);
}
void ControlPanel::OnLoadCT(wxCommandEvent &event){
	frameParent->OnOpenCT(event);
}

void ControlPanel::OnXRayGaussianSlider(wxCommandEvent &event){
	const float newVal = frameParent->GetXRayGaussianManager().IntToFloat(event.GetInt());
	char str[10];
	snprintf(str, 10, "%.2f", newVal);
	xRayGaussianTextCtrl->SetValue(wxString(str));
	frameParent->UpdateXRayGaussian(newVal);
}
void ControlPanel::OnXRayGaussianEnter(wxCommandEvent &event){
	const float newVal = std::stof(std::string(xRayGaussianTextCtrl->GetValue()));
	xRayGaussianSlider->SetValue(frameParent->GetXRayGaussianManager().FloatToInt(newVal));
	frameParent->UpdateXRayGaussian(newVal);
}
void ControlPanel::OnXRayGaussianDefault(wxCommandEvent &event){
	const float newVal = frameParent->GetXRayGaussianManager().dfault;
	char str[10];
	snprintf(str, 10, "%.2f", newVal);
	xRayGaussianTextCtrl->SetValue(wxString(str));
	xRayGaussianSlider->SetValue(frameParent->GetXRayGaussianManager().FloatToInt(newVal));
	frameParent->UpdateXRayGaussian(newVal);
}
