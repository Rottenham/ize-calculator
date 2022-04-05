#pragma once

#include <wx/wx.h>
#include "cMain.h"

class cThread : public wxThread
{
public:
    cThread(cMain* handler)
        : wxThread(wxTHREAD_DETACHED)
    {
        m_pHandler = handler
    }
    ~cThread();

protected:
    virtual ExitCode Entry();
    cMain* m_pHandler;
};

