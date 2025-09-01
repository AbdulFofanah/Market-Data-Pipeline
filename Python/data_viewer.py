"""
Data viewer stage for Market Data Pipeline.
Reads the Java processor's output CSV (tradingSignals.csv),
makes charts, and prints out the statistcs.
"""

import argparse
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


def parse_arguments():
    """
    Parse command-line arguments for input CSV and output directory.
    Returns an object with:
        input_csv_path (str) - path to the Java-produced CSV
        output_dir (str)     - directory to save charts
    """
    parser = argparse.ArgumentParser(description="Create a visual representation of tradingSignals.csv "
    "                                             and print statistics.")
    parser.add_argument("--in", dest="input_csv_path", default="output/tradingSignals.csv",
                                help="Path to the CSV produced by the Java stage.")
    parser.add_argument("--out", dest="output_dir", default="output",
                                help="Directory to save the PNG charts.")
    return parser.parse_args()


def main():
    args = parse_arguments()

    # Ensure the output directory exists
    os.makedirs(args.output_dir, exist_ok=True)

    # Read the tradingSignals CSV (created in Java processor stage) into
    # a pandas dataframe for analysis and visualization.
    signal_dataframe = pd.read_csv(args.input_csv_path)

    # Convert the "timestamp" column into datetime objects so they can be
    # sorted and plotted correctly. Any invalid values are coerced to NaT through "coerce".
    signal_dataframe["timestamp"] = pd.to_datetime(signal_dataframe["timestamp"], errors="coerce")
    signal_dataframe = signal_dataframe.dropna(subset=["timestamp"]).sort_values("timestamp").reset_index(drop=True)

    # Ensure the CSV includes all columns required by the viewer stage.
    # If any are missing, stop early with a clear error message.
    required_columns = ["timestamp", "symbol", "close", "mean", "std", "zscore", "signal"]
    missing_columns = [col for col in required_columns if col not in signal_dataframe.columns]
    if missing_columns:
        raise ValueError(f"CSV is missing columns: {missing_columns}!")

    # Focus on the z-score series as the core signal metric. Clean any
    # infinities (can arise from division by zero) by replacing them
    # with NaN so the statistics ignore those entries safely.
    zscore_series = signal_dataframe["zscore"].replace([np.inf, -np.inf], np.nan)

    # Compute mean and standard deviation of the z-score series,
    # ignoring NaN values to avoid skewing the results.
    zscore_mean = np.nanmean(zscore_series)         # calulate mean of z-scores
    zscore_std = np.nanstd(zscore_series, ddof=1) # sample std (ddof=1 --> divde by N - 1)

    # Find the largest absolute z-score in the series (ignoring NaN values)
    # and report its magnitude and corresponding timestamp.
    if zscore_series.notna().any():
        max_abs_index = np.nanargmax(np.abs(zscore_series))  # index of max |z|
        max_abs_zscore = float(np.abs(zscore_series.iloc[max_abs_index])) # value of max |z|
        max_abs_timestamp = signal_dataframe["timestamp"].iloc[max_abs_index] # timestamp at that index
    else:
        # Fallback if no valid z-scores exist
        max_abs_index = None
        max_abs_zscore = np.nan
        max_abs_timestamp = None

    # Print results
    print("--- Statisics ---")
    print(f"Rows: {len(signal_dataframe)}")
    print(f"Z-score mean: {zscore_mean:.4f}")
    print(f"Z-score std : {zscore_std:.4f}")
    if max_abs_index is not None:
        print(f"Max |z|: {max_abs_zscore:.4f} at {max_abs_timestamp}")
    else:
        print("Max |z|: n/a")

    # Plot a simple time series of the close price to visualize overall
    # price action across the dataset.
    plt.figure()
    plt.plot(signal_dataframe["timestamp"], signal_dataframe["close"]) # plot close vs. time
    plt.title("Price over time") # chart title
    plt.xlabel("Time")           # x-axis label
    plt.ylabel("Close Price")    # y-axis label
    plt.tight_layout()           # adjust layout to prevent clipping
    
    # Save chart to output directory as 'price.png'
    price_chart_path = os.path.join(args.output_dir, "price.png")
    plt.savefig(price_chart_path)
    plt.close()  # close the figure to free up memory for next plot

    # Overlay the raw close with the rolling mean (from Stage 2: Java)
    # to show short-term deviation vs trend.
    plt.figure()
    plt.plot(signal_dataframe["timestamp"], signal_dataframe["close"], label="Close Price") # plot close vs. time
    plt.plot(signal_dataframe["timestamp"], signal_dataframe["mean"], label="Rolling Mean") # plot rolling mean vs. time
    plt.title("Price vs Rolling Mean")  # chart title
    plt.xlabel("Time")                  # x-axis label
    plt.ylabel("Price")                 # y-axis label
    plt.legend()                        # show legend to differentiate lines
    plt.tight_layout()                  # adjust layout to prevent clipping
    # Save chart to output directory as 'price_vs_mean.png'
    price_vs_mean_chart_path = os.path.join(args.output_dir, "price_vs_mean.png")
    plt.savefig(price_vs_mean_chart_path)
    plt.close() # close the figure to free up memory for next plot


    # Visualize standardized deviations with +/- 2 lines as common thresholds to indicate "unusual" moves
    plt.figure()
    plt.plot(signal_dataframe["timestamp"], signal_dataframe["zscore"], label="Z-score") # plot z-score vs. time
    plt.axhline(2.0, linestyle="--", color="red", label="+2 threshold")                  # +2 threshold line
    plt.axhline(-2.0, linestyle="--", color="red", label="-2 threshold")                 # -2 threshold line
    plt.title("Z-score over time (+/- 2)") # chart title
    plt.xlabel("Time")                  # x-axis label
    plt.ylabel("Z-score")               # y-axis label
    plt.legend()                        # show legend to differentiate lines
    plt.tight_layout()                  # adjust layout to prevent clipping
    # Save chart to output directory as 'zscore.png'
    zscore_chart_path = os.path.join(args.output_dir, "zscore.png")
    plt.savefig(zscore_chart_path)
    plt.close() # close the figure to free up memory
 
    # Provide the output paths of the saved charts for easy reference
    print("\nSaved charts:")
    print(f" - {price_chart_path}")
    print(f" - {price_vs_mean_chart_path}")
    print(f" - {zscore_chart_path}")


if __name__ == "__main__":
    # Entry point guard: only execute main() when run as a script - not when imported as a module.
    main()
