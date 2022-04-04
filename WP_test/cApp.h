#pragma once

#include "wx/wx.h"
#include "cMain.h"
#include <wx/snglinst.h>

class cApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();

private:
	cMain* m_frame1 = nullptr;
	wxSingleInstanceChecker* m_checker = nullptr;
};

