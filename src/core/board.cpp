#include "board.h"
#include <sstream>
#include <cctype>

BoardState::BoardState() {
    clear();
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
}

void BoardState::remove_piece(Square sq) {
    Piece p = board[sq];
    if (p == NO_PIECE) return;
    
    Color c = color_of(p);
    PieceType pt = type_of(p);
    
    Bitboards::clear_bit(piece_bb[c][pt], sq);
    Bitboards::clear_bit(color_bb[c], sq);
    board[sq] = NO_PIECE;
}

void BoardState::set_fen(const std::string& fen) {
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
            switch(tolower(c)) {
                case 'p': pt = PAWN; break;
                case 'n': pt = KNIGHT; break;
                case 'b': pt = BISHOP; break;
                case 'r': pt = ROOK; break;
                case 'q': pt = QUEEN; break;
                case 'k': pt = KING; break;
                default: pt = PAWN; break;
            }
            place_piece(make_piece(col, pt), make_square(rank, file));
            file++;
        }
    }

    stm = (active == "w") ? WHITE : BLACK;

    st->castling_rights = NO_CASTLING;
    if (castling != "-") {
        for (char c : castling) {
            if (c == 'K') st->castling_rights |= WHITE_OO;
            if (c == 'Q') st->castling_rights |= WHITE_OOO;
            if (c == 'k') st->castling_rights |= BLACK_OO;
            if (c == 'q') st->castling_rights |= BLACK_OOO;
        }
    }

    if (enpassant != "-") {
        int f = enpassant[0] - 'a';
        int r = enpassant[1] - '1';
        st->ep_square = make_square(r, f);
    } else {
        st->ep_square = NO_SQUARE;
    }

    if (!half.empty()) st->halfmove_clock = std::stoi(half);
    if (!full.empty()) fullmove = std::stoi(full);
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
                    case PAWN: c = 'p'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case ROOK: c = 'r'; break;
                    case QUEEN: c = 'q'; break;
                    case KING: c = 'k'; break;
                }
                if (color_of(p) == WHITE) c = toupper(c);
                oss << c;
            }
        }
        if (empty > 0) oss << empty;
        if (rank > 0) oss << '/';
    }

    oss << (stm == WHITE ? " w " : " b ");

    if (st->castling_rights == NO_CASTLING) oss << "-";
    else {
        if (st->castling_rights & WHITE_OO) oss << "K";
        if (st->castling_rights & WHITE_OOO) oss << "Q";
        if (st->castling_rights & BLACK_OO) oss << "k";
        if (st->castling_rights & BLACK_OOO) oss << "q";
    }

    oss << " ";
    if (st->ep_square == NO_SQUARE) oss << "-";
    else {
        char f = 'a' + file_of(st->ep_square);
        char r = '1' + rank_of(st->ep_square);
        oss << f << r;
    }

    oss << " " << st->halfmove_clock << " " << fullmove;
    return oss.str();
}

void BoardState::do_move(Move m, StateInfo& new_st) {
    Square from = m.from();
    Square to = m.to();
    int flags = m.flags();
    Piece moving_piece = board[from];
    Piece captured = board[to];

    new_st.previous = st;
    new_st.captured_piece = captured;
    new_st.castling_rights = st->castling_rights;
    new_st.ep_square = NO_SQUARE;
    new_st.halfmove_clock = st->halfmove_clock + 1;
    new_st.zobrist_key = st->zobrist_key; // to be updated properly later

    if (type_of(moving_piece) == PAWN || captured != NO_PIECE) {
        new_st.halfmove_clock = 0;
    }

    if (captured != NO_PIECE && flags != EN_PASSANT) {
        remove_piece(to);
    }

    remove_piece(from);

    // Handle special moves
    if (flags == EN_PASSANT) {
        Square ep_cap_sq = make_square(rank_of(from), file_of(to));
        new_st.captured_piece = board[ep_cap_sq];
        remove_piece(ep_cap_sq);
    } else if (flags == CASTLING) {
        // move rook
        if (to == G1) { remove_piece(H1); place_piece(W_ROOK, F1); }
        else if (to == C1) { remove_piece(A1); place_piece(W_ROOK, D1); }
        else if (to == G8) { remove_piece(H8); place_piece(B_ROOK, F8); }
        else if (to == C8) { remove_piece(A8); place_piece(B_ROOK, D8); }
    } else if (flags == PROMOTION) {
        // for now just promote to queen, can decode exact piece from flags later
        moving_piece = make_piece(stm, QUEEN); 
    }

    place_piece(moving_piece, to);

    // Double pawn push
    if (type_of(moving_piece) == PAWN && std::abs(rank_of(to) - rank_of(from)) == 2) {
        new_st.ep_square = make_square((rank_of(from) + rank_of(to)) / 2, file_of(from));
    }

    // Update castling rights (very simplified, needs proper bitmask per square)
    if (type_of(moving_piece) == KING) {
        new_st.castling_rights &= (stm == WHITE) ? ~(WHITE_OO | WHITE_OOO) : ~(BLACK_OO | BLACK_OOO);
    }

    if (stm == BLACK) fullmove++;
    stm = ~stm;
    st = &new_st;
}

void BoardState::undo_move(Move m) {
    stm = ~stm;
    if (stm == BLACK) fullmove--;

    Square from = m.from();
    Square to = m.to();
    int flags = m.flags();

    Piece moving_piece = board[to];
    
    if (flags == PROMOTION) {
        moving_piece = make_piece(stm, PAWN);
    }

    remove_piece(to);
    
    if (flags == CASTLING) {
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
