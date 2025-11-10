#include "hybrid.hpp"
#include <iostream>

Hybrid::Hybrid(const std::vector<std::pair<double,double>>& pts,
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
               double aco_Q)
    : m_main_graph(pts),
      m_num_salesmen(num_salesmen),
      m_smo_iterations(smo_iterations),
      m_smo_population_size(smo_population_size),
      m_smo_local_limit(smo_local_limit),
      m_smo_global_limit(smo_global_limit),
      m_smo_pr(smo_pr),
      m_aco_ants(aco_ants),
      m_aco_iterations(aco_iterations),
      m_aco_alpha(aco_alpha),
      m_aco_beta(aco_beta),
      m_aco_rho(aco_rho),
      m_aco_Q(aco_Q),
      m_total_length(0.0)
{
}

void Hybrid::run() {
    // 1. Create SMO and get clusters
    std::cout << "Starting SMO clustering..." << std::endl;
    SMO smo(m_num_salesmen, m_smo_iterations, m_main_graph,
            m_smo_population_size, m_smo_local_limit, 
            m_smo_global_limit, m_smo_pr);
            
    smo.run();
    m_clusters = smo.getClusters();
    std::cout << "Clustering complete." << std::endl;

    m_final_routes.clear();
    m_total_length = 0.0;
    const auto& all_points = m_main_graph.getPoints();

    // 2. Loop through each cluster and run ACO
    for (int i = 0; i < m_clusters.size(); ++i) {
        const auto& cluster_indices = m_clusters[i];
        
        if (cluster_indices.empty()) {
            std::cout << "Warning: Cluster " << i << " is empty. Skipping." << std::endl;
            m_final_routes.push_back({}); 
            continue;
        }
        
        std::cout << "--- Solving route for cluster " << i << " (size " << cluster_indices.size() << ") ---" << std::endl;

        // 3. Create a new set of points for this cluster
        std::vector<std::pair<double, double>> cluster_points;
        cluster_points.reserve(cluster_indices.size());
        for (int original_index : cluster_indices) {
            cluster_points.push_back(all_points[original_index]);
        }

        // 4. Create and run ACO on the cluster-specific points
        ACO aco(cluster_points,
                m_aco_ants,
                m_aco_alpha,
                m_aco_beta,
                m_aco_rho,
                m_aco_Q);
                
        aco.run(m_aco_iterations);

        // 5. Get the local route and translate it back to original indices
        std::vector<int> local_route = aco.final_route();
        std::vector<int> global_route;
        global_route.reserve(local_route.size());

        for (int local_index : local_route) {
            global_route.push_back(cluster_indices[local_index]);
        }

        m_final_routes.push_back(global_route);
        m_total_length += aco.best_distance();
        std::cout << "--- Cluster " << i << " complete. Best distance: " << aco.best_distance() << " ---" << std::endl;
    }
    std::cout << "=============================================" << std::endl;
    std::cout << "All routes solved. Total combined length: " << m_total_length << std::endl;
    std::cout << "=============================================" << std::endl;
}

std::vector<std::vector<int>> Hybrid::getRoutes() const {
    return m_final_routes;
}

double Hybrid::getTotalLength() const {
    return m_total_length;
}