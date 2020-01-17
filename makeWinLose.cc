#include "allStateTable.h"
#include "dobutsu.h"

int newWinLoss(AllStateTable const &allIS, vChar const &winLoss, uint64 v) {
  State s(v);
  vUint64 ns = s.nextStates();
  bool alllose = true;
  for (size_t j = 0; j < ns.size(); j++) {
    int i1 = allIS.find(ns[j]);
    assert(0 <= i1 && i1 < allIS.size());
    if (winLoss[i1] == -1)
      return 1;
    if (winLoss[i1] != 1)
      alllose = false;
  }
  if (alllose)
    return -1;
  else
    return 0;
}
#if PERPETUAL_CHECK
bool isPerpetualCheck(AllStateTable const &allIS, vChar const &winLoss,
                      vChar &isNotPerpetual, uint64 v, vInt &pastStates) {
  State s(v);
  vUint64 ns = s.nextStates();
  for (size_t j = 0; j < ns.size(); j++) {
    int i1 = allIS.find(ns[j]);
    if (winLoss[i1] != 0)
      continue;
    vInt::iterator it = find(pastStates.begin(), pastStates.end(), i1);
    if (it != pastStates.end()) {
      for (; it < pastStates.end(); it += 2) {
        if (!State(allIS[*it]).isCheck())
          return false;
      }
      continue;
    }
    vInt pastStates1(pastStates);
    pastStates1.push_back(i1);
    State s1(allIS[i1]);
    vUint64 ns1 = s1.nextStates();
    for (size_t j1 = 0; j1 < ns1.size(); j1++) {
      int i2 = allIS.find(ns1[j1]);
      if (winLoss[i2] != 0)
        continue;
      if (isNotPerpetual[i2])
        continue;
      vInt::iterator it1 = find(pastStates1.begin(), pastStates1.end(), i2);
      if (it1 != pastStates1.end()) {
        for (it1++; it1 < pastStates1.end(); it1 += 2) {
          if (!State(allIS[*it1]).isCheck())
            return false;
        }
        goto HELL;
      }
      vInt pastStates2(pastStates1);
      pastStates2.push_back(i2);
      if (isPerpetualCheck(allIS, winLoss, isNotPerpetual, allIS[i2],
                           pastStates2))
        goto HELL;
      isNotPerpetual[i2] = 1;
    }
    return false;
  HELL:;
  }
  return true;
}

int newWinLossCountRecursive(AllStateTable const &allIS, vChar const &winLoss,
                             vChar const &winLossCount,
                             vChar const &isNotPerpetual, uint64 v,
                             vInt &pastStates) {
  State s(v);
  vUint64 ns = s.nextStates();
  int maxwlc = -1;
  for (size_t j = 0; j < ns.size(); j++) {
    int i1 = allIS.find(ns[j]);
    if (winLoss[i1] != 0) {
      if (winLossCount[i1] > maxwlc)
        maxwlc = winLossCount[i1];
      continue;
    }
    vInt::iterator it = find(pastStates.begin(), pastStates.end(), i1);
    if (it != pastStates.end()) {
      for (; it < pastStates.end(); it += 2) {
        if (!State(allIS[*it]).isCheck())
          return numeric_limits<int>::max();
      }
      continue;
    }
    vInt pastStates1(pastStates);
    pastStates1.push_back(i1);
    State s1(allIS[i1]);
    vUint64 ns1 = s1.nextStates();
    int minwlc = numeric_limits<int>::max();
    for (size_t j1 = 0; j1 < ns1.size(); j1++) {
      int i2 = allIS.find(ns1[j1]);
      if (winLoss[i2] != 0)
        continue;
      if (isNotPerpetual[i2])
        continue;
      vInt::iterator it1 = find(pastStates1.begin(), pastStates1.end(), i2);
      if (it1 != pastStates1.end()) {
        for (it1++; it1 < pastStates1.end(); it1 += 2) {
          if (!State(allIS[*it1]).isCheck())
            goto HELL;
        }
        goto HEAVEN;
      HELL:
        continue;
      }
      vInt pastStates2(pastStates1);
      pastStates2.push_back(i2);
      int wlc = newWinLossCountRecursive(
          allIS, winLoss, winLossCount, isNotPerpetual, allIS[i2], pastStates2);
      if (wlc < minwlc)
        minwlc = wlc;
    }
    if (minwlc == numeric_limits<int>::max())
      return numeric_limits<int>::max();
    if (minwlc + 1 > maxwlc)
      maxwlc = minwlc + 1;
  HEAVEN:;
  }
  return maxwlc + 1;
}

int newWinLossCount(AllStateTable const &allIS, vChar const &winLoss,
                    vChar const &winLossCount, uint64 v, int wl) {
  State s(v);
  vUint64 ns = s.nextStates();
  if (!ns.size())
    return -1;
  int wlc = wl == -1 ? -1 : numeric_limits<int>::max();
  for (size_t j = 0; j < ns.size(); j++) {
    int i1 = allIS.find(ns[j]);
    if (winLoss[i1] != -wl)
      continue;
    if (wl == -1 ? (winLossCount[i1] > wlc) : (winLossCount[i1] < wlc))
      wlc = winLossCount[i1];
  }
  return wlc + 1;
}
#endif
int main() {
  AllStateTable allIS("allstates.dat", false);
  int dSize = allIS.size();
  vChar winLoss(dSize, 0);
  vChar winLossCount(dSize, 0);
  vInt count(3);
  for (size_t i = 0; i < dSize; i++) {
    State s(allIS[i]);
    if (s.isWin()) {
      winLoss[i] = 1;
      count[2]++;
    } else if (s.isLose()) {
      winLoss[i] = -1;
      count[0]++;
    } else {
#if STALEMATE_DRAW
      if (s.isStalemate())
        winLoss[i] = 2;
#endif
      count[1]++;
    }
  }
  for (int c = 1;; c++) {
    vChar winLossNew(winLoss);
    std::cout << "iteration " << c << std::endl;
    for (int j = 0; j < 3; j++) {
      std::cout << (j - 1) << " : " << count[j] << std::endl;
    }
    bool changed = false;
    for (size_t i = 0; i < dSize; i++) {
      if (winLoss[i] == 0) {
        int nv = newWinLoss(allIS, winLoss, allIS[i]);
        if (nv != 0) {
          winLossNew[i] = nv;
          winLossCount[i] = c;
          count[nv + 1]++;
          count[1]--;
          changed = true;
        }
      }
    }
    winLoss.swap(winLossNew);
    if (changed == false)
      break;
  }
#if STALEMATE_DRAW
  for (int c = 1;; c++) {
    std::cout << "iteration " << c << std::endl;
    bool changed = false;
    for (size_t i = 0; i < dSize; i++) {
      if (winLoss[i] == 0) {
        State s(allIS[i]);
        vUint64 ns = s.nextStates();
        for (size_t j = 0; j < ns.size(); j++) {
          int i1 = allIS.find(ns[j]);
          if (winLoss[i1] == 2) {
            winLoss[i] = 2;
            changed = true;
            break;
          }
        }
      }
    }
    if (changed == false)
      break;
  }
#endif
#if PERPETUAL_CHECK
  vChar winLossOld(winLoss);
  vChar isPerpetual(dSize, 0);
  vChar isNotPerpetual(dSize, 0);
  for (size_t i = 0; i < dSize; i++) {
    if (!isNotPerpetual[i] && winLoss[i] == 0) {
      vInt pastStates;
      pastStates.push_back(i);
      if (isPerpetualCheck(allIS, winLossOld, isNotPerpetual, allIS[i],
                           pastStates)) {
        isPerpetual[i] = 1;
        winLoss[i] = -1;
        count[0]++;
        count[1]--;
      } else {
        isNotPerpetual[i] = 1;
      }
    }
  }
  for (size_t i = 0; i < dSize; i++) {
    if (isPerpetual[i]) {
      vInt pastStates;
      pastStates.push_back(i);
      winLossCount[i] =
          newWinLossCountRecursive(allIS, winLossOld, winLossCount,
                                   isNotPerpetual, allIS[i], pastStates);
    }
  }
  for (int c = 1;; c++) {
    vChar winLossNew(winLoss);
    std::cout << "iteration " << c << std::endl;
    for (int j = 0; j < 3; j++) {
      std::cout << (j - 1) << " : " << count[j] << std::endl;
    }
    bool changed = false;
    for (size_t i = 0; i < dSize; i++) {
      if (winLoss[i] == 0) {
        int nv = newWinLoss(allIS, winLoss, allIS[i]);
        if (nv != 0) {
          winLossNew[i] = nv;
          winLossCount[i] =
              newWinLossCount(allIS, winLoss, winLossCount, allIS[i], nv);
          count[nv + 1]++;
          count[1]--;
          changed = true;
        }
      }
    }
    winLoss.swap(winLossNew);
    if (changed == false)
      break;
  }
  for (int c = 1;; c++) {
    std::cout << "iteration " << c << std::endl;
    bool changed = false;
    for (size_t i = 0; i < dSize; i++) {
      if (isPerpetual[i]) {
        vInt pastStates;
        pastStates.push_back(i);
        int wlc =
            newWinLossCountRecursive(allIS, winLossOld, winLossCount,
                                     isNotPerpetual, allIS[i], pastStates);
        if (wlc != winLossCount[i]) {
          winLossCount[i] = wlc;
          changed = true;
        }
      } else if (winLoss[i] != 0) {
#if STALEMATE_DRAW
        if (winLoss[i] == 2)
          continue;
#endif
        State s(allIS[i]);
        if ((winLoss[i] == 1 && s.isWin()) || (winLoss[i] == -1 && s.isLose()))
          continue;
        int wlc =
            newWinLossCount(allIS, winLoss, winLossCount, allIS[i], winLoss[i]);
        if (wlc != -1 && wlc != winLossCount[i]) {
          winLossCount[i] = wlc;
          changed = true;
        }
      }
    }
    if (changed == false)
      break;
  }
/*
  for (size_t i = 0; i < dSize; i++) {
    if (winLoss[i] != 0) {
#if STALEMATE_DRAW
      if (winLoss[i] == 2)
        continue;
#endif
      if (!isPerpetual[i]) {
        State s(allIS[i]);
        if ((winLoss[i] == 1 && s.isWin()) || (winLoss[i] == -1 && s.isLose()))
          continue;
      }
      int wlc =
          newWinLossCount(allIS, winLoss, winLossCount, allIS[i], winLoss[i]);
      if (wlc == -1 || winLossCount[i] == wlc)
        continue;
      std::cout << "------------------" << std::endl;
      std::cout << State(allIS[i]) << std::endl;
      std::cout << (int)winLossCount[i] << " != " << wlc << std::endl;
      std::cout << (int)winLoss[i] << std::endl;
      std::cout << (isPerpetual[i] ? "true" : "false") << std::endl;
    }
  }
*/
#endif
#if STALEMATE_DRAW
  for (size_t i = 0; i < dSize; i++)
    if (winLoss[i] == 2)
      winLoss[i] = 0;
#endif
  {
    ofstream ofs("winLoss.dat");
    ofs.write((char *)&winLoss[0], winLoss.size());
  }
  {
    ofstream ofs("winLossCount.dat");
    ofs.write((char *)&winLossCount[0], winLossCount.size());
  }
}
