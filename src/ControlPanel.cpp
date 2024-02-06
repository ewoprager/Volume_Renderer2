#include "ControlPanel.h"

ControlPanel::ControlPanel(wxWindow *parent, wxWindowID id) : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER | wxHSCROLL | wxVSCROLL){
	
	mySizer = std::make_shared<wxBoxSizer>(wxVERTICAL);
	SetSizer(mySizer.get());
	
	wxCheckBox *slice_cb = new wxCheckBox(this, wxID_ANY, "show slice and reslices");
	mySizer->Add(slice_cb, 0, wxALL, 5);
	slice_cb->SetValue(true);
}
