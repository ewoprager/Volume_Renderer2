#include "ViewPanel.h"

ViewPanel::ViewPanel(MainPanel *_mainPanel, wxWindow *parent, wxWindowID id) : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, 0, ""){
	
	mainPanel = _mainPanel;
	
	mainSizer = std::make_shared<wxBoxSizer>(wxHORIZONTAL);
	SetSizer(mainSizer.get());
	
	mouseModeLabel = std::make_shared<wxStaticText>(this, wxID_ANY, "Mouse mode: ");
	mainSizer->Add(mouseModeLabel.get(), 1, wxALL, 8);
	
	static const std::array<wxString, int(MainPanel::MouseMode::_COUNT_)> mouseModeChoiceStrings = {{
		"X-Ray windowing", "View position"
	}};
	mouseModeChoice = std::make_shared<wxChoice>(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, int(MainPanel::MouseMode::_COUNT_), mouseModeChoiceStrings.data());
	mainSizer->Add(mouseModeChoice.get(), 1, wxALL, 8);
	mouseModeChoice->SetSelection(int(MainPanel::MouseMode::WINDOWING));
	mainPanel->SetMouseMode(MainPanel::MouseMode::WINDOWING);
	Bind(wxEVT_CHOICE, &ViewPanel::OnMouseModeChoice, this, mouseModeChoice->GetId());
}

void ViewPanel::OnMouseModeChoice(wxCommandEvent &event){
	mainPanel->SetMouseMode(MainPanel::MouseMode(event.GetSelection()));
}
