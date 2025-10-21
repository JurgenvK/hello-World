/*
 * AC Pulse Counter for 50Hz Signal
 *
 * This program counts pulses from a 50Hz AC signal connected to an analog input.
 * It detects zero-crossings and counts them over 100ms intervals.
 *
 * HARDWARE SETUP:
 * - Connect AC signal through appropriate voltage divider and protection circuit
 * - AC signal should be scaled to 0-5V range (centered around 2.5V)
 * - Use resistor divider and diode protection to protect Arduino input
 *
 * WARNING: Never connect mains AC voltage directly to Arduino!
 * Use proper isolation (transformer) and voltage scaling circuit.
 */

// Configuration
const int ANALOG_PIN = A0;           // Analog input pin
const int SAMPLE_INTERVAL = 1;       // Sample every 1ms
const int COUNT_PERIOD = 100;        // Count period in milliseconds
const int ZERO_THRESHOLD = 512;      // Threshold for zero crossing (midpoint of 0-1023)
const int HYSTERESIS = 20;           // Hysteresis to prevent noise triggering

// Variables
int currentValue = 0;
int previousValue = 0;
bool currentState = false;           // false = below threshold, true = above threshold
bool previousState = false;
unsigned long lastCountTime = 0;
unsigned long lastSampleTime = 0;
int pulseCount = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Configure analog input
  pinMode(ANALOG_PIN, INPUT);

  // Initialize timing
  lastCountTime = millis();
  lastSampleTime = millis();

  // Initial reading
  previousValue = analogRead(ANALOG_PIN);
  previousState = (previousValue > ZERO_THRESHOLD);

  // Print header
  Serial.println("AC Pulse Counter - 50Hz Signal");
  Serial.println("================================");
  Serial.println("Time(ms)\tPulses\tFrequency(Hz)");
  Serial.println("================================");
}

void loop() {
  unsigned long currentTime = millis();

  // Sample at regular intervals
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentTime;

    // Read analog input
    currentValue = analogRead(ANALOG_PIN);

    // Determine current state with hysteresis
    if (currentValue > (ZERO_THRESHOLD + HYSTERESIS)) {
      currentState = true;
    } else if (currentValue < (ZERO_THRESHOLD - HYSTERESIS)) {
      currentState = false;
    }
    // If between thresholds, keep previous state (hysteresis)

    // Detect state change (zero crossing)
    if (currentState != previousState) {
      pulseCount++;
      previousState = currentState;
    }

    previousValue = currentValue;
  }

  // Report counts every COUNT_PERIOD milliseconds
  if (currentTime - lastCountTime >= COUNT_PERIOD) {
    // Calculate frequency from pulse count
    // Each complete AC cycle has 2 zero crossings (positive and negative)
    // Frequency = (pulseCount / 2) / (COUNT_PERIOD / 1000)
    float frequency = (pulseCount / 2.0) / (COUNT_PERIOD / 1000.0);

    // Print results
    Serial.print(currentTime);
    Serial.print("\t\t");
    Serial.print(pulseCount);
    Serial.print("\t");
    Serial.print(frequency, 1);
    Serial.println(" Hz");

    // Reset for next counting period
    pulseCount = 0;
    lastCountTime = currentTime;
  }
}

/*
 * EXPECTED OUTPUT:
 * For a 50Hz AC signal, you should see:
 * - Approximately 10 pulses per 100ms (5 complete cycles × 2 crossings per cycle)
 * - Frequency reading around 50 Hz
 *
 * TROUBLESHOOTING:
 * - If count is 0: Check signal connection and amplitude
 * - If count is erratic: Adjust HYSTERESIS value or check for noise
 * - If frequency is double: Signal may have DC offset, adjust ZERO_THRESHOLD
 * - If frequency is half: Only detecting one edge, check signal amplitude
 */
