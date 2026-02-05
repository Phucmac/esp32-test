#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define OLED_ADDRESS 0x3C
#define BH1750_ADDRESS 0x23
#define LED_BUILTIN 2

BH1750 lightMeter;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;
float luxHistory[10];
int historyIndex = 0;
bool oledConnected = false;
bool bh1750Connected = false;

void setupI2C();
void scanI2CDevices();
void setupOLED();
void setupBH1750();
void displayOnOLED(float lux, float avgLux);
void displayOnSerial(float lux, float avgLux);
float calculateAverage(float newValue);
String getLightCondition(float lux);
void controlOnboardLED(float lux);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.println("\n==================================");
  Serial.println("   ESP32 I2C TUTORIAL PROJECT");
  Serial.println("==================================");
  Serial.println("Initializing system...");
  
  setupI2C();
  scanI2CDevices();
  setupOLED();
  setupBH1750();
  
  for(int i = 0; i < 10; i++) {
    luxHistory[i] = 0;
  }
  
  Serial.println("\n=== SYSTEM STATUS ===");
  Serial.print("OLED: ");
  Serial.println(oledConnected ? "CONNECTED" : "NOT CONNECTED");
  Serial.print("BH1750: ");
  Serial.println(bh1750Connected ? "CONNECTED" : "NOT CONNECTED");
  
  if(!bh1750Connected) {
    Serial.println("\n⚠️ WARNING: BH1750 not found!");
    Serial.println("Please check:");
    Serial.println("1. I2C connections (SDA=GPIO21, SCL=GPIO22)");
    Serial.println("2. Power (3.3V and GND)");
    Serial.println("3. I2C address (try 0x23 or 0x5C)");
  }
  
  Serial.println("\nSystem Ready!");
  Serial.println("Reading light sensor every second...");
  Serial.println("==================================\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  if(currentTime - lastUpdate >= updateInterval) {
    lastUpdate = currentTime;
    
    if(bh1750Connected) {
      float lux = lightMeter.readLightLevel();
      float avgLux = calculateAverage(lux);
      
      displayOnSerial(lux, avgLux);
      
      if(oledConnected) {
        displayOnOLED(lux, avgLux);
      }
      
      controlOnboardLED(lux);
    } else {
      static int errorCount = 0;
      errorCount++;
      
      Serial.print("BH1750 not connected! Attempt ");
      Serial.print(errorCount);
      Serial.println(". Please check wiring.");
      
      digitalWrite(LED_BUILTIN, errorCount % 2);
      
      if(errorCount % 10 == 0) {
        Serial.println("Attempting to reconnect to BH1750...");
        setupBH1750();
      }
    }
  }
  
  delay(10);
}

void setupI2C() {
  Serial.print("Initializing I2C (SDA=");
  Serial.print(I2C_SDA);
  Serial.print(", SCL=");
  Serial.print(I2C_SCL);
  Serial.println(")...");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  delay(100);
  Serial.println("I2C initialized successfully!");
}

void scanI2CDevices() {
  Serial.println("\nScanning I2C bus...");
  byte error, address;
  int devicesFound = 0;
  
  Serial.println("Address  | Status");
  Serial.println("------------------");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print("  0x");
      if(address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print("  | ");
      
      switch(address) {
        case 0x23:
        case 0x5C:
          Serial.println("BH1750 Light Sensor");
          break;
        case 0x3C:
        case 0x3D:
          Serial.println("OLED SSD1306");
          break;
        case 0x68:
          Serial.println("MPU6050/DS3231");
          break;
        case 0x76:
        case 0x77:
          Serial.println("BMP280/BME280");
          break;
        default:
          Serial.println("Unknown I2C Device");
      }
      devicesFound++;
    } else if(error == 4) {
      Serial.print("  0x");
      if(address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  | Unknown error at address");
    }
  }
  
  if(devicesFound == 0) {
    Serial.println("  No I2C devices found!");
  } else {
    Serial.print("\nTotal devices found: ");
    Serial.println(devicesFound);
  }
}

void setupOLED() {
  Serial.print("\nInitializing OLED (0x");
  Serial.print(OLED_ADDRESS, HEX);
  Serial.println(")...");
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("  ⚠️ OLED NOT FOUND! Please check connection.");
    Serial.println("  Trying alternative address 0x3D...");
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("  ❌ OLED NOT FOUND at any address!");
      oledConnected = false;
    } else {
      Serial.println("  ✅ OLED found at address 0x3D!");
      oledConnected = true;
    }
  } else {
    Serial.println("  ✅ OLED initialized successfully at 0x3C!");
    oledConnected = true;
  }
  
  if(oledConnected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("ESP32 I2C Tutorial");
    display.println("BH1750 Light Sensor");
    display.println("Initializing...");
    display.display();
    delay(1000);
  }
}

void setupBH1750() {
  Serial.print("\nInitializing BH1750...");
  
  if(!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23)) {
    Serial.println("\n  Trying address 0x5C...");
    if(!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C)) {
      Serial.println("  ❌ BH1750 NOT FOUND at any address!");
      bh1750Connected = false;
      return;
    } else {
      Serial.println("  ✅ BH1750 found at address 0x5C!");
      bh1750Connected = true;
    }
  } else {
    Serial.println("\n  ✅ BH1750 initialized successfully at 0x23!");
    bh1750Connected = true;
  }
  
  if(bh1750Connected) {
    delay(200);
    Serial.print("  Mode: Continuous High Resolution");
  }
}

void displayOnSerial(float lux, float avgLux) {
  String condition = getLightCondition(lux);
  
  int graphWidth = map(constrain(lux, 0, 1000), 0, 1000, 0, 40);
  
  Serial.println("┌──────────────────────────────────┐");
  Serial.print("│ Time: ");
  Serial.print(millis() / 1000);
  Serial.print("s");
  Serial.print(String(25 - String(millis() / 1000).length(), ' '));
  Serial.println("│");
  
  Serial.print("│ Current: ");
  Serial.print(lux, 1);
  Serial.print(" lx");
  Serial.print(String(22 - String(lux, 1).length(), ' '));
  Serial.println("│");
  
  Serial.print("│ Average: ");
  Serial.print(avgLux, 1);
  Serial.print(" lx");
  Serial.print(String(22 - String(avgLux, 1).length(), ' '));
  Serial.println("│");
  
  Serial.print("│ [");
  for(int i = 0; i < 40; i++) {
    if(i < graphWidth) Serial.print("█");
    else Serial.print(" ");
  }
  Serial.println("] │");
  
  Serial.print("│ Condition: ");
  Serial.print(condition);
  Serial.print(String(20 - condition.length(), ' '));
  Serial.println("│");
  Serial.println("└──────────────────────────────────┘");
}

void displayOnOLED(float lux, float avgLux) {
  String condition = getLightCondition(lux);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.println("Light Sensor BH1750");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setCursor(0,15);
  display.print("Lux: ");
  display.setTextSize(2);
  display.setCursor(0,25);
  display.print(lux, 0);
  display.setTextSize(1);
  display.println(" lx");
  
  int barWidth = map(constrain(lux, 0, 1000), 0, 1000, 0, 120);
  display.fillRect(0, 45, barWidth, 8, SSD1306_WHITE);
  display.drawRect(0, 45, 120, 8, SSD1306_WHITE);
  
  display.setCursor(0,55);
  display.print(condition);
  
  display.display();
}

float calculateAverage(float newValue) {
  luxHistory[historyIndex] = newValue;
  historyIndex = (historyIndex + 1) % 10;
  
  float sum = 0;
  int count = 0;
  for(int i = 0; i < 10; i++) {
    if(luxHistory[i] > 0) {
      sum += luxHistory[i];
      count++;
    }
  }
  
  return (count > 0) ? (sum / count) : newValue;
}

String getLightCondition(float lux) {
  if(lux < 1) return "Pitch Black";
  else if(lux < 10) return "Very Dark";
  else if(lux < 50) return "Dark";
  else if(lux < 200) return "Dim";
  else if(lux < 500) return "Normal";
  else if(lux < 1000) return "Bright";
  else if(lux < 5000) return "Very Bright";
  else if(lux < 10000) return "Direct Sun";
  else return "Extreme Light";
}

void controlOnboardLED(float lux) {
  if(lux < 50) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}