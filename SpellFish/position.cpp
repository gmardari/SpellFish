#include "position.h"
#include "misc.h"
#include "magics.h"
#include "uci.h"
#include <sstream>
#include <cstddef> // For offsetof()
#include <cassert>
#include <iostream>

namespace Spellfish
{
    namespace Zobrist {

        Key psq[PIECE_NB][SQUARE_NB];
        Key enpassant[FILE_NB];
        Key castling[CASTLING_RIGHT_NB];
        Key side;
    }

    void Position::init() {

        PRNG rng(1070372);


       
        for (Color c : {WHITE, BLACK})
            for (PieceType pt = PAWN; pt <= KING; ++pt)
                for (Square s = SQ_A1; s <= SQ_MAX; ++s)
                    Zobrist::psq[make_piece(c, pt)][s] = rng.rand<Key>();

        for (File f = FILE_A; f <= FILE_MAX; ++f)
            Zobrist::enpassant[f] = rng.rand<Key>();

        for (int cr = NO_CASTLING; cr <= ANY_CASTLING; ++cr)
            Zobrist::castling[cr] = rng.rand<Key>();

        Zobrist::side = rng.rand<Key>();
    
    }

	void Position::set(const std::string& fenStr, StateInfo* st)
	{
		unsigned char col, row, token;
		size_t idx;
		std::istringstream ss(fenStr);

		std::memset(this, 0, sizeof(Position));
		std::memset(st, 0, sizeof(StateInfo));

		ss >> std::noskipws;

		this->st = st;

        Rank r = RANK_MAX;
        Square sq = SQ_A1 + r * NORTH;

        // 1. Piece placement
        while ((ss >> token) && !isspace(token))
        {
            if (isdigit(token))
            {
                sq += (token - '0') * EAST; // Advance the given number of files
            }

            else if (token == '/')
            {
                sq = SQ_A1 + --r * NORTH;
                if (!is_ok(sq))
                    break;
            }
            // Ignore pieces outside the board and wait for next / or [ to return to a valid state
            else if (!is_ok(sq) || file_of(sq) > FILE_MAX || rank_of(sq) > r)
                continue;

            else if ((idx = piece_to_char().find(token)) != std::string::npos)
            {
                Piece pc = Piece(idx);
                put_piece(pc, sq, false);
                ++sq;

                st->key ^= Zobrist::psq[pc][sq];
            }

        }

        if (ss >> token)
        {
            assert(token == 'w' || token == 'b', "token invalid");
            sideToMove = token == 'w' ? WHITE : BLACK;
        }


        if (sideToMove == BLACK)
            st->key ^= Zobrist::side;

        calculate_attacks(~sideToMove);
        st->epSquare = SQ_NONE;
	}

    std::string Position::fen() const 
    {

        int emptyCnt;
        std::ostringstream ss;

        for (Rank r = RANK_8; r >= RANK_1; --r)
        {
            for (File f = FILE_A; f <= FILE_H; ++f)
            {
                for (emptyCnt = 0; f <= FILE_H && !(pieces() & make_square(f, r)); ++f)
                    ++emptyCnt;

                if (emptyCnt)
                    ss << emptyCnt;

                if (f <= FILE_H)
                {
                    ss << piece_to_char()[piece_on(make_square(f, r))];
                }
            }

            if (r > RANK_1)
                ss << '/';
        }

     

   
        ss << (sideToMove == WHITE ? " w " : " b ");


        return ss.str();
    }

    void Position::calculate_attacks(Color attackers)
    {
        //We already calculated it.
        if (st->attacksBB[attackers][ALL_PIECES] != 0)
            return;

        Color them = ~attackers;
        Square ksq = piece_square(them, KING);

        if (ksq == SQ_NONE)
        {
            //King died. This move will be a cut node anyway
            return;
        }

        //Attack bb's
        for (PieceSet ps = CHESS_PIECES; ps != 0;)
        {
            PieceType pt = pop_lsb(ps);

            if (pt != PAWN && pt != KING)
                continue;

            Bitboard pieceMap = pieces(attackers, pt);
            Bitboard attacks = Bitboard(0);

            while (pieceMap)
            {
                const Square sq = pop_lsb(pieceMap);
                Bitboard pieceAttacks = attacks_from(make_piece(attackers, pt), sq);

                if ((pieceAttacks & ksq) != 0)
                    st->checkersBB |= sq;

                attacks |= pieceAttacks;

                if (gamePly == 0 && pt == PAWN)
                {
                    std::cout << "Printing pawn attack from SQ " << UCI::square(*this, sq) << std::endl;
                    Magics::print_bitboard((U64) pieceAttacks);
                }
                    
            }

            st->attacksBB[attackers][pt] = attacks;
            st->attacksBB[attackers][ALL_PIECES] |= attacks;

        }

        //st->checkersBB = attackers_to(us, );
        st->inCheck = st->checkersBB != 0;
        if(gamePly == 0)
            Magics::print_bitboard(st->attacksBB[attackers][ALL_PIECES]);
        
    }

    void Position::do_move(Move m, StateInfo& newSt)
    {
        assert(is_ok(m));
        assert(&newSt != st);

        Key key = st->key ^ Zobrist::side;
        Square old_ep_sq = st->epSquare;

        // Copy some fields of the old state to our new StateInfo object except the
        // ones which are going to be recalculated from scratch anyway and then switch
        // our state pointer to point to the new (ready to be updated) state.
        std::memset(&newSt, 0, sizeof(StateInfo)); //Because StateInfo objs are re-used. Stops undefined behaviour
        std::memcpy(static_cast<void*>(&newSt), static_cast<void*>(st), offsetof(StateInfo, move));
        newSt.previous = st;
        st = &newSt;
        st->move = m;
        st->epSquare = SQ_NONE;

        Color us = sideToMove;
        Color them = ~us;
        Square from = from_sq(m);
        Square to = to_sq(m);
        Piece pc = moved_piece(m);
        Piece captured = piece_on(to);
        Piece ep_captured = NO_PIECE;

     
        assert(pc != NO_PIECE, "Moving nil piece");

        st->capturedPiece = captured;

        if (captured != NO_PIECE)
            remove_piece(to);

        move_piece(from, to);

        //En passant
        if (type_of(pc) == PAWN)
        {
            if (captured)
            {
                if (to == old_ep_sq)
                {
                    const Square cap_sq = to - pawn_push(us);
                    ep_captured = piece_on(cap_sq);
                    remove_piece(cap_sq);
                }
            }
            else
            {
                const Bitboard doubleJumpBB = us == WHITE ? Rank2BB : Rank7BB;

                if ((doubleJumpBB & from) != 0 && abs((int)to - (int)from) == (NORTH + NORTH))
                {
                    st->epSquare = from + pawn_push(us);
                }
            }
        }

       

        key ^= Zobrist::psq[pc][from];

        if(captured != NO_PIECE)
            key ^= Zobrist::psq[captured][to];

        key ^= Zobrist::psq[pc][to];

        //Potentially will flip the same bits back and forth doing nothing - if file(ep sq old) == file(ep sq)
        if (st->epSquare != old_ep_sq)
        {
            if (old_ep_sq != SQ_NONE)
                key ^= Zobrist::enpassant[file_of(old_ep_sq)];

            if (st->epSquare != SQ_NONE)
                key ^= Zobrist::enpassant[file_of(st->epSquare)];
        }
       
       
        st->key = key;

        sideToMove = them;

        ++gamePly;
        calculate_attacks(us);
    }

    void Position::undo_move(Move m) 
    {
        Color us = sideToMove;
        Color them = ~us;
        Square from = from_sq(m);
        Square to = to_sq(m);
        
        move_piece(to, from);

        if (st->capturedPiece)
            put_piece(st->capturedPiece, to);


        sideToMove = them;

        st = st->previous;
        --gamePly;

       // assert(pos_is_ok());
    }


    Bitboard Position::attacks_from(Piece piece, Square sq)
    {
        Color pc_color = color_of(piece);
        //Bitboard theirPieces = pieces(~pc_color);
        Bitboard bb = Bitbases::attacks_from_bb(piece, sq);

        return bb;
    }

    bool Position::has_attack_on(Piece piece, Square piece_sq, Square attack_sq)
    {
        return (attacks_from(piece, piece_sq) & attack_sq) != 0;
    }

    Bitboard Position::attackers_to(Color us, Square sq)
    {
        Bitboard attackers = Bitboard(0);
        
        for (PieceSet ps = CHESS_PIECES; ps != 0;)
        {
            PieceType pt = pop_lsb(ps);
            Piece pc = make_piece(us, pt);
            Bitboard ourPieces = pieces(us, pt);

            

            while (ourPieces)
            {
                Square from = pop_lsb(ourPieces);

                if (has_attack_on(pc, from, sq))
                {
                    attackers |= from;
                }
            }
        }
        

        return attackers;
    }

    Square Position::piece_square(Color c, PieceType pt) const
    {
        Bitboard bb = byTypeBB[pt] & byColorBB[c];
        return bb != 0 ? pop_lsb(bb) : SQ_NONE;
    }

    Square Position::piece_square(Piece piece) const
    {
        return piece_square(color_of(piece), type_of(piece));
    }
}