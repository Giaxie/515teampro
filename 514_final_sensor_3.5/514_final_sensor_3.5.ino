#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Pin definitions
#define BLUE_LED 25
#define YELLOW_LED 26
#define RED_LED 27

// Motion state thresholds (units: change rate in m/s²)
#define STILL_THRESHOLD 0.3   // Threshold for stationary state
#define SLOW_THRESHOLD 1.5    // Threshold for slow movement
#define FAST_THRESHOLD 3.0    // Threshold for fast movement

// Sampling parameters
#define SAMPLE_SIZE 5        // Sample window size
#define SAMPLE_INTERVAL 50   // Sample interval (ms)

Adafruit_MPU6050 mpu;

// Store the most recent acceleration data
float accelHistory[SAMPLE_SIZE][3]; // X,Y,Z
int sampleIndex = 0;
unsigned long lastSampleTime = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize LED pins
  pinMode(BLUE_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  // Initially turn off all LEDs
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  
  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("MPU6050 connection failed!");
    while (1) {
      // Flash red light on connection failure
      digitalWrite(RED_LED, HIGH);
      delay(200);
      digitalWrite(RED_LED, LOW);
      delay(200);
    }
  }
  
  // Configure MPU6050 parameters
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  
  Serial.println("MPU6050 initialized successfully!");
  Serial.println("-------------------------------------");
  Serial.println("Acceleration change rate (m/s²) | Motion state");
  Serial.println("-------------------------------------");

  // Initialize acceleration history data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  for(int i=0; i<SAMPLE_SIZE; i++){
    accelHistory[i][0] = a.acceleration.x;
    accelHistory[i][1] = a.acceleration.y;
    accelHistory[i][2] = a.acceleration.z;
  }
}

void loop() {
  unsigned long currentTime = millis();
  
  // Sample at fixed intervals
  if(currentTime - lastSampleTime >= SAMPLE_INTERVAL){
    lastSampleTime = currentTime;
    
    // Get new data
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // Update historical data
    accelHistory[sampleIndex][0] = a.acceleration.x;
    accelHistory[sampleIndex][1] = a.acceleration.y;
    accelHistory[sampleIndex][2] = a.acceleration.z;
    sampleIndex = (sampleIndex + 1) % SAMPLE_SIZE;
    
    // Calculate change rate (difference between current data and historical average)
    float avgX = 0, avgY = 0, avgZ = 0;
    for(int i=0; i<SAMPLE_SIZE; i++){
      avgX += accelHistory[i][0];
      avgY += accelHistory[i][1];
      avgZ += accelHistory[i][2];
    }
    avgX /= SAMPLE_SIZE;
    avgY /= SAMPLE_SIZE;
    avgZ /= SAMPLE_SIZE;
    
    float deltaX = a.acceleration.x - avgX;
    float deltaY = a.acceleration.y - avgY;
    float deltaZ = a.acceleration.z - avgZ;
    
    // Calculate total change rate (ignoring direction, only considering change magnitude)
    float accelChangeRate = sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
    
    // Print debug information
    Serial.print(accelChangeRate, 2);
    Serial.print(" m/s²\t\t| ");
    
    // Control LEDs based on change rate
    if (accelChangeRate < STILL_THRESHOLD) {
      digitalWrite(BLUE_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, LOW);
      Serial.println("Stationary");
    } 
    else if (accelChangeRate < SLOW_THRESHOLD) {
      digitalWrite(BLUE_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      Serial.println("Slow movement");
    } 
    else {
      digitalWrite(BLUE_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      Serial.println("Fast movement");
    }
  }
  
  delay(10); // Small delay in main loop
}