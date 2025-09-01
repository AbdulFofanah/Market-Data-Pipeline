/// Represents a single tick data point for a financial instrument.

public class Tick {
    public String timestamp;
    public String symbol;
    public double open;
    public double high;
    public double low;
    public double close;
    public long volume;

    public Tick() {
    } // Gson needs an empty constructor
}
