#include "smo.hpp"
#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm> 

SMO::SMO(int num_clusters, int iterations, const Graph& g,
         int population_size, int local_leader_limit,
         int global_leader_limit, double pr)
    : m_num_clusters(num_clusters),
      m_iterations(iterations),
      m_graph(g),
      m_population_size(population_size),
      m_local_leader_limit(local_leader_limit),
      m_global_leader_limit(global_leader_limit),
      m_pr(pr),
      m_global_leader_fitness(std::numeric_limits<double>::max()),
      m_global_leader_limit_count(0),
      m_num_groups(1),
      m_rng(std::random_device{}()) 
{
}

void SMO::initialize() {
    // Find graph bounds to initialize positions
    m_x_bounds = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};
    m_y_bounds = {std::numeric_limits<double>::max(), std::numeric_limits<double>::lowest()};
    for (const auto& pt : m_graph.getPoints()) {
        if (pt.first < m_x_bounds.first) m_x_bounds.first = pt.first;
        if (pt.first > m_x_bounds.second) m_x_bounds.second = pt.first;
        if (pt.second < m_y_bounds.first) m_y_bounds.first = pt.second;
        if (pt.second > m_y_bounds.second) m_y_bounds.second = pt.second;
    }

    // Initialize distributions
    std::uniform_real_distribution<double> dist_x(m_x_bounds.first, m_x_bounds.second);
    std::uniform_real_distribution<double> dist_y(m_y_bounds.first, m_y_bounds.second);

    // Initialize population
    m_population.assign(m_population_size, std::vector<std::pair<double, double>>(m_num_clusters));
    m_fitness.assign(m_population_size, std::numeric_limits<double>::max());
    m_group_id.assign(m_population_size, 0); 

    // Local leaders
    m_local_leaders.assign(1, std::vector<std::pair<double, double>>(m_num_clusters)); 
    m_local_leader_fitness.assign(1, std::numeric_limits<double>::max());
    m_local_leader_limit_count.assign(1, 0);
    m_global_leader.resize(m_num_clusters); 

    for (int i = 0; i < m_population_size; ++i) {
        for (int j = 0; j < m_num_clusters; ++j) {
            m_population[i][j].first = dist_x(m_rng);     
            m_population[i][j].second = dist_y(m_rng);    
        }
        m_fitness[i] = calculateFitness(m_population[i]);

        // Update Global Leader
        if (m_fitness[i] < m_global_leader_fitness) {
            m_global_leader_fitness = m_fitness[i];
            m_global_leader = m_population[i];
        }
    }
    
    // Set initial local leader (just copy global leader for group 0)
    m_local_leaders[0] = m_global_leader;
    m_local_leader_fitness[0] = m_global_leader_fitness;
}

double SMO::calculateFitness(const std::vector<std::pair<double, double>>& position) {
    std::vector<std::vector<int>> clusters; // Dummy variable
    return assignPointsToClusters(position, clusters);
}

double SMO::assignPointsToClusters(const std::vector<std::pair<double, double>>& position,
                                   std::vector<std::vector<int>>& clusters) const {
    clusters.assign(m_num_clusters, std::vector<int>());
    double total_sse = 0.0;
    const auto& points = m_graph.getPoints();

    for (int i = 0; i < points.size(); ++i) {
        double min_dist_sq = std::numeric_limits<double>::max();
        int best_cluster = 0;

        for (int j = 0; j < m_num_clusters; ++j) {
            double dx = points[i].first - position[j].first;
            double dy = points[i].second - position[j].second;
            double dist_sq = dx * dx + dy * dy;

            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                best_cluster = j;
            }
        }
        clusters[best_cluster].push_back(i); // Store original index
        total_sse += min_dist_sq;
    }
    return total_sse;
}

void SMO::clampCentroid(std::pair<double, double>& centroid) {
    centroid.first = std::max(m_x_bounds.first, std::min(m_x_bounds.second, centroid.first));
    centroid.second = std::max(m_y_bounds.first, std::min(m_y_bounds.second, centroid.second));
}

void SMO::run() {
    initialize();
    
    std::cout << "SMO Starting. Initial Best Fitness (SSE): " << m_global_leader_fitness << std::endl;

    for (int iter = 0; iter < m_iterations; ++iter) {
        localLeaderPhase();
        globalLeaderPhase();
        globalLeaderLearningPhase(); // Check if global leader is stagnant
        localLeaderLearningPhase();  // Check if local leaders are stagnant
        localLeaderDecisionPhase();  // Re-group if necessary
        
        if(iter % 20 == 0 || iter == m_iterations - 1) {
            std::cout << "SMO Iter " << iter << " | Groups: " << m_num_groups 
                      << " | Best Fitness (SSE): " << m_global_leader_fitness << std::endl;
        }
    }
    std::cout << "SMO Finished. Final Best Fitness (SSE): " << m_global_leader_fitness << std::endl;
}

void SMO::localLeaderPhase() {
    std::uniform_real_distribution<double> rand_01(0.0, 1.0);
    
    for (int i = 0; i < m_population_size; ++i) {
        int group = m_group_id[i];
        std::vector<std::pair<double, double>> new_pos = m_population[i];
        
        for (int j = 0; j < m_num_clusters; ++j) { // Iterate over each centroid
            double r = rand_01(m_rng);
            if (r >= m_pr) {
                // Move towards local leader
                new_pos[j].first += rand_01(m_rng) * (m_local_leaders[group][j].first - new_pos[j].first);
                new_pos[j].second += rand_01(m_rng) * (m_local_leaders[group][j].second - new_pos[j].second);

                // Move towards random monkey in same group
                int k;
                do { k = std::uniform_int_distribution<int>(0, m_population_size - 1)(m_rng); } 
                while (m_group_id[k] != group || k == i);
                
                new_pos[j].first += (rand_01(m_rng) * 2.0 - 1.0) * (m_population[k][j].first - new_pos[j].first);
                new_pos[j].second += (rand_01(m_rng) * 2.0 - 1.0) * (m_population[k][j].second - new_pos[j].second);
            }
            
            clampCentroid(new_pos[j]);
        }
        
        double new_fitness = calculateFitness(new_pos);
        if (new_fitness < m_fitness[i]) {
            m_population[i] = new_pos;
            m_fitness[i] = new_fitness;
        }
    }
}

void SMO::globalLeaderPhase() {
    std::vector<double> prob(m_population_size, 0.0);
    double max_fit = -1.0;
    for(double f : m_fitness) {
        if (f != std::numeric_limits<double>::max() && f > max_fit) {
            max_fit = f;
        }
    }
    if (max_fit <= 0.0) max_fit = 1.0; // Avoid division by zero if all fitnesses are bad/equal

    double sum_fit = 0.0;
    for (int i = 0; i < m_population_size; ++i) {
        // We invert fitness because lower SSE is better (higher probability)
        prob[i] = 0.9 * ((max_fit - m_fitness[i]) / max_fit) + 0.1; 
        sum_fit += prob[i];
    }
    
    std::uniform_real_distribution<double> rand_01(0.0, 1.0);
    
    for (int i = 0; i < m_population_size; ++i) {
        // Roulette wheel selection
        double r = rand_01(m_rng) * sum_fit;
        double cumulative = 0.0;
        int selected_monkey = -1;
        for(int k=0; k<m_population_size; ++k) {
            cumulative += prob[k];
            if(r <= cumulative) {
                selected_monkey = k;
                break;
            }
        }
        if(selected_monkey == -1) selected_monkey = i; // Fallback
        
        std::vector<std::pair<double, double>> new_pos = m_population[i];

        for (int j = 0; j < m_num_clusters; ++j) {
            double r_pr = rand_01(m_rng);
            if (r_pr >= m_pr) {
                // Move towards global leader
                new_pos[j].first += rand_01(m_rng) * (m_global_leader[j].first - new_pos[j].first);
                new_pos[j].second += rand_01(m_rng) * (m_global_leader[j].second - new_pos[j].second);

                // Move towards selected monkey
                new_pos[j].first += (rand_01(m_rng) * 2.0 - 1.0) * (m_population[selected_monkey][j].first - new_pos[j].first);
                new_pos[j].second += (rand_01(m_rng) * 2.0 - 1.0) * (m_population[selected_monkey][j].second - new_pos[j].second);
            }

            clampCentroid(new_pos[j]);
        }

        double new_fitness = calculateFitness(new_pos);
        if (new_fitness < m_fitness[i]) {
            m_population[i] = new_pos;
            m_fitness[i] = new_fitness;
        }
    }
}

void SMO::globalLeaderLearningPhase() {
    // Update local leaders
    std::fill(m_local_leader_fitness.begin(), m_local_leader_fitness.end(), std::numeric_limits<double>::max());
    
    for (int i = 0; i < m_population_size; ++i) {
        int group = m_group_id[i];
        if (m_fitness[i] < m_local_leader_fitness[group]) {
            m_local_leader_fitness[group] = m_fitness[i];
            m_local_leaders[group] = m_population[i];
        }
    }

    // Update global leader
    double best_local_fitness = m_local_leader_fitness[0];
    int best_local_leader_idx = 0;
    for (int g = 1; g < m_num_groups; ++g) {
        if (m_local_leader_fitness[g] < best_local_fitness) {
            best_local_fitness = m_local_leader_fitness[g];
            best_local_leader_idx = g;
        }
    }
    
    if (best_local_fitness < m_global_leader_fitness) {
        m_global_leader_fitness = best_local_fitness;
        m_global_leader = m_local_leaders[best_local_leader_idx];
        m_global_leader_limit_count = 0; // Reset count
    } else {
        m_global_leader_limit_count++; // Increment count
    }
}

void SMO::localLeaderLearningPhase() {
    for(int g = 0; g < m_num_groups; ++g) {
        bool local_leader_updated = false;
        // Check if any monkey in the group improved the local leader
        for(int i=0; i<m_population_size; ++i) {
            if(m_group_id[i] == g && m_fitness[i] < m_local_leader_fitness[g]) {
                local_leader_updated = true;
                break;
            }
        }
        
        if(local_leader_updated) {
            m_local_leader_limit_count[g] = 0;
        } else {
            m_local_leader_limit_count[g]++;
        }
    }
}

void SMO::localLeaderDecisionPhase() {
    if (m_global_leader_limit_count > m_global_leader_limit) {
        // Global leader is stagnant, split into groups
        m_global_leader_limit_count = 0;
        
        // Limit max groups (e.g., population / 5)
        if (m_num_groups < std::max(1, m_population_size / 5)) { 
            m_num_groups++;
            // Resize leader vectors
            m_local_leaders.resize(m_num_groups, std::vector<std::pair<double, double>>(m_num_clusters));
            m_local_leader_fitness.resize(m_num_groups, std::numeric_limits<double>::max());
            m_local_leader_limit_count.resize(m_num_groups, 0);

            // Re-assign monkeys to groups (simple split)
            int group_size = m_population_size / m_num_groups;
            for (int i = 0; i < m_population_size; ++i) {
                m_group_id[i] = std::min(i / group_size, m_num_groups - 1);
            }
        } else {
             // Max groups reached, merge all back to one
             m_num_groups = 1;
             m_local_leaders.resize(1, m_global_leader);
             m_local_leader_fitness.resize(1, m_global_leader_fitness);
             m_local_leader_limit_count.resize(1, 0);
             std::fill(m_group_id.begin(), m_group_id.end(), 0);
        }
    }

    // Check individual local leaders
    std::uniform_real_distribution<double> rand_01(0.0, 1.0);

    for (int g = 0; g < m_num_groups; ++g) {
        if (m_local_leader_limit_count[g] > m_local_leader_limit) {
            m_local_leader_limit_count[g] = 0;
            for (int i = 0; i < m_population_size; ++i) {
                if (m_group_id[i] == g) {
                    // Re-initialize or perturb
                    for (int j = 0; j < m_num_clusters; ++j) {
                        m_population[i][j].first = m_global_leader[j].first + rand_01(m_rng) * (m_local_leaders[g][j].first - m_population[i][j].first);
                        m_population[i][j].second = m_global_leader[j].second + rand_01(m_rng) * (m_local_leaders[g][j].second - m_population[i][j].second);
                        clampCentroid(m_population[i][j]);
                    }
                    m_fitness[i] = calculateFitness(m_population[i]);
                }
            }
        }
    }
}

std::vector<std::vector<int>> SMO::getClusters() const {
    std::vector<std::vector<int>> final_clusters;
    // Assign all points one last time based on the best-ever solution
    assignPointsToClusters(m_global_leader, final_clusters);
    return final_clusters;
}