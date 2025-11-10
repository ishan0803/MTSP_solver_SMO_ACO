#pragma once
#ifndef ACO_H
#define ACO_H

#include "graph.hpp"
#include <vector>
#include <random>

class ACO {
private:
    Graph graph;
    int num_ants, num_cities;
    double alpha, beta, rho, Q;

    std::vector<std::vector<double>> pher_mat;
    std::vector<std::vector<int>> tours;
    std::vector<double> tour_length;

    std::vector<int> best_tour;
    double best_length;

    std::mt19937 rng;

    int select_next_city(int ant_idx, int current_city, const std::vector<bool>& visited);
    void construct_tour(int ant_idx);
    void evaporate_pher();
    void deposit_pher(const std::vector<int>& path, double length);
    void update_pher();

public:
    ACO(const std::vector<std::pair<double,double>>& pts, int ants,
        double alpha=1.0, double beta=5.0, double rho=0.5, double Q=100.0);                        
    void run(int iterations);   
    std::vector<int> final_route() const;
    double best_distance() const;
};

#endif
