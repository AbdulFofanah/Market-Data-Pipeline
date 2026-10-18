# Market Data Pipeline

A cross-language market-data processing pipeline built with **C++17, Java and Python**.

The project ingests intraday OHLCV market data, converts it into a structured interchange format, performs rolling statistical analysis to generate trading signals, and produces time-series visualisations and summary statistics.

**Pipeline:** C++ ingestion → JSONL → Java processing → CSV → Python analysis and visualisation

## Quick Start

The commands below are for **Windows PowerShell** and should be run from the project root.

Pipeline outputs are written to `output/`, while compiled C++/Java files are written to `bin/`.

### 1. Compile and run the C++ producer

```powershell
g++ -std=c++17 C++/Market_Data_Producer.cpp -o bin/Market_Data_Producer.exe

./bin/Market_Data_Producer.exe intraday_1min_AAPL.csv
```

The producer reads:

```text
data/intraday_1min_AAPL.csv
```

and writes:

```text
output/output.jsonl
```

The input filename is used to infer the market symbol. For example, `intraday_1min_AAPL.csv` produces the symbol `AAPL`.

### 2. Compile and run the Java processor

```powershell
javac -cp ".;java/gson-2.13.1.jar" -d bin java/Tick.java java/MarketDataProcessor.java

java -cp ".;bin;java/gson-2.13.1.jar" MarketDataProcessor output/output.jsonl output/tradingSignals.csv 20
```

The processor reads:

```text
output/output.jsonl
```

and writes:

```text
output/tradingSignals.csv
```

The final argument (`20`) specifies the rolling-window size. If it is omitted, the processor uses a default window size of 20.

### 3. Run the Python viewer

```powershell
python Python/data_viewer.py --in output/tradingSignals.csv --out output
```

The viewer reads:

```text
output/tradingSignals.csv
```

and generates:

```text
output/price.png
output/price_vs_mean.png
output/zscore.png
```

### macOS / Linux

The Java classpath separator is `:` rather than `;`, and the C++ executable would normally be built without the `.exe` extension.

---

## Architecture

```text
Intraday OHLCV CSV
        │
        ▼
┌─────────────────────┐
│    C++ Producer     │
│  Parse + serialise  │
└─────────────────────┘
        │
        ▼
      JSONL
        │
        ▼
┌─────────────────────┐
│   Java Processor    │
│ Rolling statistics  │
│ Z-scores + signals  │
└─────────────────────┘
        │
        ▼
   Signal CSV
        │
        ▼
┌─────────────────────┐
│   Python Viewer     │
│ Analysis + charts   │
└─────────────────────┘
```

Each stage has a distinct responsibility and communicates through structured files, creating a simple end-to-end pipeline across three programming languages.

## C++ - Data Producer

The C++ stage ingests Alpha Vantage-style intraday OHLCV CSV data.

It:

- Reads and parses CSV market data
- Handles timestamp, open, high, low, close and volume fields
- Infers the market symbol from the input filename
- Converts each record into structured JSON
- Serialises the resulting records as **JSON Lines (JSONL)**
- Produces the intermediate dataset consumed by the Java stage

Using JSONL provides a simple language-independent interface between the C++ and Java components.

## Java - Data Processor

The Java stage reads the generated JSONL using **Gson** and performs rolling-window analysis.

It calculates:

- Rolling mean
- Rolling standard deviation
- Z-score
- Derived trading signal

The processor then writes the enriched data to CSV for downstream analysis.

The output schema is:

```text
timestamp,symbol,close,mean,std,zscore,signal
```

The rolling-window size can be configured at runtime.

## Python - Data Viewer

The final stage uses **Pandas, NumPy and Matplotlib** to analyse and visualise the processed data.

It:

- Loads the generated trading-signal CSV
- Validates the expected schema
- Computes summary statistics
- Produces price time-series visualisations
- Compares price against the rolling mean
- Visualises calculated z-scores

Generated charts include:

```text
output/price.png
output/price_vs_mean.png
output/zscore.png
```

## Input Data Format

Input files must contain the following header:

```text
timestamp,open,high,low,close,volume
```

Example:

```csv
timestamp,open,high,low,close,volume
2025-02-28 09:30:00,175.12,176.00,174.95,175.90,123456
```

Input files should follow the naming convention:

```text
anything_SYMBOL.csv
```

For example:

```text
intraday_1min_AAPL.csv
```

produces the inferred symbol:

```text
AAPL
```

## Repository Structure

```text
Market-Data-Pipeline/
├── C++/
│   ├── Market_Data_Producer.cpp
│   └── json.hpp
├── java/
│   ├── MarketDataProcessor.java
│   ├── Tick.java
│   └── gson-2.13.1.jar
├── Python/
│   └── data_viewer.py
├── data/
│   └── input CSV files
├── bin/
│   └── compiled executables and class files
├── output/
│   ├── output.jsonl
│   ├── tradingSignals.csv
│   └── generated charts
└── README.md
```

## Dependencies

### C++

- C++17-compatible compiler
- `nlohmann/json` single-header library (`json.hpp`), included in `C++/`

### Java

- Java 11+
- `javac`
- `java`
- Gson `2.13.1`, included in `java/`

### Python

Python 3.9+ with:

```text
pandas
numpy
matplotlib
```

## Data Flow

### C++ Producer

**Input**

```text
data/<filename>.csv
```

**Output**

```text
output/output.jsonl
```

A custom output filename may also be supplied.

### Java Processor

**Input**

```text
output/output.jsonl
```

**Output**

```text
output/tradingSignals.csv
```

A custom output filename and rolling-window size may be supplied.

### Python Viewer

**Input**

```text
output/tradingSignals.csv
```

**Output**

```text
output/price.png
output/price_vs_mean.png
output/zscore.png
```

## Key Technical Features

- End-to-end pipeline spanning **C++, Java and Python**
- CSV ingestion and structured JSONL serialisation
- Language-independent interchange between pipeline stages
- Sliding-window processing of time-series data
- Rolling mean and standard-deviation calculations
- Z-score calculation and signal generation
- Configurable rolling-window size
- Schema validation
- Data analysis with Pandas and NumPy
- Time-series visualisation with Matplotlib
- Separation of ingestion, processing and presentation responsibilities

## Project Motivation

I built this project independently to explore how multiple programming languages can be combined within a single data-processing workflow while applying algorithms to financial market data.

The project provided practical experience in:

- Designing interfaces between components written in different languages
- Processing structured market data
- Implementing sliding-window algorithms
- Working with rolling statistical measures
- Building an end-to-end data pipeline
- Visualising and interpreting processed time-series data

## Technologies

**C++17 · Java · Python · Gson · Pandas · NumPy · Matplotlib · JSONL · CSV**

## Notes and Constraints

- Input CSVs must use the expected OHLCV schema.
- The filename convention `*_SYMBOL.csv` is required for automatic symbol extraction.
- The Java processor produces a fixed output schema.
- The Python viewer validates required columns and exits with an error if the expected schema is not present.
- The generated trading signals are intended for software engineering and data-analysis experimentation rather than production trading.

## License

MIT License. See `LICENSE` for details.
