#include <sstream>
#include <vector>
#include <set>
#include <iostream>
#include "pvzstruct.h"

using namespace std;

//#ifndef _DEBUG
//def _DEBUG
//#endif 

const double KERNAL_RATE = 0.61;

double getButterRate(int i) {
	if (i <= 0) {
		return 1;
	}
	if (i == 1) {
		return 1.4665;
	}
	if (i == 2) {
		return 2.1467;
	}
	if (i == 3) {
		return 3.1419;
	}
	if (i == 4) {
		return 4.6022;
	}
	return 6.7470;
}

double getHSFix(int diff) {
	if (diff <= 0) {
		return 0;
	}
	if (diff == 1) {
		return 0.2;
	}
	if (diff == 2) {
		return 0.4;
	}
	if (diff == 3) {
		return 0.5;
	}
	return 0.74;
}

bool isEmpty(int idx) {
	return idx == -1 || (idx == DC_21 || (idx == TDDL_4 || idx == WG_17));
}

bool isBitable(int idx) {
	return (idx != -1 && (idx != TDDL_4 && (idx != WG_17 && (idx != JG_3 && (idx != DC_21)))));
}

bool isHarmless(int idx) {
	return idx == HJSZ_22 || (idx == CLG_31 || (idx == YZBHS_37 || (idx == XRK_1 || (idx == DC_21 || idx == JG_3))));
}

double getDPS(int idx) {
	switch (idx) {
		case WDSS_0:
		case HBSS_5:
		case SFSS_18:
		case XPG_8:
			return 2;
		case SCSS_7:
		case LJSS_28:
		case YT_29:
			return 4;
		default:
			return 0;
	}
}

double getFumeDPS(int idx) {
	switch (idx) {
		case DPG_10:
			return 2;
		case DC_21:
			return 3;
		case YMTS_34:
			return 2 * KERNAL_RATE * getButterRate(1);
		default:
			return 0;
	}
}

struct Pair {
	int x;
	int y;
	bool empty;
};

struct Row {
	int* row;
	int mode;
	double* bite = new double[6];
	double* walk = new double[6];
	double* fumeBite = new double[6];
	double* fumeWalk = new double[6];
	/*double* kernalBite = new double[6];
	double* kernalWalk = new double[6];*/
	double* extraBite = new double[6];
	double* extraWalk = new double[6];
	bool* biteSlowed = new bool[6];
	bool* walkSlowed = new bool[6];
	bool* biteFire = new bool[6];
	bool* walkFire = new bool[6];
	int* biteButter = new int[6];
	int* walkButter = new int[6];
	set<int> wallnuts;

	double BITE_DPS = -1;
	double WALK_DPS = -1;
	double DC_PASSBY = -1;
	double DC_BLOCK = -1;
	double XPG_FIX = -1;
	double YT_DPS = -1;

	// 算血的起始位置
	int biteLmt = 5;
	int walkLmt = 5;

	// 是否适合使用撑杆
	bool canPV = true;

	// 火树修正
	double HSFix = 0;

	// 0——撑杆, 1——慢速, 2——梯子, 3——橄榄, 4——撑杆梯子
	Row(int* r, int mode) {
		row = new int[5];
		this->mode = mode;
		for (int i = 0; i < 5; i++) {
			row[i] = r[i];
		}
		if (mode == 0) {
			BITE_DPS = 2;
			WALK_DPS = 4;
			XPG_FIX = 3;
			DC_PASSBY = 5;
			DC_BLOCK = 0;
			YT_DPS = 5;
			computePV();
		}
		else if (mode == 1) {
			BITE_DPS = 2;
			WALK_DPS = 4;
			XPG_FIX = 3;
			DC_PASSBY = 5;
			DC_BLOCK = 3;
			YT_DPS = 2;
			walkLmt = 5;
			biteLmt = 5;
		}
		else if (mode == 2) {
			BITE_DPS = 2;
			WALK_DPS = 1.5;
			XPG_FIX = 1;
			DC_PASSBY = 2;
			DC_BLOCK = 3;
			YT_DPS = 2;
			walkLmt = 5;
			biteLmt = 5;
		}
		else if (mode == 3) {
			BITE_DPS = 2;
			WALK_DPS = 1.75;
			XPG_FIX = 1;
			DC_PASSBY = 3;
			DC_BLOCK = 3;
			YT_DPS = 5;
			walkLmt = 5;
			biteLmt = 5;
		}
		else if (mode == 4) {
			BITE_DPS = 2;
			WALK_DPS = 1.5;
			XPG_FIX = 1;
			DC_PASSBY = 2;
			DC_BLOCK = 3;
			YT_DPS = 2;
		}
		else {
			return;
		}
		for (int i = 0; i < 6; i++) {
			bite[i] = 0;
			walk[i] = 0;
			fumeBite[i] = 0;
			fumeWalk[i] = 0;
		/*	kernalBite[i] = 0;
			kernalWalk[i] = 0;*/
			extraBite[i] = 0;
			extraWalk[i] = 0;
			biteSlowed[i] = false;
			walkSlowed[i] = false;
			biteFire[i] = false;
			walkFire[i] = false;
			biteButter[i] = 0;
			walkButter[i] = 0;
		}
	}

	~Row() {
		delete[] row;
		delete[] bite;
		delete[] walk;
		delete[] fumeBite;
		delete[] fumeWalk;
		/*delete[] kernalBite;
		delete[] kernalWalk;*/
		delete[] extraBite;
		delete[] extraWalk;
		delete[] biteSlowed;
		delete[] walkSlowed;
		delete[] biteFire;
		delete[] walkFire;
		delete[] biteButter;
		delete[] walkButter;
	}

	bool hasMagnet() {
		return indexOf(CLG_31) != -1;
	}

	// 返回非5列星星的总数量
	int getStarNum() {
		int sum = 0;
		for (int i = 0; i < 4; i++) {
			if (row[i] == YT_29) {
				sum++;
			}
		}
		return sum;
	}

	int indexOf(int plant) {
		for (int i = 0; i < 5; i++) {
			if (row[i] == plant) {
				return i;
			}
		}
		return -1;
	}

	int indexOf(int plant, int lo, int hi) {
		if (lo < 0) {
			lo = 0;
		}
		if (hi > 4) {
			hi = 4;
		}
		for (int i = lo; i <= hi; i++) {
			if (row[i] == plant) {
				return i;
			}
		}
		return -1;
	}

	void computePV() {
		for (int i = 4; i >= 0; i--) {
			if (row[i] == DC_21 || row[i] == WG_17 || row[i] == -1) {
				continue;
			}
			if (row[i] == XRK_1 || (row[i] == LJSS_28 || row[i] == YT_29)) {
				canPV = false;
			}
			biteLmt = i;
			walkLmt = i - 1;

			// 处理火树bug
			if (row[i] == HJSZ_22 && i != 0) {
				if (row[i - 1] == WDSS_0 || (row[i - 1] == HBSS_5 || (row[i - 1] == SCSS_7 || (row[i - 1] == LJSS_28)))) {
					row[i - 1] = XRK_1;
				}
			}
			return;
		}
		biteLmt = -1;
		walkLmt = -1;
	}

	void convert() {
		for (int i = 0; i < 5; i++) {

			if (row[i] == SFSS_18 || row[i] == LJSS_28) {
				row[i] = WDSS_0;
			}
			if (row[i] == CLG_31 || (row[i] == YZBHS_37 || row[i] == DZH_6)) {
				row[i] = XRK_1;
			}
			if (row[i] == TDDL_4 || row[i] == WG_17) {
				row[i] = -1;
			}
		}
	}

	void add(double dps, int start) {
		// 计算啃食输出
		for (int i = start + 1; i <= 5; i++) {
			if (!isEmpty(row[i - 1]))
				bite[i] += BITE_DPS * dps;
		}

		// 计算行走输出
		for (int i = start + 1; i < 5; i++) {
			walk[i] += WALK_DPS * dps;
		}
		walk[5] += dps;
	}

	void addFume(double dps, int start) {
		for (int i = start + 1; i <= 5; i++) {
			if (!isEmpty(row[i - 1]))
				fumeBite[i] += BITE_DPS * dps;
		}
		for (int i = start + 1; i < 5; i++) {
			fumeWalk[i] += WALK_DPS * dps;
		}
		fumeWalk[5] += dps;
	}

	void addHB(int start) {
		add(1, start);
		for (int i = start + 1; i <= 5; i++) {
			biteSlowed[i] = true;
			walkSlowed[i] = true;
		}

		// 迟钝变速
		walkSlowed[5] = false;
	}

	void addXPG(int start) {
		// 前置僵尸的小喷菇计算
		if (mode == 0 || mode == 3) {
			for (int i = start + 1; i <= 5; i++) {
				if (i > start + 4) 
					break;
				if (!isEmpty(row[i - 1]))
					bite[i] += BITE_DPS;
			}
			for (int i = start + 1; i < 5; i++) {
				if (i <= start + 3) {
					walk[i] += WALK_DPS;
				}
				else {
					break;
				}
			}
			if (start >= 2) {
				walk[5] += 1;
			}
		}
		else {
			// 计算啃食输出
			for (int i = start + 1; i <= 5; i++) {
				if (i > start + 3)
					break;
				if (!isEmpty(row[i - 1]))
					bite[i] += BITE_DPS;
			}

			// 计算行走输出
			for (int i = start + 1; i < 5; i++) {
				if (i <= start + 2) {
					walk[i] += WALK_DPS;
				}
				else if (i == start + 3) {
					walk[i] += XPG_FIX;
				}
				else {
					break;
				}
			}
			if (start >= 2) {
				walk[5] += 1;
			}
		}
	}

	void addDPG(int start) {
		addFume(1, start);
		if (mode == 2) {
			addExtra(start);
		}
	}

	// 大喷菇对梯子饰品的附加伤害
	void addExtra(int start) {
		for (int i = start + 1; i <= 5; i++) {
			if (!isEmpty(row[i - 1]))
				extraBite[i] += BITE_DPS;
		}

		// 计算行走输出
		for (int i = start + 1; i < 5; i++) {
			extraWalk[i] += WALK_DPS;
		}
		extraWalk[5] += 1;
	}

	void addDC(int start) {
		fumeWalk[start] += DC_PASSBY;
		if (start == 0 && mode != 0) {
			fumeBite[start] += DC_BLOCK - 1;
		}
		else if (!isEmpty(row[start - 1]) && mode != 0) {
				fumeBite[start] += DC_BLOCK;
		}
	}

	void addYT(int start) {
		bite[start + 1] += 2 * YT_DPS;
	}

	void addYMTS(int start) {
		// 计算DPS
		for (int i = start + 1; i <= 5; i++) {
			if (!isEmpty(row[i - 1]))
				//kernalBite[i] += KERNAL_RATE * BITE_DPS;
				fumeBite[i] += KERNAL_RATE * BITE_DPS;
		}
		for (int i = start + 1; i < 5; i++) {
			//kernalWalk[i] += KERNAL_RATE * WALK_DPS;
			fumeWalk[i] += KERNAL_RATE * WALK_DPS;
		}
		fumeWalk[5] += KERNAL_RATE;

		// 增加黄油状态
		for (int i = start + 1; i <= 5; i++) {
			biteButter[i] += 1;
			walkButter[i] += 1;
		}
	}

	void addHS(int start) {
		int peaCount = 0;
		for (int i = 0; i < start; i++) {
			int diff = start - i;
			if (row[i] == WDSS_0) {
				peaCount++;
				if (start == indexOf(HJSZ_22)) {
					HSFix -= getHSFix(diff) * 2;
				}
			}
			else if (row[i] == SCSS_7) {
				peaCount += 2;
				if (start == indexOf(HJSZ_22)) {
					HSFix -= getHSFix(diff) * 4;
				}
			}
		}
		for (int i = start + 1; i < 6; i++) {
			biteSlowed[i] = false;
			walkSlowed[i] = false;
			if (peaCount > 0) {
				biteFire[i] = true;
				walkFire[i] = true;
			}
		}
		add(peaCount, start);
	}

	void addJG(int start) {
		wallnuts.insert(start);
	}

	void fixJG() {
		for (auto w : wallnuts) {
			bite[w + 1] *= 14;
			//kernalBite[w + 1] *= 14;
			fumeBite[w + 1] *= 14;
		}
	}

	void addPlants() {
		for (int j = 0; j < 5; j++) {
			switch (row[j]) {
				case WDSS_0:
					add(1, j);
					break;
				case HBSS_5:
					addHB(j);
					break;
				case SCSS_7:
					add(2, j);
					break;
				case XPG_8:
					addXPG(j);
					break;
				case DPG_10:
					addDPG(j);
					break;
				case DC_21:
					addDC(j);
					break;
				case YT_29:
					addYT(j);
					break;
				case YMTS_34:
					addYMTS(j);
					break;
				case HJSZ_22:
					addHS(j);
					break;
				case JG_3:
					addJG(j);
					break;
			}
		}
		fixJG();
	}

	// brain A B C D E
	// bite: from 0 to 5, corresponding to eating brain / A / B / C / D / E
	// walk: from 0 to 5, corresponding to after eating A / B / C / D / E, and walk[5] is 
	// the small interval from zombie placement to eating E

	int compute() {
		convert();
		addPlants();
		int torchwood = indexOf(HJSZ_22);

		// 计算减速滞留
		double sum = 0;
		set<int> HBfix;
		for (int i = 0; i <= walkLmt; i++) {
			if (i == 5) {
				break;
			}
			bool a = !walkSlowed[i];
			bool b = walkSlowed[i + 1];
			bool c = biteSlowed[i + 1];
			if (a && b) {
				HBfix.insert(i);
			}
			else if (a && c) {
				HBfix.insert(i);
			}
		}

		// 撑杆修正
		if (mode == 0) {
			if (biteLmt >= 0) {
				if (biteLmt - 1 >= 0 && (biteLmt - 1 < 5 && row[biteLmt - 1] == YT_29)) {
					bite[biteLmt] += (bite[biteLmt] - 10) * 0.25;
					fumeBite[biteLmt] *= 1.25;
				}
				else if (biteLmt - 1 >= 0 && (biteLmt - 1 < 5 && row[biteLmt - 1] == JG_3)) {
					bite[biteLmt] += (bite[biteLmt] / 14.0) * 0.25;
					fumeBite[biteLmt] *= 1.25;
				}
				else {
					bite[biteLmt] *= 1.25;
					fumeBite[biteLmt] *= 1.25;
				}
			}
		}

		for (int i = 0; i <= biteLmt; i++) {
			double biteDPS = bite[i] + fumeBite[i];
			if (biteSlowed[i]) {
				if (biteFire[i]) {
					biteDPS *= 1.33;
				}
				else {
					biteDPS *= 2;
				}
			}
			else if (HBfix.find(i) != HBfix.end()) {
				if (!biteFire[i]) {
					if (mode == 3) {
						if (wallnuts.find(i - 1) != wallnuts.end()) {
							biteDPS += biteDPS / 14.0 * 0.5;
						}
						else {
							biteDPS *= 1.5;
						}
					}
				}
			}
			biteDPS *= getButterRate(biteButter[i]);
			if (torchwood != -1 && i == torchwood + 1) {
				biteDPS += HSFix;
			}
			sum += biteDPS;
		}

		for (int i = 0; i <= walkLmt; i++) {
			double walkDPS = walk[i] + fumeWalk[i];
			if (walkSlowed[i]) {
				if (walkFire[i]) {
					walkDPS *= 1.33;
				}
				else {
					walkDPS *= 2;
				}
			}
			else if (HBfix.find(i) != HBfix.end()) {
				if (!walkFire[i]) {
					if (mode == 3) {
						walkDPS *= 2;
					}
					else {
						walkDPS *= 1.875;
					}
				}
			}
			else if (HBfix.find(i + 1) != HBfix.end()) {
				// 考虑橄榄减速滞留的问题
				// 例如 单发 地刺 冰豆, 橄榄在地刺上会被多扎3下 （总DPS*2）
				if (!walkFire[i] 
					&& mode == 3
					&& i + 1 <= biteLmt
					&& bite[i + 1] + fumeBite[i + 1] == 0) {
					walkDPS *= 2;
				}
			}
			walkDPS *= getButterRate(walkButter[i]);
			sum += walkDPS;
		}
		return int(round(sum));
	}

	Pair computeLadder() {
		if (mode == 4) {

			// 若4列无法被啃食或5列为坚果或5列为空，则不计算撑杆梯子
			if (!isBitable(row[3]) || (row[4] == JG_3 || isEmpty(row[4]))) {
				return Pair{ -1, -1, true };
			}
			else {
				// 从walk3开始算血
				biteLmt = 3;
				walkLmt = 3;
			}
		}
		double sum = 0;
		double fumeSum = 0;
		if (mode == 4) {
			sum += getDPS(row[4]);
			fumeSum += getFumeDPS(row[4]);
		}
		convert();
		addPlants();
		bool hasLadder = true;
		int wallnut = indexOf(JG_3);
		int torchwood = indexOf(HJSZ_22);
		int walkOnWallnut = false;
		int ladderLost = -1;

		// 减速滞留
		set<int> HBfix;
		for (int i = 0; i <= walkLmt; i++) {
			if (i == 5) {
				break;
			}
			if (!walkSlowed[i] && walkSlowed[i + 1]) {
				HBfix.insert(i);
			}
		}

		// 伤害域向左修正
		walk[5] = 0;
		fumeWalk[5] = 0;
		extraWalk[5] = 0;

		// 算血
		for (int i = 5; i >= 0; i--) {

			// 行走伤害
			if (i <= walkLmt) {
				if (walkOnWallnut) {	// 若走过搭梯坚果，则所有walk改为1.33倍 (1.5->2）
					walk[i] *= 1.33;
					fumeWalk[i] *= 1.33;
					walkOnWallnut = false;
				}
				else if (!hasLadder) {	// 无梯子时，行走伤害改为2.67倍 (1.5->4)
					walk[i] *= 2.67;
					fumeWalk[i] *= 2.67;
				}
				double walkDPS = walk[i];
				if (hasLadder) {
					walkDPS += extraWalk[i];
				}
				double walkFumeDPS = fumeWalk[i];
				walkDPS *= getButterRate(walkButter[i]);
				walkFumeDPS *= getButterRate(walkButter[i]);

				// 计算过程中失去扶梯导致的附加伤害
				double offset = 0;
				if (hasLadder && sum + walkDPS >= 25) {
					double hasLadderPct = (25 - sum) / walkDPS;
					offset = (sum + walkDPS - 25 - extraWalk[i] * (1 - hasLadderPct)) * 2.67;
					walkFumeDPS += walkFumeDPS * (1 - hasLadderPct) * 1.67;
					sum = 25;
					walkDPS = 0;
					hasLadder = false;
					ladderLost = i;
				}
				else if (!hasLadder) {
					offset = walkDPS;
					walkDPS = 0;
				}
				if (!hasLadder) {
					if (walkSlowed[i]) {
						if (walkFire[i]) {
							offset *= 1.33;
							walkFumeDPS *= 1.33;
						}
						else {
							offset *= 2;
							walkFumeDPS *= 2;
						}
					}
					else if (HBfix.find(i) != HBfix.end()) {
						if (!walkFire[i] && ladderLost < i + 2) {
							offset *= 1.875;
							walkFumeDPS *= 1.875;
						}
					}
					walkFumeDPS += offset;
				}
				sum += walkDPS;
				fumeSum += walkFumeDPS;
			}

			// 啃食伤害
			if (i <= biteLmt) {
				if (wallnut != -1 && (wallnut + 1 == i && hasLadder)) {
					hasLadder = false;
					ladderLost = i;
					walkOnWallnut = true;
					continue;
				}
				double biteDPS = bite[i];
				if (hasLadder) {
					biteDPS += extraBite[i];
				}
				double biteFumeDPS = fumeBite[i];
				biteDPS *= getButterRate(biteButter[i]);
				biteFumeDPS *= getButterRate(biteButter[i]);

				// 火树修正
				if (torchwood != -1 && i == torchwood + 1) {
					biteDPS += HSFix;
				}

				// 计算过程中失去扶梯导致的附加伤害
				double offset = 0;
				if (hasLadder && sum + biteDPS >= 25) {
					double hasLadderPct = (25 - sum) / biteDPS;
					offset = sum + biteDPS - 25 - extraBite[i] * (1 - hasLadderPct);
					sum = 25;
					biteDPS = 0;
					hasLadder = false;
					ladderLost = i;
				}
				else if (!hasLadder) {
					offset = biteDPS;
					biteDPS = 0;
				}
				if (!hasLadder) {
					if (biteSlowed[i]) {
						if (biteFire[i]) {
							offset *= 1.33;
							biteFumeDPS *= 1.33;
						} else {
							offset *= 2;
							biteFumeDPS *= 2;
						}
					}
					biteFumeDPS += offset;
				}
				sum += biteDPS;
				fumeSum += biteFumeDPS;
			}
		}
		return Pair{ (int)(round(sum)), (int)(round(fumeSum)), false };
	}
};

struct Puzzle {
	int** puzzle;
	string** result;
	int** highlight;    // -1:不推荐; 0:普通; 1:推荐;
	bool checkAll;		// 是否检查邻路杨桃输出

	Puzzle(int** arr, bool isQX) {
		checkAll = isQX;
		puzzle = new int* [5];
		result = new string * [5];
		highlight = new int* [5];
		for (int i = 0; i < 5; i++) {
			puzzle[i] = new int[5];
			result[i] = new string[5];
			highlight[i] = new int[5];
		}
		for (int i = 0; i < 5; i++)
			for (int j = 0; j < 5; j++) {
				puzzle[i][j] = arr[i][j];
				result[i][j] = "";
				highlight[i][j] = 0;
			}
		analyze();
	}

	~Puzzle() {
		for (int i = 0; i < 5; i++) {
			delete[] highlight[i];
			delete[] puzzle[i];
			delete[] result[i];
		}
		delete[] puzzle;
		delete[] highlight;
		delete[] result;
	}

	void analyze() {
		if (!checkAll) {
			for (int i = 0; i < 5; i++) {
				bool hasHightlight = false;

				// 算撑杆
				Row r = Row(puzzle[i], 0);
				bool hasMagnet = r.hasMagnet();
				int temp = r.compute();
				result[i][0] = to_string(temp);
				if (!r.canPV || temp > 39) {
					highlight[i][0] = -1;
				}
				else if (temp <= 15) {
					highlight[i][0] = 1;
					hasHightlight = true;
				}

				// 算慢速
				Row r1 = Row(puzzle[i], 1);
				temp = r1.compute();
				result[i][1] = to_string(temp);
				if (!hasHightlight) {
					if (temp <= 25) {
						highlight[i][1] = 1;
						hasHightlight = true;
					}
					else if (temp <= 61 && !hasMagnet) {
						highlight[i][1] = 1;
						hasHightlight = true;
					}
				}
				if (temp > 72) {
					highlight[i][1] = -1;
				}

				// 算梯子
				Row r2 = Row(puzzle[i], 2);
				Pair pr2 = r2.computeLadder();
				result[i][2] = to_string(pr2.x) + "+" + to_string(pr2.y);
				if (!hasHightlight) {
					if (pr2.y <= 14 && !hasMagnet) {
						highlight[i][2] = 1;
						hasHightlight = true;
					}
				}
				if (hasMagnet || pr2.y >= 19) {
					highlight[i][2] = -1;
				}

				// 算橄榄
				Row r3 = Row(puzzle[i], 3);
				temp = r3.compute();
				result[i][3] = to_string(temp);
				if (!hasHightlight) {
					if (temp <= 76 && !hasMagnet) {
						highlight[i][3] = 1;
						hasHightlight = true;
					}
				}
				if (temp > 84 || hasMagnet) {
					highlight[i][3] = -1;
				}

				// 算撑杆梯子
				Row r4 = Row(puzzle[i], 4);
				Pair pr4 = r4.computeLadder();
				if (!pr4.empty) {
					result[i][4] = to_string(pr4.x) + "+" + to_string(pr4.y);
					if (!hasHightlight) {
						if (pr4.y <= 14 && !hasMagnet) {
							highlight[i][4] = 1;
							hasHightlight = true;
						}
					}
					if (hasMagnet || pr4.y >= 19) {
						highlight[i][4] = -1;
					}
				}
			}
		}
		else {
			// 先计算每路杨桃的额外输出
			int* extra = new int[5];
			for (int i = 0; i < 5; i++)
				extra[i] = 0;
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++)
					if (puzzle[i][j] == YT_29)
						updateExtra(extra, i, j);
			}

			// 算血
			for (int i = 0; i < 5; i++) {
				Row r = Row(puzzle[i], 1);
				bool hasMagnet = r.hasMagnet();
				int rawTemp = r.compute();
				int temp = rawTemp + extra[i];
				result[i][1] = to_string(temp);
				int starNum = r.getStarNum();
				if (temp <= 63 - 6*starNum && !hasMagnet) {
					highlight[i][1] = 1;
				}
				else if (temp >= 65) {
					highlight[i][1] = -1;
				}
				result[i][2] = to_string(rawTemp);
				if (rawTemp <= 25) {
					highlight[i][2] = 1;
				}
				else if (rawTemp >= 28) {
					highlight[i][2] = -1;
				}
			}
			delete[] extra;
		}

	}

	void updateExtra(int* extra, int i, int j) {
		for (int k = 0; k < 5; k++) {
			if (i == k) {
				continue;
			}
			int diff = abs(k - i);

			// https://www.bilibili.com/read/cv6263782
			if ((j == 0 || j == 1) && diff == 1) {
				extra[k] += 13;
				if (isEmpty(puzzle[k][j + 1]))
					extra[k] -= 2;
				if (isEmpty(puzzle[k][j + 2]))
					extra[k] -= 2;
			}
			else if ((j == 0 && diff == 2) || (j == 2 && diff == 1)) {
				extra[k] += 11;
				if (isEmpty(puzzle[k][3]))
					extra[k] -= 2;
				if (isEmpty(puzzle[k][4]))
					extra[k] -= 2;
			}
			else if ((j == 1 && diff == 2) || (j == 3 && diff == 1)) {
				extra[k] += 5;
				if (isEmpty(puzzle[k][4]))
					extra[k] -= 2;
			}
			else {
				extra[k] += 2;
			}
			if (k == i - 1) {
				extra[k] += 1;
			}
		}
	}
};