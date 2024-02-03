#pragma once

#include "wx/wx.h"
#include "wx/wxprec.h"

#include "Panel.h"
#include "Data.h"

class Frame : public wxFrame {
public:
    Frame(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size);
    virtual ~Frame();

	void Idle();

	bool CheckValidPanel(){ return bool(panel); }
//    VulkanCanvas *tl_canvas;
    
//    wxBoxSizer *topsizer;
//    wxBoxSizer *canvas_h_sizer;
//    wxBoxSizer *canvas_v1_sizer;
//    wxBoxSizer *canvas_v2_sizer;
private:
	std::shared_ptr<Panel> panel;
	
	std::optional<Data::DICOM::XRay> xRay;
	
    //void OnResize(wxSizeEvent& event);
    void OnCreate(wxPaintEvent &event);//(wxWindowCreateEvent& event);//(wxPaintEvent& event);
    
//    VulkanCanvas* m_canvas;
//    VulkanCanvas* m_canvas2;
    
    bool _first_paint_call;
    

//    VulkanCanvas *tr_canvas, *bl_canvas, *br_canvas;

    void OnClose(wxCloseEvent &event);
    void OnQuit(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
	void OnOpenXray(wxCommandEvent &event);
//    void OnVASlider(wxScrollEvent& event);
//    void OnSlice(wxCommandEvent& event);
	
    DECLARE_EVENT_TABLE()
};

