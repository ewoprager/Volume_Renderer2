#include "Frame.h"

#include "ViewPanel.h"

ViewPanel::ViewPanel(Frame *_frameParent, wxWindowID id) : wxPanel(_frameParent, id, wxDefaultPosition, wxDefaultSize, 0, ""){
	
	frameParent = _frameParent;
	
	// main
	mainSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSizer);
	
	// top
	topSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(topSizer, 0, wxALIGN_LEFT);
	
	mouseModeLabel = new wxStaticText(this, wxID_ANY, "Mouse mode: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	topSizer->Add(mouseModeLabel, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	static const std::array<wxString, int(MainPanel::MouseMode::_COUNT_)> mouseModeChoiceStrings = {{
		"X-Ray windowing", "View position"
	}};
	mouseModeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, int(MainPanel::MouseMode::_COUNT_), mouseModeChoiceStrings.data());
	topSizer->Add(mouseModeChoice, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	mouseModeChoice->SetSelection(int(MainPanel::MouseMode::WINDOWING));
	frameParent->GetMainPanel()->SetMouseMode(MainPanel::MouseMode::WINDOWING);
	Bind(wxEVT_CHOICE, &ViewPanel::OnMouseModeChoice, this, mouseModeChoice->GetId());
	
	// bottom
	bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(bottomSizer, 1, wxEXPAND);
	
	// windowing
	windowingSizer = new wxStaticBoxSizer(wxVERTICAL, this);
	bottomSizer->Add(windowingSizer, 1, wxTOP | wxLEFT | wxBOTTOM | wxEXPAND, UI_SPACING);
	
	windowingTitleText = new wxStaticText(this, wxID_ANY, "Windowing", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	windowingSizer->Add(windowingTitleText, 0, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	windowingInfoText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	windowingSizer->Add(windowingInfoText, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	
	windowingConstrainedBox = new wxCheckBox(this, wxID_ANY, "Constrain to value range");
	windowingSizer->Add(windowingConstrainedBox, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	windowingConstrainedBox->SetValue(false);
	Bind(wxEVT_CHECKBOX, &ViewPanel::OnSwitchWindowingConstrained, this, windowingConstrainedBox->GetId());
	
	windowingResetButton = new wxButton(this, wxID_ANY, "Reset windowing");
	windowingSizer->Add(windowingResetButton, 1, wxALL | wxALIGN_CENTRE, UI_SPACING);
	Bind(wxEVT_BUTTON, &ViewPanel::OnResetWindowing, this, windowingResetButton->GetId());
	
	// placeholders
	placeholderSizer1 = new wxStaticBoxSizer(wxVERTICAL, this);
	bottomSizer->Add(placeholderSizer1, 1, wxTOP | wxLEFT | wxBOTTOM | wxEXPAND, UI_SPACING);
	placeholderSizer2 = new wxStaticBoxSizer(wxVERTICAL, this);
	bottomSizer->Add(placeholderSizer2, 1, wxALL | wxEXPAND, UI_SPACING);
}

void ViewPanel::SetWindowingInfo(int64_t windowCentre, int64_t windowWidth){
	windowingInfoText->SetLabel(wxString{"Window centre: "} << windowCentre << "\nWindow width: " << windowWidth);
	frameParent->Layout();
}

void ViewPanel::OnMouseModeChoice(wxCommandEvent &event){
	frameParent->GetMainPanel()->SetMouseMode(MainPanel::MouseMode(event.GetSelection()));
}

void ViewPanel::OnResetWindowing(wxCommandEvent &event){
	frameParent->GetMainPanel()->DefaultXRayWindowingConstants();
}

void ViewPanel::OnSwitchWindowingConstrained(wxCommandEvent &event){
	frameParent->GetMainPanel()->SetConstrainWindowing(event.IsChecked());
}
