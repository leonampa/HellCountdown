// HellCountdown
// A small, portable countdown device, to watch the minutes drain away in class.
// Also useful for classroom time management for teachers :)
//
// https://fabricate.hackclub.com/projects/218
// https://github.com/leonampa/HellCountdown

#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pins
constexpr int SCR_SDA  = 1;
constexpr int SCR_SCL  = 2;
constexpr int PCF_INT  = 4;
constexpr int PCF_SCL  = 5;
constexpr int PCF_SDA  = 6;
constexpr int SW_LEFT  = 11; // SW2
constexpr int SW_RIGHT = 15; // SW3

constexpr uint32_t SCREEN_TIMEOUT_MS = 5000; // how long the screen stays on before sleeping
constexpr uint32_t PICKER_IDLE_MS = 10000; // picker gives up after this long without a press
constexpr uint32_t DEBOUNCE_MS = 30;

constexpr int      PICK_MIN         = 1;
constexpr int      PICK_MAX         = 99;
constexpr uint32_t REPEAT_DELAY_MS  = 400;   // hold this long before repeating starts
constexpr uint32_t REPEAT_MS        = 120;   // then repeat at this interval

constexpr uint32_t BOTH_WINDOW_MS = 250;   // two buttons within this time count as "together"

uint32_t customEndMs  = 0;    // when the custom countdown ends
int      lastDelta    = 0;    // last change the picker made (+1, -1 or 0)...
uint32_t lastChangeAt = 0;    // ...and when, so we can undo it
uint32_t lastTick     = 0; 

// Wi-Fi
constexpr const char* WIFI_SSID = "your network name";
constexpr const char* WIFI_PASS = "your password";
constexpr int      WIFI_TRIES  = 3;
constexpr uint32_t WIFI_TRY_MS = 10000;   // give each attempt 10 s

// Display
constexpr int SCR_WIDTH  = 128;
constexpr int SCR_HEIGHT = 32;
constexpr uint8_t OLED_ADDR = 0x3C;

// RTC
constexpr const char* TZ_GREECE = "EET-2EEST,M3.5.0/3,M10.5.0/4";
constexpr uint32_t WIFI_TIMEOUT_MS = 15000;
constexpr uint32_t NTP_TIMEOUT_MS  = 10000;

Adafruit_SSD1306 display(SCR_WIDTH, SCR_HEIGHT, &Wire, -1);
RTC_PCF8563 rtc;

struct Button {
  uint8_t  pin;
  bool     raw    = false;   // latest reading (true = pressed)
  bool     stable = false;   // debounced state
  uint32_t since  = 0;       // when 'raw' last changed
  uint32_t nextRepeat = 0;   // when a held button may fire again
};

Button left{SW_LEFT};
Button right{SW_RIGHT};

enum ScreenState { SCREEN_SLEEP, SCREEN_AWAKE, SCREEN_LOCKED };
ScreenState screenState = SCREEN_AWAKE; // we start the day awake
uint32_t awakeSince = 0; // when it last woke up (for the timeout)


enum Mode { MODE_TIMETABLE, MODE_PICKER, MODE_CUSTOM };
Mode mode = MODE_TIMETABLE;
int pickerMinutes = 10; // what the picker shows (remembered between uses)
uint32_t pickerLastPress = 0;

bool pressed(Button& b) {
  bool reading = (digitalRead(b.pin) == LOW); // pull-up: LOW means pressed
  if (reading != b.raw) { // reading changed
    b.raw = reading;
    b.since = millis();
  }
  if (millis() - b.since >= DEBOUNCE_MS && b.stable != b.raw) {
    b.stable = b.raw; // it stayed the same long enough: believe it
    return b.stable; // true on press, false on release
  }
  return false;
}

int32_t customRemainingMs() {
  return (int32_t)(customEndMs - millis());
}

bool connectWifi() {
  for (int attempt = 1; attempt <= WIFI_TRIES; attempt++) {
    Serial.printf("Wi-Fi attempt %d/%d\n", attempt, WIFI_TRIES);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TRY_MS) delay(250);
    if (WiFi.status() == WL_CONNECTED) return true;
    WiFi.disconnect();                     // reset before trying again
  }
  return false;                            // all attempts failed
}

// true on the press itself, then again and again while the button is held down
bool pressedOrRepeat(Button& b, bool justPressed) {
  if (justPressed) { // the press itself
    b.nextRepeat = millis() + REPEAT_DELAY_MS;
    return true;
  }
  if (b.stable && millis() >= b.nextRepeat) { // still held
    b.nextRepeat = millis() + REPEAT_MS;
    return true;
  }
  return false;
}

const char* BELLS[] = {  // The actual times the bells go off (to count down to)
  "08:15","09:00","09:55","10:50","11:45","12:35","13:25","14:10",
  "15:15","16:00","16:50","17:45","18:40","19:35","20:30","21:25","22:15"
};

int toMinutes(const char* s) { // turned into minutes since midnight
  return atoi(s) * 60 + atoi(s + 3);
}

int secondsToNextBell(const tm& t) { // Seconds until the next bell, or -1 if there are no more bells today
  int now = t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;// seconds since midnight
  for (const char* b : BELLS) {
    int bell = toMinutes(b) * 60;
    if (bell > now) return bell - now; // first bell still ahead of us
  }
  return -1;
}

void showText(const char* s) { // showing the countdown
  display.clearDisplay();
  display.setTextSize(4); // 4x the built-in font
  display.setTextColor(SSD1306_WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);   // how big will this text be?

  int x = (SCR_WIDTH  - (w + 1)) / 2 - x1;
  int y = (SCR_HEIGHT - h) / 2 - y1;

  display.setCursor(x, y);     
  display.print(s);   // draw once...
  display.setCursor(x + 1, y); 
  display.print(s);   // ...and twice (1 pixel to the right, to make it look bold
  display.display();
}

void setScreen(ScreenState s) { // sleeping
  screenState = s;
  if (s == SCREEN_SLEEP) 
    display.ssd1306_command(SSD1306_DISPLAYOFF);   // panel off
  else
    display.ssd1306_command(SSD1306_DISPLAYON);    // panel on

  Serial.print("screen: "); // debug, will remove
//  Serial.println(s == SCREEN_SLEEP ? "SLEEP" : s == SCREEN_AWAKE ? "AWAKE" : "LOCKED"); (DEBUG)

void showPicker() { // time picker for the custom timer (RB)
  char buf[8];
  snprintf(buf, sizeof buf, "%dm", pickerMinutes);   // e.g. "10m"
  showText(buf);
}
// ---------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(SW_LEFT,  INPUT_PULLUP);
  pinMode(SW_RIGHT, INPUT_PULLUP);
  pinMode(PCF_INT,  INPUT_PULLUP);
  Wire.begin(SCR_SDA, SCR_SCL);    // OLED
  Wire.setClock(400000);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  Wire1.begin(PCF_SDA, PCF_SCL);   // PCF

  rtc.begin(&Wire1);
  if (rtc.lostPower()) { // if we don't know the time
    WiFi.begin(WIFI_SSID, WIFI_PASS); // connect to WiFi
    while (WiFi.status() != WL_CONNECTED) delay(250);

    configTime(0, 0, "pool.ntp.org"); // ask for the time
    struct tm t;
    getLocalTime(&t);
    rtc.adjust(DateTime((uint32_t)time(nullptr))); // set it
//    Serial.println("CURRENT TIME (UTC): "); // say it
//    Serial.println(rtc.now().timestamp());  
    WiFi.mode(WIFI_OFF); // and switch the WiFi off
  }
//  Serial.println(rtc.now().timestamp());
  struct timeval tv = { (time_t)rtc.now().unixtime(), 0 };
  settimeofday(&tv, nullptr);
  setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1); // timezone
  tzset(); 
  awakeSince = millis(); // first timeout counts from boot
  }

// ---------------------------------------------------------------------

void loop() {
  bool leftPressed  = pressed(left);
  bool rightPressed = pressed(right);

  if (mode == MODE_PICKER) {
    if (left.stable && right.stable) {                 // both buttons down: SET
      if (millis() - lastChangeAt < BOTH_WINDOW_MS) {
        pickerMinutes -= lastDelta;                    // undo the stray tap from the first button
      }
      customEndMs = millis() + (uint32_t)pickerMinutes * 60000UL;
      mode = MODE_CUSTOM;
      awakeSince = millis();
      lastTick = 0;                                    // redraw right away
      Serial.printf("mode: CUSTOM (%d min)\n", pickerMinutes);
    } else {
      bool minus = pressedOrRepeat(left,  leftPressed);
      bool plus  = pressedOrRepeat(right, rightPressed);

      if (minus || plus) {
        int before = pickerMinutes;
        pickerLastPress = millis();
        if (minus) pickerMinutes = max(PICK_MIN, pickerMinutes - 1);
        if (plus)  pickerMinutes = min(PICK_MAX, pickerMinutes + 1);
        lastDelta = pickerMinutes - before;            // what actually changed
        lastChangeAt = millis();
        showPicker();
//        Serial.println(pickerMinutes);
      }

      if (millis() - pickerLastPress >= PICKER_IDLE_MS) {   // nobody touched it: give up
        mode = MODE_TIMETABLE;
        awakeSince = millis();
        lastTick = 0;
//        Serial.println("mode: TIMETABLE");
      }
    }
  }
  else {                                               // TIMETABLE or CUSTOM
    if (leftPressed) {                                 // wake / lock, same in both
      switch (screenState) {
        case SCREEN_SLEEP:  setScreen(SCREEN_AWAKE);  awakeSince = millis(); break;
        case SCREEN_AWAKE:  setScreen(SCREEN_LOCKED); break;
        case SCREEN_LOCKED: setScreen(SCREEN_AWAKE);  awakeSince = millis(); break;
      }
    }
    if (rightPressed) {
      if (mode == MODE_TIMETABLE) {                    // open the picker
        mode = MODE_PICKER;
        if (screenState == SCREEN_SLEEP) setScreen(SCREEN_AWAKE);
        pickerLastPress = millis();
        lastDelta = 0;
        showPicker();
//        Serial.println("mode: PICKER");
        right.nextRepeat = 0xFFFFFFFF;                 // still holding right: don't count as '+'
      } else {                                         // right during a custom countdown: cancel
        mode = MODE_TIMETABLE;
        lastTick = 0;
//        Serial.println("custom cancelled -> TIMETABLE");
      }
    }
    if (mode == MODE_CUSTOM && customRemainingMs() <= 0) {   // ran out
      mode = MODE_TIMETABLE;
      lastTick = 0;
//      Serial.println("custom finished -> TIMETABLE");
    }
  }

  // screen sleep timeout: not while picking
  if (mode != MODE_PICKER && screenState == SCREEN_AWAKE &&
      millis() - awakeSince >= SCREEN_TIMEOUT_MS) {
    setScreen(SCREEN_SLEEP);
  }

  // once per second: the countdown (timetable or custom)
  if (mode != MODE_PICKER && screenState != SCREEN_SLEEP && millis() - lastTick >= 1000) {
    lastTick = millis();

    int secsLeft;
    if (mode == MODE_CUSTOM) {
      secsLeft = (customRemainingMs() + 999) / 1000;   // round up
    } else {
      struct tm t;
      getLocalTime(&t, 0);
      secsLeft = secondsToNextBell(t);
    }

    char buf[8];
    if (secsLeft < 0 || secsLeft >= 100 * 60) snprintf(buf, sizeof buf, "--:--");
    else snprintf(buf, sizeof buf, "%02d:%02d", secsLeft / 60, secsLeft % 60);

//    Serial.println(buf);
    showText(buf);
  }
}