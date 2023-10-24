#include <sstream>
#include <iomanip>
#include <wx/clipbrd.h>
#include "cMain.h"
#include "windows.h"
#include "puzzleSolver.h"

#define HOTKEY_ID_1 0x3000

using namespace std;

class MyThread : public wxThread {
public:
	MyThread(cMain* handler)
		: wxThread(wxTHREAD_DETACHED) {
		m_pHandler = handler;
	}
	bool stop = false;

	~MyThread();

protected:
	virtual wxThread::ExitCode Entry();
	cMain* m_pHandler;
};

enum {
	ID_DOC = 2,
	ID_BGRUN,
	ID_AUTOCLT,
	ID_GAMESPEED,
	ID_SCORE_MAIDLESS,
	ID_SCORE_MAID,
	ID_EXPECT,
	ID_CHECKER,
	ID_THEME,
	ID_CHECK_BUTTON,
	ID_CHANGESUN,
	ID_CHANGELEVEL,
	ID_RESETPUFFSHROOM,
	ID_RATE,
	ID_RATE_BUTTON,
	ID_RATE_INPUT1,
	ID_RATE_INPUT2,
	ID_RATE_INPUT3,
	ID_SCORECALC,
	ID_SCORECALC_RBUTTON1,
	ID_SCORECALC_RBUTTON2,
	ID_SCORECALC_BUTTON1,
	ID_SCORECALC_BUTTON2,
	ID_SCORECALC_INPUT1,
	ID_SCORECALC_INPUT2,
	ID_SCORECALC_INPUT3,
	ID_SCORECALC_INPUT4,
	ID_SCORECALC_INPUT5,
	ID_SCORECALC_INPUT6,
	ID_SCORECALC_INPUT7,
	ID_SCORECALC_INPUT8,
	ID_SCORECALC_INPUT9,
	ID_EMBATTLE,
	ID_EMBATTLE_BUTTON1,
	ID_EMBATTLE_BUTTON2,
	ID_EMBATTLE_BUTTON3,
	ID_EMBATTLE_BUTTON4,
	ID_EMBATTLE_INDEX,
	ID_EMBATTLE_DOC,
	MYTHREAD_UPDATE = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE(cMain, wxFrame)
EVT_BUTTON(ID_CHECK_BUTTON, OnCheckerButtonClicked)
EVT_BUTTON(ID_RATE_BUTTON, OnRateButtonClicked)
EVT_BUTTON(ID_SCORECALC_BUTTON1, OnScoreCalcButtonClicked)
EVT_BUTTON(ID_SCORECALC_BUTTON2, OnScoreClearButtonClicked)
EVT_BUTTON(ID_EMBATTLE_BUTTON1, OnEmbattleButtonClicked)
EVT_BUTTON(ID_EMBATTLE_BUTTON2, OnEmbattleOnlyButtonClicked)
EVT_BUTTON(ID_EMBATTLE_BUTTON3, OnReadPuzzleButtonClicked)
EVT_BUTTON(ID_EMBATTLE_BUTTON4, OnEmbattleClearButtonClicked)
EVT_RADIOBUTTON(ID_SCORECALC_RBUTTON1, OnMaidlessButton)
EVT_RADIOBUTTON(ID_SCORECALC_RBUTTON2, OnMaidButton)
EVT_MENU(wxID_ABOUT, OnAbout)
EVT_MENU(ID_DOC, OnDoc)
EVT_MENU(ID_GAMESPEED, OnSpeed)
EVT_MENU(ID_AUTOCLT, OnAuto)
EVT_MENU(ID_CHECKER, OnChecker)
EVT_MENU(ID_RATE, OnRate)
EVT_MENU(ID_SCORE_MAID, OnScoreMaid)
EVT_MENU(ID_SCORE_MAIDLESS, OnScoreMaidless)
EVT_MENU(ID_SCORECALC, OnScoreCalculator)
EVT_MENU(ID_EMBATTLE, OnEmbattle)
EVT_MENU(ID_EMBATTLE_DOC, OnEmbattleDoc)
EVT_MENU(ID_EMBATTLE_INDEX, OnEmbattleIndex)
EVT_GRID_RANGE_SELECT(OnSelect)
EVT_GRID_CELL_LEFT_CLICK(OnCellLeftClick)
EVT_CLOSE(OnClose)
EVT_HOTKEY(HOTKEY_ID_1, OnHotKey1)
EVT_THREAD(MYTHREAD_UPDATE, OnThreadUpdate)
wxEND_EVENT_TABLE()

string curr_version = "1.5.9";

cMain::cMain() : wxFrame(nullptr, wxID_ANY, "IZE血量计算器 v" + curr_version, wxDefaultPosition, wxSize(348, 450), (wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) | wxWANTS_CHARS) {
	this->RegisterHotKey(HOTKEY_ID_1, (wxMOD_CONTROL), 0x44);
	cellFont.SetPointSize(10);
	cellFontBold = cellFont.Bold();
	cellFont2.SetPointSize(9);
	cellFont2Bold = cellFont2.Bold();
	cellFont3.SetPointSize(11);
	a_menuSettings = new wxMenu;
	a_menuSettings->AppendCheckItem(ID_BGRUN, "后台运行", "开启/关闭游戏后台运行");
	a_menuSettings->AppendCheckItem(ID_AUTOCLT, "自动收集", "开启/关闭自动收集");
	a_menuSettings->AppendCheckItem(ID_GAMESPEED, "游戏倍速\tCtrl+D", "按Ctrl+D切换一倍速/五倍速");
	a_menuSettings->Check(ID_GAMESPEED, true);
	a_menuSettings->Check(ID_BGRUN, true);
	a_menuView = new wxMenu;
	a_menuView->AppendCheckItem(ID_SCORE_MAIDLESS, "显示花数（无女仆）", "开启/关闭新标准算分（无女仆）");
	a_menuView->AppendCheckItem(ID_SCORE_MAID, "显示花数（有女仆）", "开启/关闭新标准算分（有女仆）");
	a_menuHelp = new wxMenu;
	a_menuHelp->Append(ID_DOC, "使用说明", " ");
	a_menuHelp->Append(wxID_ABOUT, "关于...", " ");
	a_menuOthers = new wxMenu;
	a_menuOthers->Append(ID_CHECKER, "珍珑检查器", "检查植物数量是否正确");
	a_menuOthers->Append(ID_RATE, "过率对比", "比较不同解法的期望花费");
	a_menuOthers->Append(ID_SCORECALC, "花数计算", "计算新标准算分");
	a_menuOthers->Append(ID_EMBATTLE, "快捷布阵", "快速布置阵型");
	wxMenuBar* a_menuBar = new wxMenuBar;
	a_menuBar->Append(a_menuSettings, "设置");
	a_menuBar->Append(a_menuView, "查看");
	a_menuBar->Append(a_menuHelp, "帮助");
	a_menuBar->Append(a_menuOthers, "其他");
	SetMenuBar(a_menuBar);
	CreateStatusBar(1, wxSB_FLAT);
	wxPanel* a_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(343, 450));
	a_panel->SetBackgroundColour(bgColour);
	a_grid = new wxGrid(a_panel, 10002, wxPoint(3, 60), wxSize(324, 350));
	a_grid->CreateGrid(7, 6);
	a_grid->SetDefaultRowSize(48);
	a_grid->SetDefaultColSize(48);
	a_grid->SetColSize(5, 66);
	a_grid->SetColSize(3, 66);
	a_grid->HideColLabels();
	a_grid->HideRowLabels();
	wxString colLabs[5] = { "撑杆", "慢速", "梯子", "橄榄", "撑杆梯子" };
	wxString rowLabs[5] = { "第一行", "第二行", "第三行", "第四行", "第五行" };
	for (int i = 1; i < 6; i++) {
		a_grid->SetCellValue(0, i, colLabs[i - 1]);
		a_grid->SetCellValue(i, 0, rowLabs[i - 1]);
		a_grid->SetCellAlignment(0, i, wxALIGN_CENTRE, wxALIGN_CENTRE);
		a_grid->SetCellAlignment(i, 0, wxALIGN_CENTRE, wxALIGN_CENTRE);
		a_grid->SetCellFont(0, i, cellFontBold);
		a_grid->SetCellFont(i, 0, cellFont);
	}
	for (int i = 0; i < 6; i++) {
		a_grid->DisableRowResize(i);
		a_grid->DisableColResize(i);
	}
	for (int i = 0; i < 7; i++)
		for (int j = 0; j < 6; j++) {
			a_grid->SetCellBackgroundColour(i, j, bgColour);
		}
	a_grid->EnableEditing(false);
	a_grid->EnableGridLines(false);
	a_grid->SetCellHighlightPenWidth(0);
	a_grid->SetCellHighlightROPenWidth(0);
	a_grid->ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
	for (int i = 1; i < 6; i++)
		for (int j = 1; j < 6; j++) {
			a_grid->SetCellAlignment(i, j, wxALIGN_CENTRE, wxALIGN_CENTRE);
			a_grid->SetCellFont(i, j, cellFont);
		}
	mem = IZE::Memory();
	a_statictext = new wxStaticText(a_panel, wxID_ANY, mem.getBrief(), wxPoint(0, 26), wxSize(348, 30), wxALIGN_CENTER_HORIZONTAL);
	a_statictext->SetFont(cellFont);
	OnStartThread();
}

void cMain::OnHotKey1(wxKeyEvent& event) {
	mem.toggleFrameDuration();
}

void cMain::OnExit(wxCommandEvent& evt) {
	m_pThread->stop = true;
	Close(true);
}

void cMain::OnAbout(wxCommandEvent& evt) {
	wxMessageBox("这是一个用于计算IZE单破血量的计算器。\n\n版本号: v" + curr_version + "\n\n开发者: Crescendo\nbilibili: Crescebdo\n贴吧: Crescendo\n\n使用工具: Visual Studio 2019, wxWidgets 3.1.4\n\n源码以 3-Clause BSD 许可证公开: https://github.com/Rottenham/izc",
		"关于IZE计算器", wxOK);
}

void cMain::OnDoc(wxCommandEvent& evt) {
	string r = R"(本计算器适用于IZE单破血量的计算，支持英文原版与汉化第二版。

模拟算血时，不会考虑其他路的杨桃、三线或磁铁（倾斜主题除外）。同时，默认土豆已引爆、窝瓜已引走、大嘴已填饱。可以计算多玉米算血。

显示结果时，带有括号的代表【不推荐】，加粗并高亮的代表【推荐】。

撑杆算血：计算撑杆跳过第一个非地刺、非窝瓜植物后所受伤害。
【推荐】的情况：算血不超过15。
【不推荐】的情况：被跳过的植物是裂荚、杨桃或向日葵，或算血超过39。

慢速算血：计算路障或铁桶所受伤害。
【推荐】的情况：算血不超过25，或算血不超过61且当前行无磁铁。
【不推荐】的情况：算血超过72。

梯子算血：计算梯子所受伤害（饰品+本体）。饰品掉落后，所有伤害转移至本体上。
【推荐】的情况：本体算血不超过14，且当前行无磁铁。
【不推荐】的情况：当前行有磁铁，或本体算血超过19。

橄榄算血：计算橄榄所受伤害。
【推荐】的情况：算血不超过76且当前行无磁铁。
【不推荐】的情况：当前行有磁铁，或算血超过84。

撑杆梯子算血：计算先放撑杆，落地后放梯子时梯子所受的伤害。
推荐/不推荐标准同梯子。

注：
算血公式见cv6263782。此程序的计算结果完全基于分段算血公式，不等同于实际游戏情况；其计算结果均可用计算器手动验算。)";
	wxMessageBox(r, "使用说明", wxOK);
}

void cMain::OnSpeed(wxCommandEvent& evt) {
	mem.resetFrameDuration();
	if (!a_menuSettings->IsChecked(ID_GAMESPEED)) {
		this->UnregisterHotKey(HOTKEY_ID_1);
	}
	else {
		this->RegisterHotKey(HOTKEY_ID_1, (wxMOD_CONTROL), 0x44);
	}
}

void cMain::OnAuto(wxCommandEvent& evt) {
	if (!a_menuSettings->IsChecked(ID_AUTOCLT)) {
		mem.setAuto(false);
	}
	else {
		mem.setAuto(true);
	}
}

void cMain::OnScoreMaidless(wxCommandEvent& evt) {
	if (a_menuView->IsChecked(ID_SCORE_MAIDLESS)) {
		a_menuView->Check(ID_SCORE_MAID, false);
	}
}

void cMain::OnScoreMaid(wxCommandEvent& evt) {
	if (a_menuView->IsChecked(ID_SCORE_MAID)) {
		a_menuView->Check(ID_SCORE_MAIDLESS, false);
	}
}

void cMain::OnChecker(wxCommandEvent& evt) {
	wxWindow* wnd = FindWindowByName("珍珑检查器", this);
	if (wnd) {
		b_frame->Raise();
		return;
	}
	b_frame = new wxFrame(this, wxID_ANY, "珍珑检查器", this->GetPosition(), wxSize(335, 160), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX));
	b_frame->SetIcon(wxICON(sample2));
	b_frame->Show();
	wxPanel* b_panel = new wxPanel(b_frame, wxID_ANY, wxDefaultPosition, wxSize(335, 160));
	b_panel->SetBackgroundColour(bgColour);
	wxString choices[] = { "综合", "控制", "即死", "输出", "爆炸", "倾斜", "穿刺", "胆小" };
	b_choice = new wxChoice(b_panel, ID_THEME, wxPoint(33, 45), wxSize(80, 23), 8, choices);
	b_choice->SetFont(cellFont2);
	b_choice->SetSelection(0);
	wxStaticText* b_text = new wxStaticText(b_panel, wxID_ANY, "请选择主题：", wxPoint(33, 17), wxSize(100, 23));
	wxButton* b_button = new wxButton(b_panel, ID_CHECK_BUTTON, "一键合规", wxPoint(193, 30), wxSize(70, 32));
	b_button->SetFont(cellFont2Bold);
	b_menuSettings = new wxMenu;
	b_menuSettings->AppendCheckItem(ID_CHANGESUN, "修改阳光");
	b_menuSettings->AppendCheckItem(ID_CHANGELEVEL, "修改关数");
	b_menuSettings->AppendCheckItem(ID_RESETPUFFSHROOM, "小喷归位");
	b_menuSettings->Check(ID_CHANGESUN, true);
	b_menuSettings->Check(ID_CHANGELEVEL, true);
	b_menuSettings->Check(ID_RESETPUFFSHROOM, true);
	wxMenuBar* b_menuBar = new wxMenuBar;
	b_menuBar->Append(b_menuSettings, "设置");
	b_frame->SetMenuBar(b_menuBar);
}

void cMain::OnCheckerButtonClicked(wxCommandEvent& evt) {
	wxString result = mem.checkPlants(b_choice->GetCurrentSelection(), b_menuSettings->IsChecked(ID_CHANGESUN), b_menuSettings->IsChecked(ID_CHANGELEVEL), b_menuSettings->IsChecked(ID_RESETPUFFSHROOM));
	wxMessageBox(result, "检查结果");
}

void cMain::OnRate(wxCommandEvent& evt) {
	wxWindow* wnd = FindWindowByName("过率对比", this);
	if (wnd) {
		c_frame->Raise();
		return;
	}
	c_frame = new wxFrame(this, wxID_ANY, "过率对比", this->GetPosition(), wxSize(420, 222), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX));
	c_frame->SetIcon(wxICON(sample3));
	c_frame->Show();
	wxPanel* c_panel = new wxPanel(c_frame, wxID_ANY, wxDefaultPosition, wxSize(420, 222));
	c_panel->SetBackgroundColour(bgColour);
	wxStaticText* c_text1 = new wxStaticText(c_panel, wxID_ANY, "RP解价格", wxPoint(25, 10), wxSize(50, 20));
	c_text1->SetFont(cellFont);
	c_input1 = new wxTextCtrl(c_panel, ID_RATE_INPUT1, "", wxPoint(17, 40), wxSize(77, 25), wxTE_CENTRE);
	c_input1->SetFont(cellFont);
	wxStaticText* c_text2 = new wxStaticText(c_panel, wxID_ANY, "补刀花费", wxPoint(25 + 100 + 4, 10), wxSize(45, 20));
	c_text2->SetFont(cellFont);
	c_input2 = new wxTextCtrl(c_panel, ID_RATE_INPUT2, "", wxPoint(17 + 100, 40), wxSize(77, 25), wxTE_CENTRE);
	c_input2->SetFont(cellFont);
	wxStaticText* c_text3 = new wxStaticText(c_panel, wxID_ANY, "稳解价格", wxPoint(25 + 200 + 4, 10), wxSize(45, 20));
	c_text3->SetFont(cellFont);
	c_input3 = new wxTextCtrl(c_panel, ID_RATE_INPUT3, "", wxPoint(17 + 200, 40), wxSize(77, 25), wxTE_CENTRE);
	c_input3->SetFont(cellFont);
	wxButton* c_button = new wxButton(c_panel, ID_RATE_BUTTON, "计算", wxPoint(25 + 290 + 4, 22), wxSize(65, 25));
	c_output = new wxStaticText(c_panel, wxID_ANY, "过率合格线：", wxPoint(0, 107), wxSize(420, 30), wxALIGN_CENTER_HORIZONTAL);
	c_output->SetFont(cellFontBold);
}

void cMain::OnRateButtonClicked(wxCommandEvent& evt) {
	string str1 = c_input1->GetValue().ToStdString();
	string str2 = c_input2->GetValue().ToStdString();
	string str3 = c_input3->GetValue().ToStdString();
	try {
		double price1 = stod(str1);
		double price2 = stod(str2);
		double price3 = stod(str3);
		if (price1 <= 0 || price2 <= 0 || price3 <= 0 || price1 >= price3) {
			throw std::invalid_argument("输入格式错误");
		}
		double result = 100 * (price1 + price2 - price3) / price2;
		stringstream stream;
		stream << std::fixed << std::setprecision(1) << result;
		string rate = "过率合格线：" + stream.str() + "%";
		c_output->SetLabel(rate);
		c_output->Refresh();
	}
	catch (const std::invalid_argument & ia) {
		c_output->SetLabel("输入格式错误");
		c_output->Refresh();
		return;
	}
}

void cMain::OnScoreCalculator(wxCommandEvent& evt) {
	wxWindow* wnd = FindWindowByName("花数计算", this);
	if (wnd) {
		d_frame->Raise();
		return;
	}
	d_frame = new wxFrame(this, wxID_ANY, "花数计算", this->GetPosition(), wxSize(440, 355), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX));
	d_frame->SetIcon(wxICON(sample4));
	d_frame->Show();
	wxPanel* d_panel = new wxPanel(d_frame, wxID_ANY, wxDefaultPosition, wxSize(440, 355));
	d_panel->SetBackgroundColour(bgColour);
	wxStaticText* d_text1 = new wxStaticText(d_panel, wxID_ANY, "完成关数", wxPoint(30, 13), wxSize(50, 20));
	d_text1->SetFont(cellFont2);
	d_input[0] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT1, "", wxPoint(17, 40), wxSize(77, 24), wxTE_CENTRE);
	d_input[0]->SetFont(cellFont2);
	wxStaticText* d_text2 = new wxStaticText(d_panel, wxID_ANY, "剩余阳光", wxPoint(30 + 101, 13), wxSize(50, 20));
	d_text2->SetFont(cellFont2);
	d_input[1] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT2, "", wxPoint(17 + 100, 40), wxSize(77, 23), wxTE_CENTRE);
	d_input[1]->SetFont(cellFont2);
	wxStaticText* d_text3 = new wxStaticText(d_panel, wxID_ANY, "开局阳光", wxPoint(30 + 202, 13), wxSize(50, 20));
	d_text3->SetFont(cellFont2);
	d_input[2] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT3, "", wxPoint(17 + 200, 40), wxSize(77, 23), wxTE_CENTRE);
	d_input[2]->SetFont(cellFont2);
	wxStaticText* d_text4 = new wxStaticText(d_panel, wxID_ANY, "胆小关数", wxPoint(30 + 303 + 6, 13), wxSize(50, 20));
	d_text4->SetFont(cellFont2);
	d_input[3] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT4, "", wxPoint(17 + 305, 40), wxSize(77, 23), wxTE_CENTRE);
	d_input[3]->SetFont(cellFont2);
	wxStaticText* d_text5 = new wxStaticText(d_panel, wxID_ANY, "倾斜关数", wxPoint(30, 13 + 75), wxSize(50, 20));
	d_text5->SetFont(cellFont2);
	d_input[4] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT5, "", wxPoint(17, 40 + 75), wxSize(77, 23), wxTE_CENTRE);
	d_input[4]->SetFont(cellFont2);
	wxStaticText* d_text6 = new wxStaticText(d_panel, wxID_ANY, "穿刺关数", wxPoint(30 + 101, 13 + 75), wxSize(50, 20));
	d_text6->SetFont(cellFont2);
	d_input[5] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT6, "", wxPoint(17 + 100, 40 + 75), wxSize(77, 23), wxTE_CENTRE);
	d_input[5]->SetFont(cellFont2);
	wxStaticText* d_text7 = new wxStaticText(d_panel, wxID_ANY, "输出关数", wxPoint(30 + 202, 13 + 75), wxSize(50, 20));
	d_text7->SetFont(cellFont2);
	d_input[6] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT7, "", wxPoint(17 + 200, 40 + 75), wxSize(77, 23), wxTE_CENTRE);
	d_input[6]->SetFont(cellFont2);
	wxStaticText* d_text8 = new wxStaticText(d_panel, wxID_ANY, "11关起倾斜\n与胆小关数", wxPoint(30 + 302, 13 + 65), wxSize(120, 31));
	d_text8->SetFont(cellFont2);
	d_input[7] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT8, "", wxPoint(17 + 300, 40 + 75), wxSize(87, 23), wxTE_CENTRE);
	d_input[7]->SetFont(cellFont2);
	d_input[7]->Enable(false);
	wxStaticText* d_text9 = new wxStaticText(d_panel, wxID_ANY, "总花数", wxPoint(38, 13 + 150), wxSize(50, 20));
	d_text9->SetFont(cellFont2);
	d_input[8] = new wxTextCtrl(d_panel, ID_SCORECALC_INPUT9, "", wxPoint(17, 40 + 150), wxSize(77, 23), wxTE_CENTRE);
	d_input[8]->SetFont(cellFont2);
	d_rbutton1 = new wxRadioButton(d_panel, ID_SCORECALC_RBUTTON1, "无女仆", wxPoint(17 + 110, 40 + 125), wxSize(67, 23), wxRB_GROUP);
	d_rbutton2 = new wxRadioButton(d_panel, ID_SCORECALC_RBUTTON2, "有女仆", wxPoint(17 + 110, 40 + 152), wxSize(67, 23));
	d_rbutton1->SetValue(true);
	wxButton* d_button1 = new wxButton(d_panel, ID_SCORECALC_BUTTON1, "计算", wxPoint(30 + 298, 40 + 140), wxSize(70, 25));
	wxButton* d_button2 = new wxButton(d_panel, ID_SCORECALC_BUTTON2, "清空", wxPoint(30 + 210, 40 + 140), wxSize(70, 25));
	d_output = new wxStaticText(d_panel, wxID_ANY, "花数：", wxPoint(0, 255), wxSize(440, 30), wxALIGN_CENTER_HORIZONTAL);
	d_output->SetFont(cellFontBold);
}

void cMain::OnScoreCalcButtonClicked(wxCommandEvent& evt) {
	bool maid = d_rbutton2->GetValue();
	string str[9];
	for (int i = 0; i < 9; i++) {
		str[i] = d_input[i]->GetValue().ToStdString();
	}
	try {
		int param[9];
		for (int i = 0; i < 9; i++) {
			if (!maid && i == 7) {
				continue;
			}
			param[i] = stoi(str[i]);
		}
		if (param[0] <= 0 || param[1] < 0 || param[2] < 0 || param[2] > 1400) {
			throw std::invalid_argument("输入格式错误");
		}
		int sum = 0;
		for (int i = 3; i < 7; i++) {
			sum += param[i];
			if (param[i] < 0) {
				throw std::invalid_argument("输入格式错误");
			}
		}
		if (sum > param[0] - 1) {
			throw std::invalid_argument("输入格式错误");
		}
		if (maid && param[0] > 10 && (param[7] < 0 || param[7] > param[0] - 10)) {
			throw std::invalid_argument("输入格式错误");
		}
		if (param[8] < 8) {
			throw std::invalid_argument("输入格式错误");
		}
		double result = param[8] - (1400.0 - param[2]) / 200.0 + 1.75 * param[3] + 0.75 * param[4] + 0.25 * param[5] - 0.5 * param[6] - param[1] / 200.0
			+ 3.75 * (20.0 - param[0]);
		if (maid && param[0] > 10) {
			result += 0.125 * (param[0] - 10.0 - param[7]);
		}
		string str = to_string(result);
		if (str.find('.') != string::npos) {
			str = str.substr(0, str.find_last_not_of('0') + 1);
			if (str.find('.') == str.size() - 1) {
				str = str.substr(0, str.size() - 1);
			}
		}
		str = "花数：" + str + "/20";
		d_output->SetLabel(str);
		d_output->Refresh();
	}
	catch (const std::invalid_argument & ia) {
		d_output->SetLabel("输入格式错误");
		d_output->Refresh();
		return;
	}
}

void cMain::OnScoreClearButtonClicked(wxCommandEvent& evt) {
	for (int i = 0; i < 9; i++) {
		d_input[i]->Clear();
	}
}

void cMain::OnMaidlessButton(wxCommandEvent& evt) {
	d_input[7]->Enable(false);
	d_input[7]->Clear();
}

void cMain::OnMaidButton(wxCommandEvent& evt) {
	d_input[7]->Enable(true);
}

void cMain::OnEmbattle(wxCommandEvent& evt) {
	wxWindow* wnd = FindWindowByName("快捷布阵", this);
	if (wnd) {
		e_frame->Raise();
		return;
	}
	e_frame = new wxFrame(this, wxID_ANY, "快捷布阵", this->GetPosition(), wxSize(330, 260), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX));
	e_frame->SetIcon(wxICON(sample5));
	e_frame->Show();
	wxPanel* e_panel = new wxPanel(e_frame, wxID_ANY, wxDefaultPosition, wxSize(330, 260));
	wxStaticText* e_text1 = new wxStaticText(e_panel, wxID_ANY, "输入布阵代码：", wxPoint(10, 13), wxSize(50, 20));
	e_text1->SetFont(cellFont);
	e_input = new wxTextCtrl(e_panel, wxID_ANY, "", wxPoint(10, 43), wxSize(180, 120), wxTE_MULTILINE);
	wxFont monospace_font(wxFontInfo(13).FaceName("Consolas"));
	e_input->SetFont(monospace_font);
	wxButton* e_button1 = new wxButton(e_panel, ID_EMBATTLE_BUTTON1, "布阵并合规", wxPoint(210, 13), wxSize(90, 27));
	e_button1->SetFont(cellFont);
	wxButton* e_button2 = new wxButton(e_panel, ID_EMBATTLE_BUTTON2, "仅布阵", wxPoint(210, 53), wxSize(90, 27));
	e_button2->SetFont(cellFont);
	wxButton* e_button3 = new wxButton(e_panel, ID_EMBATTLE_BUTTON3, "读取当前阵型", wxPoint(210, 93), wxSize(90, 27));
	e_button3->SetFont(cellFont);
	wxButton* e_button4 = new wxButton(e_panel, ID_EMBATTLE_BUTTON4, "清空", wxPoint(210, 133), wxSize(90, 27));
	e_button4->SetFont(cellFont);
	e_frame->CreateStatusBar(1, wxSB_FLAT);
	e_menuSettings = new wxMenu;
	e_menuSettings->AppendCheckItem(ID_EMBATTLE_INDEX, "导出编号", "是否导出植物编号（也称“栈位”）大小关系");
	e_menuSettings->Check(ID_EMBATTLE_INDEX, exportIndexChecked);
	e_menuHelp = new wxMenu;
	e_menuHelp->Append(ID_EMBATTLE_DOC, "使用说明");
	wxMenuBar* e_menuBar = new wxMenuBar;
	e_menuBar->Append(e_menuSettings, "设置");
	e_menuBar->Append(e_menuHelp, "帮助");
	e_frame->SetMenuBar(e_menuBar);
}

void cMain::OnEmbattleButtonClicked(wxCommandEvent& evt) {
	wxString result = mem.embattleFromCode(e_input->GetValue(), true);
	if (result == "成功") {
		e_frame->SetStatusText(wxT("布阵完毕"), 0);
	}
	else {
		e_frame->SetStatusText(result, 0);
	}
}

void cMain::OnEmbattleOnlyButtonClicked(wxCommandEvent& evt) {
	wxString result = mem.embattleFromCode(e_input->GetValue(), false);
	if (result == "成功") {
		e_frame->SetStatusText(wxT("布阵完毕"), 0);
	}
	else {
		e_frame->SetStatusText(result, 0);
	}
}

void cMain::OnReadPuzzleButtonClicked(wxCommandEvent& evt) {
	wxString result = mem.readPlantsToCode(e_menuSettings->IsChecked(ID_EMBATTLE_INDEX));
	int match = result.rfind("成功：", 0);
	if (match != 0) {
		e_frame->SetStatusText(result, 0);
	}
	else {
		result = result.substr(result.find("：") + 1);
		e_input->Clear();
		(*e_input) << result;
		if (wxTheClipboard->Open()) {
			wxTheClipboard->SetData(new wxTextDataObject(result));
			wxTheClipboard->Close();
			e_frame->SetStatusText(wxT("已复制到剪贴板"), 0);
		}
		else {
			e_frame->SetStatusText(wxT("读取成功，但未能复制到剪贴板"), 0);
		}
	}
}

void cMain::OnEmbattleClearButtonClicked(wxCommandEvent& evt) {
	e_input->Clear();
	e_frame->SetStatusText(wxT(""), 0);
}

void cMain::OnEmbattleDoc(wxCommandEvent& evt) {
	string r = R"(植物编码规则如下：
小喷 - p；向日葵 - h；冰豆 - b；裂荚 - l；单发 - 1；双发 - 2；三线 - 3；杨桃 - 5；磁铁 - c；大喷 - d；土豆 - t；窝瓜 - w；大嘴 - z；地刺 - _；玉米 - y；火炬 - j；伞叶 - s；坚果 - o；胆小 - x。

其余任意字符均代表空格。

行与行之间需换行，亦可在行末加上*号代表重复此行。

例如：【2b22l*5】会布出五行均为双冰双双裂的阵图。

可在某植物后添加"+"号（可添加多个），代表延迟一轮（或多轮）种植。每轮种植的植物的编号（也称作"栈位"）严格高于前一轮。

例如：【wt+】表示窝瓜于第一轮种植，土豆于第二轮种植；土豆编号必定高于窝瓜。

每行超出5列，以及超出5行的部分不会被读入，也不会报错。)";
	wxMessageBox(r, "使用说明");
}

void cMain::OnEmbattleIndex(wxCommandEvent& evt) {
	exportIndexChecked = e_menuSettings->IsChecked(ID_EMBATTLE_INDEX);
}

// 禁止选择
void cMain::OnSelect(wxGridRangeSelectEvent& evt) {
	wxGridCellCoordsArray selTop = a_grid->GetSelectionBlockTopLeft();
	wxGridCellCoordsArray selBottom = a_grid->GetSelectionBlockBottomRight();
	for (size_t i = 0; i < selTop.Count(); i++) {
		a_grid->ClearSelection();
	}
	evt.Skip();
}

// 禁止选择
void cMain::OnCellLeftClick(wxGridEvent& evt) {
	a_grid->SelectRow(evt.GetRow());
	evt.Skip();
}

void cMain::resetStyle() {
	a_statictext->SetLabel("当前关数：");
	a_statictext->Refresh();
	for (int i = 1; i < 6; i++)
		for (int j = 1; j < 6; j++) {
			a_grid->SetCellValue(i, j, "");
			a_grid->SetCellBackgroundColour(i, j, bgColour);
			a_grid->SetCellFont(i, j, cellFont);
			a_grid->SetCellTextColour(i, j, *wxBLACK);
		}
	string colLabs[5] = { "撑杆", "慢速", "梯子", "橄榄", "撑杆梯子" };
	for (int i = 1; i < 6; i++) {
		a_grid->SetCellValue(0, i, colLabs[i - 1]);
	}
}

// 更新血量
void cMain::SetValues() {
	int scoreMode = 0;
	if (a_menuView->IsChecked(ID_SCORE_MAIDLESS)) {
		scoreMode = -1;
	}
	else if (a_menuView->IsChecked(ID_SCORE_MAID)) {
		scoreMode = 1;
	}
	int** plants = mem.readPlants(a_menuSettings->IsChecked(ID_BGRUN), false, scoreMode, a_menuSettings->IsChecked(ID_GAMESPEED));
	if (plants == nullptr) {
		SetStatusText(wxT("尚未进入IZE"), 0);
		resetStyle();
	}
	else {
		a_statictext->SetLabel(mem.getBrief());
		a_statictext->Refresh();
		if (a_menuSettings->IsChecked(ID_AUTOCLT)) {
			mem.setAuto(true);
		}
		SetStatusText(wxT(""), 0);
		Puzzle puzzle = Puzzle(plants, mem.getQX());
		string** result = puzzle.result;
		int** highlight = puzzle.highlight;
		if (mem.getQX()) {
			string colLabs[5] = { "", "慢速", "独立算血", "", "" };
			for (int i = 1; i < 6; i++) {
				a_grid->SetCellValue(0, i, colLabs[i - 1]);
			}
		}
		else {
			string colLabs[5] = { "撑杆", "慢速", "梯子", "橄榄", "撑杆梯子" };
			for (int i = 1; i < 6; i++) {
				a_grid->SetCellValue(0, i, colLabs[i - 1]);
			}
		}
		for (int i = 1; i < 6; i++)
			for (int j = 1; j < 6; j++) {
				string str = result[i - 1][j - 1];
				int hl = highlight[i - 1][j - 1];
				if (hl == -1) {
					a_grid->SetCellFont(i, j, cellFont);
					a_grid->SetCellTextColour(i, j, wxColour(150, 150, 150));
					a_grid->SetCellBackgroundColour(i, j, bgColour);
					str = "(" + str + ")";
				}
				else if (hl == 0) {
					a_grid->SetCellFont(i, j, cellFont);
					a_grid->SetCellTextColour(i, j, *wxBLACK);
					a_grid->SetCellBackgroundColour(i, j, bgColour);
				}
				else if (hl == 1) {
					a_grid->SetCellFont(i, j, cellFontBold);
					a_grid->SetCellTextColour(i, j, *wxBLACK);
					a_grid->SetCellBackgroundColour(i, j, wxColour(210, 210, 210));
				}
				a_grid->SetCellValue(i, j, wxString(str));
			}
		for (int i = 0; i < 5; i++) {
			delete[] plants[i];
		}
		delete[] plants;
	}
	a_grid->ForceRefresh();
}

void cMain::OnClose(wxCloseEvent&) {
	mem.resetFrameDuration();
	this->UnregisterHotKey(HOTKEY_ID_1);
	m_pThread->stop = true;
	StopThread();

	// now wait till thread is actually destroyed
	while (1) {
		{ // was the ~MyThread() function executed?
			wxCriticalSectionLocker enter(m_pThreadCS);
			if (!m_pThread) break;
		}

		// wait for thread completion
		wxThread::This()->Sleep(1);
	}

	Destroy();
}

void cMain::OnStartThread() {
	if (m_pThread != NULL) return;

	m_pThread = new MyThread(this);
	if (m_pThread->Run() != wxTHREAD_NO_ERROR) {
		wxLogError("Can't create the thread!");
		delete m_pThread;
		m_pThread = NULL;
	}
}

void cMain::OnThreadUpdate(wxThreadEvent& evt) {
	SetValues();
}

void cMain::StopThread() {
	wxCriticalSectionLocker enter(m_pThreadCS);
	if (m_pThread) // does the thread still exist?
	{
		if (m_pThread->Delete() != wxTHREAD_NO_ERROR)
			wxLogError("Can't delete the thread!");
	}
}

wxThread::ExitCode MyThread::Entry() {
	while (true) {
		if (stop) break;
		wxThread::This()->Sleep(500);		// sleep 500 ms
		if (stop) break;
		wxQueueEvent(m_pHandler, new wxThreadEvent(wxEVT_THREAD, MYTHREAD_UPDATE));
	}
	return (wxThread::ExitCode)0; // success
}

MyThread::~MyThread() {
	wxCriticalSectionLocker enter(m_pHandler->m_pThreadCS);
	m_pHandler->m_pThread = NULL;
}