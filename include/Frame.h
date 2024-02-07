#pragma once

#include "wx/wx.h"
#include "wx/wxprec.h"

#include "ControlPanel.h"
#include "MainPanel.h"
#include "ViewPanel.h"
#include "Data.h"

class Frame : public wxFrame {
public:
    Frame(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size);
    virtual ~Frame();

	void Idle();

	bool CheckValidPanel(){ return bool(mainPanel); }
private:
	std::shared_ptr<wxBoxSizer> mainSizer;
	std::shared_ptr<ControlPanel> controlPanel;
	std::shared_ptr<wxBoxSizer> rightSizer;
	std::shared_ptr<MainPanel> mainPanel;
	std::shared_ptr<ViewPanel> viewPanel;
	
	std::optional<Data::XRay> xRay;
	
    //void OnResize(wxSizeEvent& event);
    void OnCreate(wxPaintEvent &event);
    
    bool _first_paint_call;

    void OnClose(wxCloseEvent &event);
    void OnQuit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
	void OnOpenXray(wxCommandEvent &event);
	
    DECLARE_EVENT_TABLE()
};

