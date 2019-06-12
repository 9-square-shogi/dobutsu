/**
 * countReachable
 * 初期状態から引き分けのままで到達可能な状態数をカウントする
 * obsolete : 初期状態が引き分けでないと分かったのでこのプログラムは意味がない．
 */
#include "allStateTable.h"
#include "dobutsu.h"
#include "winLoseTable.h"

void searchRec(int index, vChar &visited, WinLoseTable const &winLose,
               int &count, int &maxLose) {
  if (visited[index] != 0)
    return;
  count++;
  visited[index] = 1;
  assert(winLose.getWinLose(index) == 0);
  State s(winLose.getAllS()[index]);
  vMove moves = s.nextMoves();
  for (size_t i = 0; i < moves.size(); i++) {
    State s1(s);
    s1.applyMove(moves[i]);
#if 0
    State s2=s1.rotateReverse();
    s2.changeTurn();
    int index1=winLose.getAllS().find(s2.normalize());
#else
    int index1 = winLose.getAllS().find(s1.normalize());
#endif
    if (winLose.getWinLose(index1) == 0) {
      searchRec(index1, visited, winLose, count, maxLose);
    } else {
      assert(winLose.getWinLose(index1) == 1);
      int lose = winLose.getWinLoseCount(index1);
      if (lose > maxLose) {
        std::cerr << "lose=" << lose << std::endl;
        std::cerr << s << std::endl;
        std::cerr << "move=" << moves[i] << std::endl;
        for (size_t j = 0; j < moves.size(); j++) {
          int wlc, wl;
          wl = winLose.getWinLose(s, moves[j], wlc);
          if (wl == 0)
            std::cerr << moves[j] << ",wl=" << wl << "(" << wlc << ")"
                      << std::endl;
        }
        maxLose = lose;
      }
    }
  }
}

int main(int ac, char **ag) {
  AllStateTable allS("allstates.dat");
  WinLoseTable winLose(allS, "winLoss.dat", "winLossCount.dat");
  vChar visited(allS.size(), 0);
  int count = 0;
  int maxLose = 0;
  int maxLoseIndex = 0;
  State s;
  int initialIndex = allS.find(s.normalize());
  searchRec(initialIndex, visited, winLose, count, maxLose);
  std::cout << "count=" << count << std::endl;
}
