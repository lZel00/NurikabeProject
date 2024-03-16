#include <iostream>
#include <string>
#include <set>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#include "cell.h"
#include "solver.h"
#include "utils.h"

//TODO stanja daš na queue
Solver queue_solver(std::vector<std::vector<Cell>> &data, std::vector<Cell*> &nodes) {
    std::queue<Solver> q;
    Cell* new_island;
    Cell* new_node;

    q.push(Solver(data, nodes));
    q.front().Init();
    while(!q.front().CheckEnd()){
        if(q.front().SolveTrivial()){

            for(auto const& node: q.front().nodes){
                if(node.first->num_islands < node.first->max_num_islands){
                    for(auto const& island: node.second){
                        q.front().neighbours = getColorNeighbours(q.front().data, island, {Unknown});
                        for(auto const& n: q.front().neighbours){
                            q.push(Solver(q.front()));
                            new_island = &q.back().data[n->row][n->column];
                            new_node = &q.back().data[node.first->row][node.first->column];

                            new_island->changeColor(Island);
                            new_island->owner_node = new_node;
                            q.back().nodes.find(new_node)->second.insert(new_island);
                            new_node->num_islands++;

                            if(new_node->max_num_islands == new_node->num_islands){
                                q.back().FinishIsland(new_node);
                            }

                        }
                    }
                }
            }
            //means that this accuary isnt' dead yet
        }
        q.pop();

    }
    return q.front();
}
Solver* stack_out;
bool stack_recursion(Solver &a){
    Cell* new_island;
    Cell* new_node;

    if(a.CheckEnd()){
        stack_out = new Solver(a);
        return true;
    }

    if(a.SolveTrivial()){
        for(auto const& node: a.nodes){
            if(node.first->num_islands < node.first->max_num_islands){

                for(auto const& island: node.second){
                    a.neighbours = getColorNeighbours(a.data, island, {Unknown});
                    for(auto const& n: a.neighbours){
                        Solver temp(a);

                        new_island = &temp.data[n->row][n->column];
                        new_node = &temp.data[node.first->row][node.first->column];

                        new_island->changeColor(Island);
                        new_island->owner_node = new_node;
                        new_node->num_islands++;
                        temp.nodes.find(new_node)->second.insert(new_island);

                        if(new_node->max_num_islands == new_node->num_islands){
                            temp.FinishIsland(new_node);
                        }

                        if(stack_recursion(temp))
                            return true;
                    }

                }
            }
        }
    }
    return false;
}
Solver stack_solver(std::vector<std::vector<Cell>> &data, std::vector<Cell*> &nodes) {

    Solver in(data, nodes);
    in.Init();
    stack_recursion(in);

    Solver out(*stack_out);
    delete stack_out;

    return out;
}
int main(){
    std::string example = "0";
    srand (time(NULL));

    const std::string in_filename = "D:/Faks/5/OptimizacijskeMetode/Nurikabe/nurikabe-primer" + example + ".txt";
    const std::string solution_filename = "D:/Faks/5/OptimizacijskeMetode/Nurikabe/nurikabe-resitev" + example + ".txt";

    std::vector<std::vector<Cell>> data = readFromFile(in_filename);
    std::vector<Cell*> nodes = getAllNodes(data);
    for(auto &a : nodes)
        a->owner_node = a;

    auto start = std::chrono::high_resolution_clock::now();

    print(data);

    Solver start_solver(data, nodes);
    start_solver.Init();
    if(!start_solver.SolveTrivial()){
        std::cout << "MISTAKE FOUND!!" << std::endl;
    }
    //VCASIH ONLYONEOPTIONCHECK NE DELA
    //Solver solution = stack_solver(data, nodes);


/*
    OnesCheck(data,nodes);
    UnreachablesCheck(data,nodes);
    solve_until_no_change(data, nodes);
    //while(!random_recursive_search(data));
    if(solve_until_no_change(data, nodes)){
        std::cout << "VSE DOBRO" << std::endl;
    }
    else{
        std::cout << "NEKAJ SLO NAROBE!!" << std::endl;
    }
*/
    //now the idea is to create recursive random funtion that randomly changes a possible value. THen solve until change
    //if an error happens it returns from recursion - be carefull not refrenece data with reference!!!!!


    checkSolution(start_solver.data, solution_filename);
    //checkSolution(solution.data, solution_filename);
    auto stop =  std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "That took " << duration.count() << "ms" << std::endl;
    return 0;
}
