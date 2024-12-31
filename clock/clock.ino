/*

Wiring for Buttons:
Button Pin	    Arduino Nano Pin
Button 1	    D2
Button 2	    D3
GND (common)	GND

The wiring is the same as before since both the RTC and OLED use the I2C bus.

RTC Module Connections:
RTC Pin	Arduino Nano Pin
VCC	5V
GND	GND
SDA	A4
SCL	A5

OLED Connections:
OLED Pin	Arduino Nano Pin
VCC	5V
GND	GND
SDA	A4
SCL	A5

*/
#include <Wire.h>
#include <RTClib.h>
#include <U8g2lib.h>

// Initialize RTC and OLED
RTC_DS3231 rtc;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Button pins
#define INC_BUTTON 2 // Increment button
#define SET_BUTTON 3 // Set/Save button

// Variables for time adjustment
int hour = 12, minute = 0;
bool settingMode = false;

void setup() {
  // Initialize Serial
  Serial.begin(9600);

  // Initialize OLED
  u8g2.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Set buttons as input with pull-down resistors
  pinMode(INC_BUTTON, INPUT_PULLDOWN);
  pinMode(SET_BUTTON, INPUT_PULLDOWN);

  // Uncomment to set RTC initially (dummy time):
  // rtc.adjust(DateTime(2024, 1, 1, 12, 0, 0)); 
}

void loop() {
  // Toggle time-setting mode when SET_BUTTON is pressed
  if (digitalRead(SET_BUTTON) == HIGH) {
    settingMode = !settingMode;
    delay(500); // Debounce delay
  }

  // Handle time adjustment or normal display
  if (settingMode) {
    adjustTime();
  } else {
    displayTime();
  }
}

void adjustTime() {
  // Increment minute and wrap around when button is pressed
  if (digitalRead(INC_BUTTON) == HIGH) {
    minute++;
    if (minute == 60) {
      minute = 0;
      hour++;
      if (hour > 12) hour = 1; // Wrap around for 12-hour format
    }
    delay(200); // Debounce delay
  }

  // Display the time being set
  char buffer[16];
  const char* meridiem = (hour >= 12) ? "PM" : "AM";
  sprintf(buffer, "Set: %02d:%02d %s", hour, minute, meridiem);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(35, 15);
  u8g2.print(buffer);
  u8g2.sendBuffer();

  // Save time to RTC when SET_BUTTON is pressed again
  if (digitalRead(SET_BUTTON) == HIGH) {
    int rtcHour = (hour == 12) ? (meridiem == "AM" ? 0 : 12) : (meridiem == "AM" ? hour : hour + 12);
    rtc.adjust(DateTime(2024, 1, 1, rtcHour, minute, 0)); // Save time to RTC
    settingMode = false;
    delay(500); // Debounce delay
  }
}

void displayTime() {
  // Fetch current time from RTC
  DateTime now = rtc.now();

  // Convert to 12-hour format with AM/PM
  int hour = now.hour();
  const char* meridiem = (hour >= 12) ? "PM" : "AM";
  hour = (hour == 0) ? 12 : (hour > 12 ? hour - 12 : hour);

  // Format the time as HH:MM:SS AM/PM
  char buffer[16];
  sprintf(buffer, "%02d:%02d:%02d %s", hour, now.minute(), now.second(), meridiem);

  // Display the time on the OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(10, 32);
  u8g2.print(buffer);
  u8g2.sendBuffer();

  delay(1000); // Update every second
}
