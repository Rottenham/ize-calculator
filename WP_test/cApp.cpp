#include "cApp.h"
#include "resource.h"
wxIMPLEMENT_APP(cApp);
bool cApp::OnInit()
{
	m_checker = new wxSingleInstanceChecker;
	if (m_checker->IsAnotherRunning()) {
		delete m_checker;
		m_checker = NULL;
		return false;
	}
	m_frame1 = new cMain();
	m_frame1->SetIcon(wxICON(sample));
	m_frame1->Show();
	return true;
}

int cApp::OnExit() {
	delete m_checker;
	return 0;
}
