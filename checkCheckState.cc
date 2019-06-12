/**
 * csa形式の盤面ファイルを読み込んで
 * 詰みの時にその後の終局までの読み筋を表示する
 * ファイルを指定しない時は最大手数(23手)の局面の読み筋を表示する．
 */
#include "allStateTable.h"
#include "dobutsu.h"
#include "winLoseTable.h"
#include <fstream>

void usage() { std::cerr << "Usage: checkCheckState csafile" << std::endl; }

int main(int ac, char **ag) {
  if (ac < 2) {
    AllStateTable allS("allstates.dat");
    WinLoseTable winLoseCheck(allS, "winLossCheck.dat",
                              "winLossCheckCount.dat");
    int maxwlc = -1;
    int maxwlc_index = -1;
    for (size_t i = 0; i < allS.size(); i++) {
      int wlc = winLoseCheck.getWinLoseCount(i);
      if (wlc > maxwlc) {
        maxwlc = wlc;
        maxwlc_index = i;
      }
    }
    State s(allS[maxwlc_index], BLACK);
    winLoseCheck.showSequence(s);
    return 0;
  }
  std::ifstream ifs(ag[1]);
  std::string all;
  std::string line;
  while (getline(ifs, line)) {
    all += line;
  }
  State s(all);
  AllStateTable allS("allstates.dat");
  WinLoseTable winLoseCheck(allS, "winLossCheck.dat", "winLossCheckCount.dat");
  winLoseCheck.showSequence(s);
}
