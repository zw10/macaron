
Macaron is a C++ chess engine, which began life when I lost a game of correspondence chess to my friend on time some years ago. Consequently, I promised to deliver my digital representative that they can play against, any time that I wasn't available.

I had two goals for Macaron:
1. To be fun and human-like to play against, with a style resembling my own
2. To be of reasonable strength (a good opponent for club-level players), which I defined as being able to defeat me most of the time

Development of Macaron was stopped once those goals were reached. No doubt many further improvements in strength based on the existing engine are possible.

Recently, I learned that the wikispaces, the host of the chessprogramming wiki - which was where the many of the ideas for Macaron were taken from - was going to be taken down, hence the upload of the engine in commemoration. Macaron uses a number of well-known algorithms; the code was initially intended for private use only, so please reach out if there are any parts of the code base that you feel should be attributed.

Interface
-------------
1. Winboard: Macaron is Winboard-compatible
2. Console: alternatively, Macaron can be used in the command-line console 
(the piece names are in Chinese)

Move ordering
-------------
The move ordering of Macaron works as follows; each move is given some bonus according
to the criteria below, and then ordered by sorting:
1. Prioritising captures: captures are also ordered by LVA/MVV (most least valuable victim/ most valuable attacker)
2. History heuristic: any moves that repeatedly crops up in the history table of cutoffs
3. Counter capture table: increased prioritisation for countercaptures in response to captures
4. Counter move table: non-capture moves that resulted in the most cut-offs
5. Killer move heuristic: prioritising moves that generate the most cutoffs
6. Piece-square table heuristic: in general prioritising moves that move the pieces to 'better squares'
7. Prioritising promotions, castling and double pawn moves in search: irreversible moves should be searched first

Quiescence Search
-------------
1. Negative priority pruning: if depth left was low enough, then captures with 
negative priority (typically heavily negative SEE) are pruned out
2. Delta ordering: moves that cause a big change in delta are searched first

Pruning & reductions (in addition to vanilla alpha beta)
-------------
1. Null move reduction: at depth == 4
2. Delta pruning

Extensions
-------------
1. Check extension
2. Singular move extension

Implemented but not fully utilised
-------------
1. Iterative deepening: although Macaron does use iterative deepening,
it doesn't use transposition tables so the impact on playing strength is not seen
time management is seen as lower priority
2. Null window searches: again, these will not be impactful until transposition tables are
implemented
3. Late move reductions: currently disabled 

Not using
-------------
1. Bitboards
2. Transposition table
3. Principal variation search
4. Multi-threading: Macaron only uses a single thread
5. Time management: deprioritised
6. Usage of endgame table bases
7. Usage of opening books: stylistically a success - Macaron uses some of the same openings as I, despite not using any opening books

Evaluation
-------------
1. Material Basic evaluation
2. Piece square tables
3. Mobility: evaluated using a pseudo mobility, the quotient of the number moves of player / number of moves of other player of an internal node; queen moves are included at a discount
4. Contempt factor: roughly a fifth of a pawn
5. Phasing: graduated phasing into endgame evaluation
6. Some positional features, such as:
  a. pawn formation:
    i. Doubled/ tripled pawns
    ii. Isolated pawns
    iii. Backward pawns and holes
    iv. Passed pawns
    v. Good/ bad bishops
  b. Piece features:
    i. King safety
    ii. Connected rooks
7. A small random factor is added for some more variation in play