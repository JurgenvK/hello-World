/*
 * AC Pulse Counter with Variable Pulse Width Detection
 *
 * This program detects and counts pulses from an AC signal on an analog input.
 * It measures individual pulse durations and counts pulses in the valid range.
 *
 * Valid pulse range: 70ms to 1000ms
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
const int REPORT_PERIOD = 5000;      // Report statistics every 5 seconds
const int HIGH_THRESHOLD = 532;      // Threshold for detecting HIGH (512 + 20)
const int LOW_THRESHOLD = 492;       // Threshold for detecting LOW (512 - 20)
const unsigned long MIN_PULSE_WIDTH = 70;   // Minimum valid pulse width (ms)
const unsigned long MAX_PULSE_WIDTH = 1000; // Maximum valid pulse width (ms)
const unsigned long DEBOUNCE_TIME = 10;     // Debounce time in ms to prevent contact bounce

// Variables
int currentValue = 0;
bool currentState = false;           // false = LOW, true = HIGH
bool previousState = false;
bool stableState = false;            // Debounced stable state
unsigned long lastStateChangeTime = 0; // Time of last state change (for debouncing)
unsigned long pulseStartTime = 0;    // Time when pulse started
unsigned long pulseEndTime = 0;      // Time when pulse ended
unsigned long pulseDuration = 0;     // Duration of last pulse
unsigned long lastReportTime = 0;
unsigned long lastSampleTime = 0;

// Statistics
unsigned long totalPulseCount = 0;   // Total valid pulses counted
unsigned long invalidPulseCount = 0; // Pulses outside valid range
unsigned long lastPulseDuration = 0; // Most recent pulse duration
unsigned long minPulseDuration = 0;  // Shortest pulse seen
unsigned long maxPulseDuration = 0;  // Longest pulse seen

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Configure analog input
  pinMode(ANALOG_PIN, INPUT);

  // Initialize timing
  lastReportTime = millis();
  lastSampleTime = millis();

  // Initial reading
  currentValue = analogRead(ANALOG_PIN);
  previousState = (currentValue > HIGH_THRESHOLD);
  stableState = previousState;
  lastStateChangeTime = millis();

  // Print header
  Serial.println("AC Pulse Counter - Variable Pulse Width");
  Serial.println("========================================");
  Serial.println("Valid pulse range: 70ms to 1000ms");
  Serial.println("Debounce time: 10ms (contact bounce protection)");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Monitoring pulses...");
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();

  // Sample at regular intervals
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL) {
    lastSampleTime = currentTime;

    // Read analog input
    currentValue = analogRead(ANALOG_PIN);

    // Determine current state with hysteresis
    if (currentValue > HIGH_THRESHOLD) {
      currentState = true;
    } else if (currentValue < LOW_THRESHOLD) {
      currentState = false;
    }
    // If between thresholds, keep previous state (hysteresis)

    // Debouncing: detect raw state changes
    if (currentState != previousState) {
      // State changed - start debounce timer
      lastStateChangeTime = currentTime;
      previousState = currentState;
    }

    // Check if state has been stable for debounce period
    if (currentTime - lastStateChangeTime >= DEBOUNCE_TIME) {
      // State is stable, check if it's different from our accepted stable state
      if (currentState != stableState) {
        // Stable state transition confirmed
        if (currentState == true) {
          // Rising edge - pulse started
          pulseStartTime = currentTime;
        } else {
          // Falling edge - pulse ended
          pulseEndTime = currentTime;
          pulseDuration = pulseEndTime - pulseStartTime;

          // Validate pulse duration
          if (pulseDuration >= MIN_PULSE_WIDTH && pulseDuration <= MAX_PULSE_WIDTH) {
            // Valid pulse
            totalPulseCount++;
            lastPulseDuration = pulseDuration;

            // Update min/max statistics
            if (minPulseDuration == 0 || pulseDuration < minPulseDuration) {
              minPulseDuration = pulseDuration;
            }
            if (pulseDuration > maxPulseDuration) {
              maxPulseDuration = pulseDuration;
            }

            // Print individual pulse info
            Serial.print("Pulse #");
            Serial.print(totalPulseCount);
            Serial.print(" - Duration: ");
            Serial.print(pulseDuration);
            Serial.println(" ms");
          } else {
            // Invalid pulse (outside range)
            invalidPulseCount++;
            Serial.print("Invalid pulse detected: ");
            Serial.print(pulseDuration);
            Serial.print(" ms (outside ");
            Serial.print(MIN_PULSE_WIDTH);
            Serial.print("-");
            Serial.print(MAX_PULSE_WIDTH);
            Serial.println(" ms range)");
          }
        }
        // Update stable state
        stableState = currentState;
      }
    }
  }

  // Report statistics periodically
  if (currentTime - lastReportTime >= REPORT_PERIOD) {
    Serial.println();
    Serial.println("========== STATISTICS ==========");
    Serial.print("Total valid pulses: ");
    Serial.println(totalPulseCount);
    Serial.print("Invalid pulses: ");
    Serial.println(invalidPulseCount);

    if (totalPulseCount > 0) {
      Serial.print("Last pulse duration: ");
      Serial.print(lastPulseDuration);
      Serial.println(" ms");
      Serial.print("Min pulse duration: ");
      Serial.print(minPulseDuration);
      Serial.println(" ms");
      Serial.print("Max pulse duration: ");
      Serial.print(maxPulseDuration);
      Serial.println(" ms");

      // Calculate average pulse rate
      float pulsesPerSecond = (float)totalPulseCount / ((currentTime - 0) / 1000.0);
      Serial.print("Average pulse rate: ");
      Serial.print(pulsesPerSecond, 2);
      Serial.println(" pulses/sec");
    }
    Serial.println("================================");
    Serial.println();

    lastReportTime = currentTime;
  }
}

/*
 * EXPECTED OUTPUT:
 * - Individual pulse notifications showing pulse number and duration
 * - Warnings for pulses outside the 70-1000ms valid range
 * - Statistics report every 5 seconds showing:
 *   - Total valid pulses counted
 *   - Invalid pulse count
 *   - Last, min, and max pulse durations
 *   - Average pulse rate (pulses/second)
 *
 * CONTACT BOUNCE PROTECTION:
 * - 10ms debounce time prevents mechanical contact bounce from creating false counts
 * - State must be stable for 10ms before being accepted as a valid transition
 * - Adjust DEBOUNCE_TIME if needed (5-20ms typical for mechanical contacts)
 *
 * TROUBLESHOOTING:
 * - If count is 0: Check signal connection and amplitude
 * - If many invalid pulses: Adjust MIN_PULSE_WIDTH and MAX_PULSE_WIDTH
 * - If erratic detection: Adjust HIGH_THRESHOLD and LOW_THRESHOLD, or increase DEBOUNCE_TIME
 * - If missing pulses: Check that signal swings above/below thresholds
 * - If counting multiple pulses for one contact: Increase DEBOUNCE_TIME
 * - Adjust thresholds based on your actual signal levels (use Serial Plotter)
 */
