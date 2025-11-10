#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- 1. Set up Python Environment ---
echo "Creating Python virtual environment..."
python3 -m venv venv

echo "Activating virtual environment..."
source venv/bin/activate

echo "Installing Python requirements..."
pip install -r requirements.txt

# --- 2. Build C++ Module ---
echo "Creating build directory..."
mkdir -p build

echo "Entering build directory..."
cd build

echo "Running CMake configuration..."
# This assumes your CMakeLists.txt is in the parent directory
cmake ..

echo "Building C++ module in Release mode..."
cmake --build . --config Release

# --- 3. Run Application ---
echo "Returning to root directory..."
cd ..

echo "Launching Streamlit application..."
streamlit run app.py