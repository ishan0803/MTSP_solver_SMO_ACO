# Hybrid m-TSP Solver (SMO + ACO)

![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Language](https://img.shields.io/badge/Python-3.8%2B-blue.svg)
![Framework](https://img.shields.io/badge/Streamlit-brightgreen.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

A high-performance C++ backend with an interactive Python Streamlit frontend for solving the Multi-Depot Traveling Salesman Problem (m-TSP) using a novel hybrid metaheuristic approach.

This project implements a "Cluster First, Route Second" strategy. It first uses the **Spider Monkey Optimization (SMO)** algorithm to find the optimal $m$ clusters (depots) for $n$ cities. Then, it solves the individual route for each cluster using **Ant Colony Optimization (ACO)**.

## 🚀 Core Features

* **Hybrid Metaheuristic:** Solves the m-TSP by combining Spider Monkey Optimization for clustering and Ant Colony Optimization for routing.
* **High-Performance Backend:** Core logic is written in C++17 for maximum speed and efficiency.
* **Python Interface:** Uses `pybind11` to expose the C++ solver to Python, allowing for easy integration.
* **Interactive Frontend:** A user-friendly web app built with Streamlit to visualize the problem and the solution.
* **Flexible Input:** Supports both random generation of city coordinates and uploading of custom city data via a `.txt` file.
* **Interactive Visualization:** Displays the final computed routes for all salesmen on an interactive Altair chart.

## ✨ The Novelty: SMO for Clustering

This project's novelty lies in its hybrid approach, specifically the use of **Spider Monkey Optimization (SMO)** for the clustering phase.

Most m-TSP solvers use simple, deterministic clustering algorithms like K-Means. While fast, K-Means is "greedy" and often gets stuck in a local optimum, leading to a suboptimal clustering of cities.

This project treats clustering *itself* as a complex optimization problem.
1.  **SMO (Clustering):** We use SMO, a powerful swarm intelligence algorithm, to search the vast solution space for the best possible set of $m$ cluster centroids. The "fitness" of a solution is measured by the total intra-cluster distance (Sum of Squared Errors). This results in more coherent and well-balanced clusters than K-Means.
2.  **ACO (Routing):** Once the cities are partitioned by SMO, the problem is reduced to $m$ independent TSP problems (one for each salesman). We then apply the Ant Colony Optimization (ACO) algorithm—which is famously effective for TSP—to find the shortest route within each cluster.

This `SMO -> ACO` pipeline provides a more robust and high-quality solution to the m-TSP than simpler hybrid models.

## 🔬 ACO vs. Dynamic Programming for TSP

The core of this project's "Route Second" step uses Ant Colony Optimization. Here’s how it compares to the classic exact algorithm for TSP: Dynamic Programming (DP), also known as the Held-Karp algorithm.

| Feature | Dynamic Programming (Held-Karp) | Ant Colony Optimization (ACO) |
| :--- | :--- | :--- |
| **Solution Quality** | **Guaranteed Optimal** | **Approximate / Near-Optimal** |
| **Time Complexity** | **Exponential: $O(n^2 \cdot 2^n)$** | **Polynomial (per iteration): $O(I \cdot A \cdot n^2)$** |
| **Space Complexity** | **Exponential: $O(n \cdot 2^n)$** | **Polynomial: $O(n^2)$** (to store pheromones) |
| **Scalability** | **Very Poor.** Only feasible for small $n$ (e.g., $n < 20$). | **Very Good.** Scales to hundreds or thousands of cities. |

*Where **n** is the number of cities, **I** is the number of iterations, and **A** is the number of ants.*

### Key Takeaway:

* **Dynamic Programming** is an *exact* algorithm. It will find the single best-undisputed answer, but the computational cost grows exponentially, making it impossible for real-world problems.
* **Ant Colony Optimization** is a *metaheuristic* algorithm. It uses a probabilistic swarm-intelligence approach to find a "good enough" solution. It does not guarantee the absolute best path, but it can find a near-optimal solution in a practical amount of time (polynomial complexity), making it the only viable choice for large-scale problems.

This project uses ACO because it is part of a larger hybrid solver designed for scalability. The goal is to get a high-quality solution for a large number of cities ($n$) and salesmen ($m$), where an exact DP approach would be computationally infeasible.

## 🛠️ Dependencies

### C++ Backend
* A C++17 compliant compiler (e.g., `g++`, `clang`, or MSVC)
* `CMake` (version 3.15 or higher)
* `pybind11` (The `CMakeLists.txt` is set up to fetch this automatically, but you can also install it via `pip install pybind11`)

### Python Frontend
All Python dependencies are listed in `requirements.txt`.
* `streamlit`
* `pandas`
* `numpy`
* `altair`

## 🏃 How to Run

This project includes a `build_and_run.sh` script that automates the entire setup and launch process for Linux and macOS.

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/ishan0803/MTSP_solver_SMO_ACO.git
    cd /MTSP_solver_SMO_ACO
    ```

2.  **Make the script executable:**
    ```bash
    chmod +x run.sh
    ```

3.  **Run the script:**
    ```bash
    ./run.sh
    ```
This single command will:
1.  Create a Python virtual environment (`venv`).
2.  Activate it.
3.  Install all `requirements.txt` dependencies.
4.  Create a `build` directory.
5.  Run `cmake` and `cmake --build` to compile the C++ module.
6.  Launch the Streamlit app.

<details>
<summary><b>Alternative: Manual Build Steps (All Platforms)</b></summary>

If you prefer to run the steps manually (e.g., on Windows).

1.  **Create and activate a Python virtual environment:**
    ```bash
    # On macOS/Linux
    python3 -m venv venv
    source venv/bin/activate
    
    # On Windows (cmd)
    python -m venv venv
    .\venv\Scripts\activate
    ```

2.  **Install Python requirements:**
    ```bash
    pip install -r requirements.txt
    ```

3.  **Configure the C++ build:**
    ```bash
    mkdir -p build
    cd build
    
    # On macOS/Linux
    cmake ..
    
    # On Windows (if you have Visual Studio Build Tools)
    cmake .. -G "Visual Studio 17 2022" 
    ```
    *If CMake can't find `pybind11`, you may need to help it:*
    `cmake .. -Dpybind11_DIR=$(python -m pybind11 --cmakedir)`

4.  **Build the C++ module:**
    ```bash
    cmake --build . --config Release
    ```
    This will create the Python module (e.g., `MTSP_SOLVER.cp310-win_amd64.pyd`) and place it in the correct directory.

5.  **Run the Streamlit app:**
    ```bash
    cd ..  # Go back to the root directory
    streamlit run streamlit_app.py
    ```
</details>

## 📱 Using the Application

1.  **Open the App:** Your browser should open to `http://localhost:8501`.
2.  **Set Parameters:** Use the sidebar to configure the problem.
    * **Number of Salesmen (m):** The number of clusters and routes to create.
    * **City Input Method:**
        * **Randomly Generate:** Choose a "Number of Cities (n)".
        * **Upload .txt File:** Provide a text file where each line is an `x,y` or `x y` coordinate.
3.  **Validation:** The app will enforce that the **Number of Cities (n)** must be *perfectly divisible* by the **Number of Salesmen (m)**.
4.  **Algorithm Parameters:** Tweak the SMO and ACO parameters (e.g., iterations, ants) or leave them as defaults for a good balance of speed and accuracy.
5.  **Run Solver:** Click the "Run Hybrid Solver" button.
6.  **View Results:** The main page will display:
    * The total time taken.
    * The total combined length of all routes.
    * An interactive plot showing all cities (color-coded by salesman) and the computed routes. You can zoom and pan this plot.

## 📄 License

This project is licensed under the MIT License.
