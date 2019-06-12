#ifndef _DOBUTSU_H
#define _DOBUTSU_H
#include <algorithm>
#include <cassert>
#include <complex>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
using namespace std;
typedef complex<int> Point;

typedef unsigned long long uint64;
typedef vector<uint64> vUint64;
typedef vector<char> vChar;
typedef unsigned char uchar;
typedef vector<uchar> vuChar;
typedef vector<short> vShort;
typedef vector<int> vInt;
// typedef map<uint64,int> sVals;

const int width = 3;
const int height = 3;
const int num_squares = width * height;
const int num_ptypes = 5;
const int num_ptypes_in_hand = 4;

#define DROP_RULE 1
#define PROMOTION 1
#define STALEMATE_DRAW 0
#define TRY_RULE 0

#define DEAD_PIECE 1
#define DOUBLE_PAWN 1
#define PAWN_DROP_MATE 1
#define PERPETUAL_CHECK 1

/**
 * ファイル入力等でフォーマット異常を発見した時にthrowする例外
 */
struct FormatException {};

/**
 * 局面等で不整合が見つかった時にthrowする例外
 */
struct InconsistentException {};

/**
 * プレイヤー
 * 囲碁にならって先手番をBLACK, 後手番をWHITEとする．
 */
enum Player { BLACK = 1, WHITE = -1 };
/**
 * 手番の反転
 */
static inline Player alt(Player pl) {
  return static_cast<Player>(-static_cast<int>(pl));
}
static int makePtypeChar(char c0, char c1);
/**
 * 駒の種類とplayerを表す整数を管理するためのクラス
 * 対応するオブジェクトはない
 */
struct Ptype {
  enum {
    EMPTY,
    PAWN,
    SILVER,
    PPAWN,
    PSILVER,
    KING,
  };
  /**
   * ptypeが負の時はWHITEの駒を表す
   */
  static int makePtype(Player p, int type) { return p * type; }
  /**
   * 文字列 s のindex文字目からの3文字を見てptypeを返す
   */
  static int makePtype(string const &s, int index);
  /**
   * 2文字の駒シンボル
   * HI, ZO, KI, NI, LI
   * との対応
   */
  static const char *strs[];
  static string str(int type) {
    if (type == EMPTY)
      return " . ";
    string pstr = "+";
    if (type < 0) {
      pstr = "-";
      type = -type;
    }
    if (type > num_ptypes)
      throw InconsistentException();
    return pstr + strs[type];
  }
};
/**
 * 2文字のcharから駒のtypeを作る
 */
static inline int makePtypeChar(char c0, char c1) {
  for (int i = 1; i <= num_ptypes; i++)
    if (c0 == Ptype::strs[i][0] && c1 == Ptype::strs[i][1])
      return i;
  std::cerr << "c0=" << c0 << ",c1=" << c1 << std::endl;
  throw FormatException();
  return -1;
}

/**
 * 文字列 s のindex文字目からの3文字を見てptypeを返す
 */
inline int Ptype::makePtype(string const &s, int index) {
  if (index + 3 > s.length())
    throw FormatException();
  if (s[index + 1] == '.') {
    if (s[index] == ' ' && s[index + 2] == ' ')
      return Ptype::EMPTY;
    throw FormatException();
  }
  Player pl;
  if (s[index] == '+')
    pl = BLACK;
  else {
    if (s[index] != '-')
      throw FormatException();
    pl = WHITE;
  }
  int ptype = makePtypeChar(s[index + 1], s[index + 2]);
  return makePtype(pl, ptype);
}

/**
 * 8近傍のベクトル
 */
extern const Point directions[8];
/**
 * 駒ごとにどの方向に動けるかを記録したもの
 */
extern const int canMoves[];

/**
 * 盤面のx,yから1次元のPositionを作る
 * ここのx,y は将棋流で
 * (2,0) (1,0) (0,0)
 * (2,1) (1,1) (0,1)
 * (2,2) (1,2) (0,2)
 * (2,3) (1,3) (0,3)
 * となっているがなぜかは聞かないで
 */
static inline int makePosition(int x, int y) {
  assert(0 <= x && x < width);
  assert(0 <= y && y < height);
  return x * height + y;
}
/**
 * 駒台を表すPosition
 */
extern const int STAND;
/**
 * Positionからxを取り出す
 */
static inline int pos2X(int pos) {
  assert(0 <= pos && pos < num_squares);
  return pos / height;
}
/**
 * Positionからyを取り出す
 */
static inline int pos2Y(int pos) {
  assert(0 <= pos && pos < num_squares);
  return pos % height;
}
/**
 * Positionを180度回転する
 * (2-x)*4+(3-y)=8+3-(x*4+y)=11-pos
 */
static inline int rot180(int pos) {
  assert(0 <= pos && pos < num_squares);
  return num_squares - 1 - pos;
}
/**
 * 文字2文字からpositionを作る
 * '0' '0' はSTANDを表すことにする
 */
static inline int makePosition(char c1, char c2) {
  if (c1 == '0' && c2 == '0')
    return STAND;
  if ('A' <= c1 && c1 < 'A' + width) {
    int x = width - 1 - (c1 - 'A');
    if ('1' <= c2 && c2 < '1' + height) {
      int y = c2 - '1';
      return x * height + y;
    }
  }
  throw FormatException();
}
/**
 * 指し手を表すクラス
 * 手番も入る．
 */
struct Move {
  // bit 0-7 to
  // bit 8-15 from if stand 0ff
  // bit 16-19 ptype - move後のptype
  // bit 31 - if player is black 0 else 1
  int v;
  Move(string const &str);
  static int makeMove(Player pl, int from, int to, int ptype) {
    assert(pl == BLACK || pl == WHITE);
    assert(0 <= ptype && ptype < 16);
    assert((0 <= from && from < num_squares) || from == 255);
    assert(0 <= to && to < num_squares);
    return (pl == BLACK ? 0 : (1 << 31)) | (ptype << 16) | (from << 8) | to;
  }
  Move(Player pl, int from, int to, int ptype) {
    v = makeMove(pl, from, to, ptype);
  }
  int from() const { return (v >> 8) & 255; }
  int to() const { return v & 255; }
  Player player() const { return ((v & (1 << 31)) != 0 ? WHITE : BLACK); }
  int ptype() const { return (v >> 16) & 255; }
  Move rotateChangeTurn() const {
    if (from() == STAND)
      return Move(alt(player()), STAND, rot180(to()), ptype());
    else
      return Move(alt(player()), rot180(from()), rot180(to()), ptype());
  }
};
bool operator==(Move const &m1, Move const &m2);
typedef vector<Move> vMove;
vMove readMoveFile(string const &fileName);
ostream &operator<<(ostream &os, Move const &m);
/**
 * 局面を表すクラス
 */
struct State {
  /**
   * 盤面の12マスに入るptype
   */
  char board[num_squares];
  /**
   * それぞれのプレイヤーの駒台の駒の個数
   * 2 * 3
   */
  int stands[num_ptypes_in_hand];
  /**
   * 次の手番のプレイヤー(BLACK or WHITE)
   */
  int turn;
  /**
   * 初期配置を作る
   */
  State() {
    *this = State(" .  . -OU"
                  " .  .  . "
                  "+OU .  . "
                  "1111"
                  "+");
  }
  /**
   * packした状態からplayerを指定して作る
   */
  State(uint64 p, Player pl = BLACK) {
    if (pl == BLACK)
      *this = makeBlackFromUint64(p);
    else
      *this = makeBlackFromUint64(p).rotateChangeTurn();
  }
  /**
   * CSA風の表記から作る
   */
  State(string const &s);
  /**
   * packした状態から黒番で作る
   * bit 59 - 48 : 48+2*jビットからの2ビットづつで stands[j]を表す
   * bit 47 - 0  : 4*posビットからの4ビットずつで board[pos] を表す
   *             : ptypeの下位4ビットなのでbit3がsetされている時は，
   *             : WHITE用に補正
   */
  static State makeBlackFromUint64(uint64 p) {
    State s;
    int i = 0;
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++) {
        char c = (p >> (i * 4)) & 15;
        if ((c & 8) != 0)
          c -= 16;
        s.board[x * height + y] = c;
        i++;
      }
    for (int j = 0; j < num_ptypes_in_hand; j++) {
      s.stands[j] = (p >> (48 + j * 2)) & 3;
    }
    s.turn = BLACK;
    return s;
  }
  /**
   * 盤面を180度回転してturnを反転したStateを作る
   */
  State rotateChangeTurn() const {
    State ret;
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++)
        ret.board[x * height + y] =
            -board[(width - 1 - x) * height + height - 1 - y];
    for (int i = 0; i < num_ptypes_in_hand / 2; i++) {
      ret.stands[i + num_ptypes_in_hand / 2] = stands[i];
      ret.stands[i] = stands[i + num_ptypes_in_hand / 2];
    }
    ret.turn = -turn;
    return ret;
  }
  /**
   * turnを反転
   */
  void changeTurn() { turn = -turn; }
  /**
   * 左右を反転したStateを作る
   */
  State flip() const {
    State ret(*this);
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++)
        ret.board[x * height + y] = board[(width - 1 - x) * height + y];
    return ret;
  }
  /**
   * 黒番の盤面を64ビット整数に変換
   */
  uint64 pack() const {
    assert(turn == BLACK);
    uint64 ret = 0ull;
    int i = 0;
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++) {
        ret |= static_cast<uint64>(board[x * height + y] & 15) << (i * 4);
        i++;
      }
    for (int j = 0; j < num_ptypes_in_hand; j++)
      ret |= static_cast<uint64>(stands[j]) << (48 + j * 2);
    return ret;
  }
  /**
   * 黒番に正規化した上で64ビット整数に変換し，flipしたものと小さい方を返す
   */
  uint64 normalize() const {
    if (turn == BLACK) {
      uint64 u1 = pack();
      uint64 u2 = flip().pack();
      return std::min(u1, u2);
    } else {
      State news = rotateChangeTurn();
      uint64 u1 = news.pack();
      uint64 u2 = news.flip().pack();
      return std::min(u1, u2);
    }
  }
  /**
   * 指定されたPlayerのlionを捜す
   */
  Point lion(Player p) const {
    char pLion = Ptype::makePtype(p, Ptype::KING);
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++)
        if (pLion == board[x * height + y])
          return Point(x, y);
    std::cerr << *this << std::endl;
    std::cerr << "no lion" << std::endl;
    throw InconsistentException();
    return Point(0, 0);
  }
  /**
   * Point pが盤面の内部かどうかを判定する
   */
  static bool isInside(Point p) {
    return 0 <= p.real() && p.real() < width && 0 <= p.imag() &&
           p.imag() < height;
  }
  /**
   * 黒番のptypeがdir方向に動けるかどうかを判定する
   */
  static bool canMove(char ptype, int dir) {
    return ((1 << dir) & canMoves[ptype - 1]) != 0;
  }
  /**
   * 黒番の盤面が手番の勝ちかどうかを判定する
   */
  bool isWinByBlack() const {
    assert(turn == BLACK);
    // can capture the opponent's lion
    Point pLion = lion(WHITE);
    //    std::cout << "pLion=" << pLion << std::endl;
    for (int dir = 0; dir < 8; dir++) {
      Point pos = pLion - directions[dir];
      if (!isInside(pos))
        continue;
      char ptype = board[pos.real() * height + pos.imag()];
      //      std::cout << "dir=" << dir << ",pos=" << pos << ",ptype=" << ptype
      //      << std::endl;
      if (ptype > 0 && canMove(ptype, dir))
        return true;
    }
    return false;
  }
  /**
   * 盤面が手番の勝ちかどうかを判定する
   */
  bool isWin() const {
    if (turn == BLACK)
      return isWinByBlack();
    return rotateChangeTurn().isWinByBlack();
  }
  bool isCheck() const {
    if (turn == BLACK)
      return rotateChangeTurn().isWinByBlack();
    return isWinByBlack();
  }
  /**
   * 黒番の盤面が手番の負けかどうかを判定する
   * isWinでないことはチェック済みとする
   */
  bool isLoseByBlack() const {
    assert(turn == BLACK);
    assert(!isWin());
#if TRY_RULE
    // can capture the opponent's lion
    Point pLion = lion(WHITE);
    return pLion.imag() == height - 1;
#else
    return false;
#endif
  }
  /**
   * 盤面が手番の負けかどうかを判定する
   * isWinでないことはチェック済みとする
   */
  bool isLose() const {
    if (turn == BLACK)
      return isLoseByBlack();
    return rotateChangeTurn().isLoseByBlack();
  }
#if STALEMATE_DRAW
  bool isStalemate() const {
    if (isWin() || isCheck())
      return false;
    vUint64 ns = nextStates();
    if (!ns.size())
      return true;
    for (size_t j = 0; j < ns.size(); j++) {
      State news(ns[j]);
      if (!news.isWin())
        return false;
    }
    return true;
  }
#endif
  /**
   * 駒の数があっていることを確認する
   */
  bool isConsistent() const {
    vInt counts(num_ptypes + 1, 0);
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++) {
        char ptype = board[x * height + y];
        if (ptype < 0)
          ptype = -ptype;
#if PROMOTION
        if (ptype == Ptype::PPAWN)
          ptype = Ptype::PAWN;
        else if (ptype == Ptype::PSILVER)
          ptype = Ptype::SILVER;
#endif
        if (ptype > 0)
          counts[ptype]++;
      }
    if (counts[Ptype::KING] != 2)
      return false;
#if DROP_RULE
    if (counts[Ptype::PAWN] >= 0 && stands[0] >= 0 &&
        stands[num_ptypes_in_hand / 2] >= 0 &&
        counts[Ptype::PAWN] + stands[0] + stands[num_ptypes_in_hand / 2] != 2)
      return false;
    if (counts[Ptype::SILVER] >= 0 && stands[1] >= 0 &&
        stands[num_ptypes_in_hand / 2 + 1] >= 0 &&
        counts[Ptype::SILVER] + stands[1] +
                stands[num_ptypes_in_hand / 2 + 1] !=
            2)
      return false;
#endif
    return true;
  }
#if 0
  /**
   *
   */
  vUint64 nextStates() const{
    vUint64 ret;
    for(int x=0;x<3;x++)
      for(int y=0;y<4;y++){
	Point pos(x,y);
	char ptype=board[x*4+y];
	if(ptype==0){
	  for(int i=0;i<3;i++)
	    if(stands[i]>0){
	      State s(*this);
	      s.board[x*4+y]=i+Ptype::BABY;
	      s.stands[i]--;
	      s.changeTurn();
	      ret.push_back(s.normalize());
	    }
	}
	if(ptype<=0){
	  for(int dir=0;dir<8;dir++){
	    Point pos1=pos-directions[dir];
	    if(!isInside(pos1)) continue;
	    char ptype1=board[pos1.real()*4+pos1.imag()];
	    if(ptype1>0 && canMove(ptype1,dir)){
	      if(ptype1==Ptype::BABY && y==0) ptype1=Ptype::CHICKEN;
	      State s(*this);
	      s.board[x*4+y]=ptype1;
	      s.board[pos1.real()*4+pos1.imag()]=Ptype::EMPTY;
	      if( ptype < 0 ){
		if(-ptype==Ptype::CHICKEN)
		  s.stands[0]++;
		else
		  s.stands[-ptype-Ptype::BABY]++;
	      }
	      s.changeTurn();
	      ret.push_back(s.normalize());
	    }
	  }
	}
      }
    return ret;
  }
#else
  vUint64 nextStates() const {
    vMove moves = nextMoves();
    vUint64 ret;
    for (size_t i = 0; i < moves.size(); i++) {
      State news(*this);
      news.applyMove(moves[i]);
      ret.push_back(news.normalize());
    }
    return ret;
  }
#endif
#if PAWN_DROP_MATE
  bool isPawnDropMate(int x, int y) const {
    Point pLion(x, y - 1);
    for (int dir = 0; dir < 8; dir++) {
      Point pos = pLion - directions[dir];
      if (!isInside(pos))
        continue;
      char ptype = board[pos.real() * height + pos.imag()];
      if (ptype < 0)
        continue;
      for (int dir1 = 0; dir1 < 8; dir1++) {
        Point pos1 = pos - directions[dir1];
        if (!isInside(pos1))
          continue;
        char ptype1 = board[pos1.real() * height + pos1.imag()];
        if (ptype1 > 0 && canMove(ptype1, dir1))
          goto HELL;
      }
      return false;
    HELL:;
    }
    State rev_s = rotateChangeTurn();
    Point pos(width - 1 - x, height - 1 - y);
    for (int dir = 0; dir < 8; dir++) {
      Point pos1 = pos - directions[dir];
      if (!isInside(pos1))
        continue;
      char ptype = rev_s.board[pos1.real() * height + pos1.imag()];
      if (ptype > 0 && ptype != Ptype::KING && canMove(ptype, dir))
        return false;
    }
    return true;
  }
#endif
  /**
   * 黒番のstateで合法手をすべて作る
   * isWin, isLoseでは呼ばない
   */
  vMove nextMovesForBlack() const {
    assert(turn == BLACK);
    assert(!isWin());
    assert(!isLose());
    vMove ret;
    for (int x = 0; x < width; x++)
      for (int y = 0; y < height; y++) {
        Point pos(x, y);
        char ptype = board[x * height + y];
        if (ptype == 0) {
          for (int i = 0; i < num_ptypes_in_hand / 2; i++)
            if (stands[i] > 0) {
              if (i == 0) {
#if DEAD_PIECE
                if (y == 0)
                  continue;
#endif
#if DOUBLE_PAWN
                for (int y1 = 0; y1 < height; y1++)
                  if (y1 != y && board[x * height + y1] == Ptype::PAWN)
                    goto HELL;
#endif
#if PAWN_DROP_MATE
                if (board[x * height + y - 1] == -Ptype::KING &&
                    isPawnDropMate(x, y))
                  continue;
#endif
              }
              ret.push_back(Move(BLACK, STAND, makePosition(x, y), i + 1));
#if DOUBLE_PAWN
            HELL:;
#endif
            }
        }
        if (ptype <= 0) {
          for (int dir = 0; dir < 8; dir++) {
            Point pos1 = pos - directions[dir];
            if (!isInside(pos1))
              continue;
            char ptype1 = board[pos1.real() * height + pos1.imag()];
            if (ptype1 > 0 && canMove(ptype1, dir)) {
#if PROMOTION
              if (y == 0 || pos1.imag() == 0) {
                if (ptype1 == Ptype::PAWN)
                  ret.push_back(Move(BLACK,
                                     makePosition(pos1.real(), pos1.imag()),
                                     makePosition(x, y), Ptype::PPAWN));
                else if (ptype1 == Ptype::SILVER)
                  ret.push_back(Move(BLACK,
                                     makePosition(pos1.real(), pos1.imag()),
                                     makePosition(x, y), Ptype::PSILVER));
              }
#endif
#if DEAD_PIECE
              if (ptype1 == Ptype::PAWN && y == 0)
                continue;
#endif
              ret.push_back(Move(BLACK, makePosition(pos1.real(), pos1.imag()),
                                 makePosition(x, y), ptype1));
            }
          }
        }
      }
    return ret;
  }
  /**
   * stateで合法手をすべて作る
   * isWin, isLoseでは呼ばない
   */
  vMove nextMoves() const {
    if (turn == BLACK)
      return nextMovesForBlack();
    else {
      State rev_s = rotateChangeTurn();
      vMove rev = rev_s.nextMovesForBlack();
      vMove ret;
      for (size_t i = 0; i < rev.size(); i++)
        ret.push_back(rev[i].rotateChangeTurn());
      return ret;
    }
  }
  /**
   * moveがあるstateでvalidかどうかを判定する．
   * generateしてからそのメンバーであることで判定するという
   * 遅い方法
   */
  bool isValidWithGenerate(Move const &move) const {
    vMove ret = nextMoves();
    for (size_t i = 0; i < ret.size(); i++)
      if (ret[i] == move)
        return true;
    return false;
  }
  /**
   * moveがstateでvalidかどうかを判定する．
   * ruleによって可能なことを前提にする　
   */
  bool isValid(Move const &move) const {
    if (move.player() != turn)
      return false;
    int ptype = move.ptype();
    int capture_ptype = board[move.to()];
    if (move.from() == STAND) {
      if (stands[(turn == BLACK ? 0 : num_ptypes_in_hand / 2) + ptype - 1] == 0)
        return false;
      return capture_ptype == Ptype::EMPTY;
    } else {
      int from_ptype = board[move.from()];
      if (turn == WHITE)
        from_ptype = -from_ptype;
      //      std::cerr << "from_ptype=" << from_ptype << std::endl;
      if (from_ptype <= 0)
        return false;
      if (from_ptype != ptype) {
#if PROMOTION
        if (((ptype != Ptype::PPAWN || from_ptype != Ptype::PAWN) &&
             (ptype != Ptype::PSILVER || from_ptype != Ptype::SILVER)) ||
            (pos2Y(move.to()) != (turn == BLACK ? 0 : height - 1) &&
             pos2Y(move.from()) != (turn == BLACK ? 0 : height - 1)))
#endif
          return false;
      }
      return (turn == BLACK ? capture_ptype <= 0 : capture_ptype >= 0);
    }
  }
  /**
   * Stateにmoveをapplyして変化させる
   */
  void applyMove(Move const &move) {
    assert(isValid(move));
    assert(isValidWithGenerate(move));
    if (!isValid(move) || !isValidWithGenerate(move)) {
      std::cerr << *this << std::endl;
      std::cerr << "invalidMove " << move << std::endl;
      throw InconsistentException();
    }
    if (move.from() == STAND) {
      int index =
          (turn == BLACK ? 0 : num_ptypes_in_hand / 2) + move.ptype() - 1;
      assert(0 <= index && index < num_ptypes_in_hand);
      stands[index]--;
    } else {
      assert(move.ptype() == abs(board[move.from()])
#if PROMOTION
             || (move.ptype() == Ptype::PPAWN &&
                 abs(board[move.from()]) == Ptype::PAWN) ||
             (move.ptype() == Ptype::PSILVER &&
              abs(board[move.from()]) == Ptype::SILVER)
#endif
                 );
      board[move.from()] = Ptype::EMPTY;
    }
    int ptype = move.ptype();
    if (turn == WHITE)
      ptype = -ptype;
#if DROP_RULE
    int capture_ptype = board[move.to()];
    if (turn == BLACK)
      capture_ptype = -capture_ptype;
    assert(capture_ptype >= 0);
    if (capture_ptype != Ptype::EMPTY) {
#if PROMOTION
      if (capture_ptype == Ptype::PPAWN)
        capture_ptype = Ptype::PAWN;
      else if (capture_ptype == Ptype::PSILVER)
        capture_ptype = Ptype::SILVER;
#endif
      stands[(turn == BLACK ? 0 : num_ptypes_in_hand / 2) + capture_ptype -
             1]++;
    }
#endif
    board[move.to()] = ptype;
    changeTurn();
    if (!isConsistent()) {
      std::cerr << *this << std::endl;
      throw InconsistentException();
    }
  }
  friend ostream &operator<<(ostream &os, State const &s);
  friend bool operator==(State const &s1, State const &s2);
};
bool operator==(State const &s1, State const &s2);
bool operator!=(State const &s1, State const &s2);
#endif
