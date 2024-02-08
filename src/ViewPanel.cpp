#include "Frame.h"

#include "ViewPanel.h"

ViewPanel::ViewPanel(Frame *_frameParent, wxWindowID id) : wxPanel(_frameParent, id, wxDefaultPosition, wxDefaultSize, 0, ""){
	
	frameParent = _frameParent;
	
	// main
	mainSizer = std::make_shared<wxBoxSizer>(wxVERTICAL);
	SetSizer(mainSizer.get());
	
	// top
	topSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);
	mainSizer->Add(topSizer.get(), 0, wxEXPAND);
	
	mouseModeLabel = std::make_shared<wxStaticText>(this, wxID_ANY, "Mouse mode: ", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	topSizer->Add(mouseModeLabel.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	
	static const std::array<wxString, int(MainPanel::MouseMode::_COUNT_)> mouseModeChoiceStrings = {{
		"X-Ray windowing", "View position"
	}};
	mouseModeChoice = std::make_shared<wxChoice>(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, int(MainPanel::MouseMode::_COUNT_), mouseModeChoiceStrings.data());
	topSizer->Add(mouseModeChoice.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	mouseModeChoice->SetSelection(int(MainPanel::MouseMode::WINDOWING));
	frameParent->GetMainPanel()->SetMouseMode(MainPanel::MouseMode::WINDOWING);
	Bind(wxEVT_CHOICE, &ViewPanel::OnMouseModeChoice, this, mouseModeChoice->GetId());
	
	// bottom
	bottomSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);
	mainSizer->Add(bottomSizer.get(), 1, wxEXPAND);
	
	// windowing
	windowingSizer = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	bottomSizer->Add(windowingSizer.get(), 1, wxTOP | wxLEFT | wxBOTTOM | wxEXPAND, UI_SPACING);
	
	windowingTitleText = std::make_shared<wxStaticText>(this, wxID_ANY, "Windowing", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	windowingSizer->Add(windowingTitleText.get(), 0, wxALL | wxEXPAND, UI_SPACING);
	
	windowingInfoText = std::make_shared<wxStaticText>(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
	windowingSizer->Add(windowingInfoText.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	
	windowingConstrainedBox = std::make_shared<wxCheckBox>(this, wxID_ANY, "Constrain to value range");
	windowingSizer->Add(windowingConstrainedBox.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	windowingConstrainedBox->SetValue(false);
	Bind(wxEVT_CHECKBOX, &ViewPanel::OnSwitchWindowingConstrained, this, windowingConstrainedBox->GetId());
	
	windowingResetButton = std::make_shared<wxButton>(this, wxID_ANY, "Reset windowing");
	windowingSizer->Add(windowingResetButton.get(), 1, wxALL | wxEXPAND, UI_SPACING);
	Bind(wxEVT_BUTTON, &ViewPanel::OnResetWindowing, this, windowingResetButton->GetId());
	
	// placeholders
	placeholderSizer1 = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	bottomSizer->Add(placeholderSizer1.get(), 1, wxTOP | wxLEFT | wxBOTTOM | wxEXPAND, UI_SPACING);
	placeholderSizer2 = std::make_shared<wxStaticBoxSizer>(wxVERTICAL, this);
	bottomSizer->Add(placeholderSizer2.get(), 1, wxALL | wxEXPAND, UI_SPACING);
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
