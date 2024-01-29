#include <wx/file.h>
#include <wx/log.h>

#include "App.h"
#include "Frame.h"

BEGIN_EVENT_TABLE(Frame, wxFrame)
    EVT_MENU(wxID_EXIT, Frame::OnQuit)
    EVT_MENU(wxID_ABOUT, Frame::OnAbout)
//    EVT_COMMAND_SCROLL(VA_SLIDER_ID, Frame::OnVASlider)
//    EVT_CHECKBOX(SLICE_CHECKBOX_ID, Frame::OnSlice)
    EVT_CLOSE(Frame::OnClose)
END_EVENT_TABLE()

Frame::Frame(wxWindow *parent, const wxString &title, const wxPoint &pos, const wxSize &size)
    : wxFrame(parent, wxID_ANY, title, pos, size) {

    // Lay out the canvases and controls
    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(wxID_ABOUT, "&About");
    fileMenu->Append(wxID_EXIT, "&Quit");
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&File");
    SetMenuBar(menuBar);
    wxStatusBar *statusBar = CreateStatusBar(2);
    int widths[2] = {-2, -1};
    statusBar->SetStatusWidths(2, widths);
    int styles[2] = {wxSB_SUNKEN, wxSB_SUNKEN};
    statusBar->SetStatusStyles(2, styles);

    // The window is layed out using topsizer, inside which are lower level windows and sizers for the canvases and controls
//    topsizer = new wxBoxSizer(wxHORIZONTAL);
//     SetSizer(topsizer);
//     canvas_h_sizer = new wxBoxSizer(wxHORIZONTAL);
//     canvas_v1_sizer = new wxBoxSizer(wxVERTICAL);
//     canvas_v2_sizer = new wxBoxSizer(wxVERTICAL);
//     wxScrolledWindow* controlwin = new wxScrolledWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL);
//     topsizer->Add(canvas_h_sizer, 1, wxEXPAND);
//     topsizer->Add(controlwin, 0, wxEXPAND);
//     canvas_h_sizer->Add(canvas_v1_sizer, 1, wxEXPAND);
//     canvas_h_sizer->Add(canvas_v2_sizer, 1, wxEXPAND);

    // // Create the four canvases and add them to the relevant sizer
     //tl_canvas = new VulkanCanvas(this, wxID_ANY);//, CANVAS_TL);
     //tr_canvas = new VulkanCanvas(this, wxID_ANY);//, CANVAS_TR);
     //bl_canvas = new VulkanCanvas(this, wxID_ANY);//, CANVAS_BL);
     //br_canvas = new VulkanCanvas(this, wxID_ANY);//, CANVAS_BR);
     //canvas_v1_sizer->Add(tl_canvas, 1, wxEXPAND | wxALL, 2);
     //canvas_v1_sizer->Add(bl_canvas, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
     //canvas_v2_sizer->Add(tr_canvas, 1, wxEXPAND | wxTOP | wxBOTTOM, 2);
     //canvas_v2_sizer->Add(br_canvas, 1, wxEXPAND | wxBOTTOM, 2);

    // Create a sizer to manage the contents of the scrolled control window
//    wxBoxSizer *control_sizer = new wxBoxSizer(wxVERTICAL);
//    controlwin->SetSizer(control_sizer);
//    controlwin->SetScrollRate(10, 10);
//    controlwin->SetMinSize(wxSize(210,-1));

    // Create the rendering controls and add them to the control sizer
//     show_skybox = false;
//     wxCheckBox* skybox_cb = new wxCheckBox(controlwin, SKYBOX_CHECKBOX_ID, "show skybox");
//     control_sizer->Add(skybox_cb, 0, wxALL, 5);
//     skybox_cb->SetValue(show_skybox);
       
//     view_angle = 0;
//     control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "view angle (degrees)"), 0, wxTOP | wxLEFT, 10);
//     control_sizer->Add(new wxSlider(controlwin, VA_SLIDER_ID, view_angle, 0, 135, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);

//     reslice_alpha = 5;
//     control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "reslice opacity"), 0, wxTOP | wxLEFT, 10);
//     control_sizer->Add(new wxSlider(controlwin, RO_SLIDER_ID, reslice_alpha, 0, 10, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
//     surface_alpha = 10;
//     control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "surface opacity"), 0, wxTOP | wxLEFT, 10);
//     control_sizer->Add(new wxSlider(controlwin, SO_SLIDER_ID, surface_alpha, 0, 10, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 0,  wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Separate the rendering controls from the reslice controls
//     control_sizer->AddSpacer(54);

    // Create the reslice controls and add them to the control sizer
//    interpolate = true;
//    wxCheckBox *interp_cb = new wxCheckBox(controlwin, INTERP_CHECKBOX_ID, "linear interpolation");
//    control_sizer->Add(interp_cb, 0, wxALL, 5);
//    interp_cb->SetValue(interpolate);
//
//    data_slice_number = NUM_SLICES/2.0;
//    control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "x-y data slice"), 0, wxLEFT, 10);
//    control_sizer->Add(new wxSlider(controlwin, XY_SLIDER_ID, data_slice_number, 0, NUM_SLICES-1, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL), 0, wxALL, 5);
//
//    y_reslice_position = VOLUME_HEIGHT/2.0;
//    control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "x-z reslice"), 0, wxLEFT, 10);
//    control_sizer->Add(new wxSlider(controlwin, XZ_SLIDER_ID, y_reslice_position, 0, VOLUME_HEIGHT-1, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL), 0, wxALL, 5);
//
//    x_reslice_position = VOLUME_WIDTH/2.0;
//    control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "y-z reslice"), 0, wxLEFT, 10);
//    control_sizer->Add(new wxSlider(controlwin, YZ_SLIDER_ID, x_reslice_position, 0, VOLUME_WIDTH-1, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL), 0, wxALL, 5);
//
//    // Separate the reslice controls from the isosurface controls
//    control_sizer->AddSpacer(54);
//
//    // Create the isosurface controls and add them to the control sizer
//    threshold = 255;
//    set_threshold_colourmap();
//    control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "surface threshold"), 0, wxTOP | wxLEFT, 10);
//    control_sizer->Add(new wxSlider(controlwin, ST_SLIDER_ID, threshold, 0, 255, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 0,  wxLEFT | wxRIGHT | wxBOTTOM, 5);
//
//    surface_resolution = 5;
//    control_sizer->Add(new wxStaticText(controlwin, wxID_ANY, "surface resolution"), 0, wxTOP | wxLEFT, 10);
//    control_sizer->Add(new wxSlider(controlwin, SR_SLIDER_ID, surface_resolution, 1, 40, wxDefaultPosition, wxSize(150, -1), wxSL_HORIZONTAL | wxSL_VALUE_LABEL), 0,  wxLEFT | wxRIGHT | wxBOTTOM, 5);
//    control_sizer->Add(new wxButton(controlwin, ISO_BUTTON_ID, "Calculate isosurface"), 0, wxALIGN_CENTRE | wxALL, 5);

    //set_threshold_colourmap();
    //construct_data_slice_image();
    //construct_xz_reslice();
    //construct_yz_reslice();

    Bind(wxEVT_PAINT, &Frame::OnCreate, this);
}


Frame::~Frame() {}

//void VulkanWindow::OnResize(wxSizeEvent& event)
//{
//    wxSize clientSize = GetClientSize();
//    //m_canvas->SetSize(clientSize);
//}

void Frame::OnCreate(wxPaintEvent &event) {
    // Create the four canvases and add them to the relevant sizer

//     tl_canvas = new VulkanCanvas(this, &texture_store, CANVAS_TL, wxID_ANY);//, CANVAS_TL);
//     tr_canvas = new VulkanCanvas(this, &texture_store, CANVAS_TR, wxID_ANY);//, CANVAS_TR);
//     bl_canvas = new VulkanCanvas(this, &texture_store, CANVAS_BL, wxID_ANY);//, CANVAS_BL);
//     br_canvas = new VulkanCanvas(this, &texture_store, CANVAS_BR, wxID_ANY);//, CANVAS_BR);
//     canvas_v1_sizer->Add(tl_canvas, 1, wxEXPAND | wxALL, 2);
//     canvas_v1_sizer->Add(bl_canvas, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
//     canvas_v2_sizer->Add(tr_canvas, 1, wxEXPAND | wxTOP | wxBOTTOM, 2);
//     canvas_v2_sizer->Add(br_canvas, 1, wxEXPAND | wxBOTTOM, 2);
// 
//     SendSizeEvent();
// 
//     construct_data_slice_image();
//     tl_canvas->objects[0].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, data_slice_number * (float)SLICE_SEPARATION));
//     construct_xz_reslice();
//     tl_canvas->objects[1].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, y_reslice_position, 0.0f));
//     construct_yz_reslice();
//     tl_canvas->objects[2].model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x_reslice_position, 0.0f, 0.0f));
//     tl_canvas->uniformBufferUpdateRequired = true;
//     tl_canvas->drawFrame();
//     tr_canvas->drawFrame();
//     bl_canvas->drawFrame();
//     br_canvas->drawFrame();

	panel = std::make_shared<Panel>(this);
	
	SendSizeEvent();
	
	panel->DrawFrame();
	
    Unbind(wxEVT_PAINT, &Frame::OnCreate, this);

#ifdef __linux__
    WindowHandleInfo handle;
    tl_canvas->gui_initHandleContextFromWxWidgetsWindow(handle, this);
    if (handle.backend == WindowHandleInfo::Backend::X11) {
        wxTheApp->Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(App::OnXLaunchIdle));
    }
#endif
}

void Frame::Idle(){
	panel->DrawFrame();
}

void Frame::OnClose(wxCloseEvent& event)
// Called when application is closed down (including window manager close)
{
    Destroy();
    exit(0); // a workaround for a wxWidgets bug, should be able to remove this once the wxWidgets bug is fixed
}

void Frame::OnQuit(wxCommandEvent& event)
// Callback for the "Quit" menu item
{
    Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
// Callback for the "About" menu item
{
    wxMessageDialog about(this, "Volume renderer and 2D/3D registration tool for radiographic cochlear implant data\nEdmund Prager - 2024", "About Volume_Renderer2", wxICON_INFORMATION | wxOK);
    about.ShowModal();
}
// 
// void VulkanWindow::OnROSlider(wxScrollEvent& event)
// // Callback for the reslice opacity slider
// {
//     reslice_alpha = event.GetPosition();
//     construct_data_slice_image();
//     construct_xz_reslice();
//     construct_yz_reslice();
//     tl_canvas->slice_opacity = reslice_alpha/10.0f;
//     tl_canvas->drawFrame();
// }

//void VulkanWindow::OnSlice(wxCommandEvent& event)
// // Callback for the slice/reslice checkbox
// {
//     show_reslices = event.IsChecked();
//     tl_canvas->show_slices = show_reslices;
//     tl_canvas->drawFrame();
// }
