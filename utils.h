#ifndef UTILS_H
#define UTILS_H

#include "cell.h"
#include <string>
#include <stack>
#include <set>

struct FillIsland{
    std::stack<Cell*> s;
    int max_depth;
    std::set<Cell*> visited;
    FillIsland(){max_depth = 1;}
};

std::vector<std::vector<Cell>> readFromFile(const std::string path);
std::vector<Cell*> getAllNodes(std::vector<std::vector<Cell>> &data);

void print(std::vector<std::vector<Cell>> &data);
std::vector<Cell*> getColorNeighbours(std::vector<std::vector<Cell>> &data, Cell* cell, std::vector<Color> c);

int getDistance(Cell* first, Cell* second);
bool forEveryCell(std::vector<std::vector<Cell>> &data, bool (*func)(std::vector<std::vector<Cell>>&,Cell*), Color c);
bool forEveryNode(std::vector<std::vector<Cell>> &data, std::vector<Cell*> nodes, bool (*func)(std::vector<std::vector<Cell>>&,Cell*));

bool finishIsland(std::vector<std::vector<Cell>> &data, Cell* node);
void checkSolution(std::vector<std::vector<Cell>> &data, std::string sol_path);
#endif // UTILS_H
