#!/usr/bin/env python3
# File: visualize_benchmark.py

import glob
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def find_latest_benchmark():
    """Find the most recently created benchmark CSV file."""
    benchmark_files = glob.glob("benchmark_results/*.csv")
    if not benchmark_files:
        print("No benchmark results found. Run 'make run-benchmark' first.")
        sys.exit(1)

    # Get the most recently modified file
    latest_file = max(benchmark_files, key=os.path.getmtime)
    return latest_file


def visualize_benchmark(benchmark_file):
    """Create visualization of benchmark results."""
    # Read benchmark data
    try:
        df = pd.read_csv(benchmark_file)
    except Exception as e:
        print(f"Error reading benchmark file: {e}")
        sys.exit(1)

    # Create output directory if it doesn't exist
    os.makedirs("benchmark_plots", exist_ok=True)

    # Extract base filename
    base_filename = os.path.basename(benchmark_file).replace(".csv", "")

    # Create a plot comparing array size vs time
    plt.figure(figsize=(12, 6))

    # Plot time in milliseconds
    plt.plot(df["Size"], df["Time (ms)"], "b-o", linewidth=2, markersize=8)

    # Add best-fit line and determine complexity
    x = df["Size"].values
    y = df["Time (ms)"].values

    # Try different curve fits to determine complexity
    # O(n log n) - expected for heapsort
    def nlogn(x, a, b):
        return a * x * np.log(x) + b

    # O(n^2)
    def n_squared(x, a, b):
        return a * x**2 + b

    # O(n)
    def linear(x, a, b):
        return a * x + b

    from scipy.optimize import curve_fit

    try:
        # Fit O(n log n)
        popt_nlogn, _ = curve_fit(nlogn, x, y)
        y_nlogn = nlogn(x, *popt_nlogn)
        r2_nlogn = 1 - np.sum((y - y_nlogn) ** 2) / np.sum((y - np.mean(y)) ** 2)

        # Fit O(n^2)
        popt_n2, _ = curve_fit(n_squared, x, y)
        y_n2 = n_squared(x, *popt_n2)
        r2_n2 = 1 - np.sum((y - y_n2) ** 2) / np.sum((y - np.mean(y)) ** 2)

        # Fit O(n)
        popt_linear, _ = curve_fit(linear, x, y)
        y_linear = linear(x, *popt_linear)
        r2_linear = 1 - np.sum((y - y_linear) ** 2) / np.sum((y - np.mean(y)) ** 2)

        # Plot the best fitting model
        plt.plot(x, y_nlogn, "r--", label=f"O(n log n) fit, R²={r2_nlogn:.4f}")

        # Determine the most likely complexity
        complexities = {"O(n log n)": r2_nlogn, "O(n²)": r2_n2, "O(n)": r2_linear}

        best_complexity = max(complexities.items(), key=lambda x: x[1])
        complexity_text = (
            f"Best fit: {best_complexity[0]} (R²={best_complexity[1]:.4f})"
        )
    except:
        complexity_text = "Could not determine complexity"

    # Formatting the plot
    plt.title(f"HeapSort Performance Analysis\n{complexity_text}", fontsize=14)
    plt.xlabel("Array Size (n)", fontsize=12)
    plt.ylabel("Time (milliseconds)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.legend()

    # Add point annotations
    for i, (size, time) in enumerate(zip(df["Size"], df["Time (ms)"])):
        if i % 2 == 0:  # Only label every other point to avoid crowding
            plt.annotate(
                f"{time:.2f} ms",
                xy=(size, time),
                xytext=(0, 10),
                textcoords="offset points",
                ha="center",
                fontsize=8,
            )

    # Save the plot
    plot_filename = f"benchmark_plots/{base_filename}_plot.png"
    plt.tight_layout()
    plt.savefig(plot_filename, dpi=300)

    # Create logarithmic plot to verify O(n log n)
    plt.figure(figsize=(12, 6))
    plt.loglog(df["Size"], df["Time (ms)"], "g-o", linewidth=2, markersize=8)

    # Reference slopes
    sizes = np.array([min(x), max(x)])

    # O(n) reference
    plt.loglog(
        sizes,
        sizes * df["Time (ms)"].iloc[0] / df["Size"].iloc[0],
        "k--",
        label="O(n) reference",
    )

    # O(n log n) reference
    nlogn_ref = (
        sizes
        * np.log2(sizes)
        * df["Time (ms)"].iloc[0]
        / (df["Size"].iloc[0] * np.log2(df["Size"].iloc[0]))
    )
    plt.loglog(sizes, nlogn_ref, "r--", label="O(n log n) reference")

    # O(n²) reference
    n2_ref = sizes**2 * df["Time (ms)"].iloc[0] / df["Size"].iloc[0] ** 2
    plt.loglog(sizes, n2_ref, "b--", label="O(n²) reference")

    plt.title("HeapSort Performance (Log-Log Scale)", fontsize=14)
    plt.xlabel("Array Size (n)", fontsize=12)
    plt.ylabel("Time (milliseconds)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.legend()

    # Save the log-log plot
    log_plot_filename = f"benchmark_plots/{base_filename}_logplot.png"
    plt.tight_layout()
    plt.savefig(log_plot_filename, dpi=300)

    return plot_filename, log_plot_filename


def main():
    # Find the latest benchmark file
    if len(sys.argv) > 1:
        benchmark_file = sys.argv[1]
        if not os.path.exists(benchmark_file):
            print(f"Benchmark file not found: {benchmark_file}")
            sys.exit(1)
    else:
        benchmark_file = find_latest_benchmark()

    # Create visualizations
    plot_file, log_plot_file = visualize_benchmark(benchmark_file)

    print(f"Benchmark visualization complete.")
    print(f"Linear plot saved to: {plot_file}")
    print(f"Log-log plot saved to: {log_plot_file}")

    # Show the plots if running in an interactive environment
    try:
        plt.show()
    except:
        pass


if __name__ == "__main__":
    main()
