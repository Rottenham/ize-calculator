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
		wxString checkPlants(int theme, bool changeSun, bool changeLevel, bool resetPuffshroom);
		wxString readPlantsToCode(bool readIndex);
		wxString embattleFromCode(wxString plant_code, bool checkCompliance);
		void update(std::vector<int**>& result, wxString chr, int row, int col, int round);
		void clearPlantStack();
		void resetBrains();

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
		wxString toCode(int** plants, int** index = nullptr);
		wxString checkTheme(int** plants, int* counter, int theme);
		std::vector<int**> compilePlantCode(wxString input);
		int getPlantTypeByCode(wxString plant_code);
		void setSun(int xrkNum);
		void setLevel(int xrkNum);
		void resetPuff();

		// �ڴ��������
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
		HWND hwnd = nullptr;		// ���ھ��
		DWORD pid = 0;				// ���̱�ʶ
		HANDLE handle = nullptr;	// ���̾��
		int level = -1;				// ����ɹ���
		int sun = -1;				// ����
		bool isQX = false;			// ��ǰ�ؿ��Ƿ�Ϊ��б
		int dxCount = 0;			// �����ĵ�С��
		int qxCount = 0;			// ��������б��
		int scCount = 0;			// �����������
		int ccCount = 0;			// �����Ĵ�����
		int initialSun = -1;		// ��������
		int xrkCount = 0;			// �ܻ���
		int maidLevelCount = 0;		// 10f�����б���ǵ�С��������
		double score = -1;			// �����
		bool setButter = false;		// �Ƿ������˻���
		bool displayExpect = false;	// ���¼ģʽ
		int scoreMode = 0;			// �Ƿ�չʾ������-1 - ��Ů�ͣ� 1 - ��Ů��; 0 - ����ʾ��
		bool removeDX = false;		// ��ǰ��С�ز������ܹ���
		bool hasRecord = false;		// ������¼������
		bool checkBrief = false;	// �Ƿ����ù�������¼
		bool speedFlag = false;		// �л�һ����/�屶��
		std::string brief = "��ǰ������";	// ������¼
		std::vector<int> missing = std::vector<int>();	// ȱ�ٵ�ֲ��
		std::vector<int> exessive = std::vector<int>();	// �����ֲ��

		// ֲ������
		wxString name[TOTAL_NUM] = { "�㶹����", "���տ�", "���", "��������", "��������", "���컨", "˫������", "С�繽", "���繽", "��С��", "����",
		"��������", "�ش�", "�����׮", "�Ѽ�����", "����", "������", "����Ͷ��", "Ҷ�ӱ���ɡ" };

		// ֲ����
		int type[TOTAL_NUM] = {
			0,  // �㶹����
			1,  // ���տ�
			3,  // ���
			4,  // ��������
			5,  // ��������
			6,  // ���컨
			7,  // ˫������
			8,  // С�繽
			10, // ���繽
			13, // ��С��
			17, // ����
			18, // ��������
			21, // �ش�
			22, // �����׮
			28, // �Ѽ�����
			29, // ����
			31, // ������
			34, // ����Ͷ��
			37, // Ҷ�ӱ���ɡ
		};

		// ֲ�����
		wxString code[TOTAL_NUM] = { "1", "h", "o", "t", "b", "z", "2", "p", "d", "x", "w", "3", "_", "j", "l", "5", "c", "y", "s" };
	};
}