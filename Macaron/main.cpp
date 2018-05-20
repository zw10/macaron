#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <assert.h>

/*
list of namespaces: weights, misc, gen, debug, eval, srch

weights:	piece values, pst, etc.
misc:		console i/o
gen:		generator code
debug:		debugging tools
eval:		evaluation code
src:		search code
ui:			user interface
*/

//in global namespace
bool DEBUG = false;

const int POS_INFINITY = 99999;
const int NEG_INFINITY = -99999;
const int DRAW_VALUE = 0;
const int SEARCH_DEPTH = 7;
const int QSEARCH_DEPTH = 20;
const int REDUCTION_DEPTH = 2;
const int RANDOM_RANGE = 5;
//int legalMoveCount = 0;

enum Colour//
{//COLOUR
	BLACK = -1,
	WHITE = 1
};

enum Piece
{//PIECE
	X = 0,//illegal
	Z = 1, //empty
	BP = -2,
	BN = -3,
	BB = -4,
	BR = -5,
	BQ = -6,
	BK = -7,
	WP = 2,
	WN = 3,
	WB = 4,
	WR = 5,
	WQ = 6,
	WK = 7
};

int board2sixtyfour[120] =
{//120 to 64 mailbox
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
	-1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
	-1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	-1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	-1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	-1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	-1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	-1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

namespace weights
{
	const int MAX_GAME_LENGTH = 512;

	enum Value
	{//PIECE_VALUES	
		PAWN = 100,
		KNIGHT = 320,
		BISHOP = 345,
		ROOK = 510,
		QUEEN = 960,
		KING = 99999
	};

	//PIECE SQUARE TABLES
	int BPT[120] =
	{//BLACK PAWN TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,-34,-34,-34,-34,-34,-34,-34,-34,  0,
		0, -8, -5, -7,-16,-16, -7, -5, -8,  0,
		0,  0,  8,  4,-13,-13,  0,  8,  0,  0,
		0, 13, 13,  0, -8, -8, 18, 13, 13,  0,
		0,  8, 13, 13,  5,  5, 13, 13,  8,  0,
		0,  8,  8, 22, 25, 25,  3,  8,  8,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WPT[120] =
	{//WHITE PAWN TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, -8, -8,-22,-25,-25, -3, -8, -8,  0,
		0, -8,-13,-13, -5, -5,-13,-13, -8,  0,
		0,-13,-13,  0,  8,  8,-18,-13,-13,  0,
		0,  0, -8, -4, 13, 13,  0, -8,  0,  0,
		0,  8,  5,  7, 16, 16,  7,  5,  8,  0,
		0, 34, 34, 34, 34, 34, 34, 34, 34,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int BNT[120] =
	{//BLACK KNIGHT TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, 24,  9,  9,  7,  4,  5,  7, 24,  0,
		0, 19,  9, -6, -6, -6, -6,  7, 16,  0,
		0,  9, -6,-10,-13,-13,-10, -6,  6,  0,
		0,  7,-11,-12,-15,-15,-12,-11,  6,  0,
		0,  8, -6,-11,-15,-15,-11, -6,  7,  0,
		0, 12, -8,-10,-13,-13,-11, -8, 12,  0,
		0, 19, 14, -6, -8, -8, -6, 14, 14,  0,
		0, 29, 15, 16, 16, 16, 16, 16, 16,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WNT[120] =
	{//WHITE KNIGHT TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,-29,-25,-16,-16,-16,-16,-16,-16,  0,
		0,-19,-14,  6,  8,  8,  6,-14,-14,  0,
		0,-12,  8, 10, 13, 13, 11,  8,-12,  0,
		0, -8,  6, 11, 15, 15, 11,  6, -7,  0,
		0, -7, 11, 12, 15, 15, 12, 11, -6,  0,
		0, -9,  6, 10, 13, 13, 10,  6, -6,  0,
		0,-19, -9,  6,  6,  6,  6, -7,-16,  0,
		0,-24, -9, -9, -7, -4, -5, -7,-24,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int BBT[120] =
	{//BLACK BISHOP TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, 20, 10, 10, 10, 10, 10, 10, 20,  0,
		0, 10,  0,  0,  0,  0,  0,  0, 10,  0,
		0, 10,  0, -5,-10,-10, -5,  0, 10,  0,
		0, 10, -5, -5,-10,-10, -5, -5, 10,  0,
		0, 10,  0,-10,-10,-10,-10,  0, 10,  0,
		0, 10, -5,-10,-10,-10,-10, -5, 10,  0,
		0,  5, -6,  0,  0,  0,  0, -6,  5,  0,
		0,  5, 10, 10, 10, 10, 10, 10,  5,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WBT[120] =
	{//WHITE BISHOP TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, -5,-10,-10,-10,-10,-10,-10, -5,  0,
		0, -5,  6,  0,  0,  0,  6, 10, -5,  0,
		0,-10,  5, 10, 10, 10, 10,  5,-10,  0,
		0,-10,  0, 10, 10, 10, 10,  0,-10,  0,
		0,-10,  5,  5, 10, 10,  5,  5,-10,  0,
		0,-10,  0,  5, 10, 10,  5,  0,-10,  0,
		0,-10,  0,  0,  0,  0,  0,  0,-10,  0,
		0,-20,-10,-10,-10,-10,-10,-10,-20,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int BRT[120] =
	{//BLACK ROOK TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, -8, -8, -8, -8, -8, -8, -8, -8,  0,
		0,-15,-10,-20,-20,-20,-20,-20,-15,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0, -7, -7, -7, -7, -7, -7,  0,  0,
		0,  1, -5, -5, -6, -6, -5, -5,  1,  0,
		0,  1,  0, -4, -6, -6, -4,  0,  1,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WRT[120] =
	{//WHITE ROOK TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, -1,  0,  4,  6,  6,  4,  0, -1,  0,
		0, -1,  5,  5,  6,  6,  5,  5, -1,  0,
		0,  0,  7,  7,  7,  7,  7,  7,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, 15, 20, 20, 20, 20, 20, 20, 15,  0,
		0,  8,  8,  8,  8,  8,  8,  8,  8,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int BQT[120] =
	{//BLACK QUEEN TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, 20, 10, 10,  5,  5, 10, 10, 20,  0,
		0, 10,  0,  0,  0,  0,  0,  0, 10,  0,
		0, 10,  0, -5, -5, -5, -5,  0, 10,  0,
		0,  5,  0, -5, -5, -5, -5,  0,  5,  0,
		0,  0,  0, -5, -5, -5, -5,  0,  5,  0,
		0, 10, -5, -5, -5, -5, -5,  0, 10,  0,
		0, 10,  0, -5,  0,  0,  0,  0, 10,  0,
		0,  5, 10, 10,  5,  5, 10, 10, 20,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WQT[120] =
	{//WHITE QUEEN TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, -5,-10,-10, -5, -5,-10,-10,-20,  0,
		0,-10,  0,  5,  0,  0,  0,  0,-10,  0,
		0,-10,  5,  5,  5,  5,  5,  0,-10,  0,
		0,  0,  0,  5,  5,  5,  5,  0, -5,  0,
		0, -5,  0,  5,  5,  5,  5,  0, -5,  0,
		0,-10,  0,  5,  5,  5,  5,  0,-10,  0,
		0,-10,  0,  0,  0,  0,  0,  0,-10,  0,
		0,-20,-10,-10, -5, -5,-10,-10,-20,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int BKT[120] =
	{//BLACK KING TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, 30, 40, 40, 50, 50, 40, 40, 30,  0,
		0, 30, 40, 40, 50, 50, 40, 40, 30,  0,
		0, 30, 40, 40, 50, 50, 40, 40, 30,  0,
		0, 30, 40, 40, 50, 50, 40, 40, 30,  0,
		0, 25, 30, 30, 40, 40, 30, 30, 25,  0,
		0, 18, 23, 23, 23, 23, 23, 23, 18,  0,
		0, 15, 15, 18, 20, 20, 18, 15, 15,  0,
		0, 15,  0,  0,  0,  0,  0,  0, 15,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int WKT[120] =
	{//WHITE KING TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,-15,  0,  0,  0,  0,  0,  0,-15,  0,
		0,-15,-15,-18,-20,-20,-18,-15,-15,  0,
		0,-18,-23,-23,-23,-23,-23,-23,-18,  0,
		0,-25,-30,-30,-40,-40,-30,-30,-25,  0,
		0,-30,-40,-40,-50,-50,-40,-40,-30,  0,
		0,-30,-40,-40,-50,-50,-40,-40,-30,  0,
		0,-30,-40,-40,-50,-50,-40,-40,-30,  0,
		0,-30,-40,-40,-50,-50,-40,-40,-30,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};

	int ENDGAME_KING[120] =
	{//WHITE KING TABLE
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,-20,-15,-14,-13,-13,-14,-15,-20,  0,
		0,-15,  1,  5,  3,  3,  5,  1,-15,  0,
		0,-14,  5,  8, 10, 10,  8,  5,-14,  0,
		0,-13,  3, 10, 12, 12, 10,  3,-13,  0,
		0,-13,  3, 10, 12, 12, 10,  3,-13,  0,
		0,-14,  5,  8, 10, 10,  8,  5,-14,  0,
		0,-15,  1,  5,  3,  3,  5,  1,-15,  0,
		0,-20,-15,-14,-13,-13,-14,-15,-20,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};
}

namespace gen
{
	int FILE[120] =
	{//corresponds to +- 1
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	int RANK[120] =
	{//corresponds to +- 10
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 3, 3, 3, 3, 3, 3, 3, 3, 0,
		0, 4, 4, 4, 4, 4, 4, 4, 4, 0,
		0, 5, 5, 5, 5, 5, 5, 5, 5, 0,
		0, 6, 6, 6, 6, 6, 6, 6, 6, 0,
		0, 7, 7, 7, 7, 7, 7, 7, 7, 0,
		0, 8, 8, 8, 8, 8, 8, 8, 8, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	int DIAG[120] =
	{//corresponds to +- 9
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 0,
		0, 2, 3, 4, 5, 6, 7, 8, 9, 0,
		0, 3, 4, 5, 6, 7, 8, 9,10, 0,
		0, 4, 5, 6, 7, 8, 9,10,11, 0,
		0, 5, 6, 7, 8, 9,10,11,12, 0,
		0, 6, 7, 8, 9,10,11,12,13, 0,
		0, 7, 8, 9,10,11,12,13,14, 0,
		0, 8, 9,10,11,12,13,14,15, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	int ANTI[120] =
	{//corresponds to +- 11
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 9, 8, 7, 6, 5, 4, 3, 2, 0,
		0,10, 9, 8, 7, 6, 5, 4, 3, 0,
		0,11,10, 9, 8, 7, 6, 5, 4, 0,
		0,12,11,10, 9, 8, 7, 6, 5, 0,
		0,13,12,11,10, 9, 8, 7, 6, 0,
		0,14,13,12,11,10, 9, 8, 7, 0,
		0,15,14,13,12,11,10, 9, 8, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	int DARK_LIGHT[120] =
	{//light is 0, dark is 1
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
		0, 1, 0, 1, 0, 1, 0, 1, 0, 0,
		0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	struct Move
	{//MOVE DECLARATION
		Move();
		bool operator==(const Move& other);
		bool operator!=(const Move& other);
		int from;
		int to;
		int priority;
		Piece captured;
		Piece promotion;
		bool isDoubleMove;//castling counts as double move
		bool isPawnMove;
	};

	Move::Move() : from(0), to(0), priority(NEG_INFINITY), captured(X), promotion(X),
		isDoubleMove(false), isPawnMove(false) {}

	bool Move::operator==(const Move& other)
	{
		return (from == other.from && to == other.to &&
			promotion == other.promotion);
	}

	bool Move::operator!=(const Move& other)
	{
		return (from != other.from || to != other.to ||
			promotion != other.promotion);
	}

	typedef std::vector<Move> MoveList;
	//MOVELIST DECLARATION

	struct PieceSquare
	{//PIECE_SQUARE DEFINITION
		PieceSquare();
		Piece piece;
		int square;
	};

	PieceSquare::PieceSquare() : piece(X), square(0) {}

	struct GameState
	{//GAME_STATE DECLARATION
		GameState();
		Colour toMove;
		int fifty;
		int epSquare;//defaults to 0
		bool blackCanOO;//cannot castle if rook has moved or captured!
		bool blackCanOOO;
		bool whiteCanOO;
		bool whiteCanOOO;
		//		Move lastMove;
	};

	GameState::GameState() : toMove(WHITE), fifty(0), epSquare(0),
		blackCanOO(false), blackCanOOO(false),
		whiteCanOO(false), whiteCanOOO(false)/*,
											 lastMove( Move() )*/ {}

	GameState* gameHistory = new GameState[weights::MAX_GAME_LENGTH];
	//GAMEHISTORY DECLARATION
	Move* movesMade;/* = new Move[weights::MAX_GAME_LENGTH];*/
					//MOVESMADE DECLARATION
	int moveCount = 0;

	struct Board
	{//BOARD DECLARATION
		Board();
		void display() const;
		void displayLists() const;
		void displayState() const;
		bool verify() const;
		void clear();
		void updateConverter();//called after pieces sorted in setFen()
		int setFen(const std::string& fen);
		void sortPieces(Colour colour);//only used upon initialisation
		void captureSort(Colour toMove, int index);//colour is for colour captured
		void promotionSort(Colour toMove, int index, Piece promotion, int square);//promotions also disturb order
		void castle(Colour toMove, int to, PieceSquare* fromPieces);
		void rookCaptured(Piece captured, int to);
		void rookOrKing(Piece moving, int from);//disabling castling moves
		void unCaptureSort(Colour moved, Piece captured, int to);
		void unPromotionSort(Colour moved, int from, int to);
		void unCastle(Colour moved, int to, PieceSquare* fromPieces);
		bool isOpen(int src, int dest, int disp) const;
		bool isAttacked(int square, Colour colour) const;
		bool isInCheck() const;
		bool isValid() const;
		void makeMove(const Move& move);
		void makeNullMove();
		void unmakeMove();
		void unmakeNullMove();
		//int isGameEnd() const;

		Piece position[120];//position information
		int converter[120];//for finding position of piece within piecelists
		PieceSquare blackPieces[17];//piece lists
		PieceSquare whitePieces[17];
		int material;
		int pst;

		GameState currentState;
	};

	typedef std::vector< Board > FullHistory;
	//FULLHISTORY DECLARATION
	FullHistory fullHistory;

	Board::Board() : material(0), pst(0), currentState(GameState()) {}

	struct Generator
	{//GENERATOR DECLARATION
		Generator() { moveList.reserve(64); }
		void generate(const Board& board);//generate all moves
		void knightMoveBlack(const Piece* position, int from);
		void knightMoveWhite(const Piece* position, int from);
		void kingMoveBlack(const Piece* position, int from);
		void kingMoveWhite(const Piece* position, int from);
		void rookMoveBlack(const Piece* position, int from);
		void rookMoveWhite(const Piece* position, int from);
		void bishopMoveBlack(const Piece* position, int from);
		void bishopMoveWhite(const Piece* position, int from);
		void queenMoveBlack(const Piece* position, int from);
		void queenMoveWhite(const Piece* position, int from);
		void pawnMoveBlack(const Piece* position, int from);
		void pawnMoveWhite(const Piece* position, int from);
		void enPassantBlack(const Piece* position, int to);
		void enPassantWhite(const Piece* position, int to);
		void castleBlack(const Board& position, const GameState& current);
		void castleWhite(const Board& position, const GameState& current);
		bool blackTest(int from, int to, Piece on);
		bool whiteTest(int from, int to, Piece on);
		void displayMoves() const;
		//void discovery( const Board& board, int from );//find discovered checks
		MoveList moveList;
	};
}

namespace misc
{
	std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems);
	std::vector<std::string> split(const std::string& s, char delim);
	template <typename T> T str2num(const std::string& text);
	template <typename T> std::string num2str(T number);
	int alpha2board(std::string sq);
	std::string board2alpha(int square);
	void piece2display(const Piece p);
	void move2display(gen::Move& move);
	int inputMove(gen::Board& b);
}

namespace debug
{
	long long perft(int depth, gen::Board& b);
	void timePerft(gen::Board& b);
	void testPerft(gen::Board& b);
	void divideEval(gen::Board& b);
	void timeSearch(gen::Board& b);
}

namespace eval
{
	int randomRange;//used for randomisation
	int materialValue(const Piece p);
	int materialAbsolute(const Piece p);//only used in quiescence search
	int* getPST(const Piece p);
	int basicEval(gen::Board& b);
	int mobileEval();
	int formationEval(gen::Board& b);
	//int passerEval(gen::Board& b); //TODO
	//int safetyEval(gen::Board& b); //TODO
	bool isPawnPromoting(gen::Board& b);
	bool isNonStalemateDraw(gen::Board& b);
	int testGameEnd(gen::Board& b);//1 is game end, 0 is not
}

namespace srch
{
	//internal mobility buffer
	int mobilityBuffer[QSEARCH_DEPTH + 5];
	int mobilityIndex = 0;

	int depthReached = 0;

	//ENGINE DECLARATION
	struct Engine
	{
		Engine();
		Engine(gen::Board& board);
		~Engine();
		int see(int square, Colour toMove);//not currently used and not yet debugged
		int seeCapture(gen::Move capture);//assumes no 2+ queens, or 2+ bishops on same-colour squares
		int findQuiescence(int alpha, int beta, int depthLeft);
		static bool moveCompare(const gen::Move& a, const gen::Move& b);
		static void orderMoves(gen::MoveList& moveList);
		void iterativeDeepening(int depth, gen::MoveList& moveList);
		int internalDeepening(int alpha, int beta, int depth, gen::MoveList& moveList);
		int nullWindowSearch(int beta, int depth);
		int internalSearch(int alpha, int beta, int depth);
		int shallowSearch(int alpha, int beta, int depth);
		gen::Move search(int alpha, int beta, int depth);
		void clearHistory();
		void updateHistory();
		gen::Board b;
		int *counterCapture;//lastMove.from * 120 + lastMove.to, from, to, only used in quiescence
		int *counterMove;
		int *historyTable;
		gen::Move* killerMove;//only one killer move stored per ply, problem with extensions
	};

	//Engine Constructor 1
	Engine::Engine()
	{
		counterCapture = new int[28800];
		counterMove = new int[28800];
		historyTable = new int[14400];
		killerMove = new gen::Move[20];
		b.clear();
		b.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		clearHistory();
	}//end Engine Constructor 1

	 //Engine Constructor 2
	Engine::Engine(gen::Board& board)
	{
		b = board;
		counterCapture = new int[28800];
		counterMove = new int[28800];
		historyTable = new int[14400];
		killerMove = new gen::Move[20];
	}//end Engine Constructor 2

	 //Engine Destructor
	Engine::~Engine()
	{
		delete counterCapture;
		delete counterMove;
		delete historyTable;
		delete killerMove;
	}//end Engine Destructor
}

namespace ui
{
	//Driver DECLARATION
	struct Driver
	{
		Driver();
		void force();//force
		void go();//go
		void initiate();
		void loop();
		int parseInput(std::string input);
		void playOther();//playother
		void playWhite();//white
		void playBlack();//black
		int receiveMove(std::string input);//MOVE
		void sendFeatures();//protover N
		void startNew();//new
		void undo();//undo
		void remove();//remove
		srch::Engine eng;
		bool xboardMode;
		bool forceMode;
		Colour engineSide;
	};
}

namespace gen
{
	//display()
	void Board::display() const
	{
		std::cout << std::endl << "\t\t================================" << std::endl << "\t\t";

		for (int i = 90; i >= 20; i -= 10)
		{
			for (int j = 1; j <= 8; j++)
			{
				misc::piece2display(position[i + j]);
			}
			std::cout << std::endl << "\t\t================================" << std::endl << "\t\t";
		}

		std::cout << std::endl;

		if (DEBUG)
		{
			Generator gen;
			gen.generate(*this);
			gen.displayMoves();
		}
	}//end display()

	 //displayLists()
	void Board::displayLists() const
	{
		std::cout << std::endl;

		for (int i = 0; i < 16; i++)
		{
			std::cout << "\t\t";
			misc::piece2display(blackPieces[i].piece);
			std::cout << " on " << misc::board2alpha(blackPieces[i].square) << "\t";
			misc::piece2display(whitePieces[i].piece);
			std::cout << " on " << misc::board2alpha(whitePieces[i].square) << std::endl;
		}
	}
	//end displayLists()

	void Board::displayState() const
	{
		std::cout << "Current move count is " << moveCount << std::endl;
		if (BLACK == currentState.toMove)
			std::cout << "Black to move ";
		else
			std::cout << "White to move ";
		std::cout << "with " << moveCount << " half-moves made so far" << std::endl;
		if (currentState.blackCanOO)
			std::cout << "Black can castle Kingside" << std::endl;
		if (currentState.blackCanOOO)
			std::cout << "Black can castle Queenside" << std::endl;
		if (!currentState.blackCanOO && !currentState.blackCanOOO)
			std::cout << "Black cannot castle" << std::endl;
		if (currentState.whiteCanOO)
			std::cout << "White can castle Kingside" << std::endl;
		if (currentState.whiteCanOOO)
			std::cout << "White can castle Queenside" << std::endl;
		if (!currentState.whiteCanOO && !currentState.whiteCanOOO)
			std::cout << "White cannot castle" << std::endl;
		if (!currentState.epSquare)
		{
			std::cout << "e.p. capture possible on ";
			std::cout << misc::board2alpha(currentState.epSquare);
			std::cout << std::endl;
		}
		else
		{
			std::cout << "No possible e.p. captures " << std::endl;
		}

		std::cout << "Number of half-moves made since last capture or pawn move: ";
		std::cout << currentState.fifty << std::endl;
		std::cout << "Last move was " << std::endl;
		if (1 <= moveCount)
		{
			gen::Move lastMove = movesMade[moveCount - 1];
			misc::move2display(lastMove);
			if (lastMove.captured)
			{
				std::cout << "captured ";
				misc::piece2display(lastMove.captured);
				std::cout << std::endl;
			}
			if (lastMove.isDoubleMove)
				std::cout << "last move was a double move" << std::endl;
			if (lastMove.isPawnMove)
				std::cout << "last move was a pawn move" << std::endl;
			if (lastMove.promotion)
			{
				std::cout << "last move promoted pawn to";
				misc::piece2display(lastMove.promotion);
			}
		}
		std::cout << std::endl;
	}

	//verify()
	bool Board::verify() const
	{
		if (DEBUG)
			std::cout << "\nBeginning board verification..." << std::endl;

		for (int i = 0; i < 16; i++)
		{
			PieceSquare b_i = blackPieces[i];
			PieceSquare w_i = whitePieces[i];

			if (b_i.piece > blackPieces[i + 1].piece)
			{
				std::cout << "problem with blackPieces ordering" << std::endl;
				this->displayLists();
				return false;
			}

			if (w_i.piece < whitePieces[i + 1].piece)
			{
				std::cout << "problem with whitePieces ordering" << std::endl;
				this->displayLists();
				return false;
			}

			if (b_i.piece != position[b_i.square])
			{
				std::cout << "position & blackPieces inconsistent " << std::endl;
				std::cout << "for square: " << misc::board2alpha(b_i.square) << std::endl;
				return false;
			}

			if (w_i.piece != position[w_i.square])
			{
				std::cout << "position & whitePieces inconsistent " << std::endl;
				std::cout << "for square: " << misc::board2alpha(w_i.square);
				std::cout << " and "; misc::piece2display(w_i.piece); std::cout << std::endl;
				return false;
			}

			if (i != converter[b_i.square] && b_i.square != 0)//0 == square if X == piece
			{
				std::cout << "converter & blackPieces inconsistent " << std::endl;
				std::cout << "for position " << i << std::endl;
				std::cout << "b_i.square is " << misc::board2alpha(b_i.square) << std::endl;
				std::cout << "but converter is " << converter[b_i.square] << std::endl;
				return false;
			}

			if (i != converter[w_i.square] && w_i.square != 0)
			{
				std::cout << "converter & whitePieces inconsistent " << std::endl;
				std::cout << "for position " << i << std::endl;
				std::cout << "w_i.square is " << misc::board2alpha(w_i.square) << std::endl;
				std::cout << "but converter is " << converter[w_i.square] << std::endl;
				return false;
			}
		}

		for (int i = 0; i < 120; i++)
		{
			if (X != position[i] && Z != position[i])
			{
				if (Z > position[i])//if black
				{
					PieceSquare black = blackPieces[converter[i]];

					if (position[i] != black.piece)
					{
						std::cout << "position[ i ] != black.piece  " << std::endl;
						return false;
					}

					if (i != blackPieces[converter[i]].square)
					{
						std::cout << i << " != blackPieces[ converter[" << i << "] ].square " << std::endl;
						return false;
					}
				}
				else//if white
				{
					PieceSquare white = whitePieces[converter[i]];

					if (position[i] != white.piece)
					{
						std::cout << "position[ i ] != white.piece  " << std::endl;
						return false;
					}

					if (i != whitePieces[converter[i]].square)
					{
						std::cout << "i != whitePieces[ converter[ i ] ].square " << std::endl;
						return false;
					}
				}
			}
		}

		int index = 0;
		int materialCount = 0;
		int countPST = 0;
		PieceSquare next = blackPieces[0];

		while (X != next.piece)
		{
			countPST += eval::getPST(next.piece)[next.square];
			materialCount += eval::materialValue(next.piece);

			if (DEBUG)
			{
				std::cout << "blackPieces countPST " << countPST << "     ";
				misc::piece2display(next.piece);
				std::cout << " on " << misc::board2alpha(next.square) << std::endl;
				std::cout << "blackPieces materialCount   " << materialCount << std::endl;
			}

			next = blackPieces[++index];

		}

		index = 0;
		next = whitePieces[0];

		while (X != next.piece)
		{
			countPST += eval::getPST(next.piece)[next.square];
			materialCount += eval::materialValue(next.piece);

			if (DEBUG)
			{
				std::cout << "whitePieces countPST " << countPST << "     ";
				misc::piece2display(next.piece);
				std::cout << " on " << misc::board2alpha(next.square) << std::endl;
				std::cout << "whitePieces materialCount   " << materialCount << std::endl;
			}

			next = whitePieces[++index];
		}

		if (material != materialCount)
		{
			std::cout << "material incorrect " <<
				material << " vs " << materialCount << std::endl;
			display();
			displayLists();
			return false;
		}

		if (pst != countPST)
		{
			std::cout << "countPST incorrect " <<
				pst << " vs " << countPST << std::endl;
			Move lastMove = movesMade[moveCount - 1];
			std::cout << "from " << misc::board2alpha(lastMove.from)
				<< " to " << misc::board2alpha(lastMove.to)
				<< " was last move " << std::endl;
			display();
			displayLists();
			return false;
		}

		if (DEBUG)
			std::cout << "Verification complete, no inconsistencies found " << std::endl;

		return true;
	}//end verify()

	 //clear()
	void Board::clear()
	{
		for (int i = 0; i < 120; i++)
		{
			position[i] = X;
			converter[i] = 0;
		}

		for (int i = 20; i <= 90; i += 10)
		{
			for (int j = 1; j <= 8; j++)
			{
				position[i + j] = Z;
			}
		}

		for (int i = 0; i < 17; i++)
		{
			blackPieces[i].piece = X;
			blackPieces[i].square = 0;
			whitePieces[i].piece = X;
			whitePieces[i].square = 0;
		}

		delete gameHistory;
		delete movesMade;
		gameHistory = new GameState[weights::MAX_GAME_LENGTH];
		movesMade = new Move[weights::MAX_GAME_LENGTH];
		Move dummy;
		GameState dummyState;
		for (int i = 0; i < weights::MAX_GAME_LENGTH; i++)
		{
			movesMade[i] = dummy;
			gameHistory[i] = dummyState;
		}
		currentState = dummyState;//necessary for new games
	}//end clear()

	 //updateConverter()
	void Board::updateConverter()
	{
		for (int i = 0; i < 16; i++)
		{//also fills in converter
			converter[blackPieces[i].square] = i;
			converter[whitePieces[i].square] = i;
		}
	}//end updateConverter()

	 //setFen()
	int Board::setFen(const std::string& fen)
	{
		std::vector<std::string>& fenInfo = misc::split(fen, ' ');

		if (6 != fenInfo.size())
		{
			std::cout << "Incorrect FEN format " << std::endl;
			return 1;
		}

		std::string setUp = fenInfo[0];
		std::string side = fenInfo[1];
		std::string castle = fenInfo[2];
		std::string enPassant = fenInfo[3];
		int half = misc::str2num <int>(fenInfo[4]);
		int full = misc::str2num <int>(fenInfo[5]);

		if (DEBUG)
		{
			std::cout << "Position: " << setUp << std::endl;
			std::cout << "Side to move: " << side << std::endl;
			std::cout << "Castling rights: " << castle << std::endl;
			std::cout << "En passant square: " << enPassant << std::endl;
			std::cout << "Fifty-move counter: " << half << std::endl;
			std::cout << "Move counter: " << full << std::endl;
		}

		int index = 0;//how far along position string
		int square = 91;//starting from rook's square
		int b = 0;
		int w = 0;

		while (index < setUp.size())
		{
			PieceSquare& black = blackPieces[b];
			PieceSquare& white = whitePieces[w];

			switch (setUp[index])
			{
			case '/': square -= 18; break;//move down one row
			case 'p': black.piece = BP; black.square = square;
				material += weights::PAWN * BLACK; pst += weights::BPT[square];
				position[square++] = BP; b++; break;
			case 'P': white.piece = WP; white.square = square;
				material += weights::PAWN * WHITE; pst += weights::WPT[square];
				position[square++] = WP; w++; break;
			case 'k': black.piece = BK; black.square = square;
				material += weights::KING * BLACK; pst += weights::BKT[square];
				position[square++] = BK; b++; break;//King PST not currently used
			case 'K': white.piece = WK; white.square = square;
				material += weights::KING * WHITE; pst += weights::WKT[square];
				position[square++] = WK; w++; break;
			case 'r': black.piece = BR; black.square = square;
				material += weights::ROOK * BLACK; pst += weights::BRT[square];
				position[square++] = BR; b++; break;
			case 'R': white.piece = WR; white.square = square;
				material += weights::ROOK * WHITE; pst += weights::WRT[square];
				position[square++] = WR; w++; break;
			case 'n': black.piece = BN; black.square = square;
				material += weights::KNIGHT * BLACK; pst += weights::BNT[square];
				position[square++] = BN; b++; break;
			case 'N': white.piece = WN; white.square = square;
				material += weights::KNIGHT * WHITE; pst += weights::WNT[square];
				position[square++] = WN; w++; break;
			case 'b': black.piece = BB; black.square = square;
				material += weights::BISHOP * BLACK; pst += weights::BBT[square];
				position[square++] = BB; b++; break;
			case 'B': white.piece = WB; white.square = square;
				material += weights::BISHOP * WHITE; pst += weights::WBT[square];
				position[square++] = WB; w++; break;
			case 'q': black.piece = BQ; black.square = square;
				material += weights::QUEEN * BLACK; pst += weights::BQT[square];
				position[square++] = BQ; b++; break;
			case 'Q': white.piece = WQ; white.square = square;
				material += weights::QUEEN * WHITE; pst += weights::WQT[square];
				position[square++] = WQ; w++; break;
			case '1': square += 1; break;
			case '2': square += 2; break;
			case '3': square += 3; break;
			case '4': square += 4; break;
			case '5': square += 5; break;
			case '6': square += 6; break;
			case '7': square += 7; break;
			case '8': square += 8; break;
			default: std::cout << "Invalid piece placement in FEN " << std::endl; return 1;
			}

			index++;
		}//end setFen()

		sortPieces(BLACK);
		sortPieces(WHITE);
		updateConverter();

		if (DEBUG)
		{
			std::cout << "Material score: " << material << std::endl;
			std::cout << "PST score: " << pst << std::endl;
			display();
			displayLists();
		}

		switch (side[0])
		{
		case 'b': currentState.toMove = BLACK; break;
		case 'w': currentState.toMove = WHITE; break;
		default: std::cout << "Invalid colour to move in FEN " << std::endl; return 1;
		}

		for (int i = 0; i < castle.size(); i++)
		{
			switch (castle[i])
			{
			case '-': break;
			case 'k': currentState.blackCanOO = true; break;
			case 'q': currentState.blackCanOOO = true; break;
			case 'K': currentState.whiteCanOO = true; break;
			case 'Q': currentState.whiteCanOOO = true; break;
			default: std::cout << "Invalid castling rights in FEN " << std::endl; return 1;
			}
		}

		if ('-' != enPassant[0])
		{
			currentState.epSquare = misc::alpha2board(enPassant);
		}

		currentState.fifty = half;
		if (BLACK == currentState.toMove)
			moveCount = (full - 1) * 2 + 1;
		else
			moveCount = (full - 1) * 2;
		Move dummy;
		movesMade[moveCount] = dummy;
		gameHistory[moveCount++] = currentState;
		return 0;
	}

	//sortPieces()
	void Board::sortPieces(Colour colour)
	{
		if (DEBUG)
		{
			std::cout << "Sorting list for " << ((BLACK == colour) ? "black " : "white ")
				<< "pieces" << std::endl;
		}
		PieceSquare* list = (BLACK == colour) ? blackPieces : whitePieces;
		int square[8][8];
		int count[8] = { 0, 0, 0, 0, 0 ,0, 0, 0 };

		for (int i = 0; i < 16; i++)
		{
			PieceSquare& pcSq = list[i];

			switch (pcSq.piece * colour)
			{
			case WP: square[2][count[2]++] = pcSq.square; break;
			case WK: square[7][count[7]++] = pcSq.square; break;
			case WR: square[5][count[5]++] = pcSq.square; break;
			case WN: square[3][count[3]++] = pcSq.square; break;
			case WB: square[4][count[4]++] = pcSq.square; break;
			case WQ: square[6][count[6]++] = pcSq.square; break;
			}

			pcSq.piece = X;
		}

		int i = 7;//denotes piece type
		int j = 0;//index for how far along

		while (1 < i)
		{
			while (count[i] > 0)
			{
				PieceSquare& pcSq = list[j++];
				pcSq.piece = (Piece)(i * colour);
				pcSq.square = square[i][--count[i]];
			}//just a counting sort

			i--;
		}
	}//end sortPieces()

	 //captureSort()
	void Board::captureSort(Colour toMove, int index)
	{//remember colour represents side to move
		PieceSquare* list = (WHITE == toMove) ? blackPieces : whitePieces;

		for (int i = index + 1; i < 17; i++)
		{
			list[i - 1] = list[i];//move left by one
			converter[list[i].square] = i - 1;//should be equal to i - 1 now
		}//also takes care of converter

	}//end captureSort()

	 //promotionSort()
	void Board::promotionSort(Colour toMove, int index, Piece promotion, int square)
	{
		PieceSquare* list = (BLACK == toMove) ? blackPieces : whitePieces;

		int newIndex = 1;

		for (; promotion * toMove <= list[newIndex].piece * toMove; newIndex++);

		for (int i = index; i > newIndex; i--)
		{
			list[i] = list[i - 1];//move right by one
			converter[list[i - 1].square] = i;//should be equal to i now
		}//also takes care of converter

		PieceSquare& promoted = list[newIndex];
		promoted.piece = promotion;
		promoted.square = square;
		converter[square] = newIndex;
	}//end promotionSort()

	 //castle()
	void Board::castle(Colour toMove, int to, PieceSquare* fromPieces)
	{

		if (BLACK == toMove)
		{
			currentState.blackCanOO = false;
			currentState.blackCanOOO = false;

			if (97 == to)//if kingside castle
			{
				position[98] = Z;
				position[96] = BR;
				fromPieces[converter[98]].square = 96;
				converter[96] = converter[98];
				int* BRT = eval::getPST(BR);
				pst += BRT[96] - BRT[98];
				//king position updated in makeMove()
			}
			else//queenside castle
			{
				position[91] = Z;
				position[94] = BR;
				fromPieces[converter[91]].square = 94;
				converter[94] = converter[91];
				int* BRT = eval::getPST(BR);
				pst += BRT[94] - BRT[91];
			}
		}
		else//white castling
		{
			currentState.whiteCanOO = false;
			currentState.whiteCanOOO = false;

			if (27 == to)//if kingside castle
			{

				position[28] = Z;
				position[26] = WR;
				fromPieces[converter[28]].square = 26;
				converter[26] = converter[28];
				int* WRT = eval::getPST(WR);
				pst += WRT[26] - WRT[28];
			}
			else//queenside castle
			{
				position[21] = Z;
				position[24] = WR;
				fromPieces[converter[21]].square = 24;
				converter[24] = converter[21];
				int* WRT = eval::getPST(WR);
				pst += WRT[24] - WRT[21];
			}
		}
	}//end castle()

	 //rookCaptured()
	void Board::rookCaptured(Piece captured, int to)
	{
		if (BR == captured)
		{
			if (98 == to)
				currentState.blackCanOO = false;
			else if (91 == to)
				currentState.blackCanOOO = false;
		}
		else if (WR == captured)
		{
			if (28 == to)
				currentState.whiteCanOO = false;
			else if (21 == to)
				currentState.whiteCanOOO = false;
		}
	}//end rookCaptured()

	 //rookOrKing()
	void Board::rookOrKing(Piece moving, int from)
	{
		if (BK == moving)
		{
			currentState.blackCanOO = false;
			currentState.blackCanOOO = false;
		}
		else if (WK == moving)
		{
			currentState.whiteCanOO = false;
			currentState.whiteCanOOO = false;
		}
		else if (BR == moving)
		{
			if (98 == from)
				currentState.blackCanOO = false;
			else if (91 == from)
				currentState.blackCanOOO = false;
		}
		else if (WR == moving)
		{
			if (28 == from)
				currentState.whiteCanOO = false;
			else if (21 == from)
				currentState.whiteCanOOO = false;
		}
	}//end rookOrKing()

	 //unCaptureSort()
	void Board::unCaptureSort(Colour moved, Piece captured, int to)
	{
		PieceSquare* list = (BLACK == moved) ? whitePieces : blackPieces;
		int index = 1;
		Piece next = list[1].piece;//only sorting the toPieces

		while (captured * moved >= next * moved)
		{
			next = list[++index].piece;//obtains where to insert piece
		}

		for (int i = 16; i > index; i--)
		{
			list[i] = list[i - 1];//shift pieces right
			converter[list[i - 1].square] = i;
		}

		PieceSquare& pcSq = list[index];
		pcSq.piece = captured;//replacing piece on relevant square
		pcSq.square = to;
		converter[to] = index;
	}//end unCaptureSort()

	 //unPromotionSort()
	void Board::unPromotionSort(Colour moved, int from, int to)
	{
		PieceSquare* list = (BLACK == moved) ? blackPieces : whitePieces;
		Piece pawn = (BLACK == moved) ? BP : WP;
		int index = converter[to];

		for (int i = index + 1; i < 17; i++)
		{
			list[i - 1] = list[i];//shift pieces left
			converter[list[i].square] = i - 1;
		}

		int pawnIndex = 0;

		while (X != list[++pawnIndex].piece);//last pawn

		PieceSquare& demoted = list[pawnIndex];
		demoted.piece = pawn;
		demoted.square = from;
		converter[from] = pawnIndex;
	}//end unPromotionSort()

	 //unCastle()
	void Board::unCastle(Colour moved, int to, PieceSquare* fromPieces)
	{
		if (BLACK == moved)
		{
			if (97 == to)//black OO
			{
				position[96] = Z;
				position[98] = BR;
				fromPieces[converter[96]].square = 98;
				converter[98] = converter[96];
				pst += eval::getPST(BR)[98] - eval::getPST(BR)[96];
			}
			else//black OOO
			{
				position[94] = Z;
				position[91] = BR;
				fromPieces[converter[94]].square = 91;
				converter[91] = converter[94];
				pst += eval::getPST(BR)[91] - eval::getPST(BR)[94];
			}
		}
		else
		{
			if (27 == to)//white OO
			{
				position[26] = Z;
				position[28] = WR;
				fromPieces[converter[26]].square = 28;
				converter[28] = converter[26];
				pst += eval::getPST(WR)[28] - eval::getPST(WR)[26];
			}
			else//white OOO
			{
				position[24] = Z;
				position[21] = WR;
				fromPieces[converter[24]].square = 21;
				converter[21] = converter[24];
				pst += eval::getPST(WR)[21] - eval::getPST(WR)[24];
			}
		}
	}//end unCastle()

	 //isOpen()
	bool Board::isOpen(int src, int dest, int disp) const
	{
		if (src > dest)
			disp = -disp;

		src += disp;//initiate by moving out one square

		while (Z == position[src] && src != dest)
			src += disp;

		return (src == dest) ? true : false;
	}//end isOpen

	 //isAttacked()
	bool Board::isAttacked(int square, Colour colour) const
	{

		const PieceSquare* attacker;
		int knight;
		int king;

		if (BLACK == colour)
		{
			attacker = blackPieces;
			knight = BN;
			king = BK;

			if (BP == position[square + 11]) return true;
			if (BP == position[square + 9]) return true;
		}
		else
		{
			attacker = whitePieces;
			knight = WN;
			king = WK;

			if (WP == position[square - 9]) return true;
			if (WP == position[square - 11]) return true;
		}

		if (knight == position[square + 21]) return true;
		if (knight == position[square + 19]) return true;
		if (knight == position[square + 12]) return true;
		if (knight == position[square + 8]) return true;
		if (knight == position[square - 8]) return true;
		if (knight == position[square - 12]) return true;
		if (knight == position[square - 19]) return true;
		if (knight == position[square - 21]) return true;

		if (king == position[square + 11]) return true;
		if (king == position[square + 10]) return true;
		if (king == position[square + 9]) return true;
		if (king == position[square + 1]) return true;
		if (king == position[square - 1]) return true;
		if (king == position[square - 9]) return true;
		if (king == position[square - 10]) return true;
		if (king == position[square - 11]) return true;

		int i = 1;//position 0 holds king
		PieceSquare pcSq = attacker[1];
		int next = (BLACK == colour) ? -pcSq.piece : pcSq.piece;

		while (WN < next)//sliders only
		{
			if (WQ == next)
			{
				int src = pcSq.square;

				if (DIAG[src] == DIAG[square])
				{
					if (isOpen(src, square, 9)) return true;
				}
				else if (ANTI[src] == ANTI[square])
				{
					if (isOpen(src, square, 11)) return true;
				}
				else if (FILE[src] == FILE[square])
				{
					if (isOpen(src, square, 10)) return true;
				}
				else if (RANK[src] == RANK[square])
				{
					if (isOpen(src, square, 1)) return true;
				}
			}
			else if (WR == next)
			{
				int src = pcSq.square;

				if (FILE[src] == FILE[square])
				{
					if (isOpen(src, square, 10))
						return true;
				}
				else if (RANK[src] == RANK[square])
				{
					if (isOpen(src, square, 1))
						return true;
				}
			}
			else if (WB == next)
			{
				int src = pcSq.square;

				if (DIAG[src] == DIAG[square])
				{
					if (isOpen(src, square, 9))
						return true;
				}
				else if (ANTI[src] == ANTI[square])
				{
					if (isOpen(src, square, 11)) return true;
				}
			}
			else
			{
				std::cout << "Error in isAttacked(), check pieceList ordering " << std::endl;
				displayLists();
			}
			pcSq = attacker[++i];
			next = (BLACK == colour) ? -pcSq.piece : pcSq.piece;//update
		}

		return false;
	}//end isAttacked()

	 //isInCheck()
	bool Board::isInCheck() const
	{
		Colour toMove = currentState.toMove;
		const PieceSquare* pieces = (BLACK == toMove) ? blackPieces : whitePieces;
		Colour other = (BLACK == toMove) ? WHITE : BLACK;
		return (isAttacked(pieces[0].square, other));
	}//end isInCheck()

	 //isValid()
	bool Board::isValid() const
	{
		Colour toMove = currentState.toMove;
		const Move& lastMove = movesMade[moveCount - 1];

		if (lastMove.isDoubleMove && !lastMove.isPawnMove)
		{//check if castled through check	
			int from = lastMove.from;
			if (95 == from)
			{
				int to = lastMove.to;

				if (97 == to)
				{
					if (isAttacked(96, toMove))
						return false;
				}
				else if (93 == to)
				{
					if (isAttacked(94, toMove))
						return false;
				}
			}
			else if (25 == from)
			{
				int to = lastMove.to;

				if (27 == to)
				{
					if (isAttacked(26, toMove))
						return false;
				}
				if (23 == to)
				{
					if (isAttacked(24, toMove))
						return false;
				}
			}
		}

		const PieceSquare* pieces = (BLACK == toMove) ? whitePieces : blackPieces;
		return (isAttacked(pieces[0].square, toMove)) ? false : true;
	}//end isValid()

	 //makeMove()
	void Board::makeMove(const Move& move)
	{

		gameHistory[moveCount] = currentState;
		currentState.epSquare = 0;//necessary to clear the epSquare every time
		Colour toMove = currentState.toMove;
		int from = move.from;
		int to = move.to;
		Piece captured = move.captured;
		Piece moving = position[from];
		Piece promotion = move.promotion;
		bool isPawnMove = move.isPawnMove;
		bool isDoubleMove = move.isDoubleMove;
		PieceSquare* fromPieces = (BLACK == toMove) ? blackPieces : whitePieces;
		int fromIndex = converter[from];

		//toIndex only necessary when capturing
		rookOrKing(moving, from);//disable castling if rook moving
		position[from] = Z;//lifting piece from square...
		pst -= eval::getPST(moving)[from];

		if (isPawnMove)
		{
			currentState.fifty = 0;
			int toIndex = converter[to];

			if (captured)
			{
				if (Z == captured)//en Passant
				{
					captured = (BLACK == toMove) ? WP : BP;
					int pawnOn = (BLACK == toMove) ? to + 10 : to - 10;
					position[pawnOn] = Z;
					toIndex = converter[pawnOn];
					pst -= eval::getPST(captured)[pawnOn];
					goto en_passant_capture;
				}
				else if (promotion)
				{
					material += eval::materialValue(promotion) - eval::materialValue(moving);
					moving = promotion;
					promotionSort(toMove, fromIndex, promotion, to);
					pst -= eval::getPST(captured)[to];
					material -= eval::materialValue(captured);//for e.p. captured originally Z
					captureSort(toMove, toIndex);
					rookCaptured(captured, to);//disable castling if rook captured
					goto case_is_promotion;
				}
				pst -= eval::getPST(captured)[to];
			en_passant_capture:
				material -= eval::materialValue(captured);//for e.p. captured originally Z
				captureSort(toMove, toIndex);
				rookCaptured(captured, to);//disable castling if rook captured
			}
			else if (isDoubleMove)
				currentState.epSquare = (BLACK == toMove) ? to + 10 : to - 10;
			else if (promotion)
			{
				material += eval::materialValue(promotion) - eval::materialValue(moving);
				moving = promotion;
				promotionSort(toMove, fromIndex, promotion, to);
				goto case_is_promotion;
			}
		}
		else if (captured)
		{
			currentState.fifty = 0;
			int toIndex = converter[to];
			pst -= eval::getPST(captured)[to];
			material -= eval::materialValue(captured);//for e.p. captured originally Z
			captureSort(toMove, toIndex);
			rookCaptured(captured, to);//disable castling if rook captured
		}
		else
		{
			currentState.fifty++;
			if (isDoubleMove)//is castling
				castle(toMove, to, fromPieces);
		}

		fromPieces[fromIndex].square = to;//these two lines not necessary for promotions
		converter[to] =/* converter[ from ]*/fromIndex;
	case_is_promotion:
		position[to] = moving;
		pst += eval::getPST(moving)[to];
		currentState.toMove = (BLACK == toMove) ? WHITE : BLACK;
		movesMade[moveCount++] = move;

	}//end makeMove()

	 //makeNullMove()
	void Board::makeNullMove()
	{
		currentState.epSquare = 0;
		gameHistory[moveCount] = currentState;
		Colour& toMove = this->currentState.toMove;
		toMove = (BLACK == toMove) ? WHITE : BLACK;

		if (moveCount >= 2)//use countermove if possible
			movesMade[moveCount] = movesMade[moveCount - 2];

		moveCount++;
	}//makeNullMove()

	 //unmakeMove()
	void Board::unmakeMove()
	{
		currentState = gameHistory[--moveCount];
		Move lastMove = movesMade[moveCount];
		int from = lastMove.from;
		int to = lastMove.to;
		Colour toMove = currentState.toMove;
		int fromIndex = converter[to];
		Piece moving = position[to];
		Piece captured = lastMove.captured;
		Piece promotion = lastMove.promotion;
		bool isPawnMove = lastMove.isPawnMove;
		bool isDoubleMove = lastMove.isDoubleMove;
		PieceSquare* fromPieces = (BLACK == toMove) ? blackPieces : whitePieces;
		position[to] = Z;
		pst -= eval::getPST(moving)[to];

		if (isPawnMove)
		{
			if (captured)
			{
				if (Z == captured)//if en passsant
				{
					Piece pawn = (BLACK == toMove) ? WP : BP;
					int pawnOn = (BLACK == toMove) ? to + 10 : to - 10;
					position[pawnOn] = pawn;
					pst += eval::getPST(pawn)[pawnOn];
					material += eval::materialValue(pawn);
					unCaptureSort(toMove, pawn, pawnOn);
					goto en_passant_uncapture;
				}
				else if (promotion)
				{
					moving = (BLACK == toMove) ? BP : WP;
					material += -eval::materialValue(promotion) + eval::materialValue(moving);
					unPromotionSort(toMove, from, to);
					//pst -= eval::getPST( promotion )[ to ];
					unCaptureSort(toMove, captured, to);
					position[to] = captured;//for en passant square already empty
					material += eval::materialValue(captured);
					pst += eval::getPST(captured)[to];
					goto unpromotion;
				}

				unCaptureSort(toMove, captured, to);
				position[to] = captured;//for en passant square already empty
				material += eval::materialValue(captured);
				pst += eval::getPST(captured)[to];
			}
			else if (promotion)
			{
				moving = (BLACK == toMove) ? BP : WP;
				material += -eval::materialValue(promotion) + eval::materialValue(moving);
				unPromotionSort(toMove, from, to);
				goto unpromotion;
			}
		}
		else if (captured)
		{
			unCaptureSort(toMove, captured, to);
			position[to] = captured;//for en passant square already empty
			material += eval::materialValue(captured);
			pst += eval::getPST(captured)[to];
		}

		else if (isDoubleMove)//is castling
		{
			unCastle(toMove, to, fromPieces);
		}
	en_passant_uncapture:
		fromPieces[fromIndex].square = from;
		converter[from] = /*converter[ to ]*/fromIndex;
	unpromotion:
		position[from] = moving;
		pst += eval::getPST(moving)[from];

	}//end unmakeMove

	 //unmakeNullMove()
	void Board::unmakeNullMove()
	{
		gameHistory[--moveCount] = currentState;
		Colour& toMove = this->currentState.toMove;
		toMove = (BLACK == toMove) ? WHITE : BLACK;
	}
	//end unmakeNullMove()

	//generate()
	void Generator::generate(const Board& board)
	{//assumes pieceLists sorted and converter updated
		GameState current = board.currentState;
		bool blackMove = (BLACK == current.toMove) ? true : false;
		const PieceSquare* pieces = (blackMove) ? board.blackPieces : board.whitePieces;
		const Piece* position = board.position;

		if (blackMove)
		{
			enPassantBlack(position, current.epSquare);
			castleBlack(board, current);

			int index = 1;
			PieceSquare pcSq = pieces[1];
			Piece next = pcSq.piece;

		generate_black:
			switch (next)
			{
			case BP: goto pawn_black;
			case BR: goto rook_black;
			case BB: goto bishop_black;
			case BN: goto knight_black;
			case BQ: goto queen_black;
			case X: goto king_black;
			default: std::cout << "Error in generate_black " << std::endl;
				misc::piece2display(next);
				std::cout << " should not be present " << std::endl;
				return;
			}
		pawn_black:
			pawnMoveBlack(position, pcSq.square); goto update_black;
		rook_black:
			rookMoveBlack(position, pcSq.square); goto update_black;
		bishop_black:
			bishopMoveBlack(position, pcSq.square); goto update_black;
		knight_black:
			knightMoveBlack(position, pcSq.square); goto update_black;
		queen_black:
			queenMoveBlack(position, pcSq.square); goto update_black;

		update_black:
			pcSq = pieces[++index];
			next = pcSq.piece;
			goto generate_black;
		king_black:
			kingMoveBlack(position, pieces[0].square);

		}
		else
		{
			enPassantWhite(position, current.epSquare);
			castleWhite(board, current);

			int index = 1;

			PieceSquare pcSq = pieces[1];
			Piece next = pcSq.piece;

		generate_white:
			switch (next)
			{
			case WP: goto pawn_white;
			case WR: goto rook_white;
			case WB: goto bishop_white;
			case WN: goto knight_white;
			case WQ: goto queen_white;
			case X: goto king_white;
			default: std::cout << " Error in generate_white " << std::endl;
				misc::piece2display(next);
				std::cout << " should not be present " << std::endl;
				return;
			}
		pawn_white:
			pawnMoveWhite(position, pcSq.square); goto update_white;
		rook_white:
			rookMoveWhite(position, pcSq.square); goto update_white;
		bishop_white:
			bishopMoveWhite(position, pcSq.square); goto update_white;
		knight_white:
			knightMoveWhite(position, pcSq.square); goto update_white;
		queen_white:
			queenMoveWhite(position, pcSq.square); goto update_white;
		update_white:
			pcSq = pieces[++index];
			next = pcSq.piece;
			goto generate_white;
		king_white:
			kingMoveWhite(position, pieces[0].square);
		}
	}//end generate()

	 //knightMoveBlack()
	void Generator::knightMoveBlack(const Piece* position, int from)
	{
		int to = from - 21;//forward moves first
		blackTest(from, to, position[to]);
		to = from - 19;
		blackTest(from, to, position[to]);
		to = from - 12;
		blackTest(from, to, position[to]);
		to = from - 8;
		blackTest(from, to, position[to]);
		to = from + 8;
		blackTest(from, to, position[to]);
		to = from + 12;
		blackTest(from, to, position[to]);
		to = from + 19;
		blackTest(from, to, position[to]);
		to = from + 21;
		blackTest(from, to, position[to]);
	}//end knightMoveBlack()

	 //knightMoveWhite()
	void Generator::knightMoveWhite(const Piece* position, int from)
	{
		int to = from + 21;//forward moves first
		whiteTest(from, to, position[to]);
		to = from + 19;
		whiteTest(from, to, position[to]);
		to = from + 12;
		whiteTest(from, to, position[to]);
		to = from + 8;
		whiteTest(from, to, position[to]);
		to = from - 8;
		whiteTest(from, to, position[to]);
		to = from - 19;
		whiteTest(from, to, position[to]);
		to = from - 21;
		whiteTest(from, to, position[to]);
		to = from - 12;
		whiteTest(from, to, position[to]);
	}//end knightMoveWhite()

	 //kingMoveBlack()
	void Generator::kingMoveBlack(const Piece* position, int from)
	{
		int to = from + 10;//starting from top, clockwise
		blackTest(from, to, position[to]);
		to = from + 11;
		blackTest(from, to, position[to]);
		to = from + 1;
		blackTest(from, to, position[to]);
		to = from - 9;
		blackTest(from, to, position[to]);
		to = from - 10;
		blackTest(from, to, position[to]);
		to = from - 11;
		blackTest(from, to, position[to]);
		to = from - 1;
		blackTest(from, to, position[to]);
		to = from + 9;
		blackTest(from, to, position[to]);
	}//end kingMoveBlack()

	 //kingMoveWhite()
	void Generator::kingMoveWhite(const Piece* position, int from)
	{
		int to = from + 10;//starting from top, clockwise
		whiteTest(from, to, position[to]);
		to = from + 11;
		whiteTest(from, to, position[to]);
		to = from + 1;
		whiteTest(from, to, position[to]);
		to = from - 9;
		whiteTest(from, to, position[to]);
		to = from - 10;
		whiteTest(from, to, position[to]);
		to = from - 11;
		whiteTest(from, to, position[to]);
		to = from - 1;
		whiteTest(from, to, position[to]);
		to = from + 9;
		whiteTest(from, to, position[to]);
	}//end kingMoveWhite()

	 //rookMoveBlack()
	void Generator::rookMoveBlack(const Piece* position, int from)
	{
		for (int to = from - 10; blackTest(from, to, position[to]); to -= 10);
		for (int to = from - 1; blackTest(from, to, position[to]); to -= 1);
		for (int to = from + 1; blackTest(from, to, position[to]); to += 1);
		for (int to = from + 10; blackTest(from, to, position[to]); to += 10);
	}//end rookMoveBlack()

	 //rookMoveWhite()
	void Generator::rookMoveWhite(const Piece* position, int from)
	{
		for (int to = from + 10; whiteTest(from, to, position[to]); to += 10);
		for (int to = from + 1; whiteTest(from, to, position[to]); to += 1);
		for (int to = from - 1; whiteTest(from, to, position[to]); to -= 1);
		for (int to = from - 10; whiteTest(from, to, position[to]); to -= 10);
	}//end rookMoveWhite()

	 //bishopMoveBlack()
	void Generator::bishopMoveBlack(const Piece* position, int from)
	{
		for (int to = from - 11; blackTest(from, to, position[to]); to -= 11);
		for (int to = from - 9; blackTest(from, to, position[to]); to -= 9);
		for (int to = from + 9; blackTest(from, to, position[to]); to += 9);
		for (int to = from + 11; blackTest(from, to, position[to]); to += 11);
	}//end bishopMoveBlack()

	 //bishopMoveWhite()
	void Generator::bishopMoveWhite(const Piece* position, int from)
	{
		for (int to = from + 11; whiteTest(from, to, position[to]); to += 11);
		for (int to = from + 9; whiteTest(from, to, position[to]); to += 9);
		for (int to = from - 9; whiteTest(from, to, position[to]); to -= 9);
		for (int to = from - 11; whiteTest(from, to, position[to]); to -= 11);
	}//end bishopMoveWhite()

	 //queenMoveBlack()
	void Generator::queenMoveBlack(const Piece* position, int from)
	{
		for (int to = from - 11; blackTest(from, to, position[to]); to -= 11);
		for (int to = from - 10; blackTest(from, to, position[to]); to -= 10);
		for (int to = from - 9; blackTest(from, to, position[to]); to -= 9);
		for (int to = from - 1; blackTest(from, to, position[to]); to -= 1);
		for (int to = from + 1; blackTest(from, to, position[to]); to += 1);
		for (int to = from + 9; blackTest(from, to, position[to]); to += 9);
		for (int to = from + 10; blackTest(from, to, position[to]); to += 10);
		for (int to = from + 11; blackTest(from, to, position[to]); to += 11);
	}//end queenMoveBlack()

	 //queenMoveWhite()
	void Generator::queenMoveWhite(const Piece* position, int from)
	{
		for (int to = from + 11; whiteTest(from, to, position[to]); to += 11);
		for (int to = from + 10; whiteTest(from, to, position[to]); to += 10);
		for (int to = from + 9; whiteTest(from, to, position[to]); to += 9);
		for (int to = from + 1; whiteTest(from, to, position[to]); to += 1);
		for (int to = from - 1; whiteTest(from, to, position[to]); to -= 1);
		for (int to = from - 9; whiteTest(from, to, position[to]); to -= 9);
		for (int to = from - 10; whiteTest(from, to, position[to]); to -= 10);
		for (int to = from - 11; whiteTest(from, to, position[to]); to -= 11);
	}//end queenMoveWhite()

	 //pawnMoveBlack()
	void Generator::pawnMoveBlack(const Piece* position, int from)
	{
		int to = from - 10;
		Piece on = position[to];
		Move move;
		move.from = from;
		move.isPawnMove = true;

		if (Z == on)//if square directly in front is clear
		{
			move.to = to;

			if (39 > from)//i.e. on seventh rank from black's POV
			{
				move.promotion = BQ;
				moveList.push_back(move);
				move.promotion = BN;
				moveList.push_back(move);
				move.promotion = BR;
				moveList.push_back(move);
				move.promotion = BB;
				moveList.push_back(move);
			}
			else
			{
				moveList.push_back(move);

				if (80 < from)//i.e. on second rank from black's POV
				{
					to = from - 20;
					on = position[to];

					if (Z == on)
					{
						move.to = to;
						move.isDoubleMove = true;
						moveList.push_back(move);
					}
				}
			}
		}//only add move to moveList if square is empty

		to = from - 11;//capture left
		on = position[to];

		if (Z < on)//if some white piece on square
		{
			move.captured = on;

			if (39 > from)//i.e. on seventh rank from black's POV
			{
				move.to = to;
				move.promotion = BQ;
				moveList.push_back(move);
				move.promotion = BN;
				moveList.push_back(move);
				move.promotion = BR;
				moveList.push_back(move);
				move.promotion = BB;
				moveList.push_back(move);
			}
			else
			{
				move.to = to;
				moveList.push_back(move);
			}
		}

		to = from - 9;//capture right
		on = position[to];

		if (Z < on)//if some white piece on square
		{
			move.captured = on;

			if (39 > from)//i.e. on seventh rank from black's POV
			{
				move.to = to;
				move.promotion = BQ;
				moveList.push_back(move);
				move.promotion = BN;
				moveList.push_back(move);
				move.promotion = BR;
				moveList.push_back(move);
				move.promotion = BB;
				moveList.push_back(move);
			}
			else
			{
				move.to = to;
				moveList.push_back(move);
			}
		}
	}//end pawnMoveBlack()

	 //pawnMoveWhite()
	void Generator::pawnMoveWhite(const Piece* position, int from)
	{
		int to = from + 10;
		Piece on = position[to];
		Move move;
		move.from = from;
		move.isPawnMove = true;

		if (Z == on)//if square directly in front is clear
		{
			move.to = to;

			if (80 < from)//i.e. on seventh rank from white's POV
			{
				move.promotion = WQ;
				moveList.push_back(move);
				move.promotion = WN;
				moveList.push_back(move);
				move.promotion = WR;
				moveList.push_back(move);
				move.promotion = WB;
				moveList.push_back(move);
			}
			else
			{
				moveList.push_back(move);

				if (39 > from)//i.e. on second rank from white's POV
				{
					to = from + 20;
					on = position[to];

					if (Z == on)
					{
						move.to = to;
						move.isDoubleMove = true;
						moveList.push_back(move);
					}
				}
			}
		}//only add move to moveList if square is empty

		to = from + 11;//capture right
		on = position[to];

		if (X > on)//if some black piece on square
		{
			move.captured = on;

			if (80 < from)//i.e. on seventh rank from white's POV
			{
				move.to = to;
				move.promotion = WQ;
				moveList.push_back(move);
				move.promotion = WN;
				moveList.push_back(move);
				move.promotion = WR;
				moveList.push_back(move);
				move.promotion = WB;
				moveList.push_back(move);
			}
			else
			{
				move.to = to;
				moveList.push_back(move);
			}
		}

		to = from + 9;//capture left
		on = position[to];

		if (X > on)//if some black piece on square
		{
			move.captured = on;

			if (80 < from)//i.e. on seventh rank from white's POV
			{
				move.to = to;
				move.promotion = WQ;
				moveList.push_back(move);
				move.promotion = WN;
				moveList.push_back(move);
				move.promotion = WR;
				moveList.push_back(move);
				move.promotion = WB;
				moveList.push_back(move);
			}
			else
			{
				move.to = to;
				moveList.push_back(move);
			}
		}
	}//end pawnMoveWhite()

	 //enPassantBlack()
	void Generator::enPassantBlack(const Piece* position, int to)
	{
		if (0 != to)//if en passant possible
		{
			int from = to + 9;

			if (BP == position[from])
			{
				Move move;
				move.from = from;
				move.to = to;
				move.captured = Z;//captures 'nothing'
				move.isPawnMove = true;
				moveList.push_back(move);
			}

			from = to + 11;

			if (BP == position[from])
			{
				Move move;
				move.from = from;
				move.to = to;
				move.captured = Z;//captures 'nothing'
				move.isPawnMove = true;
				moveList.push_back(move);
			}
		}
	}//end enPassantBlack()

	 //enPassantWhite()
	void Generator::enPassantWhite(const Piece* position, int to)
	{
		if (0 != to)//if en passant possible
		{
			int from = to - 9;

			if (WP == position[from])
			{
				Move move;
				move.from = from;
				move.to = to;
				move.captured = Z;//captures 'nothing
				move.isPawnMove = true;
				moveList.push_back(move);
			}

			from = to - 11;

			if (WP == position[from])
			{
				Move move;
				move.from = from;
				move.to = to;
				move.captured = Z;//captures nothing
				move.isPawnMove = true;
				moveList.push_back(move);
			}
		}
	}//end enPassantWhite()

	 //castleBlack()
	void Generator::castleBlack(const Board& board, const GameState& current)
	{
		if (!board.isAttacked(95, WHITE))
		{
			const Piece* position = board.position;

			if (current.blackCanOO)
			{
				if ((Z == position[96]) /*&& !board.isAttacked( 96, WHITE )*/
					&& (Z == position[97]) /*&& !board.isAttacked( 97, WHITE )*/
					&& (BR == position[98]) && (BK == position[95]))
				{
					Move move;
					move.from = 95;
					move.to = 97;
					move.isDoubleMove = true;
					moveList.push_back(move);
				}
			}
			if (current.blackCanOOO)
			{
				if ((Z == position[92])
					&& (Z == position[93]) /*&& !board.isAttacked( 93, WHITE )*/
					&& (Z == position[94]) /*&& !board.isAttacked( 94, WHITE )*/
					&& (BR == position[91]) && (BK == position[95]))
				{
					Move move;
					move.from = 95;
					move.to = 93;
					move.isDoubleMove = true;
					moveList.push_back(move);
				}
			}
		}
	}//end castleBlack()

	 //castleWhite()
	void Generator::castleWhite(const Board& board, const GameState& current)
	{
		if (!board.isAttacked(25, BLACK))
		{
			const Piece* position = board.position;

			if (current.whiteCanOO)
			{
				if ((Z == position[26]) /*&& !board.isAttacked( 26, BLACK )*/
					&& (Z == position[27]) /*&& !board.isAttacked( 27, BLACK )*/
					&& (WR == position[28]) && (WK == position[25]))
				{
					Move move;
					move.from = 25;
					move.to = 27;
					move.isDoubleMove = true;
					moveList.push_back(move);

				}
			}

			if (current.whiteCanOOO)
			{
				if ((Z == position[22])
					&& (Z == position[23]) /*&& !board.isAttacked( 23, BLACK )*/
					&& (Z == position[24]) /*&& !board.isAttacked( 24, BLACK )*/
					&& (WR == position[21]) && (WK == position[25]))
				{
					Move move;
					move.from = 25;
					move.to = 23;
					move.isDoubleMove = true;
					moveList.push_back(move);
				}
			}
		}
	}//end castleWhite()

	 //blackTest()
	bool Generator::blackTest(int from, int to, Piece on)
	{
		if (Z == on)//moving to empty square
		{
			Move move;
			move.from = from;
			move.to = to;
			moveList.push_back(move);
			return true;
		}
		else if (X < on)//capturing white piece
		{
			Move move;
			move.from = from;
			move.to = to;
			move.captured = on;
			moveList.push_back(move);
			return false;
		}

		return false;
	}//blackTest()

	 //whiteTest()
	bool Generator::whiteTest(int from, int to, Piece on)
	{
		Move move;

		if (Z == on)
		{
			move.from = from;
			move.to = to;
			moveList.push_back(move);
			return true;
		}
		else if (X > on)
		{
			move.from = from;
			move.to = to;
			move.captured = on;
			moveList.push_back(move);
			return false;//cannot keep moving in this direction
		}

		return false;
	}//end whiteTest

	 //displayMoves()
	void Generator::displayMoves() const
	{
		std::cout << std::endl;

		for (int i = 0; i < moveList.size(); i++)
		{
			const Move& move = moveList[i];
			std::cout << misc::board2alpha(move.from)
				<< " to " << misc::board2alpha(move.to);

			if (DEBUG)
			{
				if (move.isDoubleMove)
					std::cout << "\tdoubleMove";

				if (move.isPawnMove)
					std::cout << "\t pawnMove";

				if (move.promotion)
				{
					std::cout << "\tpromote to ";
					misc::piece2display(move.promotion);
				}
				if (move.captured)
				{
					std::cout << "\tcapturing ";
					misc::piece2display(move.captured);
				}
			}

			std::cout << std::endl;
		}
	}//end displayMoves()
}//end gen

namespace misc
{
	std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
	{
		std::stringstream ss(s);
		std::string item;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item);
		}
		return elems;
	}
	std::vector<std::string> split(const std::string& s, char delim)
	{
		std::vector< std::string > elems;
		split(s, delim, elems);
		return elems;
	}

	//str2num()
	template <typename T> T str2num(const std::string& text)
	{
		std::istringstream ss(text);
		T result;
		return ss >> result ? result : 0;
	}//end str2num()

	 //num2str()
	template <typename T> std::string num2str(T number)
	{
		std::ostringstream ss;
		ss << number;
		return ss.str();
	}//end num2str()

	 //alpha2board
	int alpha2board(std::string sq)
	{
		if (2 != sq.size())
		{
			std::cout << "invalid square name" << std::endl;
			return 1;
		}
		//return ( sq[ 1 ] - 47 ) * 10 + sq[ 0 ] - 96;
		return sq[1] * 10 + sq[0] - 566;
	}//end alpha2board

	 //board2alpha()
	std::string board2alpha(int square)
	{
		std::string str;

		if (0 == square)
			return str = "0";

		switch (square % 10)
		{
		case 1: str = "a";
			break;
		case 2: str = "b";
			break;
		case 3: str = "c";
			break;
		case 4: str = "d";
			break;
		case 5: str = "e";
			break;
		case 6: str = "f";
			break;
		case 7: str = "g";
			break;
		case 8: str = "h";
			break;
		default: std::cout << "Error in board2alpha file for square " << square << std::endl;
			std::cout << "File should not be equal to " << square % 10 << std::endl;
		}

		switch (square / 10)
		{
		case 2: str = str + "1";
			break;
		case 3: str = str + "2";
			break;
		case 4: str = str + "3";
			break;
		case 5: str = str + "4";
			break;
		case 6: str = str + "5";
			break;
		case 7: str = str + "6";
			break;
		case 8: str = str + "7";
			break;
		case 9: str = str + "8";
			break;
		default: std::cout << "Error in board2alpha rank for square " << square << std::endl;
			std::cout << "Rank should not be equal to " << square / 10 << std::endl;
		}

		return str;
	}//end board2alpha()

	 //piece2display()
	void piece2display(const Piece p)
	{
		std::setw(6);
		switch (p)
		{
		case Z:  std::cout << "|  |"; break;
		case BP: std::cout << "|卒|"; break;
		case BN: std::cout << "|馬|"; break;
		case BB: std::cout << "|象|"; break;
		case BR: std::cout << "|車|"; break;
		case BQ: std::cout << "|妃|"; break;
		case BK: std::cout << "|帝|"; break;
		case WP: std::cout << "|兵|"; break;
		case WN: std::cout << "|马|"; break;
		case WB: std::cout << "|相|"; break;
		case WR: std::cout << "|车|"; break;
		case WQ: std::cout << "|后|"; break;
		case WK: std::cout << "|皇|"; break;
		case X: std::cout << " X "; break;
		default: std::cout << " Error in piece2display() " << std::endl;
		}
	}//end piece2display()

	 //move2display()
	void move2display(gen::Move& move)
	{
		std::cout << ">> ";
		std::cout << misc::board2alpha(move.from) << misc::board2alpha(move.to) << std::endl;
	}//end move2display()

	 //inputMove()
	int inputMove(gen::Board& b)
	{
		std::string input;
		std::cin >> input;

		if (input.size() != 4)
		{
			std::cout << "Invalid input (correct format e2e4) " << std::endl;
			return 1;
		}

		int from = misc::alpha2board(input.substr(0, 2));
		int to = misc::alpha2board(input.substr(2, 2));

		if (DEBUG)
		{
			std::cout << "from " << from << " to " << to << std::endl;
		}

		gen::Generator gen;
		gen.generate(b);//generate moveList
		gen::MoveList& moveList = gen.moveList;

		int i = 0;
		Piece promotion = X;//promotions specially handled

		for (; i < moveList.size(); i++)
		{
			gen::Move move = moveList[i];

			if (from == move.from && to == move.to)
			{
				if (move.promotion && X == promotion)//if promotion not decided
				{
					char decision;
					Colour toMove = b.currentState.toMove;
					std::cout << "Pick the piece to promote to (q, n, r, b) " << std::endl;
					std::cin >> decision;

					switch (decision)
					{
					case 'q': promotion = Piece(toMove * WQ); break;
					case 'n': promotion = Piece(toMove * WN); break;
					case 'r': promotion = Piece(toMove * WR); break;
					case 'b': promotion = Piece(toMove * WB); break;
					default: std::cout << "Invalid selection " << std::endl; return 1;
					}

					if (promotion == move.promotion)
						break;
					else
						continue;
				}
				else
					break;
			}
		}

		//end reached without having found move
		if (moveList.size() == i)
		{
			std::cout << "Invalid move " << std::endl;
			return 1;
		}

		b.makeMove(moveList[i]);
		return 0;
	}//end inputMove()

}//end misc

namespace debug
{
	int initialDepth = 0;
	long long divideTemp = 0;
	//perft()
	long long perft(int depth, gen::Board& b)
	{
		if (0 == depth)
			return 1;

		long long nodes = 0;
		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;

		for (int i = 0; i < moveList.size(); i++)
		{
			b.makeMove(moveList[i]);

			if (b.isValid())
			{
				//gameState[ b.moveCount ] = b;
				nodes += perft(depth - 1, b);

				if (initialDepth == depth)
				{
					std::cout << misc::board2alpha(moveList[i].from) <<
						misc::board2alpha(moveList[i].to) << " "
						<< nodes - divideTemp << std::endl;
					divideTemp = nodes;
				}
			}
			//b = gameState[ b.moveCount - 1 ];//naive unmake
			b.unmakeMove();
		}
		return nodes;
	}//end inputMove

	 //timePerft
	void timePerft(gen::Board& b)
	{
		int depth = 1;
		std::cout << "input required depth" << std::endl;
		std::cin >> depth;
		initialDepth = depth;
		LARGE_INTEGER frequency, t1, t2;
		double elapsedTime;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&t1);
		long long nodeCount = perft(depth, b);
		QueryPerformanceCounter(&t2);
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
		std::cout << "perft returns: " << nodeCount << std::endl;
		std::cout << "time taken: " << elapsedTime << " ms. " << std::endl;
	}//end timePerft

	 //testPerft()
	void testPerft(gen::Board& b)
	{
		//		DEBUG = false;	
		b.clear();
		int choice = 1;

		while (choice)
		{
			std::cout << "Choose test position (enter integer between 1~6)." << std::endl;
			std::cout << "Enter 0 to exit perft tests." << std::endl;
			std::cin >> choice;
			if (0 == choice)
				break;
			switch (choice)
			{
			case 1: b.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
				break;
			case 2: b.setFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
				break;
			case 3: b.setFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
				break;
			case 4: b.setFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
				break;
			case 5: b.setFen("rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6");
				break;
			case 6: b.setFen("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
				break;
			default: b.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			}
			timePerft(b);
			b.clear();
		}
		//	b.displayLists();
	}//end testPerft()

	 //divideEval()
	void divideEval(gen::Board& b)
	{
		gen::Generator gen;
		gen.generate(b);
		std::vector < gen::Move >& moveList = gen.moveList;
		for (int i = 0; i < moveList.size(); i++)
		{
			b.makeMove(moveList[i]);
			if (b.isValid())
			{
				srch::Engine eng(b);
				if (DEBUG)
					misc::move2display(moveList[i]);
				eng.search(NEG_INFINITY, POS_INFINITY, 4);
			}
			b.unmakeMove();
		}
	}//end divideEval()

	 //timeSearch()
	void timeSearch(gen::Board& b)
	{
		std::cout << "Input depth:" << std::endl;
		int depth;
		std::cin >> depth;
		LARGE_INTEGER frequency, t1, t2;
		double elapsedTime;
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&t1);
		srch::Engine eng(b);
		eng.search(NEG_INFINITY, POS_INFINITY, depth);
		QueryPerformanceCounter(&t2);
		elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
		std::cout << "time taken: " << elapsedTime << " ms. " << std::endl;
	}//end timeSearch()
}//end debug

namespace eval
{
	//materialValue()
	int materialValue(const Piece p)
	{
		switch (p)
		{
			//case Z: break;
		case BP: return -weights::PAWN; break;
		case BN: return -weights::KNIGHT; break;
		case BB: return -weights::BISHOP; break;
		case BR: return -weights::ROOK; break;
		case BQ: return -weights::QUEEN; break;
		case BK: return -weights::KING; break;
		case WP: return weights::PAWN; break;
		case WN: return weights::KNIGHT; break;
		case WB: return weights::BISHOP; break;
		case WR: return weights::ROOK; break;
		case WQ: return weights::QUEEN; break;
		case WK: return weights::KING; break;
		default: std::cout << " Error in materialValue() ";
			misc::piece2display(p);
			std::cout << " should not be present " << std::endl;
			return 0;
		}

		return 0;
	}//end materialValue()

	 //materialAbsolute()
	int materialAbsolute(const Piece p)
	{
		switch (p)
		{
			//case Z: break;
		case BP: return weights::PAWN; break;
		case BN: return weights::KNIGHT; break;
		case BB: return weights::BISHOP; break;
		case BR: return weights::ROOK; break;
		case BQ: return weights::QUEEN; break;
		case BK: return weights::KING; break;
		case WP: return weights::PAWN; break;
		case WN: return weights::KNIGHT; break;
		case WB: return weights::BISHOP; break;
		case WR: return weights::ROOK; break;
		case WQ: return weights::QUEEN; break;
		case WK: return weights::KING; break;
		case Z: return weights::PAWN; break;
		default: std::cout << " Error in materialAbsolute() ";
			misc::piece2display(p);
			std::cout << " should not be present " << std::endl;
			return 0;
		}

		return 0;
	}//end materialAbsolute()

	 //getPST()
	int* getPST(const Piece p)
	{
		switch (p)
		{
		case BP: return weights::BPT; break;
		case BN: return weights::BNT; break;
		case BB: return weights::BBT; break;
		case BR: return weights::BRT; break;
		case BQ: return weights::BQT; break;
		case BK: return weights::BKT; break;//use this later?
		case WP: return weights::WPT; break;
		case WN: return weights::WNT; break;
		case WB: return weights::WBT; break;
		case WR: return weights::WRT; break;
		case WQ: return weights::WQT; break;
		case WK: return weights::WKT; break;//use this later?
		default: std::cout << "Error in getPST() " << std::endl;
			std::cout << "Should not be ";
			misc::piece2display(p);
			std::cout << std::endl;
			return 0;
		}
	}//end getPST()

	 //basicEval()
	int basicEval(gen::Board& b)
	{
		if (!randomRange)
			return b.currentState.toMove *  (b.material + b.pst + eval::formationEval(b)) + mobileEval() / 6;
		else
			return b.currentState.toMove *  (b.material + b.pst + eval::formationEval(b)) + mobileEval() / 6
			+ (rand() % eval::randomRange) - (randomRange / 2);
	}//end basicEval()

	 //mobileEval()
	int mobileEval()
	{
		if (0 == srch::mobilityIndex)
		{
			std::cout << "should not happen " << std::endl;//only reaches here in even ply shallow searches
			return 0;//mobility likely makes little difference here
		}
		return srch::mobilityBuffer[srch::mobilityIndex - 1] * 128 /
			srch::mobilityBuffer[srch::mobilityIndex] - 128;
	}//end mobileEval()

	 // //formationEval()
	int formationEval(gen::Board& b)
	{
		int penalty[2] = { 0, 0 };
		int end = 0;
		int pawnSkeleton[48] = { 0, 0, 0, 0, 0, 0, 0, 0, //number of white pawns
			7, 7, 7, 7, 7, 7, 7, 7, //most advanced black pawns
			0, 0, 0, 0, 0, 0, 0, 0, //least advanced black pawns
			0, 0, 0, 0, 0, 0, 0, 0, //number of black pawns
			0, 0, 0, 0, 0, 0, 0, 0, //most advanced white pawns
			7, 7, 7, 7, 7, 7, 7, 7 };//least advanced white pawns

		int fileStatus[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };//0 closed, 1 semiopen black, 2 semiopen white, 3 open

		Piece* position = b.position;

		//first black then white
		gen::PieceSquare* pieces = b.blackPieces;
		gen::PieceSquare nextPcSq = pieces[0];
		Piece nextPiece = nextPcSq.piece;
		int nextSquare = nextPcSq.square;
		int file = 0;
		int rank = 0;
		int bishopCount = 0;
		int blackRookSquare[2] = { -1,-1 };
		int blackRookCount = 0;
		int lightPawns = 0;
		int darkPawns = 0;
		int lightBishop = 0;
		int darkBishop = 0;
		int blackPhase = 512;
		int whitePhase = 512;

		for (int pieceIndex = 0; X != nextPiece;)
		{
			//read in black formation
			switch (nextPiece)
			{
			case BK://basic king safety
				if (WQ == b.whitePieces[1].piece)
				{
					if (97 == nextSquare)
					{
						//short castle
						penalty[0] -= 10;
						if (BR == position[98])
							penalty[0] += 20;
					}
					else if (93 == nextSquare)
					{
						//long castle
						penalty[0] -= 8;
						if (BR == position[91])
							penalty[0] += 20;
					}
					else if (92 == nextSquare)
					{
						//still long castle
						penalty[0] -= 10;
						if (BR == position[91])
							penalty[0] += 20;
					}
				}
				break;
			case BQ:
				blackPhase -= 128;
				break;
			case BR:
				blackPhase -= 64;
				if (blackRookCount < 2)
					blackRookSquare[blackRookCount++] = nextSquare;
				break;
			case BB:
				blackPhase -= 16;
				if (gen::DARK_LIGHT[nextSquare])
					darkBishop++;
				else
					lightBishop++;
				if (Z != position[nextSquare + 11])
					penalty[0] += 1;
				if (Z != position[nextSquare + 9])
					penalty[0] += 1;
				if (Z != position[nextSquare - 9])
					penalty[0] += 6;
				if (Z != position[nextSquare - 11])
					penalty[0] += 6;
				if (BP == position[nextSquare - 18])
					penalty[0] += 3;
				if (BP == position[nextSquare - 22])
					penalty[0] += 3;
				if (nextSquare == 74)
				{
					if (BP == position[84])
						penalty[0] += 4;
				}
				else if (nextSquare == 75)
				{
					if (BP == position[85])
						penalty[0] += 4;
				}
				bishopCount++;
				break;
			case BN:
				blackPhase -= 16;
				if (73 == nextSquare && BP == position[83])
				{
					if (BP != position[65])
					{
						penalty[0] += 5;
						if (BP == position[64])
							penalty[0] += 4;
					}
					if (WP == position[54] || WP == position[64])
						penalty[0] += 12;
				}
				break;
			case BP:
				blackPhase -= 8;
				if (gen::DARK_LIGHT[nextSquare])
					darkPawns++;
				else
					lightPawns++;
				file = gen::FILE[nextSquare] - 1;
				rank = gen::RANK[nextSquare] - 1;
				pawnSkeleton[file]++;
				if (rank < pawnSkeleton[8 + file])
					pawnSkeleton[8 + file] = rank;
				if (rank > pawnSkeleton[16 + file])
					pawnSkeleton[16 + file] = rank;
				break;
			default: break;
			}
			nextPcSq = pieces[++pieceIndex];
			nextPiece = nextPcSq.piece;
			nextSquare = nextPcSq.square;
		}

		//connected rook bonus
		if (2 == blackRookCount)
		{
			int rookOne = (blackRookSquare[0] < blackRookSquare[1]) ?
				blackRookSquare[0] : blackRookSquare[1];
			int rookTwo = (blackRookSquare[0] > blackRookSquare[1]) ?
				blackRookSquare[0] : blackRookSquare[1];
			if (gen::FILE[rookOne] == gen::FILE[rookTwo])
			{
				while (Z == position[rookOne += 10 ]);
				if (rookOne == rookTwo)
				{
					penalty[0] -= 8;
					end -= 2;
				}
			}
			else if (gen::RANK[rookOne] == gen::RANK[rookTwo])
			{
				while (Z == position[rookOne += 1]);
				if (rookOne == rookTwo)
				{
					penalty[0] -= 8;
					end -= 2;
				}
			}
		}

		//good and bad bishops
		if (lightBishop && lightPawns > 1)
		{
			penalty[0] += 4 * lightPawns;
			end += 6 * lightPawns;
		}
		if (darkBishop && darkPawns > 1)
		{
			penalty[0] += 4 * darkPawns;
			end += 6 * darkPawns;
		}

		int blackPawnTotal = lightPawns + darkPawns;

		//bishop pair bonus
		if (2 == bishopCount)
		{
			penalty[0] -= 45;
			end -= 55;
		}

		pieces = b.whitePieces;
		nextPcSq = pieces[0];
		nextPiece = nextPcSq.piece;
		nextSquare = nextPcSq.square;
		file = 0;
		rank = 0;
		bishopCount = 0;
		int whiteRookSquare[2] = { -1, -1 };
		int whiteRookCount = 0;
		lightPawns = 0;
		darkPawns = 0;
		lightBishop = 0;
		darkBishop = 0;
		for (int pieceIndex = 0; X != nextPiece;)
		{
			//read in white formation
			switch (nextPiece)
			{
			case WK://king safety
				if (BQ == b.blackPieces[1].piece)
				{
					if (27 == nextSquare)
					{
						//short castle
						penalty[1] -= 10;
						if (WR == position[28])
							penalty[1] += 20;
					}
					else if (23 == nextSquare)
					{
						//long castle
						penalty[1] -= 8;
						if (WR == position[21])
							penalty[1] += 20;
					}
					else if (22 == nextSquare)
					{
						//still long castle
						penalty[1] -= 10;
						if (WR == position[21])
							penalty[1] += 20;
					}
				}
				break;
			case WQ:
				whitePhase -= 128;
				break;
			case WR:
				whitePhase -= 64;
				if (whiteRookCount < 2)
					whiteRookSquare[whiteRookCount++] = nextSquare;
				break;
			case WB:
				whitePhase -= 16;
				if (gen::DARK_LIGHT[nextSquare])
					darkBishop++;
				else
					lightBishop++;
				if (Z != position[nextSquare + 11])
					penalty[1] += 6;
				if (Z != position[nextSquare + 9])
					penalty[1] += 6;
				if (Z != position[nextSquare - 9])
					penalty[1] += 1;
				if (Z != position[nextSquare - 11])
					penalty[1] += 1;
				if (WP == position[nextSquare + 18])
					penalty[1] += 3;
				if (WP == position[nextSquare + 22])
					penalty[1] += 3;
				if (nextSquare == 44)
				{
					if (WP == position[34])
						penalty[1] += 4;
				}
				else if (nextSquare == 45)
				{
					if (WP == position[35])
						penalty[1] += 4;
				}
				bishopCount++;
				break;
			case WN:
				whitePhase -= 16;
				if (43 == nextSquare && WP == position[33])
				{
					if (WP != position[55])
					{
						penalty[1] += 5;
						if (WP == position[54])
							penalty[1] += 4;
					}
					if (BP == position[64] || BP == position[54])
						penalty[1] += 12;
				}
				break;
			case WP:
				whitePhase -= 8;
				if (gen::DARK_LIGHT[nextSquare])
					darkPawns++;
				else
					lightPawns++;
				file = gen::FILE[nextSquare] - 1;
				rank = gen::RANK[nextSquare] - 1;
				pawnSkeleton[24 + file]++;
				if (rank > pawnSkeleton[32 + file])
					pawnSkeleton[32 + file] = rank;
				if (rank < pawnSkeleton[40 + file])
					pawnSkeleton[40 + file] = rank;
				break;
			default: break;
			}
			nextPcSq = pieces[++pieceIndex];
			nextPiece = nextPcSq.piece;
			nextSquare = nextPcSq.square;
		}

		//connected rook bonus
		if (2 == whiteRookCount)
		{
			int rookOne = (whiteRookSquare[0] < whiteRookSquare[1]) ?
				whiteRookSquare[0] : whiteRookSquare[1];
			int rookTwo = (whiteRookSquare[0] > whiteRookSquare[1]) ?
				whiteRookSquare[0] : whiteRookSquare[1];
			if (gen::FILE[rookOne] == gen::FILE[rookTwo])
			{
				while (Z == position[rookOne += 10]);
				if (rookOne == rookTwo)
				{
					penalty[1] -= 8;
					end += 2;
				}
			}
			else if (gen::RANK[rookOne] == gen::RANK[rookTwo])
			{
				while (Z == position[rookOne += 1]);
				if (rookOne == rookTwo)
				{
					penalty[1] -= 8;
					end += 2;
				}
			}
		}

		//good and bad bishops
		if (lightBishop && lightPawns > 1)
		{
			penalty[1] += 4 * lightPawns;
			end -= 6 * lightPawns;
		}
		if (darkBishop && darkPawns > 1)
		{
			penalty[1] += 4 * darkPawns;
			end -= 6 * darkPawns;
		}

		int whitePawnTotal = lightPawns + darkPawns;

		//bishop pair bonus
		if (2 == bishopCount)
		{
			penalty[1] -= 45;
			end += 55;
		}
		//black formation
		pieces = b.blackPieces;
		for (int i = 1; i <= 6; i++)
		{
			int pawnNumber = pawnSkeleton[i];

			if (pawnNumber)
			{
				//stacked pawns and open files
				if (2 == pawnNumber)
				{
					penalty[0] += 40;
					end += 50;
				}
				else if (2 < pawnNumber)
				{
					penalty[0] += 130;
					end += 150;
				}
				//passed pawns
				int mostAdvanced = pawnSkeleton[8 + i];
				int pawnPosition = mostAdvanced * 10 + 21 + i;

				if (!pawnSkeleton[24 + i] || mostAdvanced < pawnSkeleton[40 + i])
				{
					int bonus = 0;
					if (!pawnSkeleton[24 + i - 1] && !pawnSkeleton[24 + i + 1])
					{
						bonus += 40;
						if (BP == position[pawnPosition + 9] || BP == position[pawnPosition + 11])
							bonus += 15;
					}
					else if (mostAdvanced <= pawnSkeleton[40 + i - 1] &&
						mostAdvanced <= pawnSkeleton[40 + i + 1])
					{
						bonus += 40;
						if (BP == position[pawnPosition + 9] || BP == position[pawnPosition + 11])
							bonus += 15;
					}

					int temp = bonus * (16 - mostAdvanced);
					penalty[0] -= temp / 12;//scaled bonus
					end -= temp / 8;
				}


				//isolated pawns
				if (!pawnSkeleton[i - 1] && !pawnSkeleton[i + 1])
				{
					penalty[0] += 30;
					end += 40;
				}
				//backward pawns and holes
				else
				{
					int leastRank = pawnSkeleton[16 + i];
					int leftLeast = pawnSkeleton[16 + i - 1];
					int rightLeast = pawnSkeleton[16 + i + 1];
					if (leastRank > rightLeast && leastRank > leftLeast)
					{
						penalty[0] += 10;
						end += 30;
						if (leftLeast > pawnSkeleton[40 + i + 1])
							penalty[0] += 5;
						if (rightLeast > pawnSkeleton[40 + i - 1])
							penalty[0] += 5;
						if (WP == position[(leftLeast + 1) * 10 + i])
							penalty[0] += 15;
						if (WP == position[(rightLeast + 1) * 10 + i + 2])
							penalty[0] += 15;
					}
				}
			}
			else//open file
			{
				//holes
				fileStatus[i]++;
				int leftLeast = pawnSkeleton[16 + i - 1];
				int rightLeast = pawnSkeleton[16 + i + 1];
				int least = (leftLeast > rightLeast) ? leftLeast : rightLeast;
				int outpost = least * 10 + 21 + i;

				if (WB == position[outpost] || WN == position[outpost])
				{
					if (WP == position[outpost - 11])
						penalty[0] += 8;
					if (WP == position[outpost - 9])
						penalty[0] += 8;
				}
			}
		}

		//black flank pawns
		for (int i = 0; i <= 7; i += 7)
		{
			int pawnNumber = pawnSkeleton[i];
			int adjacentFile = (0 == i) ? 1 : 6;


			if (pawnNumber)
			{
				//stacked pawns and open files
				if (2 == pawnNumber)
				{
					penalty[0] += 50;
					end += 55;
				}
				else if (2 < pawnNumber)
				{
					penalty[0] += 160;
					end += 170;
				}
				//passed pawns
				int mostAdvanced = pawnSkeleton[8 + i];
				int pawnPosition = mostAdvanced * 10 + 21 + i;
				int supportOffset = (0 == i) ? 11 : 9;

				if (!pawnSkeleton[24 + i] || mostAdvanced < pawnSkeleton[40 + i])
				{
					int bonus = 0;
					if (!pawnSkeleton[24 + adjacentFile])
					{
						bonus += 45;
						if (BP == position[pawnPosition + supportOffset])
							bonus += 15;
					}
					else if (mostAdvanced <= pawnSkeleton[40 + adjacentFile])
					{
						bonus += 45;
						if (BP == position[pawnPosition + supportOffset])
							bonus += 15;
					}

					int temp = bonus * (16 - mostAdvanced);
					penalty[0] -= temp / 12;//scaled bonus
					end -= temp / 8;
				}

				//isolated pawns
				if (!pawnSkeleton[adjacentFile])
				{
					penalty[0] += 25;
					end += 35;
				}
				//backward pawns and holes
				else
				{
					int leastRank = pawnSkeleton[16 + i];
					int adjacentLeast = pawnSkeleton[16 + adjacentFile];
					if (leastRank > adjacentLeast)
					{
						penalty[0] += 1;
						end += 1;
						if (adjacentLeast > pawnSkeleton[40 + adjacentFile])
							penalty[0] += 1;
						if (WP == position[(adjacentLeast + 1) * 10 + adjacentFile + 1])
							penalty[0] += 25;
					}
				}
			}
			else//open file
				fileStatus[i]++;
			//discount flank holes
		}

		//white formation
		pieces = b.whitePieces;
		for (int i = 1; i <= 6; i++)
		{
			int pawnNumber = pawnSkeleton[24 + i];

			if (pawnNumber)
			{
				//stacked pawns and open files
				if (2 == pawnNumber)
				{
					penalty[1] += 40;
					end -= 50;
				}
				else if (2 < pawnNumber)
				{
					penalty[1] += 130;
					end -= 150;
				}
				//passed pawns
				int mostAdvanced = pawnSkeleton[32 + i];
				int pawnPosition = mostAdvanced * 10 + 21 + i;

				if (!pawnSkeleton[i] || mostAdvanced > pawnSkeleton[16 + i])
				{
					int bonus = 0;
					if (!pawnSkeleton[24 + i - 1] && !pawnSkeleton[24 + i + 1])
					{
						bonus += 40;
						if (WP == position[pawnPosition - 9] || WP == position[pawnPosition - 11])
							bonus += 15;
					}
					else if (mostAdvanced >= pawnSkeleton[16 + i - 1] &&
						mostAdvanced >= pawnSkeleton[16 + i + 1])
					{
						bonus += 40;
						if (WP == position[pawnPosition - 9] || WP == position[pawnPosition - 11])
							bonus += 15;
					}

					int temp = bonus * (9 + mostAdvanced);
					penalty[1] -= (bonus * (9 + mostAdvanced)) / 12;//scaled bonus
					end += temp / 8;
				}

				//isolated pawns
				if (!pawnSkeleton[24 + i - 1] && !pawnSkeleton[24 + i + 1])
				{
					penalty[1] += 30;
					end -= 40;
				}
				//backward pawns and holes
				if (pawnSkeleton[24 + i - 1] || pawnSkeleton[24 + i + 1])
				{
					int leastRank = pawnSkeleton[40 + i];
					int leftLeast = pawnSkeleton[40 + i - 1];
					int rightLeast = pawnSkeleton[40 + i + 1];
					if (leastRank < rightLeast && leastRank < leftLeast)
					{
						penalty[1] += 10;
						end -= 30;
						if (leftLeast < pawnSkeleton[16 + i + 1])
							penalty[1] += 5;
						if (rightLeast < pawnSkeleton[16 + i - 1])
							penalty[1] += 5;
						if (BP == position[(leftLeast + 3) * 10 + i])
							penalty[1] += 15;
						if (BP == position[(rightLeast + 3) * 10 + i + 2])
							penalty[1] += 15;
					}
				}
			}
			else//open file
			{
				//holes
				fileStatus[i] += 2;
				int leftLeast = pawnSkeleton[40 + i - 1];
				int rightLeast = pawnSkeleton[40 + i + 1];
				int least = (leftLeast < rightLeast) ? leftLeast : rightLeast;
				int outpost = least * 10 + 21 + i;

				if (BB == position[outpost] || BN == position[outpost])
				{
					if (BP == position[outpost + 11])
						penalty[0] += 8;
					if (BP == position[outpost + 9])
						penalty[0] += 8;
				}
			}
		}

		//white flank pawns
		for (int i = 0; i <= 7; i += 7)
		{
			int pawnNumber = pawnSkeleton[24 + i];
			int adjacentFile = (0 == i) ? 1 : 6;

			if (pawnNumber)
			{
				//stacked pawns and open files
				if (2 == pawnNumber)
				{
					penalty[1] += 50;
					end -= 55;
				}
				else if (2 < pawnNumber)
				{
					penalty[1] += 160;
					end -= 170;
				}
				//passed pawns
				int mostAdvanced = pawnSkeleton[32 + i];
				int pawnPosition = mostAdvanced * 10 + 21 + i;
				int supportOffset = (0 == i) ? -11 : -9;

				if (!pawnSkeleton[i] || mostAdvanced > pawnSkeleton[16 + i])
				{
					int bonus = 0;
					if (!pawnSkeleton[adjacentFile])
					{
						bonus += 45;
						if (WP == position[pawnPosition + supportOffset])
							bonus += 15;
					}
					else if (mostAdvanced >= pawnSkeleton[16 + adjacentFile])
					{
						bonus += 45;
						if (WP == position[pawnPosition + supportOffset])
							bonus += 15;
					}

					int temp = bonus * (9 + mostAdvanced);
					penalty[1] -= (bonus * (9 + mostAdvanced)) / 12;//scaled bonus
					end += temp / 8;
				}

				//isolated pawns
				if (!pawnSkeleton[24 + adjacentFile])
				{
					penalty[1] += 25;
					end -= 35;
				}
				//backward pawns and holes
				else
				{
					int leastRank = pawnSkeleton[40 + i];
					int adjacentLeast = pawnSkeleton[40 + adjacentFile];
					if (leastRank < adjacentLeast)
					{
						penalty[1] += 1;
						end -= 1;
						if (adjacentLeast < pawnSkeleton[16 + adjacentFile])
							penalty[1] += 1;
						if (BP == position[(adjacentLeast + 3) * 10 + adjacentFile + 1])
							penalty[1] += 25;
					}
				}
			}
			else//open file
				fileStatus[i] += 2;
			//discount flank holes
		}

		//black king open file penalty

		int blackKingSquare = b.blackPieces[0].square;
		int blackKingFile = gen::FILE[blackKingSquare] - 1;
		if (WQ == b.whitePieces[1].piece)
		{
			switch (fileStatus[blackKingFile])
			{
			case 0: break;
			case 1: penalty[0] += 4; break;
			case 2: penalty[0] += 6; break;//open for other side
			case 3: penalty[0] += 12; break;
			}
			if (0 < blackKingFile)
			{
				switch (fileStatus[blackKingFile - 1])
				{
				case 0: break;
				case 1: penalty[0] += 2; break;
				case 2: penalty[0] += 3; break;//open for other side
				case 3: penalty[0] += 8; break;
				}
			}
			if (7 > blackKingFile)
			{
				switch (fileStatus[blackKingFile + 1])
				{
				case 0: break;
				case 1: penalty[0] += 2; break;
				case 2: penalty[0] += 3; break;//open for other side
				case 3: penalty[0] += 8; break;
				}
			}
		}

		//white king open file penalty
		int whiteKingSquare = b.whitePieces[0].square;
		int whiteKingFile = gen::FILE[whiteKingSquare] - 1;
		if (BQ == b.blackPieces[1].piece)
		{
			switch (fileStatus[whiteKingFile])
			{
			case 0: break;
			case 1: penalty[1] += 6; break;//open for other side
			case 2: penalty[1] += 4; break;
			case 3: penalty[1] += 12; break;
			}
			if (0 < whiteKingFile)
			{
				switch (fileStatus[whiteKingFile - 1])
				{
				case 0: break;
				case 1: penalty[1] += 3; break;
				case 2: penalty[1] += 2; break;//open for other side
				case 3: penalty[1] += 8; break;
				}
			}
			if (7 > whiteKingFile)
			{
				switch (fileStatus[whiteKingFile + 1])
				{
				case 0: break;
				case 1: penalty[1] += 3; break;
				case 2: penalty[1] += 2; break;//open for other side
				case 3: penalty[1] += 8; break;
				}
			}
		}

		//black rook open file bonus
		for (int i = 0; i < blackRookCount; i++)
		{
			int blackRookFile = gen::FILE[blackRookSquare[i]];
			switch (fileStatus[blackRookFile - 1])
			{
			case 1: penalty[0] -= 5;
				if (whiteKingFile == blackRookFile)
					penalty[0] -= 5;
				else if (whiteKingFile - 1 == blackRookFile)
					penalty[0] -= 3;
				else if (whiteKingFile + 1 == blackRookFile)
					penalty[0] -= 3;
				break;
			case 3: penalty[0] -= 15;
				if (whiteKingFile == blackRookFile)
					penalty[0] -= 10;
				else if (whiteKingFile - 1 == blackRookFile)
					penalty[0] -= 4;
				else if (whiteKingFile + 1 == blackRookFile)
					penalty[0] -= 4;
				break;
			default: break;
			}
		}

		//white rook open file bonus
		for (int i = 0; i < whiteRookCount; i++)
		{
			int whiteRookFile = gen::FILE[whiteRookSquare[i]];
			switch (fileStatus[whiteRookFile - 1])
			{
			case 2: penalty[1] -= 5;
				if (blackKingFile == whiteRookFile)
					penalty[1] -= 5;
				else if (blackKingFile - 1 == whiteRookFile)
					penalty[1] -= 3;
				else if (blackKingFile + 1 == whiteRookFile)
					penalty[1] -= 3;
				break;
			case 3: penalty[1] -= 15;
				if (blackKingFile == whiteRookFile)
					penalty[1] -= 10;
				else if (blackKingFile - 1 == whiteRookFile)
					penalty[1] -= 4;
				else if (blackKingFile + 1 == whiteRookFile)
					penalty[1] -= 4;
				break;
			default: break;
			}
		}

		//extra pawn stuff here
		if (BP == b.blackPieces[1].piece && WP == b.whitePieces[1].piece)//pawn endings
		{
			if (blackPawnTotal > whitePawnTotal && 1 < blackPawnTotal)
				end -= 64;
			else if (whitePawnTotal > blackPawnTotal && 1 < whitePawnTotal)
				end += 64;
		}

		return ((penalty[0] - penalty[1]) * (1024 - (whitePhase + blackPhase))
			+ end * (whitePhase + blackPhase)
			+ 8 * (weights::ENDGAME_KING[whiteKingSquare] * blackPhase
			- weights::ENDGAME_KING[blackKingSquare] * whitePhase))
			/ 1024;//will be multipled by toMove and added
				   //		return end;
	}//end formationEval()

	bool isPawnPromoting(gen::Board& b)
	{//does not test for pins
		Colour toMove = b.currentState.toMove;
		Piece* position = b.position;
		if (WHITE == toMove)
		{
			for (int i = 81; i <= 88; i++)
			{
				if (WP == position[i])
				{
					if (Z == position[i + 10] || 0 > position[i + 9] || 0 > position[i + 11])
						return true;
				}
			}
		}
		else
		{
			for (int i = 31; i <= 38; i++)
			{
				if (BP == position[i])
				{
					if (Z == position[i - 10] || 1 < position[i - 9] || 1 < position[i - 11])
						return true;
				}
			}
		}

		return false;
	}

	bool isNonStalemateDraw(gen::Board& b)
	{	//draw by 50 move rule
		gen::GameState& currentState = b.currentState;
		if (50 == currentState.fifty)
			return true;
		//TODO: draw by threefold repetition
		//draw by insufficient material
		gen::PieceSquare* blackPieces = b.blackPieces;
		gen::PieceSquare* whitePieces = b.whitePieces;
		int i = 1;
		Piece nextPiece = blackPieces[i].piece;
		while (X > nextPiece)
		{
			if (BR <= nextPiece)
				return false;
			if (BP == nextPiece)
				return false;
			if (BB == blackPieces[i + 1].piece)//i+1 causes problems??
				return false;//bishop pair
			nextPiece = blackPieces[++i].piece;
		}
		i = 1;
		nextPiece = whitePieces[i].piece;
		while (1 < nextPiece)
		{
			if (WR >= nextPiece)
				return false;
			if (WP == nextPiece)
				return false;
			if (WB == whitePieces[i + 1].piece)
				return false;//bishop pair
			nextPiece = whitePieces[++i].piece;
		}
		return true;
	}

	int testGameEnd(gen::Board& b)
	{
		if (isNonStalemateDraw(b))
		{
			std::cout << "Draw!" << std::endl;
			return 1;
		}
		int legalMoveCount = 0;
		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;
		for (int i = 0; i < moveList.size(); i++)
		{
			b.makeMove(moveList[i]);
			if (b.isValid())
				legalMoveCount++;
			b.unmakeMove();
		}

		if (0 == legalMoveCount)
		{
			if (b.isInCheck())
				std::cout << "Checkmate!" << std::endl;
			else
			{
				std::cout << "Stalemate!" << std::endl;
				b.displayLists();
			}
			return 1;
		}

		return 0;
	}
}//end eval

namespace srch
{
	const int NOffset[] = { -21, -19, -12, -8, 8, 12, 19, 21 };
	const int BOffset[] = { -11, -9, 9, 11 };
	const int ROffset[] = { -10, -1, 1, 10 };
	const int KOffset[] = { -11, -10, -9, -1, 1, 9, 10, 11 };
	Piece BattackingPiece[] = { BN, BB, BR, BQ, BK };
	Piece WattackingPiece[] = { WN, WB, WR, WQ, WK };
	const int captureValue[] = { weights::PAWN,  weights::KNIGHT,
		weights::BISHOP, weights::ROOK,
		weights::QUEEN, weights::KING };
	Piece blackAttackers[16] = { X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
	Piece whiteAttackers[16] = { X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X };
	Piece batteryAttackers[2] = { X, X };//only up to triple batteries
	int behindIndex[2] = { 0, 0 };
	int exchangeSequence[32] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


	//see()
	int Engine::see(int square, Colour toMove)
	{
		Piece* position = b.position;
		if (0 <= toMove * position[square] || 1 == position[square])
			return 0;//wrong input
		int blackIndex = 0;
		int whiteIndex = 0;

		//find all the attackers

		//pawns first
		if (BP == position[square + 9])
			blackAttackers[blackIndex++] = BP;
		if (BP == position[square + 11])
			blackAttackers[blackIndex++] = BP;

		//then all the other pieces
		const int* offSet = NOffset;
		int size;
		for (int i = 0; i < 5; i++)
		{
			Piece attackingPiece = BattackingPiece[i];
			switch (attackingPiece)
			{
			case BN: offSet = NOffset; size = 8; break;
			case BB: offSet = BOffset; size = 4; goto sliders_black;
			case BR: offSet = ROffset; size = 4; goto sliders_black;
			case BQ: offSet = KOffset; size = 8; goto sliders_black;
			case BK: offSet = KOffset; size = 8; break;
			}

			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				if (attackingPiece == position[square + k])
					blackAttackers[blackIndex++] = attackingPiece;
			}
			continue;
		sliders_black:
			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				int currentSquare = square + k;
				while (Z == position[currentSquare])
					currentSquare += k;
				if (attackingPiece == position[currentSquare])
					blackAttackers[blackIndex++] = attackingPiece;
				else
					continue;
				//check for batteries
				Piece front = attackingPiece;
				if (BQ == front)
				{
					if (-11 == k || -9 == k || 9 == k || 11 == k)
						front = BB;
					else
						front = BR;
				}
				while (Z == position[currentSquare += k]);
				Piece behind = position[currentSquare];
				if (front == behind || BQ == behind)
				{
					batteryAttackers[0] = behind;
					behindIndex[0] = blackIndex;
				}
				else
					continue;
				if (attackingPiece == BB)
					break;//triple batteries only for rooks
				while (Z == position[currentSquare += k]);
				Piece rear = position[currentSquare];
				if (front == rear || BQ == rear)
				{
					batteryAttackers[1] = rear;
					behindIndex[1] = blackIndex + 1;
				}
			}
		}

		//insert battery attackers
		for (int i = 0; i < 2; i++)
		{
			Piece& battery = batteryAttackers[i];
			if (!battery)
				break;//stop if no batteries
			int j = behindIndex[i];
			for (; battery < blackAttackers[j]; j++)
				if (!blackAttackers[j])
					goto battery_insert_black;
			for (int k = 8; k > j; k--)
				blackAttackers[k] = blackAttackers[k - 1];
		battery_insert_black:
			blackAttackers[j] = battery;
			battery = X;
			behindIndex[i] = 0;
		}

		//pawns first
		if (WP == position[square - 9])
			whiteAttackers[whiteIndex++] = WP;
		if (WP == position[square - 11])
			whiteAttackers[whiteIndex++] = WP;
		//then all the other pieces
		for (int i = 0; i < 5; i++)
		{
			Piece attackingPiece = WattackingPiece[i];
			switch (attackingPiece)
			{
			case WN: offSet = NOffset; size = 8; break;
			case WB: offSet = BOffset; size = 4; goto sliders_white;
			case WR: offSet = ROffset; size = 4; goto sliders_white;
			case WQ: offSet = KOffset; size = 8; goto sliders_white;
			case WK: offSet = KOffset; size = 8; break;
			}

			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				if (attackingPiece == position[square + k])
					whiteAttackers[whiteIndex++] = attackingPiece;
			}
			continue;
		sliders_white:
			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				int currentSquare = square + k;
				while (Z == position[currentSquare])
					currentSquare += k;
				if (attackingPiece == position[currentSquare])
					whiteAttackers[whiteIndex++] = attackingPiece;
				else
					continue;
				//check for batteries
				Piece front = attackingPiece;
				if (WQ == front)
				{
					if (-11 == k || -9 == k || 9 == k || 11 == k)
						front = WB;
					else
						front = WR;
				}
				while (Z == position[currentSquare += k]);
				Piece behind = position[currentSquare];
				if (front == behind || WQ == behind)
				{
					batteryAttackers[0] = behind;
					behindIndex[0] = whiteIndex;
				}
				else
					continue;
				if (front == WB)
					continue;//triple batteries only for rooks
				while (Z == position[currentSquare += k]);
				Piece rear = position[currentSquare];
				if (front == rear || WQ == rear)
				{
					batteryAttackers[1] = rear;
					behindIndex[1] = whiteIndex + 1;
				}
			}
		}

		//insert battery attackers
		for (int i = 0; i < 2; i++)
		{
			Piece& battery = batteryAttackers[i];
			if (!battery)
				break;//stop if no batteries
			int j = behindIndex[i];
			for (; battery > whiteAttackers[j]; j++)
				if (!whiteAttackers[j])
					goto battery_insert_white;
			for (int k = 8; k > j; k--)
				whiteAttackers[k] = whiteAttackers[k - 1];
		battery_insert_white:
			whiteAttackers[j] = battery;
			battery = X;
			behindIndex[i] = 0;
		}

		//fill exchange sequence
		Piece* attackers = (BLACK == toMove) ? blackAttackers : whiteAttackers;
		Piece attacker = attackers[0];
		if (!attacker)
			return 0;//no piece attacking square
		exchangeSequence[0] = eval::materialAbsolute(position[square]);
		int attackerIndex = 1;
		while (attacker)
		{
			exchangeSequence[attackerIndex] = eval::materialAbsolute(attacker) - exchangeSequence[attackerIndex - 1];
			attackerIndex++;
			attackers = (blackAttackers == attackers) ? whiteAttackers : blackAttackers;
			attacker = attackers[(attackerIndex - 1) / 2];
		}

		//evaluate exchanges
		attackerIndex--;
		while (attackerIndex > 1)
		{
			attackerIndex--;
			if (exchangeSequence[attackerIndex - 1] > -exchangeSequence[attackerIndex])
				exchangeSequence[attackerIndex - 1] = -exchangeSequence[attackerIndex];
		}

		int seeValue = exchangeSequence[0];

		//clear all buffers
		for (attackerIndex = 0; attackerIndex < 16; attackerIndex++)
			exchangeSequence[attackerIndex] = 0;
		for (attackerIndex = 0; blackAttackers[attackerIndex]; attackerIndex++)
			blackAttackers[attackerIndex] = X;
		for (attackerIndex = 0; whiteAttackers[attackerIndex]; attackerIndex++)
			whiteAttackers[attackerIndex] = X;

		return seeValue;
	}//end see()

	 //seeCapture()
	int Engine::seeCapture(gen::Move capture)
	{
		if (BK == capture.captured || WK == capture.captured)
			return POS_INFINITY;
		Piece* position = b.position;
		int square = capture.to;
		Colour toMove = b.currentState.toMove;
		Piece initialAttacker = b.position[capture.from];
		int blackIndex = 0;
		int whiteIndex = 0;

		//find all the attackers

		//pawns first
		if (BP == position[square + 9])
			blackAttackers[blackIndex++] = BP;
		if (BP == position[square + 11])
			blackAttackers[blackIndex++] = BP;

		//then all the other pieces
		const int* offSet = NOffset;
		int size;
		for (int i = 0; i < 5; i++)
		{
			Piece attackingPiece = BattackingPiece[i];
			switch (attackingPiece)
			{
			case BN: offSet = NOffset; size = 8; break;
			case BB: offSet = BOffset; size = 4; goto sliders_black_capture;
			case BR: offSet = ROffset; size = 4; goto sliders_black_capture;
			case BQ: offSet = KOffset; size = 8; goto sliders_black_capture;
			case BK: offSet = KOffset; size = 8; break;
			}

			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				if (attackingPiece == position[square + k])
					blackAttackers[blackIndex++] = attackingPiece;
			}
			continue;
		sliders_black_capture:
			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				int currentSquare = square + k;
				while (Z == position[currentSquare])
					currentSquare += k;
				if (attackingPiece == position[currentSquare])
					blackAttackers[blackIndex++] = attackingPiece;
				else
					continue;
				//check for batteries
				Piece front = attackingPiece;
				if (BQ == front)
				{
					if (-11 == k || -9 == k || 9 == k || 11 == k)
						front = BB;
					else
						front = BR;
				}
				while (Z == position[currentSquare += k]);
				Piece behind = position[currentSquare];
				if (front == behind || BQ == behind)
				{
					batteryAttackers[0] = behind;
					behindIndex[0] = blackIndex;
				}
				else
					continue;//may still be rook from another direction
				if (front == BB)//assume no 2 bishops on same colours
					break;//triple batteries only for rooks
				while (Z == position[currentSquare += k]);
				Piece rear = position[currentSquare];
				if (front == rear || BQ == rear)
				{
					batteryAttackers[1] = rear;
					behindIndex[1] = blackIndex + 1;
				}
			}
		}

		//exceptional case for Queen/Bishop battery
		if (BQ == initialAttacker)
			behindIndex[0] = (BB == batteryAttackers[0]) ? 1 : behindIndex[0];

		//insert battery attackers
		for (int i = 0; i < 2; i++)
		{
			Piece& battery = batteryAttackers[i];
			if (!battery)
				break;//stop if no batteries
			int j = behindIndex[i];
			for (; battery < blackAttackers[j]; j++)
				if (!blackAttackers[j])
					goto battery_insert_black;
			for (int k = 8; k > j; k--)
				blackAttackers[k] = blackAttackers[k - 1];
		battery_insert_black:
			blackAttackers[j] = battery;
			battery = X;
			behindIndex[i] = 0;
		}

		//pawns first
		if (WP == position[square - 9])
			whiteAttackers[whiteIndex++] = WP;
		if (WP == position[square - 11])
			whiteAttackers[whiteIndex++] = WP;
		//then all the other pieces
		for (int i = 0; i < 5; i++)
		{
			Piece attackingPiece = WattackingPiece[i];
			switch (attackingPiece)
			{
			case WN: offSet = NOffset; size = 8; break;
			case WB: offSet = BOffset; size = 4; goto sliders_white_capture;
			case WR: offSet = ROffset; size = 4; goto sliders_white_capture;
			case WQ: offSet = KOffset; size = 8; goto sliders_white_capture;
			case WK: offSet = KOffset; size = 8; break;
			}

			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				if (attackingPiece == position[square + k])
					whiteAttackers[whiteIndex++] = attackingPiece;
			}
			continue;
		sliders_white_capture:
			for (int j = 0; j < size; j++)
			{
				int k = offSet[j];
				int currentSquare = square + k;
				while (Z == position[currentSquare])
					currentSquare += k;
				if (attackingPiece == position[currentSquare])
					whiteAttackers[whiteIndex++] = attackingPiece;
				else
					continue;
				//check for batteries
				Piece front = attackingPiece;
				if (WQ == front)
				{
					if (-11 == k || -9 == k || 9 == k || 11 == k)
						front = WB;
					else
						front = WR;
				}
				while (Z == position[currentSquare += k]);
				Piece behind = position[currentSquare];
				if (front == behind || WQ == behind)
				{
					batteryAttackers[0] = behind;
					behindIndex[0] = whiteIndex;
				}
				else
					continue;
				if (front == WB)
					break;//triple batteries only for rooks
				while (Z == position[currentSquare += k]);
				Piece rear = position[currentSquare];
				if (front == rear || WQ == rear)
				{
					batteryAttackers[1] = rear;
					behindIndex[1] = whiteIndex + 1;
				}
			}
		}

		//exceptional case for Queen/Bishop battery
		if (WQ == initialAttacker)
			behindIndex[0] = (WB == batteryAttackers[0]) ? 1 : behindIndex[0];

		//insert battery attackers
		for (int i = 0; i < 2; i++)
		{
			Piece& battery = batteryAttackers[i];
			if (!battery)
				break;//stop if no batteries
			int j = behindIndex[i];
			for (; battery > whiteAttackers[j]; j++)
				if (!whiteAttackers[j])
					goto battery_insert_white;
			for (int k = 8; k > j; k--)
				whiteAttackers[k] = whiteAttackers[k - 1];
		battery_insert_white:
			whiteAttackers[j] = battery;
			battery = X;
			behindIndex[i] = 0;
		}

		//fill exchange sequence
		Piece* attackers = (BLACK == toMove) ? blackAttackers : whiteAttackers;
		Piece attacker = attackers[0];
		if (!attacker)
			return 0;//no piece attacking square
		int initialAttackerIndex = 0;
		while (initialAttacker != attacker)//maybe a &&attacker for safety?
			attacker = attackers[++initialAttackerIndex];

		//16 pieces on board of same colour
		if (initialAttackerIndex >= 16)
		{
			std::cout << "lastmove ";
			misc::move2display(gen::movesMade[gen::moveCount - 1]);
			misc::move2display(capture);
			b.display();
			std::cout << "too large " << std::endl;
			std::cout << "initial attacker is ";
			misc::piece2display(initialAttacker);
			std::cout << std::endl << "attackers vector " << std::endl;
			for (int i = 0; i < 8; i++)
			{
				misc::piece2display(attackers[i]);
				std::cout << std::endl;
			}
		}
		while (initialAttackerIndex > 0)
			initialAttackerIndex--;//can skip setting attackers[0] = initialAttacker

		attacker = initialAttacker;
		exchangeSequence[0] = eval::materialAbsolute(position[square]);
		int attackerIndex = 1;
		while (attacker)
		{
			exchangeSequence[attackerIndex] = eval::materialAbsolute(attacker) - exchangeSequence[attackerIndex - 1];
			attackerIndex++;
			attackers = (blackAttackers == attackers) ? whiteAttackers : blackAttackers;
			attacker = attackers[(attackerIndex - 1) / 2];
		}

		//evaluate exchanges
		attackerIndex--;
		while (attackerIndex > 1)
		{
			attackerIndex--;
			if (exchangeSequence[attackerIndex - 1] > -exchangeSequence[attackerIndex])
				exchangeSequence[attackerIndex - 1] = -exchangeSequence[attackerIndex];
		}

		int seeValue = exchangeSequence[0];

		//clear all buffers
		for (attackerIndex = 0; attackerIndex < 16; attackerIndex++)
			exchangeSequence[attackerIndex] = 0;
		for (attackerIndex = 0; blackAttackers[attackerIndex]; attackerIndex++)
			blackAttackers[attackerIndex] = X;
		for (attackerIndex = 0; whiteAttackers[attackerIndex]; attackerIndex++)
			whiteAttackers[attackerIndex] = X;

		return seeValue;
	}//end seeCapture()

	 //findQuiescence()
	int Engine::findQuiescence(int alpha, int beta, int depthLeft)
	{
		int legalMoveCount = 0;
		int standPat = eval::basicEval(b);
		int score = NEG_INFINITY;
		if (standPat >= beta)
			return standPat;

		//check if any move can improve alpha
		int DELTA_MAX = weights::QUEEN;
		if (eval::isPawnPromoting(b))
			DELTA_MAX += weights::QUEEN - weights::PAWN;
		if (standPat + DELTA_MAX < alpha)
			return alpha;
		if (alpha < standPat)
			alpha = standPat;

		gen::Generator gen;
		gen.generate(b);
		std::vector < gen::Move >& moveList = gen.moveList;
		const gen::Move& lastMove = gen::movesMade[gen::moveCount - 1];
		Piece* position = b.position;
		int queenMoves = 0;

		//counterCapture heuristic
		int foundCounter = 0;
		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int from = move.from;
			int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
			if (counterC[0] == from && counterC[1] == move.to)
			{
				gen::Move temp = moveList[0];
				moveList[0] = move;
				move = temp;
				foundCounter += 1;
			}

			//see and delta ordering
			if (depthLeft >= 2 && move.captured)
			{
				//delta ordering (pruning for deeper than depth 10)
				if (!move.promotion && standPat + eval::materialAbsolute(move.captured) < -200)
					continue;//mobility likely insignificant here

				int& priority = move.priority;
				priority = seeCapture(move);
			}

			//count queen moves
			if (WQ == position[from] || BQ == position[from])
				queenMoves++;
		}

		//internal mobility
		mobilityBuffer[++mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;

		//see move ordering
		std::sort(moveList.begin() + foundCounter, moveList.end(), moveCompare);

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			if (move.captured)
			{
				//negative see pruning
				if (depthLeft <= QSEARCH_DEPTH / 2 && depthLeft >= 2 && move.priority < 0)
					continue;

				b.makeMove(move);
				if (b.isValid())
				{
					legalMoveCount++;
					score = -findQuiescence(-beta, -alpha, depthLeft - 1);
				}
				b.unmakeMove();

				if (score >= beta)
				{
					int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
					counterC[0] = move.from;
					counterC[1] = move.to;
					mobilityIndex--;
					return score;
				}
				if (score > alpha)
					alpha = score;
			}
			else
			{
				b.makeMove(move);
				if (b.isValid())
				{
					legalMoveCount++;
				}
				b.unmakeMove();
			}
		}
		mobilityIndex--;
		if (0 == legalMoveCount)
		{
			if (b.isInCheck())
			{
				return NEG_INFINITY + QSEARCH_DEPTH - depthLeft;//checkmated
			}
			else
				return DRAW_VALUE;//stalemated
		}

		return standPat;
	}//end findQuiescence()

	 //moveCompare()
	bool Engine::moveCompare(const gen::Move& a, const gen::Move& b)
	{
		return (a.priority > b.priority);
	}//end moveCompare()

	 //orderMoves()
	void Engine::orderMoves(gen::MoveList& moveList)
	{
		std::sort(moveList.begin(), moveList.end(), Engine::moveCompare);
	}
	//end orderMoves()

	//iterativeDeepening()
	void Engine::iterativeDeepening(int depth, gen::MoveList& moveList)
	{
		if (0 >= depth)//TODO: sort moves here; counter moves especially - note to clear priority afterwards
			return;

		iterativeDeepening(depth - 2, moveList);

		int currentEval = b.material + b.pst + eval::formationEval(b);

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			b.makeMove(move);
			if (b.isValid())
			{
				move.priority = -shallowSearch(currentEval - 200, currentEval + 200, depth - 1);
			}
			b.unmakeMove();
		}

		orderMoves(moveList);

		for (int i = 0; i < moveList.size(); i++)
			moveList[i].priority = NEG_INFINITY;

	}//end iterativeDeepening()

	 //internalDeepening()
	int Engine::internalDeepening(int alpha, int beta, int depth, gen::MoveList& moveList)
	{
		int bestScore = NEG_INFINITY;
		int i = 0;
		int bestMove = 0;

		for (; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			b.makeMove(move);
			if (NEG_INFINITY - 1000 < move.priority)//can optimise this
			{
				int score = -internalSearch(-beta, -alpha, depth - 1);

				if (score >= beta)
				{
					b.unmakeMove();//unmake before cut-off!
					return i;
				}
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = i;

					if (score > alpha)
						alpha = score;
				}
			}

			b.unmakeMove();//unmake
		}

		return bestMove;
	}//end internalDeepening()

	 //nullWindowSearch()
	int Engine::nullWindowSearch(int beta, int depth)
	{
		if (0 >= depth)
			return findQuiescence(beta - 1, beta, QSEARCH_DEPTH);

		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;

		int queenMoves = 0;
		Piece* position = b.position;

		//counterMove and counterCapture
		gen::Move& lastMove = gen::movesMade[gen::moveCount - 1];;
		for (int i = 0; i < moveList.size(); i++)
		{
			int foundCounter = 0;
			gen::Move& move = moveList[i];
			int from = move.from;
			int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
			int* counterM = &counterMove[lastMove.from * 240 + lastMove.to * 2];
			if (counterC[0] == move.from && counterC[1] == move.to)
			{
				gen::Move temp = moveList[0];
				moveList[0] = move;
				move = temp;
				foundCounter += 1;
			}
			if (counterM[0] == move.from && counterM[1] == move.to)
			{
				gen::Move temp = moveList[foundCounter];
				moveList[foundCounter] = move;
				move = temp;
				foundCounter += 1;
			}

			//count queen moves
			if (WQ == position[from] || BQ == position[from])
				queenMoves++;
		}

		//internal mobility
		if (1 == depth)
		{
			mobilityIndex = 1;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}
		else if (2 == depth)
		{
			mobilityIndex = 0;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}

		for (int i = 0; i < moveList.size(); i++)
		{
			int score = 0;
			b.makeMove(moveList[i]);
			if (b.isValid())
				score = -nullWindowSearch(1 - beta, depth - 1);

			b.unmakeMove();
			if (score >= beta)
				return score;
		}

		return beta - 1;
	}//end nullWindowSearch()

	 //internalSearch()
	int Engine::internalSearch(int alpha, int beta, int depth)
	{
		if (0 == depth)//be careful of any problems if not 0 <= depth
			return findQuiescence(alpha, beta, QSEARCH_DEPTH);
		//return eval::basicEval(b);

		depthReached++;

		int legalMoveCount = 0;
		int bestScore = NEG_INFINITY;
		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;

		const gen::Move& lastMove = gen::movesMade[gen::moveCount - 1];
		gen::GameState& currentState = b.currentState;
		Colour& toMove = currentState.toMove;
		int captureNumber = 0;
		int noncaptureNumber = 0;
		int captureIndex = 0;
		int noncaptureIndex = 0;
		int queenMoves = 0;
		Piece* position = b.position;

		//legality check
		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int from = move.from;
			if (WQ == position[from] || BQ == position[from])
				queenMoves++;
			b.makeMove(move);
			if (b.isValid())
			{
				legalMoveCount++;
				if (move.captured)
					captureNumber++;
				else
					noncaptureNumber++;
			}
			else
				move.priority -= POS_INFINITY;
			b.unmakeMove();
		}

		//game end detection
		bool inCheck = b.isInCheck();
		if (0 == legalMoveCount)
		{
			if (!inCheck)//stalemate
			{
				depthReached--;
				return DRAW_VALUE;
			}
			else//checkmate
			{
				depthReached--;
				return NEG_INFINITY + SEARCH_DEPTH - depth;
			}
		}

		//check extension and singular move extensions
		if (inCheck || 1 >= legalMoveCount)//TODO: need to adjust for moves to mate due to extensions (possibly history heuristic as well?)
			depth++;

		//internal mobility - after extensions!
		if (1 == depth)
		{
			mobilityIndex = 1;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}
		else if (2 == depth)
		{
			mobilityIndex = 0;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}

		//null move reduction
		else if (!inCheck && depth == 4 &&
			300 < b.material * toMove &&
			b.blackPieces[1].piece < BP && b.whitePieces[1].piece > WP)
		{
			int nullBeta = b.material * toMove + 300;;
			int nullScore = nullBeta - 1;

			//note that null move is is invalid if in check
			int& epSquare = currentState.epSquare;
			int temp = epSquare;
			//b.makeNullMove();//slower version of below
			toMove = (BLACK == toMove) ? WHITE : BLACK;
			if (2 <= gen::moveCount)
				gen::movesMade[gen::moveCount] = gen::movesMade[gen::moveCount - 2];
			gen::moveCount++;
			epSquare = 0;
			nullScore = -nullWindowSearch(1 - nullBeta, depth - REDUCTION_DEPTH - 1);
			//b.unmakeNullMove();
			toMove = (BLACK == toMove) ? WHITE : BLACK;
			gen::moveCount--;
			epSquare = temp;

			if (nullScore > beta)
			{
				if (4 == depth)
				{
					depth -= REDUCTION_DEPTH;
				}
			}
		}

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int& priority = move.priority;
			int from = move.from;
			int to = move.to;
			int toMove = b.currentState.toMove;
			int lastFromIndex = lastMove.from * 240;
			int lastToIndex = lastMove.to * 2;

			//history heuristic
			if (SEARCH_DEPTH - 4 <= depth)
				priority += historyTable[from * 120 + to] / 8;

			if (move.captured)
			{
				//lva/mvv ordering
				priority += 500;
				int captured = -toMove  * move.captured;
				int moving = toMove * b.position[move.from];
				int capturePriority = 10 * (captured - moving);
				priority += capturePriority;
				if (move.isPawnMove)
					priority += 1;
				priority += seeCapture(move);

				//counterCapture heuristic
				int* counterC = &counterCapture[lastFromIndex + lastToIndex];
				if (counterC[0] == from && counterC[1] == to)
					priority += 750;
			}
			else
			{
				//pst ordering
				Piece* position = b.position;
				int *pst = eval::getPST(position[from]);
				priority += toMove * (pst[to] - pst[from]);

				//killerMove heuristic
				if (move == killerMove[depthReached])
					priority += 150;

				//counterMove heuristic
				int* counterM = &counterMove[lastFromIndex + lastToIndex];
				if (counterM[0] == from && counterM[1] == to)
					priority += 50;
			}

			//prioritise castling and double pawn moves
			if (move.isDoubleMove)
			{
				priority += 10;
				if (!move.isPawnMove)
					priority += 100;
			}
			//prioritise promotions
			if (move.promotion)
			{
				priority += 200;
				if (BQ == move.promotion || WQ == move.promotion)
					priority += 100;
			}
		}

		orderMoves(moveList);

		if (4 < depth)
		{
			int topTryIndex = internalDeepening(alpha, beta, depth - 2, moveList);
			gen::Move shallowBest = moveList[topTryIndex];
			for (int i = topTryIndex; i >= 1; i--)
			{
				moveList[i] = moveList[i - 1];
			}
			moveList[0] = shallowBest;
		}

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int score;
			int isCapture = move.captured;

			//repetition detection
			if (5 <= gen::moveCount &&
				gen::movesMade[gen::moveCount - 1] == gen::movesMade[gen::moveCount - 5]
				&& move == gen::movesMade[gen::moveCount - 4])
			{
				score = DRAW_VALUE;
				if (score >= beta)
				{
					//std::cout << "cutoff generated in internalSearch" << std::endl;

					if (SEARCH_DEPTH - 4 <= depth)
						historyTable[move.from * 120 + move.to] += depth;
					if (isCapture)
					{
						int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
						counterC[0] = move.from;
						counterC[1] = move.to;
					}
					else
					{
						int* counterM = &counterMove[lastMove.from * 240 + lastMove.to * 2];
						counterM[0] = move.from;
						counterM[1] = move.to;
						killerMove[depthReached] = move;
					}
					depthReached--;
					return score;  // fail-soft beta-cutoff
				}
				if (score > bestScore)
				{
					bestScore = score;

					if (score > alpha)
						alpha = score;
					else
					{
						if (SEARCH_DEPTH - 4 <= depth)
							historyTable[move.from * 120 + move.to] -= depth * depth;
					}
				}
				else
				{
					if (SEARCH_DEPTH - 4 <= depth)
						historyTable[move.from * 120 + move.to]--;
				}
				continue;
			}

			b.makeMove(move);

			if (NEG_INFINITY - 1000 < move.priority)//same as b.isvalid()
			{
				if (isCapture)
					captureIndex++;
				else
					noncaptureIndex++;

				////late move reductions
				//if (4 < depth && !inCheck && !isCapture &&
				//	noncaptureIndex > noncaptureNumber / 4 + 2)
				//{
				//	if (noncaptureIndex > noncaptureNumber / 2 + 2)
				//		score = -internalSearch(-beta, -alpha, depth - 2);
				//	else
				//		score = -internalSearch(-beta, -alpha, depth - 1);
				//}

				score = -internalSearch(-beta, -alpha, depth - 1);

				if (score >= beta)
				{
					b.unmakeMove();
					//std::cout << "cutoff generated in internalSearch" << std::endl;

					if (SEARCH_DEPTH - 4 <= depth)
						historyTable[move.from * 120 + move.to] += depth;
					if (isCapture)
					{
						int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
						counterC[0] = move.from;
						counterC[1] = move.to;
					}
					else
					{
						int* counterM = &counterMove[lastMove.from * 240 + lastMove.to * 2];
						counterM[0] = move.from;
						counterM[1] = move.to;
						killerMove[depthReached] = move;
					}
					depthReached--;
					return score;  // fail-soft beta-cutoff
				}
				if (score > bestScore)
				{
					bestScore = score;

					if (score > alpha)
						alpha = score;
					else
					{
						if (SEARCH_DEPTH - 4 <= depth)
							historyTable[move.from * 120 + move.to] -= depth * depth;
					}
				}
				else
				{
					if (SEARCH_DEPTH - 4 <= depth)
						historyTable[move.from * 120 + move.to]--;
				}
			}

			b.unmakeMove();
		}
		depthReached--;
		return bestScore;
	}//end internalSearch()

	 //shallowSearch()
	int Engine::shallowSearch(int alpha, int beta, int depth)
	{
		if (0 == depth)//be careful of any problems if not 0 <= depth
			return findQuiescence(alpha, beta, QSEARCH_DEPTH);
		//return eval::basicEval(b);

		int legalMoveCount = 0;
		int bestScore = NEG_INFINITY;
		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;

		const gen::Move& lastMove = gen::movesMade[gen::moveCount - 1];
		gen::GameState& currentState = b.currentState;
		Colour& toMove = currentState.toMove;
		int queenMoves = 0;

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int& priority = move.priority;
			int from = move.from;
			int to = move.to;
			int lastFromIndex = lastMove.from * 240;
			int lastToIndex = lastMove.to * 2;
			Piece* position = b.position;

			//count queen moves
			if (WQ == position[from] || BQ == position[from])
				queenMoves++;

			//historyTable heuristic
			priority += historyTable[from * 120 + to];

			//lva/mvv ordering
			if (move.captured)
			{
				priority += 500;
				int colour = b.currentState.toMove;
				int captured = -colour * move.captured;
				int moving = colour * b.position[move.from];
				int capturePriority = 10 * (captured - moving);
				priority += capturePriority;
				if (move.isPawnMove)
					priority += 1;
				priority += seeCapture(move);

				//counterCapture heuristic
				int* counterC = &counterCapture[lastFromIndex + lastToIndex];
				int* counterM = &counterMove[lastFromIndex + lastToIndex];
				if (counterC[0] == from && counterC[1] == to)
					priority += 750;
			}
			else
			{
				//pst ordering
				int *pst = eval::getPST(position[from]);
				priority += toMove * (pst[to] - pst[from]);

				//counterMove heuristic
				int* counterM = &counterMove[lastFromIndex + lastToIndex];
				if (counterM[0] == from && counterM[1] == to)
					priority += 50;

			}
			//prioritise castling and double pawn moves
			if (move.isDoubleMove)
			{
				priority += 10;
				if (!move.isPawnMove)
					priority += 100;
			}
			//prioritise promotions
			if (move.promotion)
			{
				priority += 200;
				if (BQ == move.promotion || WQ == move.promotion)
					priority += 100;
			}
		}
		orderMoves(moveList);

		//internal mobility
		if (1 == depth)
		{
			mobilityIndex = 1;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}
		else if (2 == depth)
		{
			mobilityIndex = 0;
			mobilityBuffer[mobilityIndex] = 128 * moveList.size() - 32 * queenMoves;
		}

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int score;

			//repetition detection
			if (5 <= gen::moveCount &&
				gen::movesMade[gen::moveCount - 1] == gen::movesMade[gen::moveCount - 5]
				&& move == gen::movesMade[gen::moveCount - 4])
			{
				legalMoveCount++;
				score = DRAW_VALUE;

				if (score >= beta)
				{
					//					historyTable[move.from * 120 + move.to] += depth;//too early to decide good moves
					if (move.captured)
					{
						//						int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
						//						counterC[0] = move.from;
						//						counterC[1] = move.to;
					}
					else
					{
						//						int* counterM = &counterMove[lastMove.from * 240 + lastMove.to * 2];
						//						counterM[0] = move.from;
						//						counterM[1] = move.to;
					}
					return beta;  // fail-hard beta-cutoff
				}//has to be legal

				if (score > bestScore)
				{
					bestScore = score;

					if (score > alpha)
						alpha = score;
					//					else
					//						historyTable[move.from * 120 + move.to] -= depth;
				}
				//				else
				//					historyTable[move.from * 120 + move.to]--;

				continue;
			}

			b.makeMove(move);

			if (b.isValid())
			{
				legalMoveCount++;
				score = -internalSearch(-beta, -alpha, depth - 1);

				if (score >= beta)
				{
					b.unmakeMove();

					historyTable[move.from * 120 + move.to] += depth;//too early to decide good moves
					if (move.captured)
					{
						int* counterC = &counterCapture[lastMove.from * 240 + lastMove.to * 2];
						counterC[0] = move.from;
						counterC[1] = move.to;
					}
					else
					{
						int* counterM = &counterMove[lastMove.from * 240 + lastMove.to * 2];
						counterM[0] = move.from;
						counterM[1] = move.to;
					}
					return beta;  // fail-hard beta-cutoff
				}
				if (score > bestScore)
				{
					bestScore = score;

					if (score > alpha)
						alpha = score;
					else
						historyTable[move.from * 120 + move.to] -= depth;
				}
				else
					historyTable[move.from * 120 + move.to]--;
			}

			b.unmakeMove();
		}

		if (0 == legalMoveCount)
			if (!b.isInCheck())//stalemate
				return DRAW_VALUE;
			else//checkmate
				return NEG_INFINITY + SEARCH_DEPTH - depth;

		return bestScore;
	}
	//end shallowSearch()

	//search()
	gen::Move Engine::search(int alpha, int beta, int depth)
	{//alpha negative, beta positive, 1 for white and - 1 for black
	 //		clearCounterCaptureTable();
		int legalMoveCount = 0;
		int bestScore = NEG_INFINITY;
		gen::Move bestMove;
		gen::Generator gen;
		gen.generate(b);
		gen::MoveList& moveList = gen.moveList;

		//root iterative deepening
		iterativeDeepening(depth / 2 - 1, moveList);

		//		std::cout << "mobility index is now " << srch::mobilityIndex << std::endl;

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];
			int score;

			//repetition detection
			if (5 <= gen::moveCount &&
				gen::movesMade[gen::moveCount - 1] == gen::movesMade[gen::moveCount - 5]
				&& move == gen::movesMade[gen::moveCount - 4])
			{
				legalMoveCount++;
				score = DRAW_VALUE;
				if (score >= beta)
				{
					move.priority = beta;
					return move;
				}
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = move;
				}
				continue;
			}

			b.makeMove(move);

			if (b.isValid())
			{
				legalMoveCount++;
				score = -internalSearch(-beta, -alpha, depth - 1);
				move.priority = score;//can make sorted list of moves

				if (score >= beta)
				{
					b.unmakeMove();//unmake before cut-off!
					return move;
				}
				if (score > bestScore)
				{
					bestScore = score;
					bestMove = move;

					if (score > alpha)
						alpha = score;
				}

			}

			b.unmakeMove();//unmake
		}

		if (DEBUG)
			std::cout << "score: " << bestScore << std::endl;

		if (0 == legalMoveCount)
			if (!b.isInCheck())
				bestScore = DRAW_VALUE;
		//	else//quiescence search takes care of this already
		//		bestScore = NEG_INFINITY;

		orderMoves(moveList);

		return bestMove;
	}//search

	 //clearhistoryTable()
	void Engine::clearHistory()
	{
		for (int i = 0; i < 14400; i++)
			historyTable[i] = 0;

		for (int i = 0; i < 28800; i++)
		{
			counterCapture[i] = 0;
			counterMove[i] = 0;
		}

		gen::Move dummy;

		for (int i = 0; i < 20; i++)
			killerMove[i] = dummy;

	}//end clearhistoryTable()

	 //updatehistoryTable()
	void Engine::updateHistory()
	{
		for (int i = 0; i < 14400; i++)
			historyTable[i] /= 2;

		for (int i = 0; i < 19; i++)
			killerMove[i] = killerMove[i + 1];
	}//end updatehistoryTable()


}//end srch

namespace ui
{
	Driver::Driver() :xboardMode(false), forceMode(false), engineSide(BLACK)
	{}

	//force()
	void Driver::force()
	{
		forceMode = true;
	}//end force()

	 //go()
	void Driver::go()
	{
		forceMode = false;
		Colour toMove = eng.b.currentState.toMove;
		engineSide = toMove;
	}//end go()

	 //initiate()
	void Driver::initiate()
	{

		std::string selection;
		std::cin >> selection;
		if ("xboard" == selection)
		{
			xboardMode = true;
			std::cout.setf(std::ios::unitbuf);
		}

	}//end initiate()

	 //loop()
	void Driver::loop()
	{
		gen::Board& board = eng.b;
		std::string input;
		while ("quit" != input)
		{
			if (!xboardMode)
				board.display();

			if (!forceMode && board.currentState.toMove == engineSide &&
				!eval::testGameEnd(board))
			{
				eng.updateHistory();
				gen::Move& move = eng.search(NEG_INFINITY, POS_INFINITY, SEARCH_DEPTH);
				board.makeMove(move);
				std::cout << "move ";
				std::cout << misc::board2alpha(move.from);
				std::cout << misc::board2alpha(move.to);
				std::cout << std::endl;
			}
			else
			{
				std::cin >> input;
				switch (parseInput(input))
				{
				case 0: break;//ok
				case 1:
					std::cout << "Illegal move: " << input << std::endl;
					break;//illegal move
				case 2:
					std::cout << "Error (unknown command): " << input << std::endl;
					break;//unrecognised command
				}
			}
		}
	}//end loop()

	 //parseInput()
	int Driver::parseInput(std::string input)
	{
		std::ofstream fout;
		fout.open("D:\\commands_log.txt", std::ofstream::out | std::ofstream::app);
		fout << input << std::endl;
		if ("accepted" == input) { return 0; }
		else if ("draw" == input) { return 0; }//possibly implement later
		else if ("sigterm" == input) { return 0; }
		else if ("reuse" == input) { return 0; }
		else if ("analyze" == input) { return 0; }
		else if ("colors" == input) { return 0; }
		else if ("nps" == input) { return 0; }
		else if ("post" == input) { return 0; }
		else if ("protover" == input) 
		{ 
			sendFeatures(); 
		}
		else if ("new" == input)
		{
			startNew();
			eval::randomRange = 0;
			return 0;
		}
		else if ("random" == input)
		{
			eval::randomRange = RANDOM_RANGE;
			return 0;
		}
		else if ("level" == input)
		{
			int mps, base, inc;
			std::cin >> mps >> base >> inc;
			return 0;
		}
		else if ("force" == input)
		{
			force();
			return 0;
		}
		else if ("quit" == input)
			return -1;
		else if ("hard" == input) { return 0; }
		else if ("easy" == input) { return 0; }
		else if ("time" == input)
		{
			int time;
			std::cin >> time;
			std::cout << "Error (unknown command): time" << std::endl;
			return 0;
		}
		else if ("otim" == input)
		{
			int otim;
			std::cin >> otim;
			return 0;
		}
		else if ("setboard" == input)
		{
			std::string fen;
			std::cin >> fen;
			eng.b.setFen(fen);
			return 0;
		}
		else if ("computer" == input) { return 0; }
		else if ("?" == input)
		{
			forceMode = false;
			engineSide = eng.b.currentState.toMove;
			return 0;
		}
		else if ("go" == input)
		{
			go();
			return 0;
		}
		else if ("white" == input)
		{
			playBlack();
			return 0;
		}
		else if ("black" == input)
		{
			playWhite();
			return 0;
		}

		return receiveMove(input);
	}//end parseInput()

	 //playBlack()
	void Driver::playBlack()
	{
		eng.b.currentState.toMove = WHITE;//necessary?
		forceMode = false;
		engineSide = BLACK;
	}//end playBlack()

	 //playOther()
	void Driver::playOther()
	{
		forceMode = false;
		Colour toMove = eng.b.currentState.toMove;
		engineSide = (BLACK == toMove) ? WHITE : BLACK;
	}//end playOther()

	 //playWhite()
	void Driver::playWhite()
	{
		eng.b.currentState.toMove = BLACK; //necessay?
		forceMode = false;
		engineSide = WHITE;
	}//end playWhite()

	 //receiveMove()
	int Driver::receiveMove(std::string input)
	{
		if (4 > input.size())
			return 2;//not recognised
		int from = misc::alpha2board(input.substr(0, 2));
		int to = misc::alpha2board(input.substr(2, 2));
		gen::Generator gen;
		gen::Board& board = eng.b;
		gen.generate(board);
		gen::MoveList& moveList = gen.moveList;

		bool isPromotion = false;
		Piece promotion = X;
		if (4 < input.size())
		{
			isPromotion = true;
			int temp = 0;

			switch (input.substr(4, 1)[0])
			{
			case 'q': temp = 6; break;
			case 'n': temp = 3; break;
			case 'r': temp = 5; break;
			case 'b': temp = 4; break;
			}

			promotion = static_cast <Piece> (temp * board.currentState.toMove);
		}

		for (int i = 0; i < moveList.size(); i++)
		{
			gen::Move& move = moveList[i];

			if (from == move.from && to == move.to && promotion == move.promotion)
			{
				board.makeMove(move);

				if (!board.isValid())//illegal move
				{
					board.unmakeMove();
					return 1;
				}

				return 0;//ok move
			}
		}

		return 2;//not recognised

	}//end receiveMove()

	 //sendFeatures()
	void Driver::sendFeatures()
	{
		std::ofstream fout("D:\\send_features_log.txt", std::ofstream::out | std::ofstream::app);
		std::string versionNumber, response;
		std::cin >> versionNumber;
		std::cout << "feature myname = Macaron 0.3" << std::endl;
		std::cout << "feature playother = 1" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature setboard= 1" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature time = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature otim = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature draw = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature sigint = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature sigterm = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature analyze = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature colors = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "feature nps = 0" << std::endl;
		std::cin >> response; fout << response << std::endl;
		std::cout << "done = 1" << std::endl;
	}//end sendFeatures()

	 //startNew()
	void Driver::startNew()
	{
		eng.clearHistory();
		gen::Board& board = eng.b;
		board.clear();
		board.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		engineSide = BLACK;
		forceMode = false;
	}//end startNew()

	 //undo()
	void Driver::undo()
	{
		eng.b.unmakeMove();
	}//end undo()

	 //remove()
	void Driver::remove()
	{
		gen::Board& board = eng.b;
		board.unmakeMove();
		board.unmakeMove();
	}
}

int main()
{
	std::cout << "select mode: " << std::endl;
	std::cout << "(enter xboard to set winboard mode, anything else to remain in console mode)" << std::endl;
	ui::Driver driver;
	driver.initiate();
	driver.loop();
	return 0;
}

//TODO list
//sort out game-end detection
//add end game evaluation
//add time control