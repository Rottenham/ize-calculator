#pragma once

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/thread.h>
#include "memory.h"

class MyThread;
class cMain : public wxFrame {
public:
	cMain();

public:
	// 算血
	wxGrid* a_grid = nullptr;
	wxMenu* a_menuSettings = nullptr;
	wxMenu* a_menuView = nullptr;
	wxMenu* a_menuHelp = nullptr;
	wxMenu* a_menuOthers = nullptr;
	wxStaticText* a_statictext = nullptr;
	IZE::Memory mem;
	bool stop = false;
	wxColour bgColour = wxColour(240, 240, 240);
	wxFont cellFont = wxWindow::GetFont();
	wxFont cellFontBold;
	wxFont cellFont2 = wxWindow::GetFont();
	wxFont cellFont2Bold;
	wxFont cellFont3 = wxWindow::GetFont();
	MyThread* m_pThread;
	wxCriticalSection m_pThreadCS;
	void OnHotKey1(wxKeyEvent& evt);
	void OnExit(wxCommandEvent& evt);
	void OnAbout(wxCommandEvent& evt);
	void OnDoc(wxCommandEvent& evt);
	void OnSpeed(wxCommandEvent& evt);
	void OnAuto(wxCommandEvent& evt);
	void OnSelect(wxGridRangeSelectEvent& evt);
	void OnCellLeftClick(wxGridEvent& evt);
	void SetValues();
	void OnThreadUpdate(wxThreadEvent& evt);
	void OnClose(wxCloseEvent& evt);
	void OnStartThread();
	void StopThread();
	void resetStyle();
	void OnChecker(wxCommandEvent& evt);
	void OnRate(wxCommandEvent& evt);
	void OnScoreMaidless(wxCommandEvent& evt);
	void OnScoreMaid(wxCommandEvent& evt);
	void OnScoreCalculator(wxCommandEvent& evt);
	int temp_counter = 0;

	// 珍珑检查器
	wxFrame* b_frame = nullptr;
	wxChoice* b_choice = nullptr;
	wxMenu* b_menuSettings = nullptr;
	void OnCheckerButtonClicked(wxCommandEvent& evt);

	// 过率对比
	wxFrame* c_frame = nullptr;
	wxTextCtrl* c_input1 = nullptr;
	wxTextCtrl* c_input2 = nullptr;
	wxTextCtrl* c_input3 = nullptr;
	wxStaticText* c_output = nullptr;
	void OnRateButtonClicked(wxCommandEvent& evt);

	// 花数计算
	wxFrame* d_frame = nullptr;
	wxTextCtrl* d_input[9];
	wxStaticText* d_output = nullptr;
	wxRadioButton* d_rbutton1;
	wxRadioButton* d_rbutton2;
	void OnScoreCalcButtonClicked(wxCommandEvent& evt);
	void OnScoreClearButtonClicked(wxCommandEvent& evt);
	void OnMaidButton(wxCommandEvent& evt);
	void OnMaidlessButton(wxCommandEvent& evt);

	// 快捷布阵
	wxFrame* e_frame = nullptr;
	wxTextCtrl* e_input = nullptr;
	wxMenu* e_menuHelp = nullptr;
	void OnEmbattle(wxCommandEvent& evt);
	void OnEmbattleButtonClicked(wxCommandEvent& evt);
	void OnEmbattleDoc(wxCommandEvent& evt);
	void OnEmbattleOnlyButtonClicked(wxCommandEvent& evt);
	void OnReadPuzzleButtonClicked(wxCommandEvent& evt);
	void OnEmbattleClearButtonClicked(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();

protected:
	int test = 0;
};