#include <vector>
#include <set>
#include <queue>
#include <iostream>
#include <stack>
#include "solver.h"
#include "utils.h"

void Solver::Init(){
    OnesCheck();
}

bool Solver::SolveTrivial(){
    //MAKE THIS BETTER
    do{
        UPDATED_BOARD = false;
        //productive checks
        if(!BetweenIslandsCheck()) return false;

        do{
            UPDATED_BOARD = false;
            if(!OnlyOneOptionCheck()) return false;
        }while(UPDATED_BOARD);

        if(!OceanConnectCheck()) return false;
        if(!FillIslandsCheck()) return false; //something wrong with this
        if(!TwoOptionsDiagonalCheck()) return false; //something wrong with this
        if(!UnreachablesCheck()) return false;
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
    //TODO - FIX
    std::queue<Cell*> q;
    std::vector<std::vector<bool>> reachable(data.size(),std::vector<bool>(data[0].size()));

    for(auto const& node : nodes){
        if(node.first->num_islands < node.first->max_num_islands){
            for(auto const& island: node.second){
                //i need to do every iteration seperatebly, because one small island can "hide" cells from bigger island that could reach it
                std::vector<std::vector<int>> reachableIt(data.size(),std::vector<int>(data[0].size(),999));

                q.push(island);
                reachableIt[q.front()->row][q.front()->column] = 0;
                reachable[q.front()->row][q.front()->column] = true;
                while(!q.empty()){
                    if( reachableIt[q.front()->row][q.front()->column] >= node.first->max_num_islands - node.first->num_islands){
                        q.pop();
                        continue;
                    }

                    neighbours = getColorNeighbours(data, q.front(), {Unknown,Node, Island});
                    for(auto n : neighbours){
                        if(reachableIt[n->row][n->column] > reachableIt[q.front()->row][q.front()->column]+1){
                            q.push(n);
                            reachableIt[n->row][n->column] = reachableIt[q.front()->row][q.front()->column]+1;
                            reachable[n->row][n->column] = true;
                        }
                    }
                    q.pop();
                }
            }
        }
    }
    for(uint16_t i = 0; i < data.size(); i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(reachable[i][j] == false && data[i][j].color == Unknown){
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
    std::vector<Cell*> starting_ocean_neighbours;
    std::vector<Cell*> all_unknown_neighbours;
    //we get all ocean neighbours

    for(auto& row : data){
        for(auto& cell: row){
            if(cell.color == Unknown){
                starting_ocean_neighbours = getColorNeighbours(data, &cell, {Ocean});

                if(starting_ocean_neighbours.size() == 4){
                    UPDATED_BOARD = true;
                    if(!cell.changeColor(Ocean))
                        return false;
                    continue;
                }

                for(auto &ocean: starting_ocean_neighbours){
                    std::queue<Cell*> q;
                    std::set<Cell*> this_ocean_formation;
                    all_unknown_neighbours.clear();

                    q.push(ocean);
                    this_ocean_formation.emplace(q.front());
                    while(!q.empty()){
                        //if we find any other Unknown neighbours - break
                        neighbours = getColorNeighbours(data, q.front(), {Unknown});
                        //problem - even if you have access to unknown cell it doent mean its an exit tile
                        for(auto const& n: neighbours){
                            if(n != &cell)
                                all_unknown_neighbours.emplace_back(n);
                        }

                        neighbours = getColorNeighbours(data, q.front(), {Ocean});
                        for(auto n: neighbours){
                            if(this_ocean_formation.emplace(n).second){
                                q.push(n);
                            }
                        }
                        q.pop();
                    }
                    if(all_unknown_neighbours.size() == 0){
                        UPDATED_BOARD = true;
                        if(!cell.changeColor(Ocean))
                            return false;
                    }
                    //checks if it touches any different ocean formation than this_ocean_formation

                    else{
                        bool end_search = false;
                        std::set<Cell*> this_unknows_form;
                        for(auto const& un: all_unknown_neighbours){

                            std::queue<Cell*> q;
                            q.push(un);
                            this_unknows_form.emplace(un);
                            while(!q.empty()){
                                //if we find any other Unknown neighbours - break
                                neighbours = getColorNeighbours(data, q.front(), {Ocean});
                                //problem - even if you have access to unknown cell it doent mean its an exit tile
                                for(auto const& n: neighbours){
                                    if(this_ocean_formation.find(n) == this_ocean_formation.end()){
                                        end_search = true;
                                        break;
                                    }
                                }
                                if(end_search)
                                    break;

                                neighbours = getColorNeighbours(data, q.front(), {Unknown});
                                for(auto n: neighbours){
                                    //if insertion took place
                                    if(this_unknows_form.emplace(n).second){
                                        q.push(n);
                                    }
                                }
                                q.pop();
                            }
                            if(end_search)
                                break;
                        }
                        if(!end_search){
                            uint16_t ocean_counter = 0;
                            for(uint16_t i = 0; i < data.size(); i++){
                                for(uint16_t j = 0; j < data[i].size(); j++){
                                    if(data[i][j].color == Ocean)
                                        ocean_counter++;
                                }
                            }
                            if(ocean_counter > this_ocean_formation.size()){
                                UPDATED_BOARD = true;
                                //PROBLEM  - IT SEARCHES FOR DIFFERENT FORMATION - HOW ABOUT THERE IS ONLY 1!!!
                                if(!cell.changeColor(Ocean))
                                    return false;
                            }
                        }
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
        if(node.first->num_islands < node.first->max_num_islands){
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
                //required cells + 1 raziskujem, da ni opcije, da grem prekrijem vse
                while(!possible_stacks.back().s.empty() &&
                       possible_stacks.back().max_depth < required_cells+1) {

                    std::vector<Cell*> unknown_neighbours = getColorNeighbours(data, possible_stacks.back().s.top(), {Unknown});

                    num_unknown_neighbours = 0;
                    for(auto const& n2: unknown_neighbours){
                        if(canIslandBePlaced(n2,n.first) && possible_stacks.back().visited.emplace(n2).second){
                            possible_stacks.back().max_depth++;
                            possible_stacks.back().s.push(n2);
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
            std::set<Cell*> unique_possibilities;
            //posebej zanka da res vse unikatne dam skozi - break lahko to unici tu pa tam
            for(uint16_t i = 0; i < possible_stacks.size(); i++){
                unique_possibilities.insert(possible_stacks[i].visited.begin(), possible_stacks[i].visited.end());
            }
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
            //problem tu ce slucajno imas unique tocno toliko kot required cells - ceprav vec opcij - popolna unija
            //drugi if ce ima keri se vec opcij
            else if(static_cast<int>(unique_possibilities.size()) <= required_cells){
                for(auto const& n1: unique_possibilities){
                    if(!n1->changeColor(Island))
                        return false;
                    n1->owner_node = n.first;
                    n.first->num_islands++;
                    n.second.insert(n1);
                }
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
            bool all_good = false;
            for(auto const& island: node.second){
                neighbours = getColorNeighbours(data, island, {Unknown});
                if(neighbours.size() == 2 && !all_good)
                    all_good = true;
                else if(all_good && neighbours.size() > 0){
                    all_good = false;
                    break;
                }
                else if(neighbours.size() > 0)
                    break;
            }
            if(all_good){
                for(auto const& island: node.second){
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
                        break;
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
    //HERE MISTAKE
    for(uint16_t row = 0; row < data.size(); row++){
        for(uint16_t column = 0; column < data[row].size(); column++){

            if(data[row][column].color <= Ocean  &&
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

                if(data[row][column].color == Ocean)
                    return false;
                if(!findOwner(&data[row][column])){
                    return false;
                }
            }
        }
    }
    return true;
}
struct fOwnerStruct{
    Cell* c;
    std::vector<Cell*> prev;
    fOwnerStruct(Cell* a, std::vector<Cell*> &p){
        c = a;
        prev = p;
        prev.emplace_back(a);
    }
    fOwnerStruct(Cell* a){
        c = a;
    }
};

bool Solver::findOwner(Cell* cell){
    std::vector<std::pair<Cell*, std::vector<Cell*>>> paths;

    for(auto const& node: nodes){
        if(node.first->max_num_islands > node.first->num_islands && getDistance(node.first, cell) < node.first->max_num_islands){
            //path can exist!!
            uint16_t max_changes = node.first->max_num_islands - node.first->num_islands;
            std::deque<fOwnerStruct> q;
            std::set<Cell*> been_here;
            std::set<Cell*> mark_for_destruction;

            q.push_back(fOwnerStruct(node.first));
            been_here.emplace(node.first);
            while(!q.empty()){
                //if we are too far
                if(q.front().prev.size() > max_changes || mark_for_destruction.find(q.front().c) != mark_for_destruction.end()){
                    q.pop_front();
                    continue;
                }
                neighbours = getColorNeighbours(data, q.front().c, {Unknown, Island});

                for(auto const& n: neighbours){
                    //this for is to check if there are multible paths of equal lenght to this cell. If they are this cell is no good
                    for(auto const& el: q){
                        if(el.c == n)
                            mark_for_destruction.emplace(n);
                    }
                    //if we find cell
                    if(n == cell){
                        paths.emplace_back(std::make_pair(node.first, q.front().prev));
                        paths.back().second.emplace_back(cell);
                    }
                    //if we have not been here yet
                    else if(been_here.emplace(n).second){
                        if(n->color == Unknown){
                            q.push_back(fOwnerStruct(n, q.front().prev));
                        }
                        else{
                            q.push_back(fOwnerStruct(n));
                        }
                    }
                }
                q.pop_front();
            }
        }
    }
    //now we analyse all paths - if there are from same node but are multible
    if(paths.size() == 1){
        UPDATED_BOARD = true;
        for(auto const& path_cell: paths.front().second){
            if(!path_cell->changeColor(Island))
                return false;
            path_cell->owner_node = paths.begin()->first;
            paths.begin()->first->num_islands++;
            nodes.find(paths.begin()->first)->second.insert(path_cell);
        }
        if(paths[0].first->max_num_islands == paths[0].first->num_islands)
            finishIsland(data, paths[0].first);
    }
    else if(!paths.empty()){
        bool all_good = true;
        for(uint16_t i = 1; i < paths.size(); i++){
            if(paths[i].first != paths[0].first)
                all_good = false;

        }
        if(all_good){
            UPDATED_BOARD = true;
            if(!cell->changeColor(Island))
                return false;
            cell->owner_node = paths.begin()->first;
            paths.begin()->first->num_islands++;
            nodes.find(paths.begin()->first)->second.insert(cell);

            if(paths[0].first->max_num_islands == paths[0].first->num_islands)
                finishIsland(data, paths[0].first);
        }
    }

    return true;
}

bool Solver::canIslandBePlaced(Cell* cell, Cell* would_be_owner){
    if(cell->color != Unknown){
        return false;
    }
    neighbours = getColorNeighbours(data, cell, {Node, Island});
    for(auto const& n: neighbours){
        if(n->owner_node != would_be_owner)
            return false;
    }
    return true;
}

bool Solver::CheckEnd(){
    if(!Check4x4Ocean())
        return false;
    for(auto const& node: nodes){
        if(node.first->num_islands != node.first->max_num_islands)
            return false;
    }
    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Unknown)
                return false;
        }
    }
    //if all good also check if all oceans connected
    return CheckOceanConnect();
}
