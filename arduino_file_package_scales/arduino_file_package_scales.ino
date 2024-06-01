#include "HX711.h"

// Define the pins for HX711
const int LOADCELL_DOUT_PIN = 4; // DOUT pin on HX711
const int LOADCELL_SCK_PIN = 5;  // SCK pin on HX711

HX711 scale;

void setup() {
  Serial.begin(115200); // Initialize serial communication

  // Initialize the HX711 module
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Tare the scale (reset to zero)
  Serial.println("Tare... remove any weights from the scale.");
  delay(2000);
  scale.tare();
  Serial.println("Tare done. Please place the object to be weighed on the scale.");
}

void loop() {
  // Check if HX711 is ready
  if (scale.is_ready()) {
    // Read the value from the load cell
    long reading = scale.read();
    
    // Print the reading to the Serial Monitor
    Serial.print("Load Cell Reading: ");
    Serial.println(reading);

    // Optionally, you can apply calibration here if needed
    // float weight = reading / calibration_factor;
    // Serial.print("Weight: ");
    // Serial.println(weight);

  } else {
    Serial.println("HX711 not found.");
  }

  // Wait before the next reading
  delay(1000);
}
