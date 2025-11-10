#pragma once
#ifndef SMO_H
#define SMO_H

#include "graph.hpp"
#include <vector>
#include <utility>
#include <random>

class SMO {
public:
    SMO(int num_clusters, int iterations, const Graph& g,
        int population_size = 50, int local_leader_limit = 20,
        int global_leader_limit = 20, double pr = 0.1);

    void run();

    std::vector<std::vector<int>> getClusters() const;

private:
    int m_num_clusters;
    int m_iterations;
    const Graph& m_graph;
    int m_population_size;
    int m_local_leader_limit;
    int m_global_leader_limit;
    double m_pr;

    std::pair<double, double> m_x_bounds; 
    std::pair<double, double> m_y_bounds; 

    std::vector<std::vector<std::pair<double, double>>> m_population; // [pop_size][m_num_clusters]
    std::vector<double> m_fitness;      // [pop_size]

    std::vector<std::vector<std::pair<double, double>>> m_local_leaders;
    std::vector<double> m_local_leader_fitness;
    std::vector<int> m_local_leader_limit_count;

    std::vector<std::pair<double, double>> m_global_leader;
    double m_global_leader_fitness;
    int m_global_leader_limit_count;

    std::vector<int> m_group_id; // Group ID for each monkey
    int m_num_groups;

    std::mt19937 m_rng;

    void initialize();
    double calculateFitness(const std::vector<std::pair<double, double>>& position);
    double assignPointsToClusters(const std::vector<std::pair<double, double>>& position,
                                  std::vector<std::vector<int>>& clusters) const;

    // --- SMO Phases ---
    void localLeaderPhase();
    void globalLeaderPhase();
    void globalLeaderLearningPhase();
    void localLeaderLearningPhase();
    void localLeaderDecisionPhase();

    void clampCentroid(std::pair<double, double>& centroid);
};

#endif 