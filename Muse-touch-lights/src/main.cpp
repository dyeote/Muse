#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     20     // GPIO20
#define NUM_LEDS    150    // adjust based on how many you have
// #define BRIGHTNESS  64
// #define BRIGHTNESS  255
// #define BRIGHTNESS  12
#define BRIGHTNESS  25
// #define BRIGHTNESS  32
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB

CRGB leds[NUM_LEDS];

const int testSensorPin = 2;      // GPIO2 input (was sensorPin)
const int centerSensorPin = 3;    // GPIO3 input (new sensor)
const int indicatePin = 21;       // GPIO21 output

// Define excepted pixels at the top for reuse
const int excepted[] = {0, 1, 2, 3, 4, 37};
const int exceptedCount = sizeof(excepted) / sizeof(excepted[0]);


// Inner circle: last 8 pixels
const int innerCircle[8] = {NUM_LEDS-8, NUM_LEDS-7, NUM_LEDS-6, NUM_LEDS-5, NUM_LEDS-4, NUM_LEDS-3, NUM_LEDS-2, NUM_LEDS-1};

// Circle2: odds in the next 16 pixels before inner circle
const int circle2[8] = {NUM_LEDS-23, NUM_LEDS-21, NUM_LEDS-19, NUM_LEDS-17, NUM_LEDS-15, NUM_LEDS-13, NUM_LEDS-11, NUM_LEDS-9};

// Secondaries: evens in the next 16 pixels before inner circle (subset of circle3)
const int circle3_secondaries[8] = {NUM_LEDS-24, NUM_LEDS-22, NUM_LEDS-20, NUM_LEDS-18, NUM_LEDS-16, NUM_LEDS-14, NUM_LEDS-12, NUM_LEDS-10};

// Cardinals: next 8 after secondaries (subset of circle3, adjust indices as needed)
const int circle3_cardinals[8] = {NUM_LEDS-32, NUM_LEDS-31, NUM_LEDS-30, NUM_LEDS-29, NUM_LEDS-28, NUM_LEDS-27, NUM_LEDS-26, NUM_LEDS-25};

// Circle 4: alternate cardinals and secondaries (every other pixel)
const int circle4_cardinals[8]   = {NUM_LEDS-34, NUM_LEDS-36, NUM_LEDS-38, NUM_LEDS-40, NUM_LEDS-42, NUM_LEDS-44, NUM_LEDS-46, NUM_LEDS-48};
const int circle4_secondaries[8] = {NUM_LEDS-33, NUM_LEDS-35, NUM_LEDS-37, NUM_LEDS-39, NUM_LEDS-41, NUM_LEDS-43, NUM_LEDS-45, NUM_LEDS-47};

// Circle 5: interlaced thirds (cardinal, splitLeft, splitRight, repeat)
const int circle5_cardinals[8]   = {NUM_LEDS-49, NUM_LEDS-52, NUM_LEDS-55, NUM_LEDS-58, NUM_LEDS-61, NUM_LEDS-64, NUM_LEDS-67, NUM_LEDS-70};
const int circle5_splitRight[8]   = {NUM_LEDS-50, NUM_LEDS-53, NUM_LEDS-56, NUM_LEDS-59, NUM_LEDS-62, NUM_LEDS-65, NUM_LEDS-68, NUM_LEDS-71};
const int circle5_splitLeft[8]  = {NUM_LEDS-51, NUM_LEDS-54, NUM_LEDS-57, NUM_LEDS-60, NUM_LEDS-63, NUM_LEDS-66, NUM_LEDS-69, NUM_LEDS-72};

// Circle 6: 40 pixels, 5 subgroups of 8 pixels each
const int circle6_cardinals_in[8]   = {NUM_LEDS-108, NUM_LEDS-103, NUM_LEDS-98, NUM_LEDS-93, NUM_LEDS-88, NUM_LEDS-83, NUM_LEDS-78, NUM_LEDS-73};
const int circle6_cardinals_out[8] = {NUM_LEDS-109, NUM_LEDS-104, NUM_LEDS-99, NUM_LEDS-94, NUM_LEDS-89, NUM_LEDS-84, NUM_LEDS-79, NUM_LEDS-74};
const int circle6_splitRight[8]  = {NUM_LEDS-110, NUM_LEDS-105, NUM_LEDS-100, NUM_LEDS-95, NUM_LEDS-90, NUM_LEDS-85, NUM_LEDS-80, NUM_LEDS-75};
const int circle6_secondaries[8] = {NUM_LEDS-111, NUM_LEDS-106, NUM_LEDS-101, NUM_LEDS-96, NUM_LEDS-91, NUM_LEDS-86, NUM_LEDS-81, NUM_LEDS-76};
const int circle6_splitLeft[8] = {NUM_LEDS-112, NUM_LEDS-107, NUM_LEDS-102, NUM_LEDS-97, NUM_LEDS-92, NUM_LEDS-87, NUM_LEDS-82, NUM_LEDS-77};

// Outer circle: 32 pixels, four subgroups of 8 pixels each (no repeats)
// These cover NUM_LEDS-114 to NUM_LEDS-145 (inclusive), skipping NUM_LEDS-113 (exception pixel 37)
const int outer_cardinals[8]   = {NUM_LEDS-114, NUM_LEDS-118, NUM_LEDS-122, NUM_LEDS-126, NUM_LEDS-130, NUM_LEDS-134, NUM_LEDS-138, NUM_LEDS-142};
const int outer_splitRight[8]  = {NUM_LEDS-115, NUM_LEDS-119, NUM_LEDS-123, NUM_LEDS-127, NUM_LEDS-131, NUM_LEDS-135, NUM_LEDS-139, NUM_LEDS-143};
const int outer_secondaries[8] = {NUM_LEDS-116, NUM_LEDS-120, NUM_LEDS-124, NUM_LEDS-128, NUM_LEDS-132, NUM_LEDS-136, NUM_LEDS-140, NUM_LEDS-144};
const int outer_splitLeft[8]   = {NUM_LEDS-117, NUM_LEDS-121, NUM_LEDS-125, NUM_LEDS-129, NUM_LEDS-133, NUM_LEDS-137, NUM_LEDS-141, NUM_LEDS-145};

void setup() {
    pinMode(testSensorPin, INPUT);
    pinMode(centerSensorPin, INPUT);
    pinMode(indicatePin, OUTPUT);
    digitalWrite(indicatePin, HIGH);

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    // Set all LEDs to white except excepted ones
    for (int i = NUM_LEDS-32; i < NUM_LEDS; i++) {
        bool isExcepted = false;
        for (int j = 0; j < exceptedCount; j++) {
            if (i == excepted[j]) {
                isExcepted = true;
                break;
            }
        }
        leds[i] = isExcepted ? CRGB::Black : CRGB::White;
    }

for (int i = 0; i < 8; i++) {
  leds[innerCircle[i]] = CRGB::White;
  leds[circle2[i]] = CRGB::Orange;

  leds[circle3_cardinals[i]] = CRGB::Orange;
  leds[circle3_secondaries[i]] = CRGB::OrangeRed;
  leds[circle4_cardinals[i]] = CRGB::OrangeRed;
  leds[circle4_secondaries[i]] = CRGB::OrangeRed;

  leds[circle5_cardinals[i]] = CRGB::OrangeRed;
  leds[circle5_splitLeft[i]] = CRGB::Red;
  leds[circle5_splitRight[i]] = CRGB::Red;
  leds[circle6_cardinals_in[i]] = CRGB::Red;

  leds[circle6_cardinals_out[i]] = CRGB::Red;
  leds[circle6_splitRight[i]] = CRGB::Magenta;
  leds[circle6_secondaries[i]] = CRGB::Red;
  leds[circle6_splitLeft[i]] = CRGB::Magenta;

  leds[outer_cardinals[i]]   = CRGB::Magenta;
  leds[outer_splitRight[i]]  = CRGB::Magenta;
  leds[outer_secondaries[i]] = CRGB::Magenta;
  leds[outer_splitLeft[i]]   = CRGB::Magenta;
}

// leds[NUM_LEDS-4] = CRGB::White;
// leds[NUM_LEDS-3] = CRGB::Blue;
// leds[NUM_LEDS-2] = CRGB::Chartreuse;
// leds[NUM_LEDS-1] = CRGB::Cyan;
// leds[NUM_LEDS-8] = CRGB::Orange;
// leds[NUM_LEDS-7] = CRGB::Magenta;
// leds[NUM_LEDS-6] = CRGB::OrangeRed;
// leds[NUM_LEDS-5] = CRGB::Red;

    // int TEMP = NUM_LEDS-112;
    // leds[TEMP-1] = CRGB::Green;
    // leds[TEMP-2] = CRGB::OrangeRed;
    // leds[TEMP-3] = CRGB::White;
    // leds[TEMP-4] = CRGB::Blue;
    // leds[TEMP-5] = CRGB::Yellow;

    FastLED.show();
}


unsigned long lastCenterTouch = 0;
bool mandalaActive = true;
bool altMandalaActive = false;

unsigned long lastEffectUpdate = 0;
uint8_t effectOffset = 0;

void loop() {
  int testSensorState = digitalRead(testSensorPin);
  int centerSensorState = digitalRead(centerSensorPin);

  if (testSensorState == HIGH) {
    digitalWrite(indicatePin, HIGH);
  } else {
    digitalWrite(indicatePin, LOW);
  }

  // If center sensor is touched, show inner color circle for 1 sec, then alt mandala for 9 sec, then restore setup
  if (centerSensorState == HIGH && mandalaActive) {
    mandalaActive = false;
    altMandalaActive = false;
    lastCenterTouch = millis();

    // All off except inner circle (show color wheel)
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int i = 0; i < 8; i++) {
      leds[innerCircle[i]] = CRGB::Magenta;
    }

    leds[NUM_LEDS-4] = CRGB::White;
    leds[NUM_LEDS-3] = CRGB::Blue;
    leds[NUM_LEDS-2] = CRGB::Chartreuse;
    leds[NUM_LEDS-1] = CRGB::Cyan;
    leds[NUM_LEDS-8] = CRGB::Orange;
    leds[NUM_LEDS-7] = CRGB::Magenta;
    leds[NUM_LEDS-6] = CRGB::OrangeRed;
    leds[NUM_LEDS-5] = CRGB::Red;

    FastLED.show();
    delay(1000);

    // Show all pixels on with alternate color choices
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int i = 0; i < 8; i++) {
      leds[innerCircle[i]] = CRGB::Blue;
      leds[circle2[i]] = CRGB::Blue;

      leds[circle3_cardinals[i]] = CRGB::Blue;
      leds[circle3_secondaries[i]] = CRGB::OrangeRed;
      leds[circle4_cardinals[i]] = CRGB::Blue;
      leds[circle4_secondaries[i]] = CRGB::OrangeRed;

      leds[circle5_cardinals[i]] = CRGB::Blue;
      leds[circle5_splitLeft[i]] = CRGB::Red;
      leds[circle5_splitRight[i]] = CRGB::Red;
      leds[circle6_cardinals_in[i]] = CRGB::Blue;

      leds[circle6_cardinals_out[i]] = CRGB::Blue;
      leds[circle6_splitRight[i]] = CRGB::Magenta;
      leds[circle6_secondaries[i]] = CRGB::Red;
      leds[circle6_splitLeft[i]] = CRGB::Magenta;

      leds[outer_cardinals[i]]   = CRGB::Blue;
      leds[outer_splitRight[i]]  = CRGB::Magenta;
      leds[outer_secondaries[i]] = CRGB::Magenta;
      leds[outer_splitLeft[i]]   = CRGB::Magenta;
    }

    FastLED.show();
    altMandalaActive = true;
    lastCenterTouch = millis();
  }
    
  // Dynamic effect during alt mandala phase
  if (altMandalaActive && (millis() - lastCenterTouch <= 9000)) {
      if (millis() - lastEffectUpdate > 30) { // update more frequently for smoother fade
          lastEffectUpdate = millis();
          effectOffset++;
          // Define the color wheel for the inner circle
          CRGB colors[8] = {
              CRGB::White, CRGB::Blue, CRGB::Chartreuse, CRGB::Cyan,
              CRGB::Orange, CRGB::Magenta, CRGB::OrangeRed, CRGB::Red
          };
          for (int i = 0; i < 8; i++) {
              // Blend current color toward the target color
              nblend(leds[innerCircle[i]], colors[(i + effectOffset) % 8], 32); // 32/255 = ~12% per frame
          }
          FastLED.show();
      }
  }
    // leds[NUM_LEDS-4] = CRGB::White;
    // leds[NUM_LEDS-3] = CRGB::Blue;
    // leds[NUM_LEDS-2] = CRGB::Chartreuse;
    // leds[NUM_LEDS-1] = CRGB::Cyan;
    // leds[NUM_LEDS-8] = CRGB::Orange;
    // leds[NUM_LEDS-7] = CRGB::Magenta;
    // leds[NUM_LEDS-6] = CRGB::OrangeRed;
    // leds[NUM_LEDS-5] = CRGB::Red;

  // After 9 seconds, restore setup() state
  if (!mandalaActive && altMandalaActive && millis() - lastCenterTouch > 9000) {
    mandalaActive = true;
    altMandalaActive = false;
    setup(); // Restore original mandala state
  }
}
