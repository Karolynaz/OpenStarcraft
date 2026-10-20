#pragma once
namespace port3ds {
// More than two powers keep the first on Y and open the full menu on L+Y.
inline int ability_shortcut(unsigned powers,bool secondary,bool production_menu=false) {
    return production_menu?-1:!secondary && powers?0:secondary && powers==2?1:-1;
}
}
