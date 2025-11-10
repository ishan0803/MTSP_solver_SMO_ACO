#include "aco.hpp"
#include <iostream>
#include <cmath>
#include <limits>
#include <random>

ACO::ACO(const std::vector<std::pair<double,double>>& pts, int ants,
        double alpha, double beta, double rho, double Q) :
        graph(pts), num_ants(ants), alpha(alpha), beta(beta), rho(rho), Q(Q)       
{
    num_cities = pts.size();
    rng.seed(std::random_device{}());
    double L_nn = graph.nearest_neighbor_tour_length();
    double tau_0 = 1.0 / (num_cities * L_nn);
    pher_mat.assign(num_cities, std::vector<double>(num_cities, tau_0));
    
    tours.assign(num_ants, std::vector<int>(num_cities, -1));
    tour_length.assign(num_ants, std::numeric_limits<double>::max());
    best_tour.assign(num_cities, -1);
    best_length = std::numeric_limits<double>::max();
}

int ACO::select_next_city(int ant_idx, int current_city, const std::vector<bool>& visited){
    std::vector<double> prob(num_cities, 0.0);
    double sum = 0.0;

    for (int j = 0; j < num_cities; ++j) {
        if (!visited[j] && j != current_city) {
            double tau = pher_mat[current_city][j];
            double eta = 1 / graph.getDistance(current_city, j);
            double val = pow(tau, alpha) * pow(eta, beta);
            prob[j] = val;
            sum += val;
        }
    }

    if (sum == 0.0) {
        std::uniform_int_distribution<int> dist(0, num_cities - 1);
        int j;
        do { j = dist(rng); } while (visited[j]);
        return j;
    }

    std::uniform_real_distribution<double> dist(0.0, sum);
    double r = dist(rng);

    double cumulative = 0.0;
    for (int j = 0; j < num_cities; ++j) {
        if (!visited[j] && j != current_city) {
            cumulative += prob[j];
            if (r <= cumulative)
                return j;
        }
    }

    return 0;
}

void ACO::construct_tour(int ant_idx){
    std::vector<bool> visited(num_cities, false);
    std::uniform_int_distribution<int> start_dist(0, num_cities - 1);
    int current = start_dist(rng);

    tours[ant_idx][0] = current;
    visited[current] = true;

    for (int step = 1; step < num_cities; ++step) {
        int next = select_next_city(ant_idx, current, visited);
        tours[ant_idx][step] = next;
        visited[next] = true;
        current = next;
    }

    double total_len = 0.0;
    for (int i = 0; i < num_cities - 1; ++i)
        total_len += graph.getDistance(tours[ant_idx][i], tours[ant_idx][i+1]);
    total_len += graph.getDistance(tours[ant_idx].back(), tours[ant_idx][0]);

    tour_length[ant_idx] = total_len;

    if (total_len < best_length) {
        best_length = total_len;
        best_tour = tours[ant_idx];
    }
}

void ACO::evaporate_pher(){
    for(int i = 0; i < num_cities; i++){
        for(int j = 0; j < num_cities; j++){
            pher_mat[i][j] *= (1 - rho);
        }
    }
}

void ACO::deposit_pher(const std::vector<int>& path, double length){
    int n = path.size();
    for(int i = 0; i < n-1; i++){
        pher_mat[path[i]][path[i + 1]] += Q / length;
        pher_mat[path[i + 1]][path[i]] += Q / length;
    }
    pher_mat[path[n - 1]][path[0]] += Q / length;
    pher_mat[path[0]][path[n - 1]] += Q / length;
}

void ACO::update_pher(){
    evaporate_pher();
    for(int i = 0; i < num_ants; i++){
        deposit_pher(tours[i], tour_length[i]);
    }
}          

void ACO::run(int iterations){
    double prev_best = std::numeric_limits<double>::max();
    double curr_best = std::numeric_limits<double>::max();
    for(int i = 0; i < iterations; i++){
        for(int j = 0; j < num_ants; j++){
            construct_tour(j);
        }
        update_pher();
        if (i % 100 == 0 || i == iterations - 1){
            prev_best = curr_best;
            curr_best = best_length;
            std::cout << "Iteration " << i << " best length: " << best_length << "\n";
            if(prev_best == curr_best) break;
        }
    }
}

std::vector<int> ACO::final_route() const{
    std::vector<int> route;
    route.reserve(num_cities + 1);

    auto it = std::find(best_tour.begin(), best_tour.end(), 0);
    int idx = std::distance(best_tour.begin(), it);

    for (int i = 0; i < num_cities; ++i) {
        route.push_back(best_tour[(idx + i) % num_cities]);
    }
    route.push_back(0);

    return route;
}

double ACO::best_distance() const{
    return best_length;
}