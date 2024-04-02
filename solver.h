
#ifndef SOLVER_H
#define SOLVER_H

#include <queue>
#include <set>
#include <unordered_map>
#include "cell.h"
#include <stack>

class Solver{
public:
    std::vector<std::vector<Cell>> data;
    std::unordered_map<Cell*,std::set<Cell*>> nodes;

    //std::queue<Cell*> impacted;

    bool UPDATED_BOARD;
    //util needed alot - dont need to alocate all the time
    std::vector<Cell*> neighbours;

    Solver(){}
    //ownera od noda ne vredi kopira
    Solver(const Solver &t) {
        std::queue<Cell*> q;
        std::set<Cell*> visited;
        data.insert(data.end(), t.data.begin(), t.data.end());

        for(const auto& n: t.nodes){
            nodes.insert({&data[n.first->row][n.first->column], std::set<Cell*>()});
            for(const auto& n2: n.second){
                nodes[&data[n.first->row][n.first->column]].insert(&data[n2->row][n2->column]);
                data[n2->row][n2->column].owner_node = &data[n.first->row][n.first->column];
            }
        }

        this->UPDATED_BOARD = false;
    }

    Solver(const std::vector<std::vector<Cell>> &data_in, const std::vector<Cell*> &nodes_in){
        data = data_in;
        for(auto const& n: nodes_in){
            nodes.insert({&data[n->row][n->column], std::set<Cell*>({&data[n->row][n->column]})});
            data[n->row][n->column].owner_node = &data[n->row][n->column];
        }
        UPDATED_BOARD = false;
    }

    void Init();
    bool SolveTrivial();
    bool CheckEnd();
    bool FinishIsland(Cell* node);

private:

    //PRODUCTIVE
    //ONCE AT BEGGINING
    bool OnesCheck();
    bool UnreachablesCheck();
    //EVERY ITERATION
    bool BetweenIslandsCheck();
    bool OceanConnectCheck();
    bool OnlyOneOptionCheck();
    bool FillIslandsCheck();
    bool TwoOptionsDiagonalCheck();

    //other utils
    bool findOwner(Cell* cell);
    bool canIslandBePlaced(Cell* cell, Cell* wouldBeOwner);

    //ONLY CHECK
    //ONCE AT END
    bool CheckOceanConnect();
    //EVERY ITERATION
    bool Check4x4Ocean();


};

#endif // SOLVER_H
