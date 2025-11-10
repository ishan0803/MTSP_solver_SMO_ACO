#include "graph.hpp"
#include <cmath>

Graph::Graph(const std::vector<std::pair<double,double>>& pts) : Points(pts){
    computeDistanceMatrix();
}

void Graph::computeDistanceMatrix(){
    int n = Points.size();
    dist.assign(n, std::vector<double>(n));
    for(int i = 0; i < n; i++){
        for(int j = 0; j < n; j++){
            int dx = Points[i].first - Points[j].first;
            int dy = Points[i].second - Points[j].second;
            dist[i][j] = std::sqrt(dx*dx + dy*dy);  
        }
    }
}

double Graph::getDistance(int i, int j) const {
    return dist[i][j];
}

int Graph::size() const {
    return Points.size();
}

const std::vector<std::pair<double,double>>& Graph::getPoints() const {
    return Points;
}

const std::vector<std::vector<double>>& Graph::getDist() const {
    return dist;
}

const double Graph::nearest_neighbor_tour_length() const {
    int n = Points.size();
    std::vector<bool> visited(n, false);
    double best_length = 0.0;
    int current = 0;
    visited[current] = true;

    for (int step = 1; step < n; ++step) {
        double best = 1e9;
        int next = -1;
        for (int j = 0; j < n; ++j) {
            if (!visited[j] && j != current) {
                double d = getDistance(current, j);
                if (d < best) {
                    best = d;
                    next = j;
                }
            }
        }
        best_length += best;
        visited[next] = true;
        current = next;
    }
    best_length += getDistance(current, 0);  
    return best_length;
}
