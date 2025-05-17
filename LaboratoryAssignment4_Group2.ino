// Pin Definitions
#define POT_PIN PA0     // Analog input for potentiometer
#define POT1_PIN PA1     // Analog input for potentiometer
#define PWM_PIN PA6     // PWM output pin

// ADC Configuration
const int ADC_RESOLUTION = 12;   // 12-bit resolution (0-4095)
const float VREF = 3.3;          // Reference voltage

// Moving Average Filter
const int SAMPLE_SIZE = 10;
int potSamples[SAMPLE_SIZE], pot1Samples[SAMPLE_SIZE];
int sampleIndex = 0;
long potSum = 0, pot1Sum = 0;

// Timing Control
unsigned long previousMillis = 0;
const long INTERVAL = 200;  // Transmission interval in ms

void setup() {
  Serial.begin(115200);       // Initialize serial communication
  analogReadResolution(ADC_RESOLUTION); // Set ADC resolution
  
  pinMode(PWM_PIN, OUTPUT);   // Set PWM pin as output
  analogWriteFrequency(1000); // Set PWM frequency to 1kHz (optional)
  
  // Initialize moving average arrays
  int initialPot = analogRead(POT_PIN);
  int initialPot1 = analogRead(POT1_PIN);
  for(int i=0; i<SAMPLE_SIZE; i++){
    potSamples[i] = initialPot;
    pot1Samples[i] = initialPot1;
  }
  potSum = initialPot * SAMPLE_SIZE;
  pot1Sum = initialPot1 * SAMPLE_SIZE;
}

void loop() {
  // Read raw ADC values
  int potRaw = analogRead(POT_PIN);
  int pot1Raw = analogRead(POT1_PIN);

  // Update moving averages
  updateMovingAverage(potRaw, potSamples, &potSum);
  updateMovingAverage(pot1Raw, pot1Samples, &pot1Sum);

  // Calculate filtered values
  float ldrFiltered = potSum / (float)SAMPLE_SIZE;
  float potFiltered = pot1Sum / (float)SAMPLE_SIZE;

  // Control PWM with potentiometer (0-100% duty cycle)
  int pwmValue = map(potFiltered, 0, 4095, 0, 255);
  analogWrite(PWM_PIN, pwmValue);

  // Send data periodically
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= INTERVAL) {
    previousMillis = currentMillis;
    
    // Convert to voltage and percentage
    float potVoltage = (potFiltered / 4095.0) * VREF;
    float pot1Voltage = (pot1Filtered / 4095.0) * VREF;
    float potPercent = (potFiltered / 4095.0) * 100;
    float pot1Percent = (pot1Filtered / 4095.0) * 100;

    // Create formatted string
    char buffer[80];
    snprintf(buffer, sizeof(buffer),
            "pot: %.2fV (%.1f%%)\tpot1: %.2fV (%.1f%%)",
            potVoltage, potPercent, pot1Voltage, potPercent);

    // Send via Serial
    Serial.println(buffer);
  }
}

void updateMovingAverage(int newVal, int *samples, long *sum) {
  *sum = *sum - samples[sampleIndex] + newVal;
  samples[sampleIndex] = newVal;
  sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE;
}

