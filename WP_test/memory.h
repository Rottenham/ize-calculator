#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <Windows.h>
#include <wx/string.h>
#include "code.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

namespace IZE {
	const int TOTAL_NUM = 19;
	class Memory : public Code {
	public:
		Memory();
		void toggleFrameDuration();
		bool setFrameDuration(int time_ms);
		void resetFrameDuration();
		void setAuto(bool autoCollect);
		std::string getBrief();
		bool getQX();
		int** readPlants(bool setRunInBackground, bool showExpect, int showScore, bool enableSpeed);
		wxString checkPlants(int theme, bool changeSun, bool changeLevel);
		wxString readPlantsToCode();
		wxString embattleFromCode(wxString plant_code, bool checkCompliance);
		void disableMaid();

	private:
		void resetValues(bool setRecord);
		void renewBrief();
		bool checkQX(int** plants);
		bool checkDX(int** plants);
		bool checkSC(int** plants);
		bool checkCC(int** plants);
		void updateScore(int** plants);
		void updateBrief();
		std::string getScore(bool maidOK);
		int countPlant(int** plants, int idx);
		bool checkPlant(int plant_type);
		wxString getName(int plant_type);
		int getIndex(int plant_type);
		wxString checkEach(int** plants, int* counter, int* goal, bool isDX, bool isBType);
		wxString getCode(int plant_type);
		wxString toCode(int** plants);
		wxString checkTheme(int** plants, int* counter, int theme);
		int** compilePlantCode(wxString input);
		int getPlantTypeByCode(wxString plant_code);
		void setSun(int xrkNum);
		void setLevel(int xrkNum);

		// 内存操作部分
		void asm_code_inject();
		void asm_iz_plant(int row, int col, int type);
		int GetFrameDuration();
		bool FindGame();
		bool FindGame2();
		bool IsValid();
		bool GameOn();
		void CloseGame();
		template <typename T>
		T ReadMemory(std::initializer_list<uintptr_t> addr);
		template <typename T>
		void WriteMemory(T value, std::initializer_list<uintptr_t> addr);

	private:
		HWND hwnd = nullptr;		// 窗口句柄
		DWORD pid = 0;				// 进程标识
		HANDLE handle = nullptr;	// 进程句柄
		int level = -1;				// 已完成关数
		int sun = -1;				// 阳光
		bool isQX = false;			// 当前关卡是否为倾斜
		int dxCount = 0;			// 遇到的胆小数
		int qxCount = 0;			// 遇到的倾斜数
		int scCount = 0;			// 遇到的输出数
		int ccCount = 0;			// 遇到的穿刺数
		int initialSun = -1;		// 开局阳光
		int xrkCount = 0;			// 总花数
		int maidLevelCount = 0;		// 10f后非倾斜、非胆小关数总数
		double score = -1;			// 新算分
		bool setButter = false;		// 是否锁定了黄油
		bool displayExpect = false;	// 冲纪录模式
		int scoreMode = 0;			// 是否展示花数（-1 - 无女仆； 1 - 有女仆; 0 - 不显示）
		bool removeDX = false;		// 当前胆小关不计入总关数
		bool hasRecord = false;		// 曾经记录过关数
		bool checkBrief = false;	// 是否重置过关数记录
		bool speedFlag = false;		// 切换一倍速/五倍速
		std::string brief = "当前关数：";	// 关数记录
		std::vector<int> missing = std::vector<int>();	// 缺少的植物
		std::vector<int> exessive = std::vector<int>();	// 多余的植物

		// 植物名称
		wxString name[TOTAL_NUM] = { "豌豆射手", "向日葵", "坚果", "土豆地雷", "寒冰射手", "大嘴花", "双重射手", "小喷菇", "大喷菇", "胆小菇", "倭瓜",
		"三线射手", "地刺", "火炬树桩", "裂荚射手", "杨桃", "磁力菇", "玉米投手", "叶子保护伞" };

		// 植物编号
		int type[TOTAL_NUM] = {
			0,  // 豌豆射手
			1,  // 向日葵
			3,  // 坚果
			4,  // 土豆地雷
			5,  // 寒冰射手
			6,  // 大嘴花
			7,  // 双重射手
			8,  // 小喷菇
			10, // 大喷菇
			13, // 胆小菇
			17, // 倭瓜
			18, // 三线射手
			21, // 地刺
			22, // 火炬树桩
			28, // 裂荚射手
			29, // 杨桃
			31, // 磁力菇
			34, // 玉米投手
			37, // 叶子保护伞
		};

		// 植物代码
		wxString code[TOTAL_NUM] = { "1", "h", "o", "t", "b", "z", "2", "p", "d", "x", "w", "3", "_", "j", "l", "5", "c", "y", "s" };
	};
}