#include "dobutsu.h"

#include <ext/hash_set>
typedef deque<uint128> dUint128;
struct hashUint128 {
  int operator()(uint128 const &v) const {
    return (v >> 64) ^ (v & 0xffffffffffffffff);
  }
};
typedef __gnu_cxx::hash_set<uint128, hashUint128> hUint128;

int main() {
  vUint128 allIS;
  dUint128 q;
  hUint128 v;
  q.push_back(State().normalize());
  while (!q.empty()) {
    uint128 is = q.front();
    q.pop_front();
    hUint128::const_iterator it = v.find(is);
    if (it == v.end()) {
      State s(is, BLACK);
      //      std::cout << s << ",isWin==" << s.isWin()  << std::endl;
      allIS.push_back(is);
      v.insert(is);
      if (!s.isWin() && !s.isLose()) {
        vUint128 ns = s.nextStates();
        for (size_t i = 0; i < ns.size(); i++)
          q.push_back(ns[i]);
      }
    }
  }
  cout << v.size() << endl;
  sort(allIS.begin(), allIS.end());
  ofstream ofs("allstates.dat");
  ofs.write((char *)&allIS[0], allIS.size() * sizeof(uint128));
}
