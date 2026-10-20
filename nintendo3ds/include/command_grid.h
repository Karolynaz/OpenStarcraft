#pragma once
namespace port3ds {
// Bottom catalogue cells and touch share the same bounds. Other menus retain
// their existing text rows, and closed-menu touches remain minimap-only.
struct CommandGrid {
    static constexpr int x=136,y=144,columns=2,cell_width=92,cell_height=24;
    static int hit(int px,int py,unsigned count){
        if(px<x || py<y || px>=x+columns*cell_width || py>=y+4*cell_height)return -1;
        const unsigned index=(py-y)/cell_height*columns+(px-x)/cell_width;
        return index<count?int(index):-1;
    }
    static unsigned move(unsigned selected,unsigned direction,unsigned count){
        if(!count)return 0;selected%=count;
        if(direction==1)return (selected+1)%count;
        if(direction==3)return (selected+count-1)%count;
        const unsigned col=selected%columns,rows=(count+columns-1-col)/columns,row=selected/columns;
        return ((row+(direction==0?rows-1:1))%rows)*columns+col;
    }
};
}
