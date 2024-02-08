#include <strstream>

#include "Frame.h"

#include "ControlPanel.h"

ControlPanel::ControlPanel(Frame *_frameParent, wxWindowID id) : wxScrolledWindow(_frameParent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL){
	
	frameParent = _frameParent;
	
	mySizer = std::make_shared<wxBoxSizer>(wxVERTICAL);
	SetSizer(mySizer.get());
	
	// xRay panel
	xRaySizer = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	mySizer->Add(xRaySizer.get(), 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, UI_SPACING);
	
	xRayTitleText = std::make_shared<wxStaticText>(this, wxID_ANY, "X-Ray", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	xRaySizer->Add(xRayTitleText.get(), 0, wxALL | wxEXPAND, UI_SPACING);
	
	loadXRayButton = std::make_shared<wxButton>(this, wxID_ANY, "Load X-Ray DICOM");
	xRaySizer->Add(loadXRayButton.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	Bind(wxEVT_BUTTON, &ControlPanel::OnLoadXRay, this, loadXRayButton->GetId());
	
	xRayInfoText = std::make_shared<wxStaticText>(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	xRaySizer->Add(xRayInfoText.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	
	// placeholders
	placeholderSizer1 = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	mySizer->Add(placeholderSizer1.get(), 1, wxTOP | wxLEFT | wxRIGHT | wxEXPAND, UI_SPACING);
	placeholderSizer2 = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	mySizer->Add(placeholderSizer2.get(), 1, wxALL | wxEXPAND, UI_SPACING);
}

void ControlPanel::SetXRayInfo(const Data::XRay &xRay){
	std::stringstream stream {};
	stream << "Size: [" << xRay.size.x << " x " << xRay.size.y << "]\nPixel spacing: " << xRay.pixelSpacing.x << ", " << xRay.pixelSpacing.y << " mm\nSource distance: " << xRay.sourceDistance << " mm\nImage depth: " << xRay.imageDepth << " bytes";
	xRayInfoText->SetLabel(stream.str());
	frameParent->Layout();
}

void ControlPanel::OnLoadXRay(wxCommandEvent &event){
	frameParent->OnOpenXray(event);
}
