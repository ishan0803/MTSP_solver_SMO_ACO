#pragma once
#ifndef HYBRID_H
#define HYBRID_H

#include "graph.hpp"
#include "smo.hpp"
#include "aco.hpp"
#include <vector>
#include <utility>

class Hybrid {
public:
    Hybrid(const std::vector<std::pair<double,double>>& pts,
           int num_salesmen,
           int smo_iterations,
           int smo_population_size,
           int smo_local_limit,
           int smo_global_limit,
           double smo_pr,
           int aco_ants,
           int aco_iterations,
           double aco_alpha,
           double aco_beta,
           double aco_rho,
           double aco_Q);

    void run();

    std::vector<std::vector<int>> getRoutes() const;

    double getTotalLength() const;

private:
    Graph m_main_graph;
    int m_num_salesmen;
    
    int m_smo_iterations;
    int m_smo_population_size;
    int m_smo_local_limit;
    int m_smo_global_limit;
    double m_smo_pr;

    int m_aco_ants;
    int m_aco_iterations;
    double m_aco_alpha;
    double m_aco_beta;
    double m_aco_rho;
    double m_aco_Q;

    // Results
    std::vector<std::vector<int>> m_clusters; 
    std::vector<std::vector<int>> m_final_routes; 
    double m_total_length;
};

#endif 