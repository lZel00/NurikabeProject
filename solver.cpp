#include <vector>
#include <set>
#include <queue>
#include <iostream>
#include <stack>
#include "solver.h"
#include "utils.h"

void Solver::Init(){
    OnesCheck();
    UnreachablesCheck();
}

bool Solver::SolveTrivial(){
    //MAKE THIS BETTER
    do{
        UPDATED_BOARD = false;
        //productive checks
        if(!BetweenIslandsCheck()) return false;
        if(!OnlyOneOptionCheck()) return false;
        if(!OceanConnectCheck()) return false;
        if(!FillIslandsCheck()) return false; //something wrong with this
        if(!TwoOptionsDiagonalCheck()) return false; //something wrong with this
        //only checks
        //check if 4x4 is good - doesnt do anything - good to check early so you dont create unnecesary branches
        if(!Check4x4Ocean()) return false;

    }while(UPDATED_BOARD);

    return true;
}

bool Solver::FinishIsland(Cell* node){

    for(auto cell: nodes.find(node)->second){
        //get up to one unknown neighbour
        neighbours = getColorNeighbours(data, cell, {Unknown});
        for(auto n: neighbours){
            if(!n->changeColor(Ocean)) return false;
        }
    }
    return true;
}

void Solver::MakeExploratoryMove(Cell* node, int row, int column){
    data[row][column].color = Island;
}

//only do once at beggining- mark around ones
bool Solver::OnesCheck(){
    for(auto const& node : nodes){
        if(node.first->max_num_islands == 1){
            std::vector<Cell*> temp = getColorNeighbours(data, node.first, {Unknown});
            for(auto c: temp){
                UPDATED_BOARD = true;
                if(!c->changeColor(Ocean))
                    return false;
            }
        }
    }
    return true;
}

bool Solver::UnreachablesCheck(){
    //marks nodes as ocean if they are unreahable by any island
    std::queue<Cell*> q;
    std::vector<std::vector<bool>> reachable(data.size(),std::vector<bool>(data[0].size()));

    for(auto const& node : nodes){
        q.push(node.first);
        std::vector<std::vector<bool>> reachableIt(data.size(),std::vector<bool>(data[0].size()));
        while(!q.empty()){
            if( reachableIt[q.front()->row][q.front()->column] ||
                getDistance(node.first, q.front()) > node.first->max_num_islands - node.first->num_islands){

                q.pop();
                continue;
            }

            neighbours = getColorNeighbours(data, q.front(), {Unknown,Node});
            for(auto n : neighbours)
                q.push(n);

            reachableIt[q.front()->row][q.front()->column] = true;
            reachable[q.front()->row][q.front()->column] = true;
            q.pop();
        }
    }
    for(uint16_t i = 0; i < data.size(); i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(reachable[i][j] == false){
                UPDATED_BOARD = true;
                if(!data[i][j].changeColor(Ocean))
                    return false;
            }
        }
    }
    return true;
}
//for unknowns
bool Solver::BetweenIslandsCheck(){
    //leave it like this, it works, more efficient
    std::set<Cell*> tolerated_owner;

    for(uint16_t row = 0; row < data.size(); row++){
        for(uint16_t column = 0; column < data[row].size(); column++){
            if(data[row][column].color == Unknown){
                tolerated_owner.clear();
                if(row > 0 && data[row-1][column].color >= Island )
                    tolerated_owner.emplace(data[row-1][column].owner_node);

                if(column < static_cast<int>(data[row].size()-1) && data[row][column+1].color >= Island)
                    tolerated_owner.emplace(data[row][column+1].owner_node);

                if(row < static_cast<int>(data.size()-1) && data[row+1][column].color >= Island)
                    tolerated_owner.emplace(data[row+1][column].owner_node);

                if(column > 0 && data[row][column-1].color >= Island)
                    tolerated_owner.emplace(data[row][column-1].owner_node);

                if(tolerated_owner.size() > 1){
                    UPDATED_BOARD = true;
                    if(!data[row][column].changeColor(Ocean))
                        return false;
                }
            }
        }
    }
    return true;
}
//for unknowns
bool Solver::OceanConnectCheck(){
    //quite hard - check if whole Ocean formation (set, queue) has any more than 1 Free neighbour
    //TODO make it so you don't have to recheck it for whole formation
    std::set<Cell*> ocean_formation;
    std::vector<Cell*> starting_ocean_neighbours;
    //we get all ocean neighbours

    for(auto& row : data){
        for(auto& cell: row){
            if(cell.color == Unknown){
                ocean_formation.clear();
                starting_ocean_neighbours = getColorNeighbours(data, &cell, {Ocean});

                if(starting_ocean_neighbours.size() == 4){
                    UPDATED_BOARD = true;
                    if(!cell.changeColor(Ocean))
                        return false;
                    return true;
                }

                for(auto &ocean: starting_ocean_neighbours){
                    std::queue<Cell*> q;
                    std::set<Cell*> this_ocean_formation;

                    bool isOcean = true;

                    q.push(ocean);
                    this_ocean_formation.emplace(q.front());
                    while(!q.empty()){
                        //if we find any other Unknown neighbours - break
                        neighbours = getColorNeighbours(data, q.front(), {Unknown});
                        //problem - even if you have access to unknown cell it doent mean its an exit tile
                        if(!neighbours.empty() && !(neighbours.size() == 1 && neighbours[0] == &cell)){
                            isOcean = false;
                            break;
                        }

                        neighbours = getColorNeighbours(data, q.front(), {Ocean});
                        for(auto n: neighbours){
                            if(ocean_formation.find(n) != ocean_formation.end()){
                                return true;
                            }
                            if(this_ocean_formation.find(n) == this_ocean_formation.end()){
                                this_ocean_formation.emplace(n);
                                q.push(n);
                            }
                        }
                        q.pop();
                    }
                    for(auto o : this_ocean_formation){
                        ocean_formation.emplace(o);
                    }
                    if(isOcean){
                        UPDATED_BOARD = true;
                        if(!cell.changeColor(Ocean))
                            return false;
                    }
                }
            }
        }
    }
    return true;
}
//for nodes
bool Solver::OnlyOneOptionCheck(){
    Cell* one_neighbour = nullptr;

    for(auto& node : nodes){
        one_neighbour = nullptr;
        for(auto const& island: node.second){
            neighbours = getColorNeighbours(data, island, {Unknown});
            if(neighbours.size() > 1){
                one_neighbour = nullptr; //this is set since breaking still goes to the if statement
                break;
            }
            else if(neighbours.size() == 1){
                if(one_neighbour == nullptr){
                    one_neighbour = neighbours[0];
                }
                else{
                    one_neighbour = nullptr;
                    break;
                }
            }
        }
        if(one_neighbour){
            UPDATED_BOARD = true;
            if(!one_neighbour->changeColor(Island))
                return false;
            one_neighbour->owner_node = node.first;
            node.first->num_islands++;
            node.second.insert(one_neighbour);

            if(node.first->max_num_islands == node.first->num_islands)
                finishIsland(data, node.first);
        }
    }
    return true;
}

//input nodes
bool Solver::FillIslandsCheck(){
    //this checks if i know that one direction doesnt have space, need to go to that direction
    std::vector<Cell*> all_unknown_neighbours;
    std::vector<FillIsland> possible_stacks;//first is maximal depth, second is stack
    int num_unknown_neighbours = 0;

    for(auto& n: nodes){
        all_unknown_neighbours.clear();
        const int required_cells = n.first->max_num_islands - n.first->num_islands;

        for(auto const& island: n.second){
            neighbours = getColorNeighbours(data, island, {Unknown});
            for(auto& n: neighbours){
                all_unknown_neighbours.emplace_back(n);
            }
        }

        possible_stacks.clear();
        num_unknown_neighbours = 0;
        if(all_unknown_neighbours.size() > 1){
            for(auto const& n1: all_unknown_neighbours){

                possible_stacks.emplace_back(FillIsland());
                possible_stacks.back().s.push(n1);
                possible_stacks.back().visited.emplace(n1);
                while(!possible_stacks.back().s.empty() &&
                       possible_stacks.back().max_depth < required_cells) {

                    std::vector<Cell*> unknown_neighbours = getColorNeighbours(data, possible_stacks.back().s.top(), {Unknown});

                    num_unknown_neighbours = 0;
                    for(auto const& n2: unknown_neighbours){
                        if(possible_stacks.back().visited.find(n2) == possible_stacks.back().visited.end()){
                            possible_stacks.back().max_depth++;
                            possible_stacks.back().s.push(n2);
                            possible_stacks.back().visited.emplace(n2);
                            num_unknown_neighbours++;
                        }
                    }
                    //i pop in case there are no new neighbours because we are working with stack - we want to return to this node. But if there is nothing left we dont have to
                    if(num_unknown_neighbours == 0)
                        possible_stacks.back().s.pop();
                }

            }
            //now we check the potential depth
            int sum_of_less_than = 0;
            Cell* inf = nullptr;
            for(uint16_t i = 0; i < possible_stacks.size(); i++){
                if(possible_stacks[i].max_depth < required_cells)
                    sum_of_less_than += possible_stacks[i].max_depth;
                else if(inf){
                    sum_of_less_than = required_cells+100;
                    break;
                }
                else
                    inf = all_unknown_neighbours[i];
            }
            if(sum_of_less_than <= required_cells){
                UPDATED_BOARD = true;
                if(inf){
                    if(!inf->changeColor(Island))
                        return false;
                    inf->owner_node = n.first;
                    n.first->num_islands++;
                    n.second.insert(inf);
                }
                else{
                    for(auto n1: all_unknown_neighbours){
                        if(!n1->changeColor(Island))
                            return false;
                        n1->owner_node = n.first;
                        n.first->num_islands++;
                        n.second.insert(n1);
                    }
                }
                if(n.first->max_num_islands == n.first->num_islands)
                    finishIsland(data, n.first);
            }
        }
    }
    return true;
}
//for every node
bool Solver::TwoOptionsDiagonalCheck(){
    //if 1 more nneded option and a only 2 options are diagonal, we know the diagonal is good

    for(auto const& node: nodes){
        if(node.first->max_num_islands - node.first->num_islands == 1){

            for(auto const& island: node.second){
                neighbours = getColorNeighbours(data, island, {Unknown});
                //problem - even if you have access to unknown cell it doent mean its an exit tile
                if(neighbours.size() == 2){
                    //the order of neighbours is always from 12 o clock in clockwise
                    if(neighbours[0]->row+1== island->row && neighbours[1]->column-1 == island->column &&
                        data[island->row-1][island->column+1].color != Ocean){
                        UPDATED_BOARD = true;
                        if(!data[island->row-1][island->column+1].changeColor(Ocean))
                            return false;
                    }
                    else if(neighbours[0]->column-1 == island->column && neighbours[1]->row-1 == island->row &&
                             data[island->row+1][island->column+1].color != Ocean){
                        UPDATED_BOARD = true;
                        if(!data[island->row+1][island->column+1].changeColor(Ocean))
                            return false;
                    }
                    else if(neighbours[0]->row-1 == island->row && neighbours[1]->column+1 == island->column &&
                             data[island->row+1][island->column-1].color != Ocean){
                        UPDATED_BOARD = true;
                        if(!data[island->row+1][island->column-1].changeColor(Ocean))
                            return false;
                    }
                    else if(neighbours[1]->column+1 == island->column && neighbours[0]->row+1 == island->row &&
                             data[island->row-1][island->column-1].color != Ocean){
                        UPDATED_BOARD = true;
                        if(!data[island->row-1][island->column-1].changeColor(Ocean))
                            return false;
                    }
                }
            }
        }
    }
    return true;
}

//non productive check - for every ocean
bool Solver::CheckOceanConnect(){
    std::set<Cell*> all_oceans;
    std::set<Cell*> connected_oceans;
    std::queue<Cell*> q;
    Cell* start;

    for(uint16_t i = 0; i < data.size(); i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Ocean){
                all_oceans.emplace(&data[i][j]);
                start = &data[i][j]; //random - last ocean is the start!
            }
        }
    }

    q.push(start);
    connected_oceans.emplace(q.front());
    while(!q.empty()){
        neighbours = getColorNeighbours(data, q.front(), {Ocean});
        for(auto n: neighbours){
            if(connected_oceans.find(n) == connected_oceans.end()){
                connected_oceans.emplace(n);
                q.push(n);
            }
        }
        q.pop();
    }

    return connected_oceans.size() == all_oceans.size();
}

bool Solver::Check4x4Ocean(){
    //TODO FIND ITS OWNER!!!
    for(uint16_t row = 0; row < data.size(); row++){
        for(uint16_t column = 0; column < data[row].size(); column++){

            if(data[row][column].color == Ocean &&
                ((row > 0 && column > 0 && data[row-1][column].color == Ocean &&
                 data[row][column-1].color == Ocean && data[row-1][column-1].color == Ocean)
                ||
                (row > 0 && column < static_cast<int>(data[row].size()-1) && data[row-1][column].color == Ocean &&
                 data[row][column+1].color == Ocean && data[row-1][column+1].color == Ocean)
                ||
                (row < static_cast<int>(data.size()-1) && column > 0 && data[row+1][column].color == Ocean &&
                 data[row][column-1].color == Ocean && data[row+1][column-1].color == Ocean)
                ||
                (row < static_cast<int>(data.size()-1) && column < static_cast<int>(data[row].size()-1)
                  && data[row+1][column].color == Ocean && data[row][column+1].color == Ocean && data[row+1][column+1].color == Ocean))){

                //UPDATED_BOARD = true;
                //return cell->changeColor(Island);
                return false;
            }
        }
    }
    return true;
}

bool Solver::CheckEnd(){
    if(!Check4x4Ocean())
        return false;
    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Unknown)
                return false;
        }
    }
    //if all good also check if all oceans connected
    return CheckOceanConnect();
}
