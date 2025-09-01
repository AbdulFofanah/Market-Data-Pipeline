// reading and writing files
import java.io.*; 
import java.nio.file.*;
import java.nio.charset.StandardCharsets; //file encoding
import java.util.*; //data structures
import java.text.*; //number formatting
import com.google.gson.*; //Json parsing

public class MarketDataProcessor {

    public static void main(String[] args) {
        // invalid args check
        if (args.length < 1) {
            System.err.println("You must provide an input JSONL file path as the first argument.");
            return;
        }

        // Parse args with defaults
        final String inputFile      = args[0];
        final String outputFile     = (args.length > 1) ? args[1] : "tradingSignals.csv";
        final int rollingWindowSize = (args.length > 2) ? parseWindowSize(args[2], 20) : 20;


        // JSON parser
        final Gson jsonParser = new Gson();

        // rolling window of closes
        final ArrayDeque<Double> rollingCloses = new ArrayDeque<>(rollingWindowSize); // queue of the most recent N close prices.
        double rollingSum = 0.0; // Sum of all closes in rollingCloses
        double rollingSumSquares = 0.0; // sum of the squares of all closes in rollingCloses


        // CSV number format (US style, keeps CSV consistent regardless of locale)
        final DecimalFormat priceFormat = (DecimalFormat) DecimalFormat.getInstance(Locale.US);
        priceFormat.applyPattern("0.######");

        // keep track of how many rows we write
        long rowsWritten = 0;

        // I/O: open input JSONL and output CSV (UTF-8)
        try (BufferedReader reader = Files.newBufferedReader(Path.of(inputFile), StandardCharsets.UTF_8);
            PrintWriter csvOut    = new PrintWriter(Files.newBufferedWriter(Path.of(outputFile), StandardCharsets.UTF_8))) {

            // header row for CSV
            csvOut.println("timestamp,symbol,close,mean,std,zscore,signal");

            String jsonLine;
            // process file line-by-line
            while ((jsonLine = reader.readLine()) != null) {
                if (jsonLine.isEmpty()) continue;

                //Parse JSON → Tick (skip lines that fail)
                Tick tick;
                try {
                    tick = jsonParser.fromJson(jsonLine, Tick.class);
                } catch (JsonSyntaxException ignored) {
                    continue;
                }

                // skip ticks with missing timestamp
                if (tick == null || tick.timestamp == null) continue;

                // Update rolling window with this tick's close
                double close = tick.close;
                rollingCloses.addLast(close);
                rollingSum        += close;
                rollingSumSquares += close * close;

                // If we exceed the window size, remove the oldest close
                if (rollingCloses.size() > rollingWindowSize) {
                    double oldest = rollingCloses.removeFirst();
                    rollingSum        -= oldest;
                    rollingSumSquares -= oldest * oldest;
                }

                // Compute mean, std, z-score, and signal if we have enough data
                double mean = Double.NaN, std = Double.NaN, zscore = Double.NaN, signal = Double.NaN;
                int windowCount = rollingCloses.size();
                if (windowCount >= 2) {
                    mean = rollingSum / windowCount;

                    // Sample variance
                    // Formula: variance = (Σx² - (Σx)² / n) / (n - 1)
                    // -> rollingSum        = Σx (sum of closes in the window)
                    // -> rollingSumSquares = Σx² (sum of squared closes in the window)
                    // -> n                 = number of closes currently in the window
                    double variance = (rollingSumSquares - (rollingSum * rollingSum) / windowCount) / (windowCount - 1);
                    if (variance < 0) variance = 0;      // guard against small negative variance due to floating-point errors
                    std = Math.sqrt(variance);

                    if (std > 0) {
                        zscore = (close - mean) / std;   // how many stds above/below mean
                        signal = -zscore;                // trading signal: buy low (z<0), sell high (z>0)
                    } else {
                        zscore = 0.0;
                        signal = 0.0;
                    }
                }

                // Write CSV row
                csvOut.println(String.join(",",
                    nullToEmpty(tick.timestamp),  // timestamp should never be null here due to earlier check
                    nullToEmpty(tick.symbol),     // symbol can be null
                    priceFormat.format(close),
                    priceFormat.format(mean),
                    priceFormat.format(std),
                    priceFormat.format(zscore),
                    priceFormat.format(signal)));
                rowsWritten++;
            }

        // close files
        } catch (IOException ioEx) {
            ioEx.printStackTrace();
            return;
        }

        System.out.println("Processing complete. " + "Wrote " + rowsWritten + " rows to " 
                                                   + outputFile + " using a rolling window of " 
                                                   + rollingWindowSize + ".");

    }


    // Parse window size from string, with fallback
    private static int parseWindowSize(String input, int fallback) {
        try {
            int windowSize = Integer.parseInt(input);
            return (windowSize >= 2) ? windowSize : fallback;
        } catch (Exception e) {
            return fallback;
        }
    }

    // Convert null strings to empty strings for CSV output
    private static String nullToEmpty(String value) {
        return (value == null) ? "" : value;
    }
}
