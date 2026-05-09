#include "board.h"
#include <cctype>
#include <sstream>

BoardState::BoardState() { clear(); }

BoardState::BoardState(const BoardState &other) { *this = other; }

BoardState &BoardState::operator=(const BoardState &other) {
  if (this == &other)
    return *this;

  for (int c = 0; c < COLOR_NB; ++c) {
    color_bb[c] = other.color_bb[c];
    for (int pt = 0; pt < PIECE_TYPE_NB; ++pt)
      piece_bb[c][pt] = other.piece_bb[c][pt];
  }
  for (int i = 0; i < SQUARE_NB; ++i)
    board[i] = other.board[i];

  stm = other.stm;
  fullmove = other.fullmove;

  setup_state = *other.st;
  setup_state.previous = nullptr;
  st = &setup_state;

  return *this;
}

void BoardState::clear() {
  for (int c = WHITE; c <= BLACK; ++c) {
    color_bb[c] = 0;
    for (int pt = PAWN; pt <= KING; ++pt) {
      piece_bb[c][pt] = 0;
    }
  }
  for (int i = 0; i < SQUARE_NB; ++i) {
    board[i] = NO_PIECE;
  }
  stm = WHITE;
  fullmove = 1;
  st = &setup_state;
  st->captured_piece = NO_PIECE;
  st->castling_rights = NO_CASTLING;
  st->ep_square = NO_SQUARE;
  st->halfmove_clock = 0;
  st->zobrist_key = 0;
  st->previous = nullptr;
}

void BoardState::place_piece(Piece p, Square sq) {
  board[sq] = p;
  Color c = color_of(p);
  PieceType pt = type_of(p);

  Bitboards::set_bit(piece_bb[c][pt], sq);
  Bitboards::set_bit(color_bb[c], sq);
  st->zobrist_key ^= Zobrist::piece[c][pt][sq];
}

void BoardState::remove_piece(Square sq) {
  Piece p = board[sq];
  if (p == NO_PIECE)
    return;

  Color c = color_of(p);
  PieceType pt = type_of(p);

  Bitboards::clear_bit(piece_bb[c][pt], sq);
  Bitboards::clear_bit(color_bb[c], sq);
  st->zobrist_key ^= Zobrist::piece[c][pt][sq];
  board[sq] = NO_PIECE;
}

void BoardState::set_fen(const std::string &fen) {
  clear();

  std::istringstream iss(fen);
  std::string pieces, active, castling, enpassant, half, full;
  iss >> pieces >> active >> castling >> enpassant >> half >> full;

  int rank = 7, file = 0;
  for (char c : pieces) {
    if (c == '/') {
      rank--;
      file = 0;
    } else if (isdigit(c)) {
      file += (c - '0');
    } else {
      Color col = islower(c) ? BLACK : WHITE;
      PieceType pt;
      switch (tolower(c)) {
      case 'p':
        pt = PAWN;
        break;
      case 'n':
        pt = KNIGHT;
        break;
      case 'b':
        pt = BISHOP;
        break;
      case 'r':
        pt = ROOK;
        break;
      case 'q':
        pt = QUEEN;
        break;
      case 'k':
        pt = KING;
        break;
      default:
        pt = PAWN;
        break;
      }
      place_piece(make_piece(col, pt), make_square(rank, file));
      file++;
    }
  }

  stm = (active == "w") ? WHITE : BLACK;

  st->castling_rights = NO_CASTLING;
  if (castling != "-") {
    for (char c : castling) {
      if (c == 'K')
        st->castling_rights |= WHITE_OO;
      if (c == 'Q')
        st->castling_rights |= WHITE_OOO;
      if (c == 'k')
        st->castling_rights |= BLACK_OO;
      if (c == 'q')
        st->castling_rights |= BLACK_OOO;
    }
  }

  if (enpassant != "-") {
    int f = enpassant[0] - 'a';
    int r = enpassant[1] - '1';
    st->ep_square = make_square(r, f);
  } else {
    st->ep_square = NO_SQUARE;
  }

  if (!half.empty())
    st->halfmove_clock = std::stoi(half);
  if (!full.empty())
    fullmove = std::stoi(full);

  // Initial Zobrist key calculation
  st->zobrist_key = 0;
  for (int s = 0; s < SQUARE_NB; ++s) {
      if (board[s] != NO_PIECE) {
          st->zobrist_key ^= Zobrist::piece[color_of(board[s])][type_of(board[s])][s];
      }
  }
  if (stm == WHITE) st->zobrist_key ^= Zobrist::side_to_move;
  st->zobrist_key ^= Zobrist::castling[st->castling_rights];
  if (st->ep_square != NO_SQUARE)
      st->zobrist_key ^= Zobrist::en_passant[file_of(st->ep_square)];

  // Save initial bitboards into setup_state
  for (int c = 0; c < COLOR_NB; ++c) {
      st->color_bb[c] = color_bb[c];
      for (int pt = 0; pt < PIECE_TYPE_NB; ++pt)
          st->piece_bb[c][pt] = piece_bb[c][pt];
  }
}

std::string BoardState::get_fen() const {
  // Basic FEN generation for testing
  std::ostringstream oss;
  for (int rank = 7; rank >= 0; --rank) {
    int empty = 0;
    for (int file = 0; file < 8; ++file) {
      Piece p = board[make_square(rank, file)];
      if (p == NO_PIECE) {
        empty++;
      } else {
        if (empty > 0) {
          oss << empty;
          empty = 0;
        }
        char c;
        switch (type_of(p)) {
        case PAWN:
          c = 'p';
          break;
        case KNIGHT:
          c = 'n';
          break;
        case BISHOP:
          c = 'b';
          break;
        case ROOK:
          c = 'r';
          break;
        case QUEEN:
          c = 'q';
          break;
        case KING:
          c = 'k';
          break;
        case PIECE_TYPE_NB:
          break;
        }
        if (color_of(p) == WHITE)
          c = toupper(c);
        oss << c;
      }
    }
    if (empty > 0)
      oss << empty;
    if (rank > 0)
      oss << '/';
  }

  oss << (stm == WHITE ? " w " : " b ");

  if (st->castling_rights == NO_CASTLING)
    oss << "-";
  else {
    if (st->castling_rights & WHITE_OO)
      oss << "K";
    if (st->castling_rights & WHITE_OOO)
      oss << "Q";
    if (st->castling_rights & BLACK_OO)
      oss << "k";
    if (st->castling_rights & BLACK_OOO)
      oss << "q";
  }

  oss << " ";
  if (st->ep_square == NO_SQUARE)
    oss << "-";
  else {
    char f = 'a' + file_of(st->ep_square);
    char r = '1' + rank_of(st->ep_square);
    oss << f << r;
  }

  oss << " " << st->halfmove_clock << " " << fullmove;
  return oss.str();
}

int encode_move(Move m) {
  Square from = m.from();
  Square to = m.to();
  int flags = m.flags();

  int df = file_of(to) - file_of(from);
  int dr = rank_of(to) - rank_of(from);

  int plane = -1;

  // Knight moves
  if (std::abs(dr) * std::abs(df) == 2) {
    static const int knight_map[5][5] = {
        {-1, 4, -1, 3, -1},
        {5, -1, -1, -1, 2},
        {-1, -1, -1, -1, -1},
        {6, -1, -1, -1, 1},
        {-1, 7, -1, 0, -1}
    };
    plane = 56 + knight_map[dr + 2][df + 2];
  }
  // Under-promotions
  else if (is_promotion(flags) && promotion_type(flags) != QUEEN) {
    int dir = df + 1; // 0: diag-left, 1: straight, 2: diag-right
    plane = 64 + (promotion_type(flags) - 1) * 3 + dir;
  }
  // Queen-like moves (including queen promotion)
  else {
    int dir = -1;
    if (df == 0) dir = (dr > 0 ? 0 : 4);
    else if (dr == 0) dir = (df > 0 ? 2 : 6);
    else if (std::abs(df) == std::abs(dr)) {
      if (df > 0) dir = (dr > 0 ? 1 : 3);
      else dir = (dr > 0 ? 7 : 5);
    }
    int dist = std::max(std::abs(dr), std::abs(df));
    plane = dir * 7 + (dist - 1);
  }

  return from * 73 + plane;
}

Move decode_move(int encoded, const BoardState &board) {
  int from_sq = encoded / 73;
  int plane = encoded % 73;

  Square from = static_cast<Square>(from_sq);
  int r = rank_of(from);
  int f = file_of(from);

  Square to = NO_SQUARE;
  int flags = QUIET;

  if (plane < 56) { // Queen-like
    int dir = plane / 7;
    int dist = (plane % 7) + 1;
    static const int dr[] = {1, 1, 0, -1, -1, -1, 0, 1};
    static const int df[] = {0, 1, 1, 1, 0, -1, -1, -1};
    int tr = r + dr[dir] * dist;
    int tf = f + df[dir] * dist;
    if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) {
      to = make_square(tr, tf);
      if ((board.side_to_move() == WHITE && tr == 7 && type_of(board.piece_on(from)) == PAWN) ||
          (board.side_to_move() == BLACK && tr == 0 && type_of(board.piece_on(from)) == PAWN)) {
        flags = PR_QUEEN;
      } else if (std::abs(dr[dir] * dist) == 2 && df[dir] == 0 && type_of(board.piece_on(from)) == PAWN) {
          flags = DOUBLE_PUSH;
      } else if (type_of(board.piece_on(from)) == KING && std::abs(df[dir] * dist) == 2) {
          flags = (tf == 6 ? OO : OOO);
      }
    }
  } else if (plane < 64) { // Knight
    int dir = plane - 56;
    static const int dr[] = {2, 1, -1, -2, -2, -1, 1, 2};
    static const int df[] = {1, 2, 2, 1, -1, -2, -2, -1};
    int tr = r + dr[dir];
    int tf = f + df[dir];
    if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) to = make_square(tr, tf);
  } else { // Under-promotions
    int p_idx = (plane - 64) / 3;
    int dir = (plane - 64) % 3 - 1;
    int tr = r + (board.side_to_move() == WHITE ? 1 : -1);
    int tf = f + dir;
    if (tr >= 0 && tr < 8 && tf >= 0 && tf < 8) {
      to = make_square(tr, tf);
      flags = 8 + p_idx; // PR_KNIGHT=8, PR_BISHOP=9, PR_ROOK=10
    }
  }

  if (to == NO_SQUARE) return Move();

  // Handle captures and en passant
  if (to == board.en_passant() && type_of(board.piece_on(from)) == PAWN) flags = EN_PASSANT;
  else if (board.piece_on(to) != NO_PIECE) {
      if (is_promotion(flags)) flags |= 4; // PC_...
      else flags = CAPTURE;
  }

  return Move(from, to, flags);
}

const int CastlingMasks[SQUARE_NB] = {
    ~(WHITE_OOO), 15, 15, 15, ~(WHITE_OO | WHITE_OOO), 15, 15, ~(WHITE_OO),
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    ~(BLACK_OOO), 15, 15, 15, ~(BLACK_OO | BLACK_OOO), 15, 15, ~(BLACK_OO)
};

void BoardState::do_move(Move m, StateInfo &new_st) {
  Square from = m.from();
  Square to = m.to();
  int flags = m.flags();
  Piece moving_piece = board[from];
  Piece captured = board[to];

  new_st.previous = st;
  new_st.captured_piece = captured;
  new_st.castling_rights = st->castling_rights & CastlingMasks[from] & CastlingMasks[to];
  new_st.ep_square = NO_SQUARE;
  new_st.halfmove_clock = st->halfmove_clock + 1;
  new_st.zobrist_key = st->zobrist_key;
  new_st.last_move = m;

  // XOR out old state metadata
  new_st.zobrist_key ^= Zobrist::side_to_move;
  new_st.zobrist_key ^= Zobrist::castling[st->castling_rights];
  if (st->ep_square != NO_SQUARE)
      new_st.zobrist_key ^= Zobrist::en_passant[file_of(st->ep_square)];

  st = &new_st; // Temporarily point st to new_st so remove/place_piece works on new key

  if (type_of(moving_piece) == PAWN || is_capture(flags)) {
    new_st.halfmove_clock = 0;
  }

  if (is_capture(flags) && flags != EN_PASSANT) {
    remove_piece(to);
  }

  remove_piece(from);

  // Handle special moves
  if (flags == EN_PASSANT) {
    Square ep_cap_sq = make_square(rank_of(from), file_of(to));
    new_st.captured_piece = board[ep_cap_sq];
    remove_piece(ep_cap_sq);
  } else if (flags == OO || flags == OOO) {
    // move rook
    if (to == G1) { remove_piece(H1); place_piece(W_ROOK, F1); }
    else if (to == C1) { remove_piece(A1); place_piece(W_ROOK, D1); }
    else if (to == G8) { remove_piece(H8); place_piece(B_ROOK, F8); }
    else if (to == C8) { remove_piece(A8); place_piece(B_ROOK, D8); }
  } else if (is_promotion(flags)) {
    moving_piece = make_piece(~stm, promotion_type(flags)); // side already flipped below, wait
    // Actually, stm is not flipped yet.
    moving_piece = make_piece(stm, promotion_type(flags));
  }

  place_piece(moving_piece, to);

  // Double pawn push
  if (flags == DOUBLE_PUSH) {
    new_st.ep_square = make_square((rank_of(from) + rank_of(to)) / 2, file_of(from));
  }

  // XOR in new state metadata
  new_st.zobrist_key ^= Zobrist::castling[new_st.castling_rights];
  if (new_st.ep_square != NO_SQUARE)
      new_st.zobrist_key ^= Zobrist::en_passant[file_of(new_st.ep_square)];

  if (stm == BLACK)
    fullmove++;
  stm = ~stm;

  // Save current bitboards into the state for history tracking
  for (int c = 0; c < COLOR_NB; ++c) {
      new_st.color_bb[c] = color_bb[c];
      for (int pt = 0; pt < PIECE_TYPE_NB; ++pt)
          new_st.piece_bb[c][pt] = piece_bb[c][pt];
  }
}

void BoardState::undo_move(Move m) {
  stm = ~stm;
  if (stm == BLACK)
    fullmove--;

  Square from = m.from();
  Square to = m.to();
  int flags = m.flags();

  Piece moving_piece = board[to];

  if (is_promotion(flags)) {
    moving_piece = make_piece(stm, PAWN);
  }

  remove_piece(to);

  if (flags == OO || flags == OOO) {
    if (to == G1) { remove_piece(F1); place_piece(W_ROOK, H1); }
    else if (to == C1) { remove_piece(D1); place_piece(W_ROOK, A1); }
    else if (to == G8) { remove_piece(F8); place_piece(B_ROOK, H8); }
    else if (to == C8) { remove_piece(D8); place_piece(B_ROOK, A8); }
  }

  place_piece(moving_piece, from);

  if (flags == EN_PASSANT) {
    Square ep_cap_sq = make_square(rank_of(from), file_of(to));
    place_piece(st->captured_piece, ep_cap_sq);
  } else if (st->captured_piece != NO_PIECE) {
    place_piece(st->captured_piece, to);
  }

  st = st->previous;
}
