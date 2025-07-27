#include <Arduino.h>
#include <FastLED.h>

const int testSensorPin = 2;      // GPIO2 input 
const int centerSensorPin = 3;    // GPIO3 input 
const int indicatePin = 21;       // GPIO21 output

#define LED_PIN     20     // GPIO20
#define NUM_LEDS    150    // adjust based on how many you have
#define BRIGHTNESS  64
// #define BRIGHTNESS  12
// #define BRIGHTNESS  25
const uint8_t brightnessLevels[] = {12, 25, 32, 64, 128};
const uint8_t NUM_BRIGHTNESS_LEVELS = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);
uint8_t brightnessIndex = 1; // Start at a reasonable default (e.g., 25)

#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
const unsigned long DYNAMIC_PHASE_DURATION = 300000; // 5 minutes in milliseconds

// Custom color palette: White, Orange, OrangeRed, Red, Magenta, Blue, Cyan, Chartreuse
const CRGB palette[8] = {
  CRGB::White, CRGB::Orange, CRGB::OrangeRed, CRGB::Red,
  CRGB::Magenta, CRGB::Blue, CRGB::Cyan, CRGB::Chartreuse
};

CRGB leds[NUM_LEDS];

// Define excepted pixels at the top for reuse
const int excepted[] = {0, 1, 2, 3, 4, 37};
const int exceptedCount = sizeof(excepted) / sizeof(excepted[0]);

// // Original "circles" definitions:
// Inner circle: last 8 pixels
const int innerCircle[8] = {NUM_LEDS-1, NUM_LEDS-2, NUM_LEDS-3, NUM_LEDS-4, NUM_LEDS-5, NUM_LEDS-6, NUM_LEDS-7, NUM_LEDS-8};
// Circle2: next 8 pixels (clockwise)
const int circle2[8] = {NUM_LEDS-9, NUM_LEDS-11, NUM_LEDS-13, NUM_LEDS-15, NUM_LEDS-17, NUM_LEDS-19, NUM_LEDS-21, NUM_LEDS-23};
// Circle3: cardinals and secondaries (clockwise)
const int circle3_cardinals[8] = {NUM_LEDS-25, NUM_LEDS-26, NUM_LEDS-27, NUM_LEDS-28, NUM_LEDS-29, NUM_LEDS-30, NUM_LEDS-31, NUM_LEDS-32};
const int circle3_secondaries[8] = {NUM_LEDS-10, NUM_LEDS-12, NUM_LEDS-14, NUM_LEDS-16, NUM_LEDS-18, NUM_LEDS-20, NUM_LEDS-22, NUM_LEDS-24};
// Circle 4: alternate cardinals and secondaries (every other pixel)
const int circle4_cardinals[8]   = {NUM_LEDS-34, NUM_LEDS-36, NUM_LEDS-38, NUM_LEDS-40, NUM_LEDS-42, NUM_LEDS-44, NUM_LEDS-46, NUM_LEDS-48};
const int circle4_secondaries[8] = {NUM_LEDS-33, NUM_LEDS-35, NUM_LEDS-37, NUM_LEDS-39, NUM_LEDS-41, NUM_LEDS-43, NUM_LEDS-45, NUM_LEDS-47};
// Circle 5: interlaced thirds (cardinal, splitLeft, splitRight)
const int circle5_cardinals[8]   = {NUM_LEDS-49, NUM_LEDS-52, NUM_LEDS-55, NUM_LEDS-58, NUM_LEDS-61, NUM_LEDS-64, NUM_LEDS-67, NUM_LEDS-70};
const int circle5_splitRight[8]   = {NUM_LEDS-50, NUM_LEDS-53, NUM_LEDS-56, NUM_LEDS-59, NUM_LEDS-62, NUM_LEDS-65, NUM_LEDS-68, NUM_LEDS-71};
const int circle5_splitLeft[8]  = {NUM_LEDS-51, NUM_LEDS-54, NUM_LEDS-57, NUM_LEDS-60, NUM_LEDS-63, NUM_LEDS-66, NUM_LEDS-69, NUM_LEDS-72};
// Circle 6: 40 pixels, 5 subgroups of 8 pixels each
const int circle6_cardinals_in[8]   = {NUM_LEDS-108, NUM_LEDS-103, NUM_LEDS-98, NUM_LEDS-93, NUM_LEDS-88, NUM_LEDS-83, NUM_LEDS-78, NUM_LEDS-73};
const int circle6_cardinals_out[8] = {NUM_LEDS-109, NUM_LEDS-104, NUM_LEDS-99, NUM_LEDS-94, NUM_LEDS-89, NUM_LEDS-84, NUM_LEDS-79, NUM_LEDS-74};
const int circle6_splitRight[8]  = {NUM_LEDS-110, NUM_LEDS-105, NUM_LEDS-100, NUM_LEDS-95, NUM_LEDS-90, NUM_LEDS-85, NUM_LEDS-80, NUM_LEDS-75};
const int circle6_secondaries[8] = {NUM_LEDS-111, NUM_LEDS-106, NUM_LEDS-101, NUM_LEDS-96, NUM_LEDS-91, NUM_LEDS-86, NUM_LEDS-81, NUM_LEDS-76};
const int circle6_splitLeft[8] = {NUM_LEDS-112, NUM_LEDS-107, NUM_LEDS-102, NUM_LEDS-97, NUM_LEDS-92, NUM_LEDS-87, NUM_LEDS-82, NUM_LEDS-77};
// Outer circle: 32 pixels, four subgroups of 8 pixels each
// These cover NUM_LEDS-114 to NUM_LEDS-145 (inclusive), skipping NUM_LEDS-113 (exception pixel 37)
const int outer_cardinals[8]   = {NUM_LEDS-114, NUM_LEDS-118, NUM_LEDS-122, NUM_LEDS-126, NUM_LEDS-130, NUM_LEDS-134, NUM_LEDS-138, NUM_LEDS-142};
const int outer_splitRight[8]  = {NUM_LEDS-115, NUM_LEDS-119, NUM_LEDS-123, NUM_LEDS-127, NUM_LEDS-131, NUM_LEDS-135, NUM_LEDS-139, NUM_LEDS-143};
const int outer_secondaries[8] = {NUM_LEDS-116, NUM_LEDS-120, NUM_LEDS-124, NUM_LEDS-128, NUM_LEDS-132, NUM_LEDS-136, NUM_LEDS-140, NUM_LEDS-144};
const int outer_splitLeft[8]   = {NUM_LEDS-117, NUM_LEDS-121, NUM_LEDS-125, NUM_LEDS-129, NUM_LEDS-133, NUM_LEDS-137, NUM_LEDS-141, NUM_LEDS-145};

// // 8 concentric rings definitions:
// Ring 1: innerCircle
const int ring1[8] = {
  innerCircle[0], innerCircle[1], innerCircle[2], innerCircle[3],
  innerCircle[4], innerCircle[5], innerCircle[6], innerCircle[7]
};

// Ring 2: circle2
const int ring2[8] = {
  circle2[0], circle2[1], circle2[2], circle2[3],
  circle2[4], circle2[5], circle2[6], circle2[7]
};

// Ring 3: circle3_cardinals + circle3_secondaries
const int ring3[16] = {
  circle3_cardinals[0], circle3_cardinals[1], circle3_cardinals[2], circle3_cardinals[3],
  circle3_cardinals[4], circle3_cardinals[5], circle3_cardinals[6], circle3_cardinals[7],
  circle3_secondaries[0], circle3_secondaries[1], circle3_secondaries[2], circle3_secondaries[3],
  circle3_secondaries[4], circle3_secondaries[5], circle3_secondaries[6], circle3_secondaries[7]
};

// Ring 4: circle4_cardinals + circle4_secondaries
const int ring4[16] = {
  circle4_cardinals[0], circle4_cardinals[1], circle4_cardinals[2], circle4_cardinals[3],
  circle4_cardinals[4], circle4_cardinals[5], circle4_cardinals[6], circle4_cardinals[7],
  circle4_secondaries[0], circle4_secondaries[1], circle4_secondaries[2], circle4_secondaries[3],
  circle4_secondaries[4], circle4_secondaries[5], circle4_secondaries[6], circle4_secondaries[7]
};

// Ring 5: circle5_cardinals + circle5_splitLeft + circle5_splitRight
const int ring5[24] = {
  circle5_cardinals[0], circle5_cardinals[1], circle5_cardinals[2], circle5_cardinals[3],
  circle5_cardinals[4], circle5_cardinals[5], circle5_cardinals[6], circle5_cardinals[7],
  circle5_splitLeft[0], circle5_splitLeft[1], circle5_splitLeft[2], circle5_splitLeft[3],
  circle5_splitLeft[4], circle5_splitLeft[5], circle5_splitLeft[6], circle5_splitLeft[7],
  circle5_splitRight[0], circle5_splitRight[1], circle5_splitRight[2], circle5_splitRight[3],
  circle5_splitRight[4], circle5_splitRight[5], circle5_splitRight[6], circle5_splitRight[7]
};

// Ring 6: circle6_cardinals_in + circle6_secondaries
const int ring6[16] = {
  circle6_cardinals_in[0], circle6_cardinals_in[1], circle6_cardinals_in[2], circle6_cardinals_in[3],
  circle6_cardinals_in[4], circle6_cardinals_in[5], circle6_cardinals_in[6], circle6_cardinals_in[7],
  circle6_secondaries[0], circle6_secondaries[1], circle6_secondaries[2], circle6_secondaries[3],
  circle6_secondaries[4], circle6_secondaries[5], circle6_secondaries[6], circle6_secondaries[7]
};

// Ring 7: circle6_cardinals_out, circle6_splitRight, circle6_splitLeft
const int ring7[24] = {
  circle6_cardinals_out[0], circle6_cardinals_out[1], circle6_cardinals_out[2], circle6_cardinals_out[3],
  circle6_cardinals_out[4], circle6_cardinals_out[5], circle6_cardinals_out[6], circle6_cardinals_out[7],
  circle6_splitRight[0], circle6_splitRight[1], circle6_splitRight[2], circle6_splitRight[3],
  circle6_splitRight[4], circle6_splitRight[5], circle6_splitRight[6], circle6_splitRight[7],
  circle6_splitLeft[0], circle6_splitLeft[1], circle6_splitLeft[2], circle6_splitLeft[3],
  circle6_splitLeft[4], circle6_splitLeft[5], circle6_splitLeft[6], circle6_splitLeft[7]
};

// Ring 8: the four outer groups
const int ring8[32] = {
  outer_cardinals[0], outer_cardinals[1], outer_cardinals[2], outer_cardinals[3],
  outer_cardinals[4], outer_cardinals[5], outer_cardinals[6], outer_cardinals[7],
  outer_splitRight[0], outer_splitRight[1], outer_splitRight[2], outer_splitRight[3],
  outer_splitRight[4], outer_splitRight[5], outer_splitRight[6], outer_splitRight[7],
  outer_secondaries[0], outer_secondaries[1], outer_secondaries[2], outer_secondaries[3],
  outer_secondaries[4], outer_secondaries[5], outer_secondaries[6], outer_secondaries[7],
  outer_splitLeft[0], outer_splitLeft[1], outer_splitLeft[2], outer_splitLeft[3],
  outer_splitLeft[4], outer_splitLeft[5], outer_splitLeft[6], outer_splitLeft[7]
};

void setup() {
  pinMode(testSensorPin, INPUT);
  pinMode(centerSensorPin, INPUT);
  pinMode(indicatePin, OUTPUT);
  digitalWrite(indicatePin, HIGH);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightnessLevels[brightnessIndex]);

  // // Set all LEDs to white except excepted ones
  // for (int i = 0; i < NUM_LEDS; i++) {
  //     bool isExcepted = false;
  //     for (int j = 0; j < exceptedCount; j++) {
  //         if (i == excepted[j]) {
  //             isExcepted = true;
  //             break;
  //         }
  //     }
  //     leds[i] = isExcepted ? CRGB::Black : CRGB::White;
  // }

  const int* circles[] = { // Array of the 18 original arrays
    innerCircle,
    circle2,
    circle3_cardinals,
    circle3_secondaries,
    circle4_cardinals,
    circle4_secondaries,
    circle5_cardinals,
    circle5_splitLeft,
    circle5_splitRight,
    circle6_cardinals_in,
    circle6_cardinals_out,
    circle6_splitRight,
    circle6_secondaries,
    circle6_splitLeft,
    outer_cardinals,
    outer_splitRight,
    outer_secondaries,
    outer_splitLeft
  };
  const int numCircles = sizeof(circles) / sizeof(circles[0]);

  // Loop through each group of circles
  for (int group = 0; group < numCircles; group++) { 
    // Use modulo to cycle through the first 5 colors in the palette
    int colorIndex = group % 5; 
    // Fill each circle with the corresponding color from the palette
    for (int i = 0; i < 8; i++) {
      leds[circles[group][i]] = palette[colorIndex];
    }
  }
  
  // Static Mode #1: (MVP - Minimally Viable Mandala)
  // Assign colors to each circle using palette indices
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]]         = palette[0]; // White
    leds[circle2[i]]             = palette[1]; // Orange
    leds[circle3_cardinals[i]]   = palette[1]; // Orange
    leds[circle3_secondaries[i]] = palette[2]; // OrangeRed
    leds[circle4_cardinals[i]]   = palette[2]; // OrangeRed
    leds[circle4_secondaries[i]] = palette[2]; // OrangeRed
    leds[circle5_cardinals[i]]   = palette[2]; // OrangeRed
    leds[circle5_splitLeft[i]]   = palette[3]; // Red
    leds[circle5_splitRight[i]]  = palette[3]; // Red
    leds[circle6_cardinals_in[i]]  = palette[3]; // Red
    leds[circle6_cardinals_out[i]] = palette[3]; // Red
    leds[circle6_splitRight[i]]    = palette[4]; // Magenta
    leds[circle6_secondaries[i]]   = palette[3]; // Red
    leds[circle6_splitLeft[i]]     = palette[4]; // Magenta
    leds[outer_cardinals[i]]      = palette[4]; // Magenta
    leds[outer_splitRight[i]]     = palette[4]; // Magenta
    leds[outer_secondaries[i]]    = palette[4]; // Magenta
    leds[outer_splitLeft[i]]      = palette[4]; // Magenta
  }

  // // Seasonal Wheel:
  // leds[NUM_LEDS-4] = CRGB::White;
  // leds[NUM_LEDS-3] = CRGB::Blue;
  // leds[NUM_LEDS-2] = CRGB::Chartreuse;
  // leds[NUM_LEDS-1] = CRGB::Cyan;
  // leds[NUM_LEDS-8] = CRGB::Orange;
  // leds[NUM_LEDS-7] = CRGB::Magenta;
  // leds[NUM_LEDS-6] = CRGB::OrangeRed;
  // leds[NUM_LEDS-5] = CRGB::Red;

  FastLED.show();
}

unsigned long lastCenterTouch = 0;
bool mandalaActive = true;
bool altMandalaActive = false;

unsigned long lastEffectUpdate = 0;
uint8_t effectOffset = 0;

enum EffectMode {  // Define the effect modes
  STATIC0,         // Static Mode #0: (Candy-corn)
  STATIC1,         // Static Mode #1: (MVP - Minimally Viable Mandala)
  FoxyYB,          // Static Mode #2: (Foxy YellowBlue)
  BlueCardinals,   // Dynamic Mode #3: (Blue Cardinals)
  BREATHING,       // Breathing effect for outer LEDs
  TWINKLE,         // Twinkle effect for random LEDs
  SeasonalWheel,   // Seasonal Color Wheel
  CenterBurst,     // Center burst effect
  RainbowFade,     // Rainbow fade effect
  SpiralFill       // Spiral fill effect
};

const EffectMode allModes[8] = {
  BlueCardinals, // full semi-dynamic
  CenterBurst,   // full dynamic
  BREATHING,     // full dynamic
  SpiralFill,    // full dynamic

  // FoxyYB,        // full static (Foxy YellowBlue)
  // STATIC0,       // full static (Candy-corn)

  STATIC1,       // full static (MVP)
  SeasonalWheel, // mini static 
  TWINKLE,       // mini dynamic
  RainbowFade    // full dynamic
};
const uint8_t NUM_MODES = sizeof(allModes) / sizeof(allModes[0]);
uint8_t modeIndex = 0;

EffectMode currentEffect = STATIC1; // Start with Static Mode #1

bool burstActive = false;           // Center burst effect state
bool showingColorWheel = false;     // Seasonal Color Wheel state
unsigned long colorWheelStart = 0;  // Timer for color wheel animation
bool firstSpiralRun = true;         // Flag to track first run of Spiral Fill effect

void showSeasonalWheel() { // Show the seasonal color wheel
  // All off except inner circle (showing the Seasonal Color Wheel)
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[NUM_LEDS-4] = CRGB::White;
  leds[NUM_LEDS-3] = CRGB::Blue;
  leds[NUM_LEDS-2] = CRGB::Chartreuse;
  leds[NUM_LEDS-1] = CRGB::Cyan;
  leds[NUM_LEDS-8] = CRGB::Orange;
  leds[NUM_LEDS-7] = CRGB::Magenta;
  leds[NUM_LEDS-6] = CRGB::OrangeRed;
  leds[NUM_LEDS-5] = CRGB::Red;
  FastLED.show();
}

void showBreathing() { // Breathing effect for all LEDs except exceptional pixels
  static uint8_t breathBrightness = 25; // Start with a low brightness
  static int8_t breathDirection = 1; // Smoother step
  static unsigned long lastUpdate = 0;

  // Clear all LEDs before applying breathing effect
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Update brightness every 18 ms for smoothness
  if (millis() - lastUpdate > 18) {
    lastUpdate = millis();
    breathBrightness += breathDirection;
    if (breathBrightness == 12 || breathBrightness == 128) breathDirection = -breathDirection;
  }

  // Set all LEDs to the current brightness (breathing blue), except excepted pixels
  for (int i = 0; i < NUM_LEDS; i++) {
    bool isExcepted = false;
    for (int j = 0; j < exceptedCount; j++) {
      if (i == excepted[j]) {
        isExcepted = true;
        break;
      }
    }
    if (!isExcepted) {
      leds[i] = CRGB(0, 0, breathBrightness);
    }
  }
  FastLED.show();
}

void showTwinkle() { // Twinkle effect for random LEDs
  static unsigned long lastTwinkle = 0;
  static uint8_t twinkleCount = 16; // Now 8 twinkles at once
  static int twinkleIndices[16] = {0};

  // Only update every 350 ms for a slower effect
  if (millis() - lastTwinkle > 350) {
    lastTwinkle = millis();

    // Clear all LEDs before twinkling
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Pick new random indices for twinkles
    for (uint8_t i = 0; i < twinkleCount; i++) {
      twinkleIndices[i] = random(NUM_LEDS);
      leds[twinkleIndices[i]] = CRGB(25, 25, 25); // Dim white
    }
    FastLED.show();
  }
}

void showStatic0() { // Static Mode #0: (Candy-corn)
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]] = CRGB::White;
    leds[circle2[i]] = CRGB::OrangeRed;
    leds[circle3_cardinals[i]] = CRGB::Magenta;
    leds[circle3_secondaries[i]] = CRGB::Cyan;
    leds[circle4_cardinals[i]] = CRGB::Orange;
    leds[circle4_secondaries[i]] = CRGB::Red;
    leds[circle5_cardinals[i]] = CRGB::Blue;
    leds[circle5_splitLeft[i]] = CRGB::Chartreuse;
    leds[circle5_splitRight[i]] = CRGB::White;
    leds[circle6_cardinals_in[i]] = CRGB::OrangeRed;
    leds[circle6_cardinals_out[i]] = CRGB::Magenta;
    leds[circle6_splitRight[i]] = CRGB::Cyan;
    leds[circle6_secondaries[i]] = CRGB::Orange;
    leds[circle6_splitLeft[i]] = CRGB::Red;
    leds[outer_cardinals[i]]   = CRGB::Blue;
    leds[outer_splitRight[i]]  = CRGB::Chartreuse;
    leds[outer_secondaries[i]] = CRGB::White;
    leds[outer_splitLeft[i]]   = CRGB::OrangeRed;
  }
  FastLED.show();
}

void showStatic1() { // Static Mode #1: (MVP - Minimally Viable Mandala)
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]]         = palette[0]; // White
    leds[circle2[i]]             = palette[1]; // Orange
    leds[circle3_cardinals[i]]   = palette[1]; // Orange
    leds[circle3_secondaries[i]] = palette[2]; // OrangeRed
    leds[circle4_cardinals[i]]   = palette[2]; // OrangeRed
    leds[circle4_secondaries[i]] = palette[2]; // OrangeRed
    leds[circle5_cardinals[i]]   = palette[2]; // OrangeRed
    leds[circle5_splitLeft[i]]   = palette[3]; // Red
    leds[circle5_splitRight[i]]  = palette[3]; // Red
    leds[circle6_cardinals_in[i]]  = palette[3]; // Red
    leds[circle6_cardinals_out[i]] = palette[3]; // Red
    leds[circle6_splitRight[i]]    = palette[4]; // Magenta
    leds[circle6_secondaries[i]]   = palette[3]; // Red
    leds[circle6_splitLeft[i]]     = palette[4]; // Magenta
    leds[outer_cardinals[i]]      = palette[4]; // Magenta
    leds[outer_splitRight[i]]     = palette[4]; // Magenta
    leds[outer_secondaries[i]]    = palette[4]; // Magenta
    leds[outer_splitLeft[i]]      = palette[4]; // Magenta
  }
  FastLED.show();
}

void showStatic2() { // Static Mode #2: (Foxy YellowBlue)
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]]         = palette[0]; // White
    leds[circle2[i]]             = palette[1]; // Orange
    leds[circle3_cardinals[i]]   = palette[1]; 
    leds[circle3_secondaries[i]] = palette[2]; 
    leds[circle4_cardinals[i]]   = palette[1]; 
    leds[circle4_secondaries[i]] = palette[2]; 
    leds[circle5_cardinals[i]]   = palette[1]; 
    leds[circle5_splitRight[i]]  = palette[6];
    leds[circle5_splitLeft[i]]   = palette[5];    
    leds[circle6_cardinals_in[i]]  = palette[1]; 
    leds[circle6_cardinals_out[i]] = palette[1]; 
    leds[circle6_splitRight[i]]    = palette[6]; 
    leds[circle6_secondaries[i]]   = palette[2]; 
    leds[circle6_splitLeft[i]]     = palette[5]; 
    leds[outer_cardinals[i]]      = palette[1]; 
    leds[outer_splitRight[i]]     = palette[6]; 
    leds[outer_secondaries[i]]    = palette[2]; 
    leds[outer_splitLeft[i]]      = palette[5]; 
  }
  FastLED.show();
}

void showBlueCardinals() { // semi-Static Mode #3: (Blue Cardinals)
  // Static blue cardinals setup
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
  // Dynamic: single magenta pixel cycling through a white ring
  static unsigned long lastEffectUpdateBC = 0;
  static uint8_t effectOffsetBC = 0;
  if (millis() - lastEffectUpdateBC > 150) {
    lastEffectUpdateBC = millis();
    effectOffsetBC = (effectOffsetBC + 1) % 8;
  }
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]] = CRGB::White;
  }
  leds[innerCircle[effectOffsetBC]] = CRGB::Magenta;
  FastLED.show();
}

void showCenterBurst() { // Center burst effect: breathe out and in with separate pauses
  static unsigned long burstStart = 0;
  static int burstStep = 0;
  static bool burstOut = true; // true = expanding, false = contracting
  static bool inPause = false;
  static unsigned long pauseStart = 0;
  static bool pauseAtOuter = false; // true if pausing at outermost, false if at innermost

  const unsigned long expandInterval = 100;      // Faster expansion
  const unsigned long contractInterval = 500;    // Slower contraction
  const unsigned long pauseOuter = 800;          // Pause at outermost
  const unsigned long pauseInner = 500;          // Pause at innermost

  const int* rings[] = { ring1, ring2, ring3, ring4, ring5, ring6, ring7, ring8 };
  const int ringSizes[] = { 8, 8, 16, 16, 24, 16, 24, 32 };
  const uint8_t ringPalette[] = {0, 1, 1, 2, 2, 3, 3, 4}; // MVP palette

  // On first call or after phase reset, initialize
  if (!burstActive) {
    burstActive = true;
    burstStart = millis();
    burstStep = 0;
    burstOut = true;
    inPause = false;
    pauseAtOuter = false;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    return;
  }

  // Handle pause between out and in
  if (inPause) {
    unsigned long pauseInterval = pauseAtOuter ? pauseOuter : pauseInner;
    if (millis() - pauseStart >= pauseInterval) {
      inPause = false;
      burstOut = !burstOut; // Switch direction after pause
      burstStart = millis();
    } else {
      // During contracted pause, show only the innermost ring
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      if (!pauseAtOuter) {
        for (int i = 0; i < ringSizes[0]; i++) {
          leds[rings[0][i]] = palette[ringPalette[0]];
        }
      } else {
        // During expanded pause, show all rings
        for (int r = 0; r < 8; r++) {
          for (int i = 0; i < ringSizes[r]; i++) {
            leds[rings[r][i]] = palette[ringPalette[r]];
          }
        }
      }
      FastLED.show();
      return;
    }
  }

  // Choose interval based on direction
  unsigned long interval = burstOut ? expandInterval : contractInterval;

  if (millis() - burstStart >= interval) {
    burstStart = millis();

    fill_solid(leds, NUM_LEDS, CRGB::Black);

    if (burstOut) {
      // Draw all rings up to current burstStep (expanding)
      for (int r = 0; r <= burstStep; r++) {
        for (int i = 0; i < ringSizes[r]; i++) {
          leds[rings[r][i]] = palette[ringPalette[r]];
        }
      }
    } else {
      // Draw only the inner rings up to burstStep (contracting)
      for (int r = 0; r <= burstStep; r++) {
        for (int i = 0; i < ringSizes[r]; i++) {
          leds[rings[r][i]] = palette[ringPalette[r]];
        }
      }
    }
    FastLED.show();

    // Step logic with pause at each end
    if (burstOut) {
      burstStep++;
      if (burstStep > 7) { // reached outermost, pause before contracting
        burstStep = 7;
        inPause = true;
        pauseAtOuter = true;
        pauseStart = millis();
      }
    } else {
      if (burstStep > 0) {
        burstStep--;
      } else if (burstStep == 0) {
        // Show only ring 1 for one frame, then pause on next tick
        inPause = true;
        pauseAtOuter = false;
        pauseStart = millis();
      }
    }
  }
}

void showRainbowFade() { // Rainbow fade effect across all rings
  static uint8_t baseHue = 0;
  static unsigned long lastUpdate = 0;

  // Update every 20 ms for smooth animation
  if (millis() - lastUpdate > 20) {
    lastUpdate = millis();
    baseHue++;
  }

  // Array of your ring pointers and sizes
  const int* rings[] = { ring1, ring2, ring3, ring4, ring5, ring6, ring7, ring8 };
  const int ringSizes[] = { 8, 8, 16, 16, 24, 16, 24, 32 };

  for (int r = 0; r < 8; r++) {
    uint8_t ringHue = baseHue - r * 32; // Spread hues across rings
    for (int i = 0; i < ringSizes[r]; i++) {
      leds[rings[r][i]] = CHSV(ringHue, 255, 255);
    }
  }
  FastLED.show();
}

void showSpiralFill() { // Spiral fill effect
  static unsigned long lastUpdate = 0;
  static int colorIndex = 0;
  static int pixelIndex = 0;
  
  // Build the spiral order (excluding excepted pixels)
  static int spiralOrder[NUM_LEDS - 6];
  static int spiralLen = 0;
  static bool spiralInit = false;
  if (!spiralInit) {
    // 149 down to 38
    for (int i = 149; i >= 38; i--) {
      bool isExcepted = false;
      for (int j = 0; j < exceptedCount; j++) {
        if (i == excepted[j]) { isExcepted = true; break; }
      }
      if (!isExcepted) spiralOrder[spiralLen++] = i;
    }
    // 36 down to 5
    for (int i = 36; i >= 5; i--) {
      bool isExcepted = false;
      for (int j = 0; j < exceptedCount; j++) {
        if (i == excepted[j]) { isExcepted = true; break; }
      }
      if (!isExcepted) spiralOrder[spiralLen++] = i;
    }
    spiralInit = true;
  }

  // Wipe to black only at the onset of the mode
  if (firstSpiralRun == true) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    firstSpiralRun = false;
    pixelIndex = 0;
    colorIndex = 0;
    return;
  }

  // Animate
  if (millis() - lastUpdate > 50) { // Update spiral pixels every 50 ms
    lastUpdate = millis();

    if (pixelIndex < spiralLen) {
      leds[spiralOrder[pixelIndex]] = palette[colorIndex];
      pixelIndex++;
      FastLED.show();
    } else {
      // Finished this color, move to next color
      colorIndex++;
      if (colorIndex > 6) colorIndex = 0;
      pixelIndex = 0;
      // Do NOT wipe to black, just start drawing next color on top
    }
  }
}

void loop() {
  int testSensorState = digitalRead(testSensorPin);
  int centerSensorState = digitalRead(centerSensorPin);

  if (testSensorState == HIGH) {
    digitalWrite(indicatePin, HIGH);
  } else {
    digitalWrite(indicatePin, LOW);
  }

  // Brightness control
  static bool lastTestState = LOW;
  if (testSensorState == HIGH && lastTestState == LOW) {
      // Cycle to the next brightness level
      brightnessIndex = (brightnessIndex + 1) % NUM_BRIGHTNESS_LEVELS;
      FastLED.setBrightness(brightnessLevels[brightnessIndex]);
      FastLED.show(); // Update LEDs with new brightness
  }
  lastTestState = testSensorState;

  // Example: Change effect mode on center sensor press
  // This will cycle through the effects when the center sensor is touched
  static bool lastCenterState = LOW;
  if (centerSensorState == HIGH && lastCenterState == LOW && !showingColorWheel) {
    showingColorWheel = true;
    colorWheelStart = millis();
    mandalaActive = false;
    altMandalaActive = false;

    // All off except inner circle (show Seasonal Color Wheel)
    showSeasonalWheel();

    // Cycle only through the three dynamic modes
    currentEffect = allModes[modeIndex];
    modeIndex = (modeIndex + 1) % NUM_MODES;
  }
  lastCenterState = centerSensorState;

  // Handle color wheel timing and transition to dynamic mode
  if (showingColorWheel) {
    if (millis() - colorWheelStart >= 1000) {
        showingColorWheel = false;
        altMandalaActive = true;
        lastCenterTouch = millis();
    }
    // Don't run any other effects while showing color wheel
    return;
  }

  // Dynamic effect during alt mandala phase
  static EffectMode lastEffect = STATIC1;
  if (altMandalaActive && millis() - lastCenterTouch <= DYNAMIC_PHASE_DURATION) {
    // If we just switched to SpiralFill, reset the spiral
    if (currentEffect == SpiralFill && lastEffect != SpiralFill) {
      firstSpiralRun = true;
    }

    switch (currentEffect) {
      case STATIC1:        showStatic1(); break;
      case FoxyYB:         showStatic2(); break;
      case STATIC0:        showStatic0(); break;
      case BlueCardinals:  showBlueCardinals(); break;
      case BREATHING:      showBreathing(); break;
      case TWINKLE:        showTwinkle(); break;
      case SeasonalWheel:  showSeasonalWheel(); break;
      case CenterBurst:    showCenterBurst(); break;
      case RainbowFade:    showRainbowFade(); break;
      case SpiralFill:     showSpiralFill(); break;
      default:             showStatic1(); break;
    }
    lastEffect = currentEffect;
  } else {
    burstActive = false; // Reset burst effect state when leaving dynamic phase
    showStatic1(); // Always show Static1 outside altMandala phase
  }

  // After 16 seconds, restore setup() state
  if (!mandalaActive && altMandalaActive && millis() - lastCenterTouch > DYNAMIC_PHASE_DURATION) {
    mandalaActive = true;
    altMandalaActive = false;
    setup(); // Restore MVP mandala state
  }
}
