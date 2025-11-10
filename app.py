import streamlit as st
import pandas as pd
import numpy as np
import altair as alt
import random
import time
import build.MTSP_SOLVER as MTSP_SOLVER

st.set_page_config(
    page_title="Hybrid m-TSP Solver",
    page_icon="🗺️",
    layout="wide"
)

st.title("🗺️ Hybrid m-TSP Solver (SMO + ACO)")
st.caption("Using C++ backend with Python bindings")

def parse_txt_file(uploaded_file):
    """Parses an uploaded text file into a list of (x, y) tuples."""
    points = []
    try:
        string_data = uploaded_file.getvalue().decode("utf-8")
        lines = string_data.strip().split('\n')
        
        for line in lines:
            line = line.strip()
            if not line:
                continue
            if ',' in line:
                parts = line.split(',')
            else:
                parts = line.split()
            
            if len(parts) >= 2:
                points.append((float(parts[0]), float(parts[1])))
                
    except Exception as e:
        st.sidebar.error(f"Error parsing file: {e}")
        return None
        
    return points

with st.sidebar:
    st.header("1. Problem Setup")
    
    m_salesmen = st.number_input(
        "Number of Salesmen (m)", min_value=1, value=4
    )
    
    input_method = st.radio(
        "City Input Method",
        ("Randomly Generate", "Upload .txt File"),
        horizontal=True
    )
    
    points = []
    n_cities = 0
    
    if input_method == "Randomly Generate":
        n_cities = st.number_input(
            "Number of Cities (n)", min_value=m_salesmen, value=100, step=1
        )
        
    else:
        uploaded_file = st.file_uploader(
            "Upload a .txt file",
            type=["txt"],
            help="File should have one 'x y' or 'x,y' coordinate pair per line."
        )
        if uploaded_file:
            points = parse_txt_file(uploaded_file)
            if points:
                n_cities = len(points)
                st.info(f"Successfully loaded {n_cities} cities.")
            else:
                st.stop()
        else:
            st.info("Please upload a file to proceed.")
            st.stop()
            
    if n_cities % m_salesmen != 0:
        st.error(
            f"Error: Number of cities ({n_cities}) must be "
            f"divisible by the number of salesmen ({m_salesmen})."
        )
        st.info(f"Please change 'n' or 'm'. Closest valid 'n' is {n_cities - (n_cities % m_salesmen)} or {(n_cities + (m_salesmen - (n_cities % m_salesmen)))}.")
        st.stop()
    else:
        st.success(f"{n_cities} cities / {m_salesmen} salesmen = {n_cities // m_salesmen} cities per salesman.")

    st.header("2. Algorithm Parameters")
    
    col1, col2 = st.columns(2)
    with col1:
        smo_iters = st.number_input("SMO Iterations", min_value=1, value=100)
        aco_ants = st.number_input("Ants per Cluster", min_value=1, value=20)
    with col2:
        aco_iters = st.number_input("ACO Iterations", min_value=1, value=200)

    with st.expander("Advanced ACO/SMO Parameters"):
        smo_pop = st.number_input("SMO Population", min_value=10, value=50)
        smo_ll = st.number_input("SMO Local Limit", min_value=1, value=20)
        smo_gl = st.number_input("SMO Global Limit", min_value=1, value=20)
        smo_pr = st.slider("SMO Perturbation Rate (pr)", 0.0, 1.0, 0.1)
        
        aco_alpha = st.slider("ACO Alpha (τ weight)", 0.1, 10.0, 1.0, 0.1)
        aco_beta = st.slider("ACO Beta (η weight)", 0.1, 10.0, 5.0, 0.1)
        aco_rho = st.slider("ACO Rho (Evaporation)", 0.01, 1.0, 0.5, 0.01)
        aco_Q = st.number_input("ACO Q (Pheromone)", min_value=1, value=100)

    run_button = st.button(
        "Run Hybrid Solver", type="primary", use_container_width=True
    )


if run_button:
    if input_method == "Randomly Generate":
        points = [
            (random.uniform(0, 1000), random.uniform(0, 1000)) 
            for _ in range(n_cities)
        ]
        
    if not points:
        st.error("No points to process. Upload a file or select 'Randomly Generate'.")
        st.stop()

    st.info("Solver is running... (This may take a moment)")
    with st.spinner("Clustering with SMO and routing with ACO..."):
        start_time = time.time()
        
        try:
            # Initialize the Hybrid solver
            solver = MTSP_SOLVER.Hybrid(
                pts=points,
                num_salesmen=m_salesmen,
                
                # SMO parameters
                smo_iterations=smo_iters,
                smo_population_size=smo_pop,
                smo_local_limit=smo_ll,
                smo_global_limit=smo_gl,
                smo_pr=smo_pr,
                
                # ACO parameters
                aco_ants=aco_ants,
                aco_iterations=aco_iters,
                aco_alpha=aco_alpha,
                aco_beta=aco_beta,
                aco_rho=aco_rho,
                aco_Q=aco_Q
            )
            
            # Run the solver
            solver.run() # This will print C++ cout logs to your console
            
            # Get the results
            total_length = solver.get_total_length()
            routes = solver.get_routes()
            
            end_time = time.time()
            
        except Exception as e:
            st.error(f"An error occurred while running the solver: {e}")
            st.stop()
            
    st.success(f"Solver finished in {end_time - start_time:.2f} seconds.")

    city_df = pd.DataFrame(points, columns=['x', 'y'])
    city_df['city_id'] = city_df.index
    city_df['cluster_id'] = -1  
    
    route_data = []
    
    for salesman_id, route in enumerate(routes):
        if not route:
            continue
            
        cluster_name = f"Salesman {salesman_id + 1}"
        
        city_df.loc[city_df['city_id'].isin(route), 'cluster_id'] = cluster_name
        
        for route_order, city_id in enumerate(route):
            route_data.append({
                "salesman": cluster_name,
                "route_order": route_order,
                "city_id": city_id
            })

    route_df = pd.DataFrame(route_data)
    
    route_df = route_df.merge(
        city_df, on='city_id', how='left', 
        suffixes=('_route', '_city')
    )

    st.header("Solver Results", divider="blue")
    st.metric("Total Combined Route Length", f"{total_length:,.2f}")
    
    base_points = alt.Chart(city_df).mark_circle(size=60, opacity=0.8).encode(
        x=alt.X('x', title='X Coordinate', axis=alt.Axis(grid=False)),
        y=alt.Y('y', title='Y Coordinate', axis=alt.Axis(grid=False)),
        color=alt.Color('cluster_id:N', title="Assigned Salesman"),
        tooltip=[
            alt.Tooltip('city_id', title="City ID"), 
            alt.Tooltip('cluster_id:N', title="Salesman"), 
            'x', 
            'y'
        ]
    )
    
    route_lines = alt.Chart(route_df).mark_line(point=True, opacity=0.9).encode(
        x='x',
        y='y',
        color=alt.Color('salesman:N', title="Route"),
        order='route_order', 
        tooltip=[
            alt.Tooltip('salesman', title="Salesman"),
            alt.Tooltip('route_order', title="Stop #"),
            alt.Tooltip('city_id', title="City ID"), 
            'x', 
            'y'
        ]
    )

    final_chart = (base_points + route_lines).interactive() 
    
    st.altair_chart(final_chart, use_container_width=True)
    
    with st.expander("Show Raw Route Data"):
        for i, route in enumerate(routes):
            st.text(f"Salesman {i+1}: {route}")