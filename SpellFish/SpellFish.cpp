// SpellFish.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "uci.h"
#include "magics.h"
#include "tt.h"
#include "search.h"

using namespace Spellfish;

int main(int argc, char* argv[])
{
    char c;
    std::cout << "Spell Fish.\n";
    Magics::init();
    Bitbases::init();
    Position::init();
    TranspositionTable::init();
    Search::init();
    UCI::loop(argc, argv);
    

}


