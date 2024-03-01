#ifndef CELL_H
#define CELL_H
#include <vector>
#include <unordered_map>
enum Color {Unknown = 0, Ocean = 1, Island = 2, Node = 3};

class Cell{
    public:
        Color color;

        Cell* owner_node;
        int row;
        int column;

        //only for nodes, but I am not making new class for this
        int num_islands;
        int max_num_islands;

        Cell(int r, int c){
            color = Unknown;
            owner_node = nullptr;
            row = r;
            column = c;
        }
        Cell(int r, int c,int islands){
            color = Node;
            max_num_islands = islands;
            row = r;
            column = c;
            num_islands = 1;
            owner_node = nullptr;
        }

        //if its all good returns 1, else return 0
        bool changeColor(Color c){
            if(color != Unknown)
                return false;
            color = c;
            return true;
        }
};

#endif // CELL_H
