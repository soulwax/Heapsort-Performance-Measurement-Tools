# File: visualize_benchmark.py

import argparse
import glob
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def find_latest_benchmark_file():
    """Find the most recent benchmark result file."""
    files = glob.glob("benchmark_results/*.csv")
    if not files:
        print("No benchmark result files found in 'benchmark_results/' directory.")
        sys.exit(1)

    latest_file = max(files, key=os.path.getmtime)
    print(f"Using latest benchmark file: {latest_file}")
    return latest_file


def plot_heapsort_benchmark(csv_file, output_dir="benchmark_plots"):
    """Plot the results from a heapsort benchmark CSV file."""
    if not os.path.exists(csv_file):
        print(f"File not found: {csv_file}")
        return False

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    # Load the data
    df = pd.read_csv(csv_file)

    # Extract base filename for plot titles and output filenames
    base_filename = os.path.basename(csv_file).replace(".csv", "")

    # Figure 1: Sort time vs. array size
    plt.figure(figsize=(10, 6))
    plt.plot(df["Size"], df["Time (ms)"], marker="o", linestyle="-", color="blue")
    plt.title("Sorting Time vs. Array Size")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time (milliseconds)")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_sort_time.png")

    # Figure 2: Log-Log plot to analyze complexity
    plt.figure(figsize=(10, 6))
    plt.loglog(df["Size"], df["Time (s)"], marker="o", linestyle="-", color="red")

    # Add reference lines for O(n log n)
    x_range = np.logspace(np.log10(df["Size"].min()), np.log10(df["Size"].max()), 100)
    c = df["Time (s)"].iloc[0] / (df["Size"].iloc[0] * np.log(df["Size"].iloc[0]))
    y_range_nlogn = c * x_range * np.log(x_range)

    plt.loglog(x_range, y_range_nlogn, "k--", alpha=0.7, label="O(n log n)")

    plt.title("Sorting Time Complexity (Log-Log Scale)")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time (seconds)")
    plt.legend()
    plt.grid(True, alpha=0.3, which="both")
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_loglog.png")

    # Figure 3: Time/n*log(n) which should be constant for O(n log n) algorithms
    plt.figure(figsize=(10, 6))
    df["nlogn"] = df["Size"] * np.log(df["Size"])
    df["time_per_nlogn"] = df["Time (s)"] / df["nlogn"]

    plt.plot(df["Size"], df["time_per_nlogn"], marker="o", linestyle="-", color="green")
    plt.title("Time / (n log n) vs Array Size")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time / (n log n)")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_complexity.png")

    print(f"Plots saved to {output_dir}/")
    return True


def plot_algorithm_comparison(csv_file, output_dir="benchmark_plots"):
    """Plot the comparison results between heapsort and quicksort."""
    if not os.path.exists(csv_file):
        print(f"File not found: {csv_file}")
        return False

    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)

    # Load the data
    df = pd.read_csv(csv_file)

    # Check if this is a comparison file
    if not all(
        col in df.columns for col in ["HeapSort Time (ms)", "QuickSort Time (ms)"]
    ):
        print(f"File {csv_file} does not appear to be an algorithm comparison file.")
        return False

    # Extract base filename for plot titles and output filenames
    base_filename = os.path.basename(csv_file).replace(".csv", "")

    # Figure 1: Direct comparison of algorithms
    plt.figure(figsize=(12, 7))
    plt.plot(
        df["Size"],
        df["HeapSort Time (ms)"],
        marker="o",
        linestyle="-",
        color="blue",
        label="HeapSort",
    )
    plt.plot(
        df["Size"],
        df["QuickSort Time (ms)"],
        marker="s",
        linestyle="-",
        color="red",
        label="QuickSort",
    )
    plt.title("Algorithm Comparison: HeapSort vs QuickSort")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time (milliseconds)")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_comparison.png")

    # Figure 2: Log-Log plot
    plt.figure(figsize=(12, 7))
    plt.loglog(
        df["Size"],
        df["HeapSort Time (s)"],
        marker="o",
        linestyle="-",
        color="blue",
        label="HeapSort",
    )
    plt.loglog(
        df["Size"],
        df["QuickSort Time (s)"],
        marker="s",
        linestyle="-",
        color="red",
        label="QuickSort",
    )

    # Add reference line for O(n log n)
    x_range = np.logspace(np.log10(df["Size"].min()), np.log10(df["Size"].max()), 100)
    c = min(df["HeapSort Time (s)"].iloc[0], df["QuickSort Time (s)"].iloc[0]) / (
        df["Size"].iloc[0] * np.log(df["Size"].iloc[0])
    )
    y_range_nlogn = c * x_range * np.log(x_range)

    plt.loglog(x_range, y_range_nlogn, "k--", alpha=0.7, label="O(n log n)")

    plt.title("Algorithm Complexity (Log-Log Scale)")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time (seconds)")
    plt.legend()
    plt.grid(True, alpha=0.3, which="both")
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_loglog_comparison.png")

    # Figure 3: Relative performance ratio
    plt.figure(figsize=(12, 7))
    df["Performance Ratio"] = df["HeapSort Time (ms)"] / df["QuickSort Time (ms)"]
    plt.plot(
        df["Size"], df["Performance Ratio"], marker="o", linestyle="-", color="purple"
    )
    plt.axhline(y=1.0, color="k", linestyle="--", alpha=0.7)
    plt.title("Performance Ratio: HeapSort Time / QuickSort Time")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Ratio")
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_performance_ratio.png")

    print(f"Comparison plots saved to {output_dir}/")
    return True


def main():
    parser = argparse.ArgumentParser(
        description="Visualize sorting algorithm benchmark results."
    )
    parser.add_argument(
        "csv_file", nargs="?", help="CSV file containing benchmark results"
    )
    parser.add_argument(
        "--compare",
        action="store_true",
        help="Compare algorithms (use with algorithm comparison CSV)",
    )
    parser.add_argument(
        "--output-dir", default="benchmark_plots", help="Directory to save output plots"
    )

    args = parser.parse_args()

    # If no file is specified, try to find the latest one
    csv_file = args.csv_file
    if not csv_file:
        csv_file = find_latest_benchmark_file()

    if args.compare or "comparison" in csv_file:
        success = plot_algorithm_comparison(csv_file, args.output_dir)
    else:
        success = plot_heapsort_benchmark(csv_file, args.output_dir)

    if success:
        print("Visualization complete.")
    else:
        print("Visualization failed.")
        sys.exit(1)


if __name__ == "__main__":
    main()
