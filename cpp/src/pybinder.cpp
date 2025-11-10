#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "hybrid.hpp" 

namespace py = pybind11;

PYBIND11_MODULE(MTSP_SOLVER, m) {
    m.doc() = "Hybrid m-TSP solver using SMO clustering and ACO routing";
    py::class_<Hybrid>(m, "Hybrid")
        .def(py::init<const std::vector<std::pair<double,double>>&,
                    int, int, int, int, int, double,
                    int, int, double, double, double, double>(),
            py::arg("pts"),
            py::arg("num_salesmen"),
            py::arg("smo_iterations"),
            py::arg("smo_population_size") = 50,
            py::arg("smo_local_limit") = 20,
            py::arg("smo_global_limit") = 20,
            py::arg("smo_pr") = 0.1,
            py::arg("aco_ants"),
            py::arg("aco_iterations"),
            py::arg("aco_alpha") = 1.0,
            py::arg("aco_beta") = 5.0,
            py::arg("aco_rho") = 0.5,
            py::arg("aco_Q") = 100.0)
        
        .def("run", &Hybrid::run, 
             "Runs the full SMO clustering and ACO routing pipeline")
        
        .def("get_routes", &Hybrid::getRoutes, 
             "Returns a list of routes (one list per salesman)")
        
        .def("get_total_length", &Hybrid::getTotalLength, 
             "Returns the sum of all route lengths");
}
