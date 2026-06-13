#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

#define BT Serial

// Crash detection settings
float impactThreshold = 2.5; 
bool crashDetected = false;
unsigned long crashTime = 0;

bool cancelReceived = false;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);

  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("Device connection failed!");
    while (1);
  }

  Serial.println("Helmet System Ready");
}

float getAccelerationMagnitude() {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);


  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  // magnitude
  return sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
}

void loop() {
  float accel = getAccelerationMagnitude();

  // Detect impact
  if (!crashDetected && accel > impactThreshold) {
    crashDetected = true;
    crashTime = millis();
    cancelReceived = false;

    BT.println("CRASH_DETECTED");
    Serial.println("Crash detected! Countdown started...");
  }

  // Wait for cancel window (10 seconds)
  if (crashDetected) {

    
    if (BT.available()) {
      String msg = BT.readStringUntil('\n');
      msg.trim();

      if (msg == "CANCEL") {
        cancelReceived = true;
        crashDetected = false;
        BT.println("CANCELLED");
        Serial.println("Emergency cancelled");
      }
    }

    
    if (millis() - crashTime >= 10000) {

      if (!cancelReceived) {
        BT.println("EMERGENCY_CALL");
        Serial.println("Emergency sent!");
      }

      crashDetected = false;
    }
  }

  delay(10); 
}