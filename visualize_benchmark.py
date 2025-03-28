#!/usr/bin/env python3
# File: visualize_benchmark.py

import argparse
import glob
import os
import sys

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from matplotlib.ticker import LogFormatter, ScalarFormatter


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

    # Clean data - remove invalid measurements (negative values)
    df = df[df["Time (s)"] > 0]

    if len(df) == 0:
        print("No valid data points found in the CSV file after filtering")
        return False

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

    # Clean data - replace negative values (errors) with NaN for filtering
    df["HeapSort Time (s)"] = pd.to_numeric(df["HeapSort Time (s)"], errors="coerce")
    df["QuickSort Time (s)"] = pd.to_numeric(df["QuickSort Time (s)"], errors="coerce")

    # Keep only rows where at least one algorithm has valid data
    df_filtered = df[
        (df["HeapSort Time (s)"] > 0) | (df["QuickSort Time (s)"] > 0)
    ].copy()

    if len(df_filtered) == 0:
        print("No valid data points found in the CSV file after filtering")
        return False

    # Extract base filename for plot titles and output filenames
    base_filename = os.path.basename(csv_file).replace(".csv", "")

    # Figure 1: Direct comparison of algorithms
    plt.figure(figsize=(12, 7))

    # Plot each algorithm only where it has valid data
    heapsort_valid = df_filtered[df_filtered["HeapSort Time (s)"] > 0]
    quicksort_valid = df_filtered[df_filtered["QuickSort Time (s)"] > 0]

    if not heapsort_valid.empty:
        plt.plot(
            heapsort_valid["Size"],
            heapsort_valid["HeapSort Time (ms)"],
            marker="o",
            linestyle="-",
            color="blue",
            label="HeapSort",
        )

    if not quicksort_valid.empty:
        plt.plot(
            quicksort_valid["Size"],
            quicksort_valid["QuickSort Time (ms)"],
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

    if not heapsort_valid.empty:
        plt.loglog(
            heapsort_valid["Size"],
            heapsort_valid["HeapSort Time (s)"],
            marker="o",
            linestyle="-",
            color="blue",
            label="HeapSort",
        )

    if not quicksort_valid.empty:
        plt.loglog(
            quicksort_valid["Size"],
            quicksort_valid["QuickSort Time (s)"],
            marker="s",
            linestyle="-",
            color="red",
            label="QuickSort",
        )

    # Add reference line for O(n log n)
    if not df_filtered.empty:
        x_range = np.logspace(
            np.log10(df_filtered["Size"].min()),
            np.log10(df_filtered["Size"].max()),
            100,
        )

        # Use the algorithm with the smallest initial time value for reference line
        ref_time = None
        ref_size = None

        if not heapsort_valid.empty:
            ref_time = heapsort_valid["HeapSort Time (s)"].iloc[0]
            ref_size = heapsort_valid["Size"].iloc[0]

        if not quicksort_valid.empty:
            if (
                ref_time is None
                or quicksort_valid["QuickSort Time (s)"].iloc[0] < ref_time
            ):
                ref_time = quicksort_valid["QuickSort Time (s)"].iloc[0]
                ref_size = quicksort_valid["Size"].iloc[0]

        if ref_time is not None and ref_size is not None:
            c = ref_time / (ref_size * np.log(ref_size))
            y_range_nlogn = c * x_range * np.log(x_range)
            plt.loglog(x_range, y_range_nlogn, "k--", alpha=0.7, label="O(n log n)")

    plt.title("Algorithm Complexity (Log-Log Scale)")
    plt.xlabel("Array Size (n)")
    plt.ylabel("Time (seconds)")
    plt.legend()
    plt.grid(True, alpha=0.3, which="both")
    plt.tight_layout()
    plt.savefig(f"{output_dir}/{base_filename}_loglog_comparison.png")

    # Figure 3: Relative performance ratio (for data points where both algorithms worked)
    both_valid = df_filtered[
        (df_filtered["HeapSort Time (s)"] > 0) & (df_filtered["QuickSort Time (s)"] > 0)
    ].copy()

    if not both_valid.empty:
        plt.figure(figsize=(12, 7))
        both_valid["Performance Ratio"] = (
            both_valid["HeapSort Time (ms)"] / both_valid["QuickSort Time (ms)"]
        )
        plt.plot(
            both_valid["Size"],
            both_valid["Performance Ratio"],
            marker="o",
            linestyle="-",
            color="purple",
        )
        plt.axhline(y=1.0, color="k", linestyle="--", alpha=0.7)
        plt.title("Performance Ratio: HeapSort Time / QuickSort Time")
        plt.xlabel("Array Size (n)")
        plt.ylabel("Ratio")
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig(f"{output_dir}/{base_filename}_performance_ratio.png")
    else:
        print("Not enough valid data points to create performance ratio plot")

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

    # If no CSV file specified, find the latest benchmark file
    csv_file = args.csv_file if args.csv_file else find_latest_benchmark_file()

    if args.compare:
        success = plot_algorithm_comparison(csv_file, args.output_dir)
    else:
        success = plot_heapsort_benchmark(csv_file, args.output_dir)

    if success:
        print(f"Visualization complete. Plots saved to {args.output_dir}/")
    else:
        print("Visualization failed. Check error messages above.")
        sys.exit(1)


if __name__ == "__main__":
    main()
