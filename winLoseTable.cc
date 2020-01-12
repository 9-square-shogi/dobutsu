#include "winLoseTable.h"
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>

WinLoseTable::WinLoseTable(AllStateTable const &allS_, string const &wlName,
                           string const &wlcName, bool lazy)
    : allS(allS_) {
  struct stat st1, st2;
  if (stat(wlName.c_str(), &st1) < 0 || stat(wlcName.c_str(), &st2) < 0) {
    assert(0);
  }
  if (allS.size() != st1.st_size || st1.st_size != st2.st_size) {
    std::cerr << "File size miss match " << wlName << " <=> " << wlcName
              << std::endl;
    assert(0);
  }
  ifs1.open(wlName.c_str());
  ifs2.open(wlcName.c_str());
  if (!lazy) {
    winLose.resize(st1.st_size);
    winLoseCount.resize(st2.st_size);
    ifs1.read((char *)&winLose[0], st1.st_size);
    ifs2.read((char *)&winLoseCount[0], st2.st_size);
    ifs1.close();
    ifs2.close();
  }
}

WinLoseTable::~WinLoseTable() {
  if (winLose.size() == 0) {
    ifs1.close();
  }
  if (winLoseCount.size() == 0) {
    ifs2.close();
  }
}

int WinLoseTable::getWinLose(State const &s, int &wlc) const {
#if 0
  if(s.turn==WHITE){
    State rev_s=s.rotateReverse();
    rev_s.changeTurn();
    uint64 v=rev_s.normalize();
    int index=allS.find(v);
    if(!(0<=index && index<allS.size())){
      std::cerr << s << std::endl;
      std::cerr << rev_s << std::endl;
      std::cerr << "v=" << v << std::endl;
      std::cerr << "index=" << index << std::endl;
    }
    assert(0<=index && index<allS.size());
    wlc=winLoseCount[index];
    return -winLose[index];
  }
  else{
    uint64 v=s.normalize();
    int index=allS.find(v);
    assert(0<=index && index<allS.size());
    wlc=winLoseCount[index];
    return winLose[index];
  }
#else
  uint64 v = s.normalize();
  int index = allS.find(v);
  assert(0 <= index && index < allS.size());
  wlc = getWinLoseCount(index);
#if 0
  if(s.turn==BLACK)
  else
    return -winLose[index];
#else
  return getWinLose(index);
#endif
#endif
}
int WinLoseTable::getWinLose(State const &s, Move const &move, int &wlc) const {
  State news(s);
  news.applyMove(move);
  return getWinLose(news, wlc);
}

void WinLoseTable::showSequence(State const &s, int current_wlc) const {
  if (!s.isConsistent())
    throw InconsistentException();
  uint64 v = s.normalize();
  int index = allS.find(v);
  if (current_wlc == -1)
    current_wlc = getWinLoseCount(index);
  if (current_wlc == 0 && getWinLose(index) == 0) {
    vInt pastStates;
    showDrawSequence(s, pastStates);
    return;
  }
  std::cerr << "------------------" << std::endl;
  std::cerr << s << std::endl;
  std::cerr << (int)(s.turn == BLACK ? getWinLose(index) : -getWinLose(index))
            << "(" << current_wlc << ")" << std::endl;
  if (current_wlc == 0)
    return;
  if (getWinLose(index) == 0) {
    std::cerr << s << std::endl;
    std::cerr << "winLose=0" << std::endl;
    throw InconsistentException();
  }
  vMove moves = s.nextMoves();
#if PERPETUAL_CHECK
  bool isPerpetual = true;
#endif
  for (size_t i = 0; i < moves.size(); i++) {
    int wl, wlc;
    wl = getWinLose(s, moves[i], wlc);
    std::cerr << i << " : " << moves[i] << " " << wl << "(" << wlc << ")"
              << std::endl;
#if PERPETUAL_CHECK
    if (current_wlc - 1 == wlc)
      isPerpetual = false;
#endif
  }
  for (size_t i = 0; i < moves.size(); i++) {
    int wl, wlc;
    wl = getWinLose(s, moves[i], wlc);
    if (getWinLose(index) == -1) {
      if (wl != 1) {
        //	throw InconsistentException();
      }
      if (
#if PERPETUAL_CHECK
          isPerpetual ? (current_wlc - 1 < wlc) :
#endif
                      (current_wlc - 1 == wlc)) {
        std::cerr << "Move : " << moves[i] << " " << wl << "(" << wlc << ")"
                  << std::endl;
        State news(s);
        news.applyMove(moves[i]);
        showSequence(news, current_wlc - 1);
        break;
      }
#if !(PERPETUAL_CHECK)
      else if (current_wlc - 1 < wlc) {
        throw InconsistentException();
      }
#endif
    } else {
      assert(getWinLose(index) == 1);
      if (wl == -1) {
        if (getWinLoseCount(index) - 1 == wlc) {
          std::cerr << "Move : " << moves[i] << " " << wl << "(" << wlc << ")"
                    << std::endl;
          State news(s);
          news.applyMove(moves[i]);
          showSequence(news, current_wlc - 1);
          break;
        } else if (getWinLoseCount(index) - 1 > wlc) {
          throw InconsistentException();
        }
      }
    }
  }
}

void WinLoseTable::showDrawSequence(State const &s, vInt &pastStates) const {
  uint64 v = s.normalize();
  int index = allS.find(v);
  std::cerr << "------------------" << std::endl;
  std::cerr << s << std::endl;
  std::cerr << (int)(s.turn == BLACK ? getWinLose(index) : -getWinLose(index))
            << "(" << (int)getWinLoseCount(index) << ")" << std::endl;
  if (std::find(pastStates.begin(), pastStates.end(), index) !=
      pastStates.end())
    return;
  pastStates.push_back(index);
  vMove moves = s.nextMoves();
  int draw_index = -1;
  int draw_wl = -1;
  int draw_wlc = -1;
  for (size_t i = 0; i < moves.size(); i++) {
    int wl, wlc;
    wl = getWinLose(s, moves[i], wlc);
    std::cerr << i << " : " << moves[i] << " " << wl << "(" << wlc << ")"
              << std::endl;
    if (wl == 0 && draw_index == -1) {
      draw_index = i;
      draw_wl = wl;
      draw_wlc = wlc;
    }
  }
  std::cerr << "Move : " << moves[draw_index] << " " << draw_wl << "("
            << draw_wlc << ")" << std::endl;
  State news(s);
  news.applyMove(moves[draw_index]);
  showDrawSequence(news, pastStates);
}
