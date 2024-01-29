#pragma once
#include <wx/wx.h>

class App : public wxApp {
public:
    App();
    virtual ~App();
    virtual bool OnInit() override;
    virtual void OnIdle(wxIdleEvent &event);
    virtual void OnXLaunchIdle(wxIdleEvent &event);
};

