#include "cell.h"
#include "utils.h"
#include <vector>
#include <set>
#include <queue>
#include <stack>

bool UPDATED_BOARD = true;
//only do once at beggining- mark around ones
bool OnesCheck(std::vector<std::vector<Cell>> &data, std::vector<Cell*> nodes){
    for(auto &node : nodes){
        if(node->max_num_islands == 1){
            std::vector<Cell*> temp = getColorNeighbours(data, node, {Unknown});
            for(auto c: temp){
                UPDATED_BOARD = true;
                if(!c->changeColor(Ocean))
                    return false;
            }
        }
    }
    return true;
}

bool UnreachablesCheck(std::vector<std::vector<Cell>> &data, std::vector<Cell*> nodes){
    //marks nodes as ocean if they are unreahable by any island
    std::queue<Cell*> q;
    std::vector<std::vector<bool>> reachable(data.size(),std::vector<bool>(data[0].size()));
    std::vector<Cell*> neigbours;

    for(uint16_t i = 0; i < nodes.size(); i++){
        q.push(nodes[i]);
        std::vector<std::vector<bool>> reachableIt(data.size(),std::vector<bool>(data[0].size()));
        while(!q.empty()){
            if( reachableIt[q.front()->row][q.front()->column] ||
                getDistance(nodes[i], q.front()) > nodes[i]->max_num_islands-nodes[i]->num_islands){

                q.pop();
                continue;
            }

            neigbours = getColorNeighbours(data, q.front(), {Unknown,Node});
            for(auto n : neigbours)
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
bool BetweenIslandsCheck(std::vector<std::vector<Cell>> &data,Cell* cell){
    //leave it like this, it works, more efficient
    std::set<Cell*> tolerated_owner;

    if(cell->row > 0 && data[cell->row-1][cell->column].color >= Island )
        tolerated_owner.emplace(data[cell->row-1][cell->column].owner_node);

    if(cell->column < static_cast<int>(data[cell->row].size()-1) && data[cell->row][cell->column+1].color >= Island)
        tolerated_owner.emplace(data[cell->row][cell->column+1].owner_node);

    if(cell->row < static_cast<int>(data.size()-1) && data[cell->row+1][cell->column].color >= Island)
        tolerated_owner.emplace(data[cell->row+1][cell->column].owner_node);

    if(cell->column > 0 && data[cell->row][cell->column-1].color >= Island)
        tolerated_owner.emplace(data[cell->row][cell->column-1].owner_node);

    if(tolerated_owner.size() > 1){
        UPDATED_BOARD = true;
        if(!cell->changeColor(Ocean))
            return false;
    }
    return true;
}
//for unknowns
bool OceanConnectCheck(std::vector<std::vector<Cell>> &data,Cell* cell){
    //quite hard - check if whole Ocean formation (set, queue) has any more than 1 Free neighbour
    //TODO make it so you don't have to recheck it for whole formation
    std::set<Cell*> ocean_formation;
    std::vector<Cell*> starting_ocean_neighbours;
    //we get all ocean neighbours
    starting_ocean_neighbours = getColorNeighbours(data, cell, {Ocean});

    if(starting_ocean_neighbours.size() == 4){
        UPDATED_BOARD = true;
        if(!cell->changeColor(Ocean))
            return false;
        return true;
    }

    for(auto &ocean: starting_ocean_neighbours){
        std::vector<Cell*> neighbours;
        std::queue<Cell*> q;
        std::set<Cell*> this_ocean_formation;

        bool isOcean = true;

        q.push(ocean);
        this_ocean_formation.emplace(q.front());
        while(!q.empty()){
            //if we find any other Unknown neighbours - break
            neighbours = getColorNeighbours(data, q.front(), {Unknown});
            //problem - even if you have access to unknown cell it doent mean its an exit tile
            if(!neighbours.empty() && !(neighbours.size() == 1 && neighbours[0] == cell)){
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
            return cell->changeColor(Ocean);
        }
    }
    return true;
}
//for nodes
bool OnlyOneOptionCheck(std::vector<std::vector<Cell>> &data,Cell* node){

    std::vector<Cell*> neighbours;
    std::queue<Cell*> q;
    std::set<Cell*> looked_cells;
    Cell* one_neighbour = nullptr;

    q.push(node);
    looked_cells.emplace(node); //not needed because  this is node
    while(!q.empty()){
        //get up to one unknown neighbour
        neighbours = getColorNeighbours(data, q.front(), {Unknown});
        if(neighbours.size() > 1)
            return true;
        else if(neighbours.size() == 1){
            if(one_neighbour == nullptr)
                one_neighbour = neighbours[0];
            else
                return true;
        }

        neighbours = getColorNeighbours(data, q.front(), {Island});
        for(auto n: neighbours){
            if(looked_cells.find(n) == looked_cells.end()){
                q.push(n);
                looked_cells.emplace(n);
            }
        }
        q.pop();
    }
    //IF there is no no one_neighbour there IS A MISTAKE!!
    if(one_neighbour){
        UPDATED_BOARD = true;
        if(!one_neighbour->changeColor(Island))
            return false;
        one_neighbour->owner_node = node;
        node->num_islands++;


        if(node->max_num_islands == node->num_islands)
            finishIsland(data, node);
        return true;
    }
    return false;
}

//input nodes
bool FillIslandsCheck(std::vector<std::vector<Cell>> &data, Cell* node){
    //this checks if i know that one direction doesnt have space, need to go to that direction
    std::vector<Cell*> all_unknown_neighbours;
    std::vector<Cell*> neighbours;
    std::queue<Cell*> main_search_q;
    std::vector<FillIsland> possible_stacks;//first is maximal depth, second is stack
    std::set<Cell*> looked_cells;
    const int required_cells = node->max_num_islands - node->num_islands;
    int num_new_neighbours = 0;

    main_search_q.push(node);
    looked_cells.emplace(node);
    while(!main_search_q.empty()){
        //get up to one unknown neighbour
        neighbours = getColorNeighbours(data, main_search_q.front(), {Island});

        for(auto n: neighbours){
            if(looked_cells.find(n) == looked_cells.end()){
                main_search_q.push(n);
                looked_cells.emplace(n);
            }
        }

        neighbours = getColorNeighbours(data, main_search_q.front(), {Unknown});
        for(auto n: neighbours){
            all_unknown_neighbours.emplace_back(n);
        }
        main_search_q.pop();
    }

    possible_stacks.clear();
    if(all_unknown_neighbours.size() > 1){
        for(auto n1: all_unknown_neighbours){

            possible_stacks.emplace_back(FillIsland());
            possible_stacks.back().s.push(n1);
            possible_stacks.back().visited.emplace(n1);
            while(!possible_stacks.back().s.empty() &&
                   possible_stacks.back().max_depth < required_cells) {

                std::vector<Cell*> new_neighbours = getColorNeighbours(data, possible_stacks.back().s.top(), {Unknown});

                num_new_neighbours = 0;
                for(auto n2: new_neighbours){
                    if(possible_stacks.back().visited.find(n2) == possible_stacks.back().visited.end()){
                        possible_stacks.back().max_depth++;
                        possible_stacks.back().s.push(n2);
                        possible_stacks.back().visited.emplace(n2);
                        num_new_neighbours++;
                    }
                }
                //i pop in case there are no new neighbours because we are working with stack - we want to return to this node. But if there is nothing left we dont have to
                if(num_new_neighbours == 0)
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
                inf->owner_node = node;
                node->num_islands++;
            }
            else{
                for(auto n1: all_unknown_neighbours){
                    if(!n1->changeColor(Island))
                        return false;
                    n1->owner_node = node;
                    node->num_islands++;
                }
            }
            if(node->max_num_islands == node->num_islands)
                finishIsland(data, node);
        }
    }
    return true;
}
//for every unknown
bool Check4x4Ocean(std::vector<std::vector<Cell>> &data,Cell* cell){
    //TODO FIND ITS OWNER!!!
    if((cell->row > 0 && cell->column > 0 && data[cell->row-1][cell->column].color == Ocean &&
        data[cell->row][cell->column-1].color == Ocean && data[cell->row-1][cell->column-1].color == Ocean)
        ||
        (cell->row > 0 && cell->column < static_cast<int>(data[cell->row].size()-1) && data[cell->row-1][cell->column].color == Ocean &&
         data[cell->row][cell->column+1].color == Ocean && data[cell->row-1][cell->column+1].color == Ocean)
        ||
        (cell->row < static_cast<int>(data.size()-1) && cell->column > 0 && data[cell->row+1][cell->column].color == Ocean &&
         data[cell->row][cell->column-1].color == Ocean && data[cell->row+1][cell->column-1].color == Ocean)
        ||
        (cell->row < static_cast<int>(data.size()-1) && cell->column < static_cast<int>(data[cell->row].size()-1)
         && data[cell->row+1][cell->column].color == Ocean && data[cell->row][cell->column+1].color == Ocean && data[cell->row+1][cell->column+1].color == Ocean)){

        //UPDATED_BOARD = true;
        //return cell->changeColor(Island);
        return false;
    }
    return true;
}
//for every node
bool CheckTwoOptionsDiagonal(std::vector<std::vector<Cell>> &data,Cell* node){
    //if 1 more nneded option and a only 2 options are diagonal, we know the diagonal is good
    if(node->max_num_islands - node->num_islands == 1){
        std::vector<Cell*> neighbours;
        std::queue<Cell*> q;
        std::set<Cell*> visited;

        q.push(node);
        visited.emplace(q.front());
        while(!q.empty()){
            //if we find any other Unknown neighbours - break
            neighbours = getColorNeighbours(data, q.front(), {Unknown});
            //problem - even if you have access to unknown cell it doent mean its an exit tile
            if(neighbours.size() == 2){
                //the order of neighbours is always from 12 o clock in clockwise
                if(neighbours[0]->row+1== q.front()->row && neighbours[1]->column-1 == q.front()->column &&
                    data[q.front()->row-1][q.front()->column+1].color != Ocean){
                    UPDATED_BOARD = true;
                    if(!data[q.front()->row-1][q.front()->column+1].changeColor(Ocean))
                        return false;
                }
                else if(neighbours[0]->column-1 == q.front()->column && neighbours[1]->row-1 == q.front()->row &&
                        data[q.front()->row+1][q.front()->column+1].color != Ocean){
                    UPDATED_BOARD = true;
                    if(!data[q.front()->row+1][q.front()->column+1].changeColor(Ocean))
                        return false;
                }
                else if(neighbours[0]->row-1 == q.front()->row && neighbours[1]->column+1 == q.front()->column &&
                        data[q.front()->row+1][q.front()->column-1].color != Ocean){
                    UPDATED_BOARD = true;
                    if(!data[q.front()->row+1][q.front()->column-1].changeColor(Ocean))
                        return false;
                }
                else if(neighbours[1]->column+1 == q.front()->column && neighbours[0]->row+1 == q.front()->row &&
                        data[q.front()->row-1][q.front()->column-1].color != Ocean){
                    UPDATED_BOARD = true;
                    if(!data[q.front()->row-1][q.front()->column-1].changeColor(Ocean))
                        return false;
                }
            }

            neighbours = getColorNeighbours(data, q.front(), {Island});
            for(auto n: neighbours){
                if(visited.find(n) == visited.end()){
                    visited.emplace(n);
                    q.push(n);
                }
            }
            q.pop();
        }
    }
    return true;
}
//non productive check - for every ocean
bool CheckOceanConnect(std::vector<std::vector<Cell>> &data){
    std::set<Cell*> all_oceans;
    std::set<Cell*> connected_oceans;
    std::vector<Cell*> neighbours;
    std::queue<Cell*> q;
    Cell* start;

    for(uint16_t i = 0; i < data.size(); i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Ocean){
                all_oceans.emplace(&data[i][j]);
                start = &data[i][j];
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
    //start at random ocean
}

bool CheckEnd(std::vector<std::vector<Cell>> &data){
    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Unknown)
                return false;
        }
    }
    //if all good also check if all oceans connected
    return CheckOceanConnect(data);
}
