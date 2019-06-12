/**
 * 最長の勝ちを見つける．
 */
#include "allStateTable.h"
#include "dobutsu.h"
#include "winLoseTable.h"
#include <fstream>

void usage() { std::cerr << "Usage: checkcsa csafile" << std::endl; }

int main(int ac, char **ag) {
  AllStateTable allS("allstates.dat");
  WinLoseTable winLose(allS, "winLoss.dat", "winLossCount.dat");
  int maxwin = 0;
  vInt maxwin_indices;
  for (size_t i = 0; i < allS.size(); i++) {
    if (winLose.getWinLose(i) > 0) {
      int win = winLose.getWinLoseCount(i);
      if (win >= maxwin) {
        if (win > maxwin) {
          maxwin = win;
          maxwin_indices.clear();
        }
        maxwin_indices.push_back(i);
      }
    }
  }
  std::cerr << "maxwin=" << maxwin << std::endl;
  for (auto i : maxwin_indices) {
    State s(allS[i], BLACK);
    std::cerr << "----------" << std::endl;
    std::cerr << s << std::endl;
  }
  //   WinLoseTable winLose(allS,"winLossWoDrop.dat","winLossWoDropCount.dat");
  //  winLose.showSequence(s);
}
