#include "dobutsu.h"

// clang-format off
const char *Ptype::strs[] = {
    0,
    "FU",
    // "KY",
    // "KE",
    // "GI",
    // "KI",
    "KA",
    "HI",
    "TO",
    // "NY",
    // "NK",
    // "NG",
    "UM",
    "RY",
    "OU",
};
// clang-format on
const Point directions8[8] = {Point(1, -1), Point(0, -1), Point(-1, -1),
                              Point(1, 0),  Point(-1, 0), Point(1, 1),
                              Point(0, 1),  Point(-1, 1)};
const Point directions[24] = {
    Point(2, -2), Point(1, -2), Point(0, -2), Point(-1, -2), Point(-2, -2),
    Point(2, -1), Point(1, -1), Point(0, -1), Point(-1, -1), Point(-2, -1),
    Point(2, 0),  Point(1, 0),  Point(-1, 0), Point(-2, 0),  Point(2, 1),
    Point(1, 1),  Point(0, 1),  Point(-1, 1), Point(-2, 1),  Point(2, 2),
    Point(1, 2),  Point(0, 2),  Point(-1, 2), Point(-2, 2)};
const int canMoves[] = {
    0b00000'00000'0000'00100'00000, // pawn
    // 0b00000'00000'0000'00100'00100, // lance
    // 0b00000'00000'0000'00000'01010, // knight
    // 0b00000'01010'0000'01110'00000, // silver
    // 0b00000'00100'0110'01110'00000, // gold
    0b10001'01010'0000'01010'10001, // bishop
    0b00100'00100'1111'00100'00100, // rook
    0b00000'00100'0110'01110'00000, // promoted pawn
    // 0b00000'00100'0110'01110'00000, // promoted lance
    // 0b00000'00100'0110'01110'00000, // promoted knight
    // 0b00000'00100'0110'01110'00000, // promoted silver
    0b10001'01110'0110'01110'10001, // promoted bishop
    0b00100'01110'1111'01110'00100, // promoted rook
    0b00000'01110'0110'01110'00000, // king
};

const int STAND = 0xff;

Move::Move(string const &str) {
  if (str.length() != 7) {
    assert(0);
  }
  Player pl = BLACK;
  if (str[0] == '+')
    pl = BLACK;
  else if (str[0] == '-')
    pl = WHITE;
  else
    assert(0);
  int from = makePosition(str[1], str[2]);
  int to = makePosition(str[3], str[4]);
  int ptype = makePtypeChar(str[5], str[6]);
  //  std::cerr << "pl=" << pl << ",from=" << from << ",to=" << to << ",ptype="
  //  << static_cast<int>(ptype) << std::endl;
  v = makeMove(pl, from, to, ptype);
}

bool operator==(Move const &m1, Move const &m2) { return m1.v == m2.v; }

vMove readMoveFile(string const &fileName) {
  vMove ret;
  ifstream ifs(fileName.c_str());
  for (;;) {
    string s;
    if (!(ifs >> s))
      break;
    if (s.length() != 7)
      break;
    ret.push_back(Move(s));
  }
  return ret;
}

ostream &outPosition(ostream &os, int pos) {
  if (pos == 0xff)
    return os << "00";
  int x = pos / height, y = pos % height;
  assert(0 <= x && x < width);
  assert(0 <= y && y < height);
  return os << "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[width - 1 - x] << "123456789"[y];
}
ostream &outPtype(ostream &os, int ptype) {
  assert(1 <= ptype && ptype <= num_ptypes);
  return os << Ptype::strs[ptype];
}

ostream &operator<<(ostream &os, Move const &m) {
  if (m.player() == BLACK)
    os << '+';
  else
    os << '-';
  outPosition(os, m.from());
  outPosition(os, m.to());
  outPtype(os, m.ptype());
  return os;
}

State::State(string const &s) {
  int eos = 3 * num_squares + num_ptypes_in_hand;
  assert(s.length() == eos + 1);
  for (int x = 0; x < width; x++)
    for (int y = 0; y < height; y++)
      board[x * height + y] =
          (char)Ptype::makePtype(s, y * width * 3 + (width - 1 - x) * 3);
  for (int i = 0; i < num_ptypes_in_hand; i++)
    stands[i] = s[3 * num_squares + i] - '0';
  if (s[eos] == '+')
    turn = BLACK;
  else {
    if (s[eos] != '-')
      throw FormatException();
    turn = WHITE;
  }
}

ostream &operator<<(ostream &os, State const &s) {
  for (int y = 0; y < height; y++) {
    for (int x = width - 1; x >= 0; x--)
      os << Ptype::str(s.board[x * height + y]);
    os << "\n";
  }
  for (int i = 0; i < num_ptypes_in_hand; i++)
    os << s.stands[i];
  os << "\n";
  if (s.turn == BLACK)
    os << "+\n";
  else
    os << "-\n";
  return os;
}

bool operator==(State const &s1, State const &s2) {
  if (s1.turn != s2.turn)
    return false;
  for (int i = 0; i < num_squares; i++)
    if (s1.board[i] != s2.board[i])
      return false;
  for (int i = 0; i < num_ptypes_in_hand; i++)
    if (s1.stands[i] != s2.stands[i])
      return false;
  return true;
}
bool operator!=(State const &s1, State const &s2) { return !(s1 == s2); }
