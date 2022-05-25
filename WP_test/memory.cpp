#include <iostream>
#include <sstream>
#include <initializer_list>
#include <ctime>
#include <vector>
#include "memory.h"
#include "pvzstruct.h"
#include "process.h"

namespace IZE {
	// ��������ֵ
	void Memory::resetValues(bool setRecord) {
		sun = -1;
		dxCount = 0;
		qxCount = 0;
		scCount = 0;
		ccCount = 0;
		initialSun = -1;
		xrkCount = 0;
		maidLevelCount = 0;
		score = -1;
		scoreMode = 0;
		removeDX = false;
		hasRecord = setRecord;
	}

	// ���ù�����¼
	void Memory::renewBrief() {
		brief = "��ǰ������";
		checkBrief = true;
	}

	// ��izֲ��call
	void Memory::asm_iz_plant(int row, int col, int type) {
		asm_push(-1);
		asm_push(type);
		asm_mov_exx(Reg::EAX, row);
		asm_push(col);
		asm_mov_exx_dword_ptr(Reg::EBP, 0x6a9ec0);
		asm_mov_exx_dword_ptr_exx_add(Reg::EBP, 0x768);
		asm_push_exx(Reg::EBP);
		asm_call(0x0040d120);
		asm_add_list(0x8b, 0xf0); // mov esi,eax
		asm_push_exx(Reg::EAX);
		asm_mov_exx_dword_ptr(Reg::EAX, 0x6a9ec0);
		asm_mov_exx_dword_ptr_exx_add(Reg::EAX, 0x768);
		asm_mov_exx_dword_ptr_exx_add(Reg::EAX, 0x160);
		asm_call(0x0042a530);
		asm_add_list(0x8b, 0xc6); // mov eax,esi
	}

	// ���֡ʱ��
	int Memory::GetFrameDuration() {
		int time_ms = 10;
		if (!GameOn())
			return time_ms;
		time_ms = ReadMemory<int>({ 0x6a9ec0, 0x454 });
		return time_ms;
	}

	// �Ƿ���Ч
	bool Memory::IsValid() {
		if (handle == nullptr)
			return false;
		DWORD exit_code;
		GetExitCodeProcess(handle, &exit_code);
		bool valid = (exit_code == STILL_ACTIVE);
		return valid;
	}

	// ע�����
	void Memory::asm_code_inject() {
		WriteMemory<byte>(0xfe, { 0x00552014 });
		Sleep(GetFrameDuration() * 2);
		if (IsValid()) {
			Code::asm_code_inject(handle);
		}
		WriteMemory<byte>(0xdb, { 0x00552014 });
	}

	// ��鵱ǰ�����Ƿ�Ϊ��б - ��Ҫ8���ң�9�ش̣� ʣ�¾�Ϊ��/С��������Ϊ8��С������һ��
	bool Memory::checkQX(int** plants) {
		if (countPlant(plants, YT_29) == 8 && countPlant(plants, DC_21) == 9 &&
			countPlant(plants, XRK_1) + countPlant(plants, XPG_8) == 8 && countPlant(plants, XPG_8) >= 1) {
			return true;
		}
		return false;
	}

	// ��鵱ǰ�����Ƿ�Ϊ��С - ��Ҫ12��С��ʣ�¾�Ϊ��/С��������Ϊ13���һ�����6~12֮��
	bool Memory::checkDX(int** plants) {
		if (countPlant(plants, DXG_13) == 12 && countPlant(plants, XRK_1) + countPlant(plants, XPG_8) == 13 &&
			countPlant(plants, XRK_1) >= 6 && countPlant(plants, XRK_1) <= 12) {
			return true;
		}
		return false;
	}

	// ��鵱ǰ�����Ƿ�Ϊ��� - ��Ҫ9������4˫����4�Ѽԣ�ʣ�¾�Ϊ��/С��������Ϊ8��С������1��
	bool Memory::checkSC(int** plants) {
		if (countPlant(plants, HBSS_5) == 9 && countPlant(plants, XRK_1) + countPlant(plants, XPG_8) == 8 &&
			countPlant(plants, SCSS_7) == 4 && countPlant(plants, LJSS_28) == 4 && countPlant(plants, XPG_8) >= 1) {
			return true;
		}
		return false;
	}

	// ��鵱ǰ�����Ƿ�Ϊ���� - ��Ҫ9���磬8������ʣ�¾�Ϊ��/С��������Ϊ8��С������1��
	bool Memory::checkCC(int** plants) {
		if (countPlant(plants, DPG_10) == 9 && countPlant(plants, XRK_1) + countPlant(plants, XPG_8) == 8 &&
			countPlant(plants, CLG_31) == 8 && countPlant(plants, XPG_8) >= 1) {
			return true;
		}
		return false;
	}

	// ���ص�ǰ����
	std::string Memory::getBrief() {
		return brief;
	}

	// ���ص�ǰ���Ƿ�Ϊ��б
	bool Memory::getQX() {
		return isQX;
	}

	// ���»���
	void Memory::updateScore(int** plants) {
		score = double(xrkCount) + 1.75 * dxCount + 0.75 * qxCount + 0.25 * ccCount - 0.5 * scCount - sun / 200.0 - (level - 20.0) * 3.75;
		if (initialSun != -1 && initialSun <= 1400) {
			score -= (1400.0 - initialSun) / 200.0;
		}
		xrkCount += countPlant(plants, XRK_1);
		removeDX = false;
		if (checkDX(plants)) {
			dxCount++;
			removeDX = true;
		}
		else if (checkQX(plants)) {
			qxCount++;
		}
		else {
			if (level >= 10) {
				maidLevelCount++;
			}
			if (checkSC(plants)) {
				scCount++;
			}
			else if (checkCC(plants)) {
				ccCount++;
			}
		}
	}

	// ���¹���˵��
	void Memory::updateBrief() {
		if (!displayExpect) {
			int dxNum = removeDX ? dxCount - 1 : dxCount;
			brief = "��ǰ������" + std::to_string(dxNum) + "��С" + std::to_string(level) + "��";
		}
		else {
			if (level < 6) {
				int goal = 3200 + dxCount * 1000;
				brief = "Ŀ�꣺6f���ʱ" + std::to_string(goal) + "����";
			}
			else if (level < 10) {
				int goal = 2950 + dxCount * 1000;
				brief = "Ŀ�꣺10f���ʱ" + std::to_string(goal) + "����";
			}
			else if (level < 15) {
				int goal = 1025 + dxCount * 1000;
				brief = "Ŀ�꣺15f���ʱ" + std::to_string(goal) + "����";
			}
			else if (level < 20) {
				int goal = -250 + dxCount * 1000;
				if (goal > 0) {
					brief = "Ŀ�꣺20f���ʱ" + std::to_string(goal) + "����";
				}
				else {
					brief = "Ԥ�ƹ�����" + std::to_string(dxCount) + "��С" + std::to_string(level + sun / 200) + "��";
				}
			}
			else {
				brief = "Ԥ�ƹ�����" + std::to_string(dxCount) + "��С" + std::to_string(level + sun / 200) + "��";
			}
		}
		if (scoreMode != 0) {
			if (hasRecord) {
				if (scoreMode == -1) {
					brief += " (������" + getScore(false) + "/20)";
				}
				else {
					brief += " (������" + getScore(true) + "/20)";
				}
			}
			else {
				brief += " (������δ֪)";
			}
		}
	}

	// ��������
	std::string Memory::getScore(bool maidOK) {
		std::string str;
		if (maidOK) {
			str = std::to_string(score + 0.125 * maidLevelCount);
		}
		else {
			str = std::to_string(score);
		}
		if (str.find('.') != std::string::npos) {
			str = str.substr(0, str.find_last_not_of('0') + 1);
			if (str.find('.') == str.size() - 1) {
				str = str.substr(0, str.size() - 1);
			}
		}
		return str;
	}

	// ��ֲ������
	int Memory::countPlant(int** plants, int idx) {
		int sum = 0;
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				if (plants[i][j] == idx) {
					sum++;
				}
		return sum;
	}

	// ���ĳֲ���Ƿ���IZEֲ��������
	bool Memory::checkPlant(int plant_type) {
		for (int i = 0; i < 19; i++) {
			if (type[i] == plant_type) {
				return true;
			}
		}
		return false;
	}

	// ����ĳֲ�������
	wxString Memory::getName(int plant_type) {
		int index = -1;
		for (int i = 0; i < TOTAL_NUM; i++) {
			if (type[i] == plant_type) {
				index = i;
				break;
			}
		}
		if (index == -1) {
			return "δ֪";
		}
		else {
			return name[index];
		}
	}

	// ����ĳֲ��ı��
	int Memory::getIndex(int plant_type) {
		for (int i = 0; i < TOTAL_NUM; i++) {
			if (type[i] == plant_type) {
				return i;
			}
		}
		return -1;
	}

	// ���ֲ���Ƿ���ȱ��/����
	wxString Memory::checkEach(int** plants, int* counter, int* goal, bool isDX, bool isBType) {
		missing.clear();
		exessive.clear();
		for (int i = 0; i < TOTAL_NUM; i++) {
			if (type[i] == XRK_1 || type[i] == XPG_8) {
				continue;
			}
			if (counter[i] < goal[i]) {
				missing.push_back(type[i]);
			}
			else if (counter[i] > goal[i]) {
				exessive.push_back(type[i]);
			}
		}

		// �����տ����⴦��
		if (!isDX && counter[1] < 1) {
			missing.push_back(XRK_1);
		}
		else if (isDX && counter[1] < 6) {
			missing.push_back(XRK_1);
		}

		// ��С�繽���⴦��
		if (isBType && counter[7] < 1) {
			missing.push_back(XPG_8);
		}

		wxString result = "";
		if (!missing.empty()) {
			result += "ȱ�ٵ�ֲ�\n";
			for (std::vector<int>::iterator it = missing.begin(); it != missing.end(); it++) {
				result += getName(*it) + "\n";
			}
		}
		if (!exessive.empty()) {
			result += "\n�����ֲ�\n";
			for (std::vector<int>::iterator it = exessive.begin(); it != exessive.end(); it++) {
				result += getName(*it) + "\n";
			}
		}

		// ������������λ��
		bool errorFound = false;
		for (int row = 0; row < 5; row++)
			for (int col = 0; col < 2; col++) {
				if (plants[row][col] == JG_3 || plants[row][col] == HJSZ_22) {
					if (!errorFound) {
						errorFound = true;
						if (result != "") {
							result += "\n";
						}
						result += "���������ɹ���\n";
					}
					result += std::to_string(row + 1) + "-" + std::to_string(col + 1) + "��" + getName(plants[row][col]) + "\n";
				}
			}

		if (result == "") {
			return "������";
		}
		else {
			return result;
		}
	}

	// �������
	wxString Memory::checkTheme(int** plants, int* counter, int theme) {
		if (theme == 0) {
			// �ۺ�
			int goal[TOTAL_NUM] = { 1, -1, 1, 1, 1, 2, 0, -1, 1, 0, 1, 1, 2, 1, 1, 1, 1, 1, 1 };
			return checkEach(plants, counter, goal, false, false);
		}
		else if (theme == 1) {
			// ����
			int goal[TOTAL_NUM] = { 0, -1, 0, 0, 3, 0, 1, -1, 0, 0, 0, 1, 3, 1, 3, 0, 1, 3, 1 };
			return checkEach(plants, counter, goal, false, false);
		}
		else if (theme == 2) {
			// ����
			int goal[TOTAL_NUM] = { 0, -1, 0, 4, 0, 3, 0, -1, 4, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0 };
			return checkEach(plants, counter, goal, false, false);
		}
		else if (theme == 3) {
			// ���
			int goal[TOTAL_NUM] = { 0, -1, 0, 0, 9, 0, 4, -1, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0 };
			return checkEach(plants, counter, goal, false, true);
		}
		else if (theme == 4) {
			// ��ը
			int goal[TOTAL_NUM] = { 0, -1, 0, 9, 0, 8, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			return checkEach(plants, counter, goal, false, true);
		}
		else if (theme == 5) {
			// ��б
			int goal[TOTAL_NUM] = { 0, -1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 9, 0, 0, 8, 0, 0, 0 };
			return checkEach(plants, counter, goal, false, true);
		}
		else if (theme == 6) {
			// ����
			int goal[TOTAL_NUM] = { 0, -1, 0, 0, 0, 0, 0, -1, 9, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0 };
			return checkEach(plants, counter, goal, false, true);
		}
		else if (theme == 7) {
			// ��С
			int goal[TOTAL_NUM] = { 0, -1, 0, 0, 0, 0, 0, -1, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			return checkEach(plants, counter, goal, true, true);
		}
		return "δ֪������";
	}

	// ����ĳֲ��Ĵ���
	wxString Memory::getCode(int plant_type) {
		int index = -1;
		for (int i = 0; i < TOTAL_NUM; i++) {
			if (type[i] == plant_type) {
				index = i;
				break;
			}
		}
		if (index == -1) {
			return " ";
		}
		else {
			return code[index];
		}
	}

	// ����ĳ�����Ӧ��ֲ��
	int Memory::getPlantTypeByCode(wxString plant_code) {
		for (int i = 0; i < TOTAL_NUM; i++) {
			if (code[i] == plant_code) {
				return type[i];
			}
		}
		return -1;
	}

	std::pair<int, int> indexOfPlantInPuzzle(int** plants, int plant) {
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				if (plants[i][j] == plant) return std::pair<int, int>(i, j);
		return std::pair<int, int>(-1, -1);
	}

	void compareIndex(int round[][5], int** index, std::vector<std::pair<int, int>> plants) {
		if (plants.size() < 2) return;

		// sort
		for (int i = 0; i < plants.size() - 1; i++)
			for (int j = 0; j < plants.size() - 1 - i; j++)
				if (index[plants.at(j).first][plants.at(j).second] > index[plants.at(j + 1).first][plants.at(j + 1).second]) {
					std::swap(plants[j], plants[j + 1]);
				}

		for (int i = 1; i < plants.size(); i++) {
			round[plants.at(i).first][plants.at(i).second] = i + 1;
		}
	}

	bool checkPotatoMagnet(int r1, int c1, int r2, int c2) {
		if (r1 == r2 + 2 && c1 == c2 - 2) return true;
		if (r1 == r2 + 1 && c1 == c2 - 3) return true;
		if (r1 == r2 && c1 == c2 - 3) return true;
		if (r1 == r2 - 1 && c1 == c2 - 3) return true;
		if (r1 == r2 - 2 && c1 == c2 - 2) return true;
		return false;
	}

	bool checkPotato(int** plants, int r, int c) {
		if (c > 4) return false;
		if (plants[r][c] != TDDL_4) return false;
		if (c > 0 && plants[r][c - 1] == WG_17) return true;
		if (c < 4 && plants[r][c + 1] == WG_17) return true;
		return false;
	}

	bool checkSquash(int** plants, int r, int c) {
		if (c > 4) return false;
		if (plants[r][c] != WG_17) return false;
		if (c > 0 && plants[r][c - 1] == TDDL_4) return true;
		if (c < 4 && plants[r][c + 1] == TDDL_4) return true;
		return false;
	}

	void rollback(int** plants, int round[][5], int r, int c) {
		int start = c;
		int end = c;
		while (start > 0 && (checkPotato(plants, r, start - 1) || checkSquash(plants, r, start - 1))) start--;
		while (end < 4 && (checkPotato(plants, r, end + 1) || checkSquash(plants, r, end + 1))) end++;
		for (int i = start; i <= end; i++)
			if (i >= 0 && i < 5) round[r][i]++;
	}

	// ������ת��Ϊ����
	wxString Memory::toCode(int** plants, int** index) {
		wxString result = "";
		if (index == nullptr) {
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					result += getCode(plants[i][j]);
				}
				if (i != 4) {
					result += "\n";
				}
			}
			return result;
		}
		else {
			int round[5][5];
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 5; j++)
					round[i][j] = 1;

			// ���߻���
			auto sfss_pos = indexOfPlantInPuzzle(plants, SFSS_18);
			auto hjsz_pos = indexOfPlantInPuzzle(plants, HJSZ_22);
			if (sfss_pos.first != -1 && hjsz_pos.first != -1 && abs(sfss_pos.first - hjsz_pos.first) == 1 && sfss_pos.second == hjsz_pos.second) {
				if (index[sfss_pos.first][sfss_pos.second] > index[hjsz_pos.first][hjsz_pos.second])
					round[sfss_pos.first][sfss_pos.second]++;
				else
					round[hjsz_pos.first][hjsz_pos.second]++;
			}

			// ������������������
			std::vector<std::vector<int>> relation;
			auto clg_pos = indexOfPlantInPuzzle(plants, CLG_31);
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 5; j++)
					if (plants[i][j] == TDDL_4 && checkPotatoMagnet(clg_pos.first, clg_pos.second, i, j)) {
						int diff = index[clg_pos.first][clg_pos.second] > index[i][j] ? 1 : -1;
						relation.push_back({ clg_pos.first, clg_pos.second, i, j, diff });
						break;
					}
			for (int i = 0; i < 5; i++)
				for (int j = 0; j < 4; j++)
					if (plants[i][j] == DZH_6 && plants[i][j + 1] == TDDL_4) {
						int diff = index[i][j] > index[i][j + 1] ? 1 : -1;
						relation.push_back({ i, j, i, j + 1, diff });
					}

			// �ѹ�����
			for (int i = 0; i < 5; i++) {
				std::vector<std::pair<int, int>> buffer;
				for (int j = 0; j < 6; j++) {
					if (checkPotato(plants, i, j) || checkSquash(plants, i, j)) {
						buffer.push_back(std::pair<int, int>(i, j));
					}
					else {
						if (!buffer.empty()) {
							compareIndex(round, index, buffer);
							buffer.clear();
						}
					}
				}
			}

			// ����֮ǰ��������ϵ
			for (auto lst : relation) {
				round[lst[0]][lst[1]] = round[lst[2]][lst[3]] + lst[4];
				if (round[lst[0]][lst[1]] == 0) {
					round[lst[0]][lst[1]]++;
					rollback(plants, round, lst[2], lst[3]);
				}
			}

			// ���
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					result += getCode(plants[i][j]);
					for (int k = 0; k < round[i][j] - 1; k++)
						result += "+";
				}
				if (i != 4) {
					result += "\n";
				}
			}
			return result;
		}
	}

	// �������õ�ֲ�������vector�к��ʵ�λ��
	// �ȼ��vector��С��������������
	void Memory::update(std::vector<int**>& result, wxString chr, int row, int col, int round) {
		while (result.size() < round) {
			int** puzzle = new int* [5];
			for (int i = 0; i < 5; i++) {
				puzzle[i] = new int[5];
			}
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					puzzle[i][j] = -1;
				}
			}
			result.push_back(puzzle);
		}
		result.at(round - 1)[row][col] = getPlantTypeByCode(chr);
	}

	// ����һ�δ��벢ת��
	std::vector<int**> Memory::compilePlantCode(wxString input) {
		std::vector<int**> result;
		int** puzzle = new int* [5];
		for (int i = 0; i < 5; i++) {
			puzzle[i] = new int[5];
		}
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				puzzle[i][j] = -1;
			}
		}
		std::vector<wxString> lines;
		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = input.find("\n", prev)) != std::string::npos) {
			lines.push_back(input.substr(prev, pos - prev));
			prev = pos + 1;
		}
		lines.push_back(input.substr(prev));

		// ����ת��
		size_t row = 0;
		for (size_t idx = 0; idx < lines.size(); idx++) {
			if (row >= 5) {
				break;
			}
			wxString line = lines.at(idx);
			int repeat = 1;
			int match = line.find("*", 0);
			if (match != std::string::npos) {
				wxString nextChar = line.substr(match + 1, 1);
				if (!nextChar.empty() && nextChar.find_first_not_of("12345") == std::string::npos) {
					int temp = std::stoi(nextChar.ToStdString());
					if (row + temp <= 5) {
						repeat = temp;
					}
				}
			}
			for (int r = 0; r < repeat; r++) {
				size_t col = 0;
				for (int i = 0; col < 5 && i < line.size(); i++) {
					wxString currentChar = line.substr(i, 1);
					if (currentChar == "*") {
						break;
					}
					else {
						int round = 1;
						while (i + 1 < line.size() && line.substr(i + 1, 1) == "+") {
							i++;
							round++;
						}
						update(result, currentChar, row, col, round);
						col++;
					}
				}
				row++;
			}
		}
		return result;
	}

	// ������Ϸ���ڴ򿪽��̾��
	bool Memory::FindGame() {
		hwnd = FindWindowW(L"MainWindow", L"Plants vs. Zombies");
		if (hwnd == nullptr)
			return false;

		GetWindowThreadProcessId(hwnd, &pid);
		if (pid == 0)
			return false;

		handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
		return handle != nullptr;
	}

	bool Memory::FindGame2() {
		hwnd = FindWindowW(L"MainWindow", L"ֲ���ս��ʬ���İ�");
		if (hwnd == nullptr)
			return false;

		GetWindowThreadProcessId(hwnd, &pid);
		if (pid == 0)
			return false;

		handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
		return handle != nullptr;
	}

	bool Memory::GameOn() {
		return ReadMemory<int>({ 0x6a9ec0 }) != 0;
	}

	// �ر���Ϸ���̾��
	void Memory::CloseGame() {
		if (handle != nullptr)
			CloseHandle(handle);
	}

	Memory::Memory() : Code() {
	}

	void Memory::toggleFrameDuration() {
		bool result = false;
		if (!speedFlag) {
			result = setFrameDuration(2);
		}
		else {
			result = setFrameDuration(10);
		}
		if (result) speedFlag = !speedFlag;
	}

	bool Memory::setFrameDuration(int time_ms) {
		if (!FindGame())
			if (!FindGame2())
				return false;

		if (GameOn()) {
			WriteMemory<int>(time_ms, { 0x6a9ec0, 0x454 });
		}
		else {
			return false;
		}
	}

	void Memory::resetFrameDuration() {
		setFrameDuration(10);
		speedFlag = false;
	}

	void Memory::setAuto(bool autoCollect) {
		if (!FindGame())
			if (!FindGame2())
				return;

		if (GameOn()) {
			// �Զ��ռ�
			if (autoCollect) {
				WriteMemory<byte>(0xeb, { 0x0043158f });
			}
			else {
				WriteMemory<byte>(0x75, { 0x0043158f });
			}
		}
	}

	// ��Ѫ����
	int** Memory::readPlants(bool setRunInBackground, bool showExpect, int showScore, bool enableSpeed) {
		if (!FindGame())
			if (!FindGame2()) {
				renewBrief();
				return nullptr;
			}

		if (GameOn()) {
			// ��̨����
			if (setRunInBackground) {
				WriteMemory<short>(0x00eb, { 0x0054eba8 });
			}
			else {
				WriteMemory<short>(0x2e74, { 0x0054eba8 });
			}
		}
		else {
			renewBrief();
			return nullptr;
		}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			renewBrief();
			return nullptr;
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			renewBrief();
			return nullptr;
		}

		// ��ȡ����
		bool levelChange = false;
		bool restart = false;
		int currentLevel = ReadMemory<int>({ 0x6a9ec0, 0x768, 0x160, 0x6c });
		if (currentLevel != level) {
			if (level == -1) {
				resetValues(false);
			}
			else if (currentLevel < level) {
				resetValues(false);
			}
			level = currentLevel;
			levelChange = true;
		}

		int** puzzle = new int* [5];
		for (int i = 0; i < 5; i++)
			puzzle[i] = new int[5];
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				puzzle[i][j] = -1;

		// ��ȡ����ֲ����Ϣ
		auto plants_offset = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xac });
		auto plants_count_max = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xb0 });
		for (size_t i = 0; i < plants_count_max; ++i) {
			auto plant_dead = ReadMemory<bool>({ plants_offset + 0x141 + 0x14c * i });
			auto plant_squished = ReadMemory<bool>({ plants_offset + 0x142 + 0x14c * i });
			auto plant_type = ReadMemory<int>({ plants_offset + 0x24 + 0x14c * i });
			auto plant_row = ReadMemory<int>({ plants_offset + 0x1c + 0x14c * i });
			auto plant_col = ReadMemory<int>({ plants_offset + 0x28 + 0x14c * i });
			if (!plant_dead && !plant_squished) {
				if (plant_row >= 0 && (plant_row <= 4 && (plant_col >= 0 && plant_col <= 4))) {
					puzzle[plant_row][plant_col] = plant_type;
				}
			}
		}

		// ��ȡ����
		int currentSun = ReadMemory<int>({ 0x6a9ec0, 0x768, 0x5560 });
		auto plants_now_num = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xbc });
		auto zombies_now_num = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xa0 });
		if (plants_now_num == 25 && currentSun == 150 && currentLevel == 0 && zombies_now_num == 0) {
			restart = true;
			levelChange = true;
			resetValues(true);
		}
		if (levelChange && currentSun != sun) {
			sun = currentSun;
		}

		// ¼�뿪������
		if (levelChange && level == 1) {
			initialSun = sun;
		}

		if (levelChange & enableSpeed) {
			resetFrameDuration();
		}

		// ���뻨��
		if (levelChange && hasRecord) {
			updateScore(puzzle);
		}
		if (levelChange || showScore != scoreMode || showExpect != displayExpect || checkBrief) {
			scoreMode = showScore;
			displayExpect = showExpect;
			checkBrief = false;
			updateBrief();
		}
		if (levelChange || !isQX) {
			isQX = checkQX(puzzle);
			levelChange = false;
		}

		// ����ǵ�һ�ص�1400ͼ��������������
		if (level == 0 && puzzle[1][1] == DC_21 && puzzle[1][2] == DC_21 && puzzle[4][0] == DC_21 && puzzle[0][4] == HBSS_5
			&& puzzle[1][4] == HBSS_5 && puzzle[2][4] == HBSS_5 && puzzle[3][4] == YMTS_34) {
			WriteMemory<byte>(0xeb, { 0x0045f1ec });
			setButter = true;
		}
		else if (setButter) {
			WriteMemory<byte>(0x75, { 0x0045f1ec });
			setButter = false;
		}

		CloseGame();
		return puzzle;
	}

	void Memory::setSun(int xrkNum) {
		if (xrkNum == 8) {
			WriteMemory<int>(150, { 0x6a9ec0, 0x768, 0x5560 });
		}
		else if (xrkNum == 7) {
			WriteMemory<int>(1000, { 0x6a9ec0, 0x768, 0x5560 });
		}
		else if (xrkNum >= 1 && xrkNum <= 6) {
			WriteMemory<int>(2000, { 0x6a9ec0, 0x768, 0x5560 });
		}
	}

	void Memory::setLevel(int xrkNum) {
		if (xrkNum == 8) {
			WriteMemory<int>(0, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
		else if (xrkNum == 7) {
			WriteMemory<int>(1, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
		else if (xrkNum == 6) {
			WriteMemory<int>(2, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
		else if (xrkNum == 5) {
			WriteMemory<int>(5, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
		else if (xrkNum == 4) {
			WriteMemory<int>(8, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
		else if (xrkNum >= 1 && xrkNum <= 3) {
			WriteMemory<int>(20, { 0x6a9ec0, 0x768, 0x160, 0x6c });
		}
	}

	// ������������
	wxString Memory::checkPlants(int theme, bool changeSun, bool changeLevel, bool resetPuffshroom) {
		if (!FindGame())
			if (!FindGame2()) {
				return "�Ҳ�����Ϸ";
			}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			return "��δ����IZE";
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			return "��δ����IZE";
		}

		// ��ȡ����ֲ����Ϣ
		int** puzzle = new int* [5];
		for (int i = 0; i < 5; i++)
			puzzle[i] = new int[5];
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				puzzle[i][j] = -1;
		int counter[TOTAL_NUM];
		for (int i = 0; i < TOTAL_NUM; i++) {
			counter[i] = 0;
		}
		auto plants_offset = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xac });
		auto plants_count_max = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xb0 });
		for (size_t i = 0; i < plants_count_max; ++i) {
			auto plant_dead = ReadMemory<bool>({ plants_offset + 0x141 + 0x14c * i });
			auto plant_squished = ReadMemory<bool>({ plants_offset + 0x142 + 0x14c * i });
			auto plant_type = ReadMemory<int>({ plants_offset + 0x24 + 0x14c * i });
			auto plant_row = ReadMemory<int>({ plants_offset + 0x1c + 0x14c * i });
			auto plant_col = ReadMemory<int>({ plants_offset + 0x28 + 0x14c * i });
			if (!plant_dead && !plant_squished) {
				if (plant_row >= 0 && plant_row <= 4 && plant_col >= 0 && plant_col <= 4) {
					puzzle[plant_row][plant_col] = plant_type;
				}
				if (!checkPlant(plant_type)) {
					for (int i = 0; i < 5; i++) {
						delete[] puzzle[i];
					}
					delete[] puzzle;
					return "������δֲ֪�" + std::to_string(plant_row + 1) + "-" + std::to_string(plant_col + 1);
				}
				else {
					counter[getIndex(plant_type)]++;
				}
			}
		}
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++)
				if (puzzle[i][j] == -1) {
					for (int i = 0; i < 5; i++) {
						delete[] puzzle[i];
					}
					delete[] puzzle;
					return "ȱ��ֲ�" + std::to_string(i + 1) + "-" + std::to_string(j + 1);
				}

		wxString result = checkTheme(puzzle, counter, theme);
		int xrkNum = counter[XRK_1];
		if (theme == 7) {
			xrkNum -= 5;
		}
		if (result == "������") {
			// ��������
			if (changeSun) {
				setSun(xrkNum);
			}

			// ���ù���
			if (changeLevel) {
				setLevel(xrkNum);
			}

			// С���λ
			if (resetPuffshroom) {
				resetPuff();
			}
		}

		CloseGame();
		for (int i = 0; i < 5; i++) {
			delete[] puzzle[i];
		}
		delete[] puzzle;
		return result;
	}

	void Memory::resetPuff() {
		if (!FindGame())
			if (!FindGame2()) {
				return;
			}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			return;
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			return;
		}

		auto plants_offset = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xac });
		auto plants_count_max = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xb0 });

		for (size_t i = 0; i < plants_count_max; ++i) {
			auto plant_dead = ReadMemory<bool>({ plants_offset + 0x141 + 0x14c * i });
			auto plant_squished = ReadMemory<bool>({ plants_offset + 0x142 + 0x14c * i });
			auto plant_type = ReadMemory<int>({ plants_offset + 0x24 + 0x14c * i });
			auto plant_row = ReadMemory<int>({ plants_offset + 0x1c + 0x14c * i });
			auto plant_col = ReadMemory<int>({ plants_offset + 0x28 + 0x14c * i });
			auto plant_absc = ReadMemory<int>({ plants_offset + 0x8 + 0x14c * i });
			auto plant_ord = ReadMemory<int>({ plants_offset + 0xc + 0x14c * i });
			if (!plant_dead && !plant_squished && plant_type == XPG_8 && plant_row >= 0 && plant_row <= 4 && plant_col >= 0 && plant_col <= 4) {
				int expected_absc = 40 + 80 * plant_col;
				int expected_ord = 80 + 100 * plant_row;
				if (plant_absc != expected_absc) WriteMemory<int>(expected_absc, { plants_offset + 0x8 + 0x14c * i });
				if (plant_ord != expected_ord) WriteMemory<int>(expected_ord, { plants_offset + 0xc + 0x14c * i });
			}
		}
	}

	void Memory::resetBrains() {
		if (!FindGame())
			if (!FindGame2()) {
				return;
			}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			return;
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			return;
		}

		// ��մ���
		asm_init();
		auto grid_item_count_max = ReadMemory<uint32_t>({ 0x6a9ec0, 0x768, 0x120 });
		auto grid_item_offset = ReadMemory<uintptr_t>({ 0x6a9ec0, 0x768, 0x11c });
		for (size_t i = 0; i < grid_item_count_max; i++) {
			auto grid_item_disappeared = ReadMemory<bool>({ grid_item_offset + 0x20 + 0xec * i });
			auto grid_item_type = ReadMemory<int>({ grid_item_offset + 0x8 + 0xec * i });
			if (!grid_item_disappeared && grid_item_type == 12) {
				int addr = grid_item_offset + 0xec * i;
				asm_mov_exx(Reg::ESI, addr);
				asm_call(0x0044d000);
			}
		}
		asm_ret();
		asm_code_inject();

		// 

	}


	// ��ݲ��󣺶�ȡ��ǰ����
	wxString Memory::readPlantsToCode(bool readIndex) {
		if (!FindGame())
			if (!FindGame2()) {
				return "��ȡʧ�ܣ��Ҳ�����Ϸ";
			}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			return "��ȡʧ�ܣ���δ����IZE";
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			return "��ȡʧ�ܣ���δ����IZE";
		}

		// ��ȡ����ֲ����Ϣ
		int** puzzle = new int* [5];
		int** index = new int* [5];
		for (int i = 0; i < 5; i++) {
			puzzle[i] = new int[5];
			index[i] = new int[5];
		}
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++) {
				puzzle[i][j] = -1;
				index[i][j] = -1;
			}
		auto plants_offset = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xac });
		auto plants_count_max = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xb0 });
		for (size_t i = 0; i < plants_count_max; ++i) {
			auto plant_dead = ReadMemory<bool>({ plants_offset + 0x141 + 0x14c * i });
			auto plant_squished = ReadMemory<bool>({ plants_offset + 0x142 + 0x14c * i });
			auto plant_type = ReadMemory<int>({ plants_offset + 0x24 + 0x14c * i });
			auto plant_row = ReadMemory<int>({ plants_offset + 0x1c + 0x14c * i });
			auto plant_col = ReadMemory<int>({ plants_offset + 0x28 + 0x14c * i });
			if (!plant_dead && !plant_squished) {
				if (plant_row >= 0 && plant_row <= 4 && plant_col >= 0 && plant_col <= 4) {
					puzzle[plant_row][plant_col] = plant_type;
					index[plant_row][plant_col] = i;
				}
			}
		}

		wxString result = readIndex ? toCode(puzzle, index) : toCode(puzzle);
		for (int i = 0; i < 5; i++) {
			delete[] puzzle[i];
			delete[] index[i];
		}
		delete[] puzzle;
		delete[] index;
		return "�ɹ���" + result;
	}

	void Memory::clearPlantStack() {
		WriteMemory<unsigned int>(0, { 0x6a9ec0, 0x768, 0xb0 });
		WriteMemory<unsigned int>(0, { 0x6a9ec0, 0x768, 0xb8 });
	}

	// ��ݲ���
	wxString Memory::embattleFromCode(wxString plant_code, bool checkCompliance) {
		if (!FindGame())
			if (!FindGame2()) {
				return "����ʧ�ܣ��Ҳ�����Ϸ";
			}

		// ��ȡ��Ϸģʽ
		auto gamemode = ReadMemory<int>({ 0x6a9ec0, 0x7f8 });
		if (gamemode != 70) {
			return "����ʧ�ܣ���δ����IZE";
		}

		// ��ȡ��Ϸ���棨2/3 - ��IZE�ڲ���
		auto gameui = ReadMemory<int>({ 0x6a9ec0, 0x7FC });
		if (gameui != 3 && gameui != 2) {
			return "����ʧ�ܣ���δ����IZE";
		}

		// �����������ֲ��
		unsigned int plant_struct_size = 0x14c;
		auto plants_offset = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xac });
		auto plants_count_max = ReadMemory<unsigned int>({ 0x6a9ec0, 0x768, 0xb0 });
		asm_init();
		for (size_t i = 0; i < plants_count_max; i++) {
			auto plant_dead = ReadMemory<bool>({ plants_offset + 0x141 + 0x14c * i });
			auto plant_squished = ReadMemory<bool>({ plants_offset + 0x142 + 0x14c * i });
			if (!plant_dead && !plant_squished) {
				uint32_t addr = plants_offset + plant_struct_size * i;
				asm_push(addr);
				asm_call(0x004679b0);
			}
		}

		// ����������н�ʬ����ѡ��
		auto zombie_count_max = ReadMemory<uint32_t>({ 0x6a9ec0, 0x768, 0x94 });
		auto zombie_offset = ReadMemory<uintptr_t>({ 0x6a9ec0, 0x768, 0x90 });
		for (size_t i = 0; i < zombie_count_max; i++) {
			if (!ReadMemory<bool>({ zombie_offset + 0xec + i * 0x15c })) // not disappear
			{
				WriteMemory<int>(3, { zombie_offset + 0x28 + i * 0x15c });	// kill
			}
		}

		// ���²���
		std::vector<int**> puzzle_list = compilePlantCode(plant_code);
		if (puzzle_list.size() > 1) {
			clearPlantStack();
		}
		int xrkNum = 0;
		int dxgNum = 0;
		for (int** puzzle : puzzle_list) {
			xrkNum += countPlant(puzzle, XRK_1);
			dxgNum += countPlant(puzzle, DXG_13);
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (puzzle[i][j] != -1) {
						asm_iz_plant(i, j, puzzle[i][j]);
					}
				}
			}
		}
		asm_ret();
		asm_code_inject();

		// �������������⣬С���λ
		// ���ô���
		if (checkCompliance) {
			if (dxgNum > 0) {
				xrkNum -= 5;
			}
			setSun(xrkNum);
			setLevel(xrkNum);
			resetPuff();
			resetBrains();
		}

		for (int** puzzle : puzzle_list) {
			for (int i = 0; i < 5; i++) {
				delete[] puzzle[i];
			}
			delete[] puzzle;
		}
		return "�ɹ�";
	}

	template <typename T>
	T Memory::ReadMemory(std::initializer_list<uintptr_t> addr) {
		T result = T();
		uintptr_t buffer = 0;
		for (auto it = addr.begin(); it != addr.end(); it++) {
			if (it != addr.end() - 1) {
				unsigned long read_size = 0;
				int ret = ReadProcessMemory(handle,                       //
					(const void*)(buffer + *it), //
					&buffer,                      //
					sizeof(buffer),               //
					&read_size);                  //
				if (ret == 0 || sizeof(buffer) != read_size)
					return T();
			}
			else {
				unsigned long read_size = 0;
				int ret = ReadProcessMemory(handle,                       //
					(const void*)(buffer + *it), //
					&result,                      //
					sizeof(result),               //
					&read_size);                  //
				if (ret == 0 || sizeof(result) != read_size)
					return T();
			}
		}
		return result;
	}

	template <typename T>
	void Memory::WriteMemory(T value, std::initializer_list<uintptr_t> addr) {
		uintptr_t offset = 0;
		for (auto it = addr.begin(); it != addr.end(); it++) {
			if (it != addr.end() - 1) {
				unsigned long read_size = 0;
				int ret = ReadProcessMemory(handle, (const void*)(offset + *it), &offset, sizeof(offset), &read_size);
				if (ret == 0 || sizeof(offset) != read_size)
					return;
			}
			else {
				unsigned long write_size = 0;
				int ret = WriteProcessMemory(handle, (void*)(offset + *it), &value, sizeof(value), &write_size);
				if (ret == 0 || sizeof(value) != write_size)
					return;
			}
		}
	}
}