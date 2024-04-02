#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <set>
#include "utils.h"
//flag that the board was updated with the last call

std::vector<std::vector<Cell>> readFromFile(const std::string path){

    std::vector<std::vector<int>> data;
    std::ifstream in(path);
    std::string temp_s = "";
    std::stringstream ss;
    int temp_i = 0;

    //get values in array
    while(getline(in,temp_s)){
        data.emplace_back(std::vector<int>());
        ss << temp_s;

        while(ss.good()){
            ss >> temp_i;
            //cout << temp_i;
            data.back().emplace_back(temp_i);
        }
        ss.str("");
        ss.clear();
    }

    //create cell array
    std::vector<std::vector<Cell>> cells;
    for(uint16_t i = 0; i < data.size();i++){

        cells.emplace_back(std::vector<Cell>());
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j] > 0){
                cells[i].emplace_back(Cell(i,j,data[i][j]));
            }
            else{
                cells[i].emplace_back(Cell(i,j));
            }
        }
    }
    return cells;
}

std::vector<Cell*> getAllNodes(std::vector<std::vector<Cell>> &data){
    std::vector<Cell*> out;
    for(uint16_t i = 0; i < data.size(); i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Node)
                out.emplace_back(&data[i][j]);
        }
    }
    return out;
}

void print(std::vector<std::vector<Cell>> &data){
    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Node){
                std::cout << data[i][j].max_num_islands;
                if(data[i][j].max_num_islands < 10)
                    std::cout << " ";
            }
            else if(data[i][j].color == Island)
                std::cout << ". ";
            else if(data[i][j].color == Ocean)
                std::cout << "# ";
            else
                std::cout << "  ";
        }
        std::cout << std::endl;
    }
    for(uint16_t j = 0; j < data[0].size(); j++){
        std::cout << "-";
    }
    std::cout << std::endl;
}

std::vector<Cell*> getColorNeighbours(std::vector<std::vector<Cell>> &data, Cell* cell, std::vector<Color> c){
    std::vector<Cell*> out;
    for(auto color: c){
        if(cell->row > 0 && data[cell->row-1][cell->column].color == color)
            out.emplace_back(&data[cell->row-1][cell->column]);

        if(cell->column < static_cast<int>(data[cell->row].size()-1) && data[cell->row][cell->column+1].color == color)
            out.emplace_back(&data[cell->row][cell->column+1]);

        if(cell->row < static_cast<int>(data.size()-1) && data[cell->row+1][cell->column].color == color)
            out.emplace_back(&data[cell->row+1][cell->column]);

        if(cell->column > 0 && data[cell->row][cell->column-1].color == color)
            out.emplace_back(&data[cell->row][cell->column-1]);
    }
    return out;
}

int getDistance(Cell* first, Cell* second){
    return abs(first->row - second->row) + abs(first->column - second->column);
}

bool forEveryCell(std::vector<std::vector<Cell>> &data, bool (*func)(std::vector<std::vector<Cell>>&,Cell*), Color c){
    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == c){
                if(!func(data, &data[i][j]))
                    return false;
            }
        }
    }
    return true;
}

bool forEveryNode(std::vector<std::vector<Cell>> &data, std::vector<Cell*> nodes, bool (*func)(std::vector<std::vector<Cell>>&,Cell*)){
    for(uint16_t i = 0; i < nodes.size();i++){
        if(nodes[i]->max_num_islands > nodes[i]->num_islands){
            if(!func(data, nodes[i]))
                return false;
        }
    }
    return true;
}

bool finishIsland(std::vector<std::vector<Cell>> &data, Cell* node){
    std::vector<Cell*> neighbours;
    std::queue<Cell*> q;
    std::set<Cell*> lookedCells;

    q.push(node);
    lookedCells.emplace(node);
    while(!q.empty()){
        //get up to one unknown neighbour
        neighbours = getColorNeighbours(data, q.front(), {Unknown});
        for(auto n: neighbours){
            if(!n->changeColor(Ocean)) return false;
        }

        neighbours = getColorNeighbours(data, q.front(), {Island});
        for(auto n: neighbours){
            if(lookedCells.find(n) == lookedCells.end()){
                q.push(n);
                lookedCells.emplace(n);
            }
        }
        q.pop();
    }
    return true;
}

void checkSolution(std::vector<std::vector<Cell>> &data, std::string sol_path){
    std::vector<std::vector<std::string>> sol;
    std::ifstream in(sol_path);
    std::string temp_s = "";
    std::stringstream ss;
    std::string temp_i;

    //get values in array
    while(getline(in,temp_s)){
        sol.emplace_back(std::vector<std::string>());
        ss << temp_s;

        while(ss.good()){
            ss >> temp_i;
            //cout << temp_i;
            sol.back().emplace_back(temp_i);
        }
        ss.str("");
        ss.clear();
    }
    bool all_good = true;

    for(uint16_t i = 0; i < data.size();i++){
        for(uint16_t j = 0; j < data[i].size(); j++){
            if(data[i][j].color == Node)
                std::cout << data[i][j].max_num_islands;
            else if(data[i][j].color == Island){
                if(sol[i][j] == ".")
                    std::cout << ".";
                else{
                    std::cout << "?";
                    all_good = false;
                }
            }
            else if(data[i][j].color == Ocean){
                if(sol[i][j] == "#")
                    std::cout << "#";
                else{
                    std::cout << "+";
                    all_good = false;
                }
            }
            else
                std::cout << " ";
        }
        std::cout << std::endl;
    }
    if(all_good){
        std::cout << "ALL GOOOOD" << std::endl;
    }
    else
        std::cout << "MISTAKE" << std::endl;
    for(uint16_t j = 0; j < data[0].size(); j++){
        std::cout << "-";
    }
    std::cout << std::endl;
}

