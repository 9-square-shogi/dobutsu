#ifndef _ALL_STATE_TABLE_H
#define _ALL_STATE_TABLE_H
#include "dobutsu.h"
#include <string>
using namespace std;

class AllStateTable {
  vUint128 contents;
  size_t c_size;
  mutable ifstream ifs;

public:
  AllStateTable(size_t size) : contents(size), c_size(size) {}
  AllStateTable(string const &fileName, bool lazy = true);
  ~AllStateTable();
  size_t size() const { return c_size; }
  /**
   * vが見つかれば0以上のインデックスを返す．
   * 見つからなければ-1を返す
   */
  int find(uint128 v) const;
  /**
   * read only のaccessのみを考える．
   */
  const uint128 operator[](size_t i) const {
    if (contents.size() > 0) {
      return contents[i];
    } else {
      ifs.seekg(i * sizeof(uint128), ios_base::beg);
      uint128 ret;
      ifs.read((char *)&ret, sizeof(uint128));
      return ret;
    }
  }
};
#endif
