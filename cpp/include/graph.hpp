#pragma once
#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

class Graph{
private:
    std::vector<std::vector<double>> dist;
    std::vector<std::pair<double,double>> Points;
public:
    Graph(const std::vector<std::pair<double,double>>& pts);
    
    void computeDistanceMatrix();
    double getDistance(int i, int j) const;
    int size() const;
    const std::vector<std::pair<double,double>>& getPoints() const;
    const std::vector<std::vector<double>>& getDist() const;
    const double nearest_neighbor_tour_length() const;
};
#endif