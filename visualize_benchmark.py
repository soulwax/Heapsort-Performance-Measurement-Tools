#!/usr/bin/env python3
# File: visualize_benchmark.py

import glob
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.optimize import curve_fit


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

    # Create plots
    create_sort_time_plot(df, base_filename)
    create_loglog_plot(df, base_filename)
    create_complexity_analysis(df, base_filename)

    return [
        f"benchmark_plots/{base_filename}_sort_time.png",
        f"benchmark_plots/{base_filename}_loglog.png",
        f"benchmark_plots/{base_filename}_complexity.png",
    ]


def create_sort_time_plot(df, base_filename):
    """Create a linear plot of sort time vs array size."""
    plt.figure(figsize=(12, 6))

    # Plot time in milliseconds
    plt.plot(df["Size"], df["Time (ms)"], "b-o", linewidth=2, markersize=8)

    # Formatting the plot
    plt.title("HeapSort Algorithm Performance", fontsize=16)
    plt.xlabel("Array Size (n)", fontsize=12)
    plt.ylabel("Sorting Time (milliseconds)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)

    # Add point annotations
    step = max(1, len(df) // 10)  # Only label about 10 points to avoid crowding
    for i, (size, time) in enumerate(zip(df["Size"], df["Time (ms)"])):
        if i % step == 0:
            plt.annotate(
                f"{time:.2f} ms",
                xy=(size, time),
                xytext=(0, 10),
                textcoords="offset points",
                ha="center",
                fontsize=8,
            )

    # Save the plot
    plot_filename = f"benchmark_plots/{base_filename}_sort_time.png"
    plt.tight_layout()
    plt.savefig(plot_filename, dpi=300)
    plt.close()


def create_loglog_plot(df, base_filename):
    """Create a log-log plot to help visualize algorithmic complexity."""
    plt.figure(figsize=(12, 6))

    # Plot data on log-log scale
    plt.loglog(df["Size"], df["Time (ms)"], "g-o", linewidth=2, markersize=8)

    # Reference slopes
    sizes = np.array([min(df["Size"]), max(df["Size"])])

    # O(n) reference
    ref_factor = df["Time (ms)"].iloc[0] / df["Size"].iloc[0]
    plt.loglog(sizes, sizes * ref_factor, "k--", label="O(n) reference")

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

    plt.title("HeapSort Performance (Log-Log Scale)", fontsize=16)
    plt.xlabel("Array Size (n)", fontsize=12)
    plt.ylabel("Sorting Time (milliseconds)", fontsize=12)
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend(fontsize=10)

    # Save the log-log plot
    log_plot_filename = f"benchmark_plots/{base_filename}_loglog.png"
    plt.tight_layout()
    plt.savefig(log_plot_filename, dpi=300)
    plt.close()


def create_complexity_analysis(df, base_filename):
    """Create a plot with curve fitting to determine algorithmic complexity."""
    plt.figure(figsize=(12, 6))

    x = df["Size"].values
    y = df["Time (ms)"].values

    # Define curve fitting functions for different complexities
    def linear(x, a, b):
        return a * x + b

    def nlogn(x, a, b):
        return a * x * np.log(x) + b

    def quadratic(x, a, b):
        return a * x**2 + b

    # Perform curve fitting
    try:
        popt_linear, _ = curve_fit(linear, x, y)
        y_linear = linear(x, *popt_linear)
        r2_linear = 1 - np.sum((y - y_linear) ** 2) / np.sum((y - np.mean(y)) ** 2)

        popt_nlogn, _ = curve_fit(nlogn, x, y)
        y_nlogn = nlogn(x, *popt_nlogn)
        r2_nlogn = 1 - np.sum((y - y_nlogn) ** 2) / np.sum((y - np.mean(y)) ** 2)

        popt_quadratic, _ = curve_fit(quadratic, x, y)
        y_quadratic = quadratic(x, *popt_quadratic)
        r2_quadratic = 1 - np.sum((y - y_quadratic) ** 2) / np.sum(
            (y - np.mean(y)) ** 2
        )

        # Plot the data and fitted curves
        plt.scatter(x, y, color="blue", label="Measurement data")
        plt.plot(x, y_linear, "g--", label=f"O(n) fit, R²={r2_linear:.4f}")
        plt.plot(
            x, y_nlogn, "r-", linewidth=2, label=f"O(n log n) fit, R²={r2_nlogn:.4f}"
        )
        plt.plot(x, y_quadratic, "b--", label=f"O(n²) fit, R²={r2_quadratic:.4f}")

        # Determine best fit
        complexities = {
            "O(n)": r2_linear,
            "O(n log n)": r2_nlogn,
            "O(n²)": r2_quadratic,
        }

        best_fit = max(complexities.items(), key=lambda x: x[1])

        plt.title(
            f"HeapSort Algorithm Complexity Analysis\nBest fit: {best_fit[0]} (R²={best_fit[1]:.4f})",
            fontsize=16,
        )

    except Exception as e:
        plt.scatter(x, y, color="blue", label="Measurement data")
        plt.title(
            "HeapSort Algorithm Complexity Analysis\nCould not perform curve fitting",
            fontsize=16,
        )
        print(f"Error during curve fitting: {e}")

    plt.xlabel("Array Size (n)", fontsize=12)
    plt.ylabel("Sorting Time (milliseconds)", fontsize=12)
    plt.grid(True, linestyle="--", alpha=0.7)
    plt.legend(fontsize=10)

    # Save the complexity analysis plot
    complexity_filename = f"benchmark_plots/{base_filename}_complexity.png"
    plt.tight_layout()
    plt.savefig(complexity_filename, dpi=300)
    plt.close()


def main():
    # Find the latest benchmark file or use provided file
    if len(sys.argv) > 1:
        benchmark_file = sys.argv[1]
        if not os.path.exists(benchmark_file):
            print(f"Benchmark file not found: {benchmark_file}")
            sys.exit(1)
    else:
        benchmark_file = find_latest_benchmark()

    # Create visualizations
    plot_files = visualize_benchmark(benchmark_file)

    print(f"\nBenchmark visualization complete!")
    print(f"The following visualization files were created:")
    for plot_file in plot_files:
        print(f"- {plot_file}")

    # Additional summary of the benchmark data
    try:
        df = pd.read_csv(benchmark_file)
        min_size = df["Size"].min()
        max_size = df["Size"].max()
        avg_time_ms = df["Time (ms)"].mean()
        max_time_ms = df["Time (ms)"].max()

        print(f"\nBenchmark Summary:")
        print(f"- Array sizes: {min_size} to {max_size}")
        print(f"- Average sorting time: {avg_time_ms:.2f} ms")
        print(f"- Maximum sorting time: {max_time_ms:.2f} ms")
        print(f"- Data points: {len(df)}")

    except Exception as e:
        print(f"Error generating summary: {e}")


if __name__ == "__main__":
    main()