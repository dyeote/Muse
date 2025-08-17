#include <Arduino.h>
#include <FastLED.h>
// #include <time.h>  // Add this for time functions

const int edgeSensorPin = 4;     // GPIO4 input (new)
const int adminSensorPin = 2;    // GPIO2 input
const int centerSensorPin = 3;   // GPIO7 input (swapped)
const int freeSensorPin = 7;     // GPIO3 input (swapped)

const int indicateYellow = 8;     // GPIO8 output (yellow indicator)
const int indicateWhite = 9;      // GPIO9 output (white indicator) 
const int indicateBlue = 10;      // GPIO10 output (blue indicator)
const int indicateGreen = 20;     // GPIO21 output (green indicator, renamed from indicatePin)

#define LED_PIN     21     // GPIO20 output pin for addressable LEDs
#define NUM_LEDS    184    // Updated total LED count; was 150 in v1 with numbers 0,1,2,3,4,37 unused
#define BRIGHTNESS  64
// #define BRIGHTNESS  12
// #define BRIGHTNESS  25
const uint8_t brightnessLevels[] = {12, 25, 64, 128, 192}; // 5%, 10%, 25%, 50%, 75%
const uint8_t NUM_BRIGHTNESS_LEVELS = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);
uint8_t brightnessIndex = 0; // Start at a reasonable default (e.g., 64)

#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
// const unsigned long DYNAMIC_PHASE_DURATION = 300000; // 5 minutes in milliseconds
const unsigned long DYNAMIC_PHASE_DURATION = 3000; // 3 seconds in milliseconds

// Muse palette: White, Orange(Yellow), OrangeRed(Orange), Red, Magenta, Blue, Cyan, Chartreuse
const CRGB palette[8] = {
  CRGB::White, CRGB::Orange, CRGB::OrangeRed, CRGB::Red,
  CRGB::Magenta, CRGB::Blue, CRGB::Cyan, CRGB::Chartreuse
};
// WotY palette: white Winter, blue Imbloc, chartreuse Spring, cyan Beltane,
//               yellow Summer, magenta Lammas, orange Autumn, red Samhain.
const CRGB yearPalette[8] = {
  CRGB::White, CRGB::Blue, CRGB::Chartreuse, CRGB::Cyan,
  CRGB::Orange, CRGB::Magenta, CRGB::OrangeRed, CRGB::Red
};

CRGB leds[NUM_LEDS];

// [[deprecated]] Define excepted pixels - now none are excepted (all 184 are used)
// const int excepted[] = {};
// const int exceptedCount = sizeof(excepted) / sizeof(excepted[0]);

// // Define BGR LEDs (first 34 LEDs use BGR order)
// const int bgrLEDs[] = {};
// const int bgrCount = sizeof(bgrLEDs) / sizeof(bgrLEDs[0]);

// // Helper function to set LED color with BGR compensation
// void setLEDColor(int index, CRGB color) {
//   // Check if this LED uses BGR color order
//   bool isBGR = false;
//   for (int i = 0; i < bgrCount; i++) {
//     if (index == bgrLEDs[i]) {
//       isBGR = true;
//       break;
//     }
//   }
  
//   if (isBGR) {
//     // Swap red and blue channels for BGR LEDs
//     leds[index] = CRGB(color.b, color.g, color.r);
//   } else {
//     // Normal RGB order
//     leds[index] = color;
//   }
// }

// Original "circles" definitions:
// Inner circle: last 8 pixels
const int innerCircle[8] = {NUM_LEDS-2, NUM_LEDS-3, NUM_LEDS-4, NUM_LEDS-5,
                            NUM_LEDS-6, NUM_LEDS-7, NUM_LEDS-8, NUM_LEDS-1};
// Circle2: next 8 pixels (clockwise) 
const int circle2[8] = {NUM_LEDS-11, NUM_LEDS-13, NUM_LEDS-15, NUM_LEDS-17,
                        NUM_LEDS-19, NUM_LEDS-21, NUM_LEDS-23, NUM_LEDS-9};
// Circle3: cardinals and secondaries (clockwise)
const int circle3_cardinals[8] = {NUM_LEDS-26, NUM_LEDS-27, NUM_LEDS-28, NUM_LEDS-29,
                                  NUM_LEDS-30, NUM_LEDS-31, NUM_LEDS-32, NUM_LEDS-25};
const int circle3_secondaries[8] = {NUM_LEDS-12, NUM_LEDS-14, NUM_LEDS-16, NUM_LEDS-18,
                                    NUM_LEDS-20, NUM_LEDS-22, NUM_LEDS-24, NUM_LEDS-10};
// // Circle 4: alternate cardinals and secondaries (every other pixel)
const int circle4_cardinals[8] = {NUM_LEDS-36, NUM_LEDS-38, NUM_LEDS-40, NUM_LEDS-42,
                                  NUM_LEDS-44, NUM_LEDS-46, NUM_LEDS-48, NUM_LEDS-34};
const int circle4_secondaries[8] = {NUM_LEDS-37, NUM_LEDS-39, NUM_LEDS-41, NUM_LEDS-43,
                                    NUM_LEDS-45, NUM_LEDS-47, NUM_LEDS-33, NUM_LEDS-35};
// Circle 5: interlaced thirds (cardinal, splitLeft, splitRight)
const int circle5_cardinals[8] = {NUM_LEDS-55, NUM_LEDS-58, NUM_LEDS-61, NUM_LEDS-64,
                                  NUM_LEDS-67, NUM_LEDS-70, NUM_LEDS-49, NUM_LEDS-52};
const int circle5_splitRight[8] = {NUM_LEDS-56, NUM_LEDS-59, NUM_LEDS-62, NUM_LEDS-65,
                                   NUM_LEDS-68, NUM_LEDS-71, NUM_LEDS-50, NUM_LEDS-53};
const int circle5_splitLeft[8] = {NUM_LEDS-57, NUM_LEDS-60, NUM_LEDS-63, NUM_LEDS-66,
                                  NUM_LEDS-69, NUM_LEDS-72, NUM_LEDS-51, NUM_LEDS-54};
// Circle 6: 40 pixels in 5 subgroups of 8 pixels each
const int circle6_cardinals_in[8] = {NUM_LEDS-83, NUM_LEDS-88, NUM_LEDS-93, NUM_LEDS-98,
                                     NUM_LEDS-103, NUM_LEDS-108, NUM_LEDS-73, NUM_LEDS-78};
const int circle6_secondaries[8] = {NUM_LEDS-86, NUM_LEDS-91, NUM_LEDS-96, NUM_LEDS-101,
                                    NUM_LEDS-106, NUM_LEDS-111, NUM_LEDS-76, NUM_LEDS-81};                                     
const int circle6_cardinals_out[8] = {NUM_LEDS-84, NUM_LEDS-89, NUM_LEDS-94, NUM_LEDS-99,
                                      NUM_LEDS-104, NUM_LEDS-109, NUM_LEDS-74, NUM_LEDS-79};
const int circle6_splitRight[8] = {NUM_LEDS-85, NUM_LEDS-90, NUM_LEDS-95, NUM_LEDS-100,
                                   NUM_LEDS-105, NUM_LEDS-110, NUM_LEDS-75, NUM_LEDS-80};
const int circle6_splitLeft[8] = {NUM_LEDS-87, NUM_LEDS-92, NUM_LEDS-97, NUM_LEDS-102,
                                  NUM_LEDS-107, NUM_LEDS-112, NUM_LEDS-77, NUM_LEDS-82};
// Circle 7: new penultimate circle
const int circle7_cardinals[8] = {52, 43, 34, 25, 16, 7, 70, 61};
const int circle7_splitRight[8] = {50, 41, 32, 23, 14, 5, 68, 59};
const int circle7_secondRight[8] = {48, 39, 30, 21, 12, 3, 66, 57};
const int circle7_secondLeft[8] = {46, 37, 28, 19, 10, 1, 64, 55};
const int circle7_splitLeft[8] = {44, 35, 26, 17, 8, 71, 62, 53};
// Circle 8: new outermost circle
const int circle8_cardinals[8] = {51, 42, 33, 24, 15, 6, 69, 60};
const int circle8_secondRight[8] = {49, 40, 31, 22, 13, 4, 67, 58};
const int circle8_secondaries[8] = {47, 38, 29, 20, 11, 2, 65, 56};
const int circle8_secondLeft[8] = {45, 36, 27, 18, 9, 0, 63, 54};

const int* circles[] = { // Array of pointers to the 23 circle arrays
  innerCircle,
  circle2,
  circle3_cardinals,
  circle3_secondaries,    
  circle4_cardinals,
  circle4_secondaries,
  circle5_cardinals,
  circle5_splitRight,
  circle5_splitLeft,
  
  circle6_cardinals_in,
  circle6_secondaries,
  circle6_cardinals_out,
  circle6_splitRight,
  circle6_splitLeft,

  circle7_cardinals,
  circle7_splitRight,
  circle7_secondRight,
  circle7_secondLeft,
  circle7_splitLeft,

  circle8_cardinals,
  circle8_secondRight,
  circle8_secondaries,
  circle8_secondLeft
};
const int numCircles = sizeof(circles) / sizeof(circles[0]);

// 9 concentric rings definitions:
const int ring1[8] = { // Ring 1: innerCircle
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
  circle3_cardinals[0],   circle3_secondaries[0],
  circle3_cardinals[1],   circle3_secondaries[1],
  circle3_cardinals[2],   circle3_secondaries[2],
  circle3_cardinals[3],   circle3_secondaries[3],
  circle3_cardinals[4],   circle3_secondaries[4],
  circle3_cardinals[5],   circle3_secondaries[5],
  circle3_cardinals[6],   circle3_secondaries[6],
  circle3_cardinals[7],   circle3_secondaries[7]
};
// Ring 4: circle4_cardinals + circle4_secondaries
const int ring4[16] = {
  circle4_cardinals[0],   circle4_secondaries[0],
  circle4_cardinals[1],   circle4_secondaries[1],
  circle4_cardinals[2],   circle4_secondaries[2],
  circle4_cardinals[3],   circle4_secondaries[3],
  circle4_cardinals[4],   circle4_secondaries[4],
  circle4_cardinals[5],   circle4_secondaries[5],
  circle4_cardinals[6],   circle4_secondaries[6],
  circle4_cardinals[7],   circle4_secondaries[7]
};
// Ring 5: circle5_cardinals + circle5_splitRight + circle5_splitLeft
const int ring5[24] = {
  circle5_cardinals[0],   circle5_splitRight[0],   circle5_splitLeft[0],
  circle5_cardinals[1],   circle5_splitRight[1],   circle5_splitLeft[1],
  circle5_cardinals[2],   circle5_splitRight[2],   circle5_splitLeft[2],
  circle5_cardinals[3],   circle5_splitRight[3],   circle5_splitLeft[3],
  circle5_cardinals[4],   circle5_splitRight[4],   circle5_splitLeft[4],
  circle5_cardinals[5],   circle5_splitRight[5],   circle5_splitLeft[5],
  circle5_cardinals[6],   circle5_splitRight[6],   circle5_splitLeft[6],
  circle5_cardinals[7],   circle5_splitRight[7],   circle5_splitLeft[7]
};
// Ring 6: circle6_cardinals_in + circle6_secondaries
const int ring6[16] = {
  circle6_cardinals_in[0], circle6_secondaries[0],
  circle6_cardinals_in[1], circle6_secondaries[1],
  circle6_cardinals_in[2], circle6_secondaries[2],
  circle6_cardinals_in[3], circle6_secondaries[3],
  circle6_cardinals_in[4], circle6_secondaries[4],
  circle6_cardinals_in[5], circle6_secondaries[5],
  circle6_cardinals_in[6], circle6_secondaries[6],
  circle6_cardinals_in[7], circle6_secondaries[7]
};
// Ring 7: circle6_cardinals_out, circle6_splitRight, circle6_splitLeft
const int ring7[24] = {
  circle6_cardinals_out[0], circle6_splitRight[0], circle6_splitLeft[0],
  circle6_cardinals_out[1], circle6_splitRight[1], circle6_splitLeft[1],
  circle6_cardinals_out[2], circle6_splitRight[2], circle6_splitLeft[2],
  circle6_cardinals_out[3], circle6_splitRight[3], circle6_splitLeft[3],
  circle6_cardinals_out[4], circle6_splitRight[4], circle6_splitLeft[4],
  circle6_cardinals_out[5], circle6_splitRight[5], circle6_splitLeft[5],
  circle6_cardinals_out[6], circle6_splitRight[6], circle6_splitLeft[6],
  circle6_cardinals_out[7], circle6_splitRight[7], circle6_splitLeft[7]
};
// Ring 8: the 5 circle7 groups
const int ring8[40] = {
  circle7_cardinals[0],  circle7_splitRight[0],  circle7_secondRight[0],  circle7_secondLeft[0],  circle7_splitLeft[0],
  circle7_cardinals[1],  circle7_splitRight[1],  circle7_secondRight[1],  circle7_secondLeft[1],  circle7_splitLeft[1],
  circle7_cardinals[2],  circle7_splitRight[2],  circle7_secondRight[2],  circle7_secondLeft[2],  circle7_splitLeft[2],
  circle7_cardinals[3],  circle7_splitRight[3],  circle7_secondRight[3],  circle7_secondLeft[3],  circle7_splitLeft[3],
  circle7_cardinals[4],  circle7_splitRight[4],  circle7_secondRight[4],  circle7_secondLeft[4],  circle7_splitLeft[4],
  circle7_cardinals[5],  circle7_splitRight[5],  circle7_secondRight[5],  circle7_secondLeft[5],  circle7_splitLeft[5],
  circle7_cardinals[6],  circle7_splitRight[6],  circle7_secondRight[6],  circle7_secondLeft[6],  circle7_splitLeft[6],
  circle7_cardinals[7],  circle7_splitRight[7],  circle7_secondRight[7],  circle7_secondLeft[7],  circle7_splitLeft[7]
};
// Ring 9: interleaved four circle8 subgroups: cardinals, secondRight, secondaries, secondLeft
const int ring9[32] = {
  circle8_cardinals[0],   circle8_secondRight[0],   circle8_secondaries[0],   circle8_secondLeft[0],
  circle8_cardinals[1],   circle8_secondRight[1],   circle8_secondaries[1],   circle8_secondLeft[1],
  circle8_cardinals[2],   circle8_secondRight[2],   circle8_secondaries[2],   circle8_secondLeft[2],
  circle8_cardinals[3],   circle8_secondRight[3],   circle8_secondaries[3],   circle8_secondLeft[3],
  circle8_cardinals[4],   circle8_secondRight[4],   circle8_secondaries[4],   circle8_secondLeft[4],
  circle8_cardinals[5],   circle8_secondRight[5],   circle8_secondaries[5],   circle8_secondLeft[5],
  circle8_cardinals[6],   circle8_secondRight[6],   circle8_secondaries[6],   circle8_secondLeft[6],
  circle8_cardinals[7],   circle8_secondRight[7],   circle8_secondaries[7],   circle8_secondLeft[7]
};

// Array of pointers to the 9 ring arrays
const int* rings[] = { ring1, ring2, ring3, ring4, ring5, ring6, ring7, ring8, ring9 };
const int ringSizes[] = { 8, 8, 16, 16, 24, 16, 24, 40, 32 };
const int numRings = sizeof(rings) / sizeof(rings[0]);

// Static Mode functions:
void showStatic0() { // Static Mode #0: (Something, formerly Candy-corn)
  for (int i = 0; i < 8; i++) {
    leds[ring1[i]] = CRGB::White;
    leds[ring2[i]] = CRGB::OrangeRed;
    leds[circle3_cardinals[i]] = CRGB::Magenta;
    leds[circle3_secondaries[i]] = CRGB::Orange;
    leds[circle4_cardinals[i]] = CRGB::OrangeRed;
    // leds[circle4_secondaries[i]] = CRGB::Red;
    // leds[circle5_cardinals[i]] = CRGB::Magenta;
    // leds[circle5_splitLeft[i]] = CRGB::Chartreuse;
    // leds[circle5_splitRight[i]] = CRGB::White;
    // leds[circle6_cardinals_in[i]] = CRGB::OrangeRed;
    leds[circle6_cardinals_out[i]] = CRGB::Magenta;
    leds[circle6_splitRight[i]] = CRGB::Red;
    leds[circle6_secondaries[i]] = CRGB::Orange;
    leds[circle6_splitLeft[i]] = CRGB::Red;
    
    // leds[circle7_secondLeft[i]] = CRGB::Blue;
    // leds[circle7_secondRight[i]] = CRGB::Chartreuse;
    leds[circle7_splitRight[i]] = CRGB::Magenta;
    // leds[circle7_cardinals[i]] = CRGB::OrangeRed;
    leds[circle7_splitLeft[i]] = CRGB::Magenta;
    
    leds[circle8_secondLeft[i]] = CRGB::Red;
    leds[circle8_secondaries[i]] = CRGB::Orange;
    leds[circle8_secondRight[i]] = CRGB::Red;
    leds[circle8_cardinals[i]] = CRGB::OrangeRed;
  }
  FastLED.show();
}

void showStatic1() { // Static Mode #1: (MVP - Minimally Viable Mandala)
  for (int i = 0; i < 8; i++) {
    leds[ring1[i]] = palette[0];                // White
    leds[circle2[i]] = palette[1];              //"Yellow"
    leds[circle3_cardinals[i]] = palette[1];    //"Yellow"
    leds[circle3_secondaries[i]] = palette[2];  //"Orange"
    leds[circle4_cardinals[i]] = palette[2];    //"Orange"
    leds[circle4_secondaries[i]] = palette[2];  //"Orange"
    leds[circle5_cardinals[i]] = palette[2];    //"Orange"
    leds[circle5_splitLeft[i]] = palette[3];    // Red
    leds[circle5_splitRight[i]] = palette[3];   // Red

    leds[circle6_cardinals_in[i]] = palette[3]; // Red
    leds[circle6_secondaries[i]] = palette[3];  // Red
    leds[circle6_cardinals_out[i]] = palette[3];// Red
    leds[circle6_splitRight[i]] = palette[4];   // Magenta
    leds[circle6_splitLeft[i]] = palette[4];    // Magenta

    leds[circle7_cardinals[i]] = palette[4];    // Magenta
    leds[circle7_splitRight[i]] = palette[4];   // Magenta
    leds[circle7_secondRight[i]] = palette[4];  // Magenta
    leds[circle7_secondLeft[i]] = palette[4];   // Magenta
    leds[circle7_splitLeft[i]] = palette[4];    // Magenta

    leds[circle8_cardinals[i]] = palette[5];    // Blue
    leds[circle8_secondRight[i]] = palette[4];  // Magenta
    leds[circle8_secondaries[i]] = palette[4];  // Magenta
    leds[circle8_secondLeft[i]] = palette[4];   // Magenta
  }
  FastLED.show();
}

void showStatic2() { // Static Mode #2: (Foxy YellowBlue)
  for (int i = 0; i < 8; i++) {
    leds[innerCircle[i]] = palette[0];          // White
    leds[circle2[i]] = palette[1];              //"Yellow"
    leds[circle3_cardinals[i]] = palette[1];    //"Yellow"
    leds[circle3_secondaries[i]] = palette[2];  //"Orange"
    leds[circle4_cardinals[i]] = palette[1];    //"Yellow"
    leds[circle4_secondaries[i]] = palette[2];  //"Orange"
    leds[circle5_cardinals[i]] = palette[1];    //"Yellow"
    leds[circle5_splitRight[i]] = palette[6];   // Cyan
    leds[circle5_splitLeft[i]] = palette[5];    // Blue

    leds[circle6_cardinals_in[i]] = palette[1]; //"Yellow"
    leds[circle6_secondaries[i]] = palette[2];  //"Orange"
    leds[circle6_cardinals_out[i]] = palette[1];//"Yellow"
    leds[circle6_splitRight[i]] = palette[6];   // Cyan
    leds[circle6_splitLeft[i]] = palette[5];    // Blue

    leds[circle7_cardinals[i]] = palette[1];    //"Yellow"
    leds[circle7_splitRight[i]] = palette[6];   // Cyan
    leds[circle7_secondRight[i]] = palette[2];  //"Orange"
    leds[circle7_secondLeft[i]] = palette[2];   //"Orange"
    leds[circle7_splitLeft[i]] = palette[5];    // Blue

    leds[circle8_cardinals[i]] = palette[1];     //"Yellow"
    leds[circle8_secondRight[i]] = palette[6];   // Cyan
    leds[circle8_secondaries[i]] = palette[2];   //"Orange"
    leds[circle8_secondLeft[i]] = palette[5];    // Blue
  }
  FastLED.show();
}

void showStatic3() { // Static Mode #3: Spectrum Pizza -
  // The angular octs, starting from x→axis(right),
  // arrayed counterclockwise through the color palette.

  // Loop through each group of circles
  for (int group = 0; group < numCircles; group++) { 
    for (int i = 0; i < 8; i++) {
      leds[circles[group][i]] = palette[((i) % 8)]; // Assign color from palette based on index
    }
  }
  
  FastLED.show();
}

void showWOTY(int dayOfYear) { // rotatable Wheel of the Year based on date
  // Normalize dayOfYear to 0-364 range (365 days in a year)
  dayOfYear = dayOfYear % 365;  
  // Calculate rotation offset based on day of year
  // Winter Solstice (Yule) is around day 355 (Dec 21), white should be on top
  // Each season is ~45.625 days (365/8), each palette position is ~45.625 days
  int seasonOffset = (dayOfYear + 10 + 137) / 45; // +10 to align Yule properly +137 (3/8 turn) to align 8th oct to top
  seasonOffset = seasonOffset % 8; // Keep in 0-7 range  
  for (int group = 0; group < numCircles; group++) { 
    for (int i = 0; i < 8; i++) {
      // Rotate the palette based on the season
      // (7-i) gives counterclockwise order, +seasonOffset rotates to current season
      int paletteIndex = (7 - i + seasonOffset) % 8;
      leds[circles[group][i]] = yearPalette[paletteIndex];
    }
  }
  FastLED.show();
}

void showSeasonalWheel() { // Show the seasonal color wheel
  // All off except inner circle (showing the Seasonal Color Wheel)
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[NUM_LEDS-4] = CRGB::White;      // Our palette 0
  leds[NUM_LEDS-3] = CRGB::Blue;       // Our palette 5
  leds[NUM_LEDS-2] = CRGB::Chartreuse; // Our palette 7
  leds[NUM_LEDS-1] = CRGB::Cyan;       // Our palette 6
  leds[NUM_LEDS-8] = CRGB::Orange;     // Our palette 1
  leds[NUM_LEDS-7] = CRGB::Magenta;    // Our palette 4
  leds[NUM_LEDS-6] = CRGB::OrangeRed;  // Our palette 2
  leds[NUM_LEDS-5] = CRGB::Red;        // Our palette 3
  FastLED.show();
}

void setup() {
  pinMode(adminSensorPin, INPUT);    // Renamed from testSensorPin
  pinMode(centerSensorPin, INPUT);
  pinMode(freeSensorPin, INPUT);
  pinMode(edgeSensorPin, INPUT);
  
  pinMode(indicateGreen, OUTPUT);    // Renamed from indicatePin
  pinMode(indicateBlue, OUTPUT);
  pinMode(indicateWhite, OUTPUT);
  pinMode(indicateYellow, OUTPUT);
  
  digitalWrite(indicateGreen, LOW);  // Start green off
  digitalWrite(indicateBlue, LOW);   // Start blue off
  digitalWrite(indicateWhite, HIGH); // Start white on (default)
  digitalWrite(indicateYellow, LOW); // Start yellow off

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightnessLevels[brightnessIndex]);
  FastLED.clear(); // Clear all LEDs at startup
  
  // // Loop through each ring group
  // for (int ringIndex = 0; ringIndex < numRings; ringIndex++) { 
  //   CRGB ringColor = palette[ringIndex % 6]; // Cycle through palette with mod 7
  //   // Color all pixels in this ring
  //   for (int i = 2; i < ringSizes[ringIndex]; i++) {
  //     leds[rings[ringIndex][i]] = ringColor;
  //   }
  // }

  // // Loop through each group of circles
  // for (int group = 0; group < numCircles; group++) { 
  //   leds[circles[group][3]] = palette[group % 8]; // Assign color from palette based on group index
  // }

  // showStatic3(); // Show Static Mode #3 (Spectrum Pizza)
  // showWOTY(228); // Show Wheel of the Year with at a day number in the year
  // showStatic2(); // Show Static Mode #2 (Foxy YellowBlue)
  // showStatic1(); // Show Static Mode #1 (MVP - Minimally Viable Mandala)
  // showStatic0(); // Show Static Mode #0 (Candy-corn)
  // showSeasonalWheel(); // Show the Seasonal Color Wheel

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
  SpiralFill,      // Spiral fill effect
  SpectrumPizza,   // Static Mode #3: (Spectrum Pizza)
  WotY             // Wheel of the Year
};

const EffectMode allModes[8] = {
  BlueCardinals, // full semi-dynamic
  CenterBurst,   // full dynamic
  BREATHING,     // full dynamic
  SpiralFill,    // full dynamic

  // STATIC0,       // full static (Candy-corn)
  // FoxyYB,        // full static (Foxy YellowBlue)
  // SpectrumPizza, // full static (Spectrum Pizza)
  WotY,          // full static (Wheel of the Year)
  
  STATIC1,       // full static (MVP)
  // SeasonalWheel, // mini static 
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


void showBreathing() { // Breathing effect for all LEDs
  static uint8_t breathBrightness = 25; // Start with a low brightness
  static int8_t breathDirection = 1; // Smoother step
  static unsigned long lastUpdate = 0;
  static bool lingering = false;
  static unsigned long lingerStart = 0;
  const unsigned long lingerDuration = 1000; // 1s at top and bottom of breath
  unsigned long interval = map(breathBrightness, 12, 128, 40, 10);
  // Handle lingering state, if active
  if (lingering) {
    if (millis() - lingerStart >= lingerDuration) {
      lingering = false;
      lastUpdate = millis();
    }
  } else if (millis() - lastUpdate > interval) {
    lastUpdate = millis();
    breathBrightness += breathDirection;
    if (breathBrightness <= 12) { // Linger at min
      breathDirection = 1;
      lingering = true;
      lingerStart = millis();
    }
    else if (breathBrightness >= 128) { // Linger at max
      breathDirection = -1;
      lingering = true;
      lingerStart = millis();
    }
  }
  // Set all LEDs to the current brightness (breathing blue)
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, breathBrightness);
  }
  FastLED.show();
}

void showTwinkle() { // Twinkle effect for random LEDs
  static unsigned long lastTwinkle = 0;
  static uint8_t twinkleCount = 23; // Now 23 twinkles at once
  static int twinkleIndices[23] = {0};
  // Only update every 500 ms for a slower effect
  if (millis() - lastTwinkle > 500) {
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

// revisit: make BCs dynamics more interesting
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
    leds[circle5_splitRight[i]] = CRGB::Red;
    leds[circle5_splitLeft[i]] = CRGB::Red;
    leds[circle6_cardinals_in[i]] = CRGB::Blue;
    leds[circle6_secondaries[i]] = CRGB::Red;
    leds[circle6_cardinals_out[i]] = CRGB::Blue;
    leds[circle6_splitRight[i]] = CRGB::Magenta;
    leds[circle6_splitLeft[i]] = CRGB::Magenta;

    leds[circle7_cardinals[i]] = CRGB::Blue;
    leds[circle7_splitRight[i]] = CRGB::Magenta;
    leds[circle7_secondRight[i]] = CRGB::Magenta;
    leds[circle7_secondLeft[i]] = CRGB::Magenta;
    leds[circle7_splitLeft[i]] = CRGB::Magenta;
    leds[circle8_cardinals[i]] = CRGB::Blue;
    leds[circle8_secondRight[i]] = CRGB::Magenta;
    leds[circle8_secondaries[i]] = CRGB::Red;
    leds[circle8_secondLeft[i]] = CRGB::Magenta;
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

  static uint8_t ringBrightness[9] = {0}; // Track brightness for each ring

  const unsigned long expandInterval = 80;     // Faster expansion
  const unsigned long contractInterval = 240;  // Slower contraction
  const unsigned long pauseOuter = 800;        // Pause at outermost ring
  const unsigned long pauseInner = 800;        // Pause at innermost ring
  const uint8_t fadeStep = 8;                  // Smaller step for slower fade

  const uint8_t ringPalette[] = {0, 1, 1, 2, 2, 3, 3, 3, 4};

  if (!burstActive) {
    burstActive = true;
    burstStart = millis();
    burstStep = 0;
    burstOut = true;
    inPause = false;
    pauseAtOuter = false;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int r = 0; r < 9; r++) ringBrightness[r] = 0;
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
          leds[rings[0][i]] = CRGB::Grey; // Inner ring at half brightness
        }
      } else {
        // During expanded pause, show all rings
        for (int r = 0; r < 9; r++) {
          for (int i = 0; i < ringSizes[r]; i++) {
            leds[rings[r][i]] = palette[ringPalette[r]];
          }
        }
      }
      FastLED.show();
      return;
    }
  }

  // Brightness smoothing for contraction phase (runs every frame)
  for (int r = 0; r < 9; r++) {
    if (burstOut) {
      // Expanding: rings up to burstStep are instantly on
      ringBrightness[r] = (r <= burstStep) ? 255 : 0;
    } else {
      // Contracting: fade out rings above burstStep
      if (r <= burstStep) {
        ringBrightness[r] = 255;
      } else if (ringBrightness[r] > 0) {
        ringBrightness[r] = max(0, ringBrightness[r] - fadeStep);
      }
    }
  }

  // Draw rings with current brightness (manual scaling)
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int r = 0; r < 9; r++) {
    uint8_t b = ringBrightness[r];
    for (int i = 0; i < ringSizes[r]; i++) {
      CRGB base = palette[ringPalette[r]];
      leds[rings[r][i]] = CRGB(
        (base.r * b) / 255,
        (base.g * b) / 255,
        (base.b * b) / 255
      );
    }
  }
  FastLED.show();

  // Only update burst step at interval
  unsigned long interval = burstOut ? expandInterval : contractInterval;
  if (millis() - burstStart >= interval) {
    burstStart = millis();

    if (burstOut) {
      burstStep++;
      if (burstStep > 8) { // reached outermost, pause before contracting
        burstStep = 8;
        inPause = true;
        pauseAtOuter = true;
        pauseStart = millis();
      }
    } else {
      if (burstStep > 0) {
        burstStep--;
      } else if (burstStep == 0) {
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

  for (int r = 0; r < 9; r++) { // Now 9 rings
    uint8_t ringHue = baseHue - r * 28; // Adjusted spacing for 9 rings
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
  
  // Build the spiral order (all pixels now valid)
  static int spiralOrder[NUM_LEDS];
  static int spiralLen = 0;
  static bool spiralInit = false;
  if (!spiralInit) {
    // 183 down to 0 (all pixels)
    for (int i = 183; i >= 0; i--) {
      spiralOrder[spiralLen++] = i;
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
  // showBreathing();
  // showTwinkle();
  // showBlueCardinals();
  showCenterBurst();
  // TEMPORARY: Disable all loop functionality for setup() testing
  static bool disableLoop = true;
  if (disableLoop) {
    return; // Exit loop immediately, only setup() runs
  }
  
  // Read raw sensor states
  int adminSensorState = digitalRead(adminSensorPin);
  int centerSensorState = digitalRead(centerSensorPin);
  int freeSensorState = digitalRead(freeSensorPin);        // Renamed from tableSensorState
  int edgeSensorState = digitalRead(edgeSensorPin);

  // Debouncing variables
  static bool lastAdminState = LOW;
  static bool lastCenterState = LOW;
  static bool lastFreeState = LOW;     // Renamed from lastTableState
  static bool lastEdgeState = LOW;
  
  static unsigned long lastAdminChange = 0;
  static unsigned long lastCenterChange = 0;
  static unsigned long lastFreeChange = 0;    // Renamed from lastTableChange
  static unsigned long lastEdgeChange = 0;
  
  const unsigned long debounceDelay = 50; // 50ms debounce

  // Debounced sensor states
  static bool debouncedAdminState = LOW;
  static bool debouncedCenterState = LOW;
  static bool debouncedFreeState = LOW;    // Renamed from debouncedTableState
  static bool debouncedEdgeState = LOW;

  // Admin sensor debouncing
  if (adminSensorState != lastAdminState) {
    lastAdminChange = millis();
  }
  if ((millis() - lastAdminChange) > debounceDelay) {
    if (adminSensorState != debouncedAdminState) {
      debouncedAdminState = adminSensorState;
    }
  }
  lastAdminState = adminSensorState;

  // Center sensor debouncing
  if (centerSensorState != lastCenterState) {
    lastCenterChange = millis();
  }
  if ((millis() - lastCenterChange) > debounceDelay) {
    if (centerSensorState != debouncedCenterState) {
      debouncedCenterState = centerSensorState;
    }
  }
  lastCenterState = centerSensorState;

  // Free sensor debouncing (renamed from table sensor)
  if (freeSensorState != lastFreeState) {
    lastFreeChange = millis();
  }
  if ((millis() - lastFreeChange) > debounceDelay) {
    if (freeSensorState != debouncedFreeState) {
      debouncedFreeState = freeSensorState;
    }
  }
  lastFreeState = freeSensorState;

  // Edge sensor debouncing
  if (edgeSensorState != lastEdgeState) {
    lastEdgeChange = millis();
  }
  if ((millis() - lastEdgeChange) > debounceDelay) {
    if (edgeSensorState != debouncedEdgeState) {
      debouncedEdgeState = edgeSensorState;
    }
  }
  lastEdgeState = edgeSensorState;

  // Indicator logic based on debounced sensor states
  // Yellow on when freeSensor is High (renamed from tableSensor)
  digitalWrite(indicateYellow, debouncedFreeState == HIGH ? HIGH : LOW);
  
  // Green on only when adminSensor is high
  digitalWrite(indicateGreen, debouncedAdminState == HIGH ? HIGH : LOW);
  
  // White is on by default and off if any sensor is high
  bool anySensorHigh = (debouncedAdminState == HIGH || debouncedCenterState == HIGH || 
                       debouncedFreeState == HIGH || debouncedEdgeState == HIGH);
  digitalWrite(indicateWhite, anySensorHigh ? LOW : HIGH);

  // Blue on only if centerSensor is high
  digitalWrite(indicateBlue, debouncedCenterState == HIGH ? HIGH : LOW);

  // Brightness control (using debounced adminSensor)
  static bool lastDebouncedAdminState = LOW;
  if (debouncedAdminState == HIGH && lastDebouncedAdminState == LOW) {
      // Cycle to the next brightness level
      brightnessIndex = (brightnessIndex + 1) % NUM_BRIGHTNESS_LEVELS;
      FastLED.setBrightness(brightnessLevels[brightnessIndex]);
      FastLED.show(); // Update LEDs with new brightness
  }
  lastDebouncedAdminState = debouncedAdminState;

  // Effect mode cycling (using debounced centerSensor)
  static bool lastDebouncedCenterState = LOW;
  if (debouncedCenterState == HIGH && lastDebouncedCenterState == LOW && !showingColorWheel) {
    showingColorWheel = true;
    colorWheelStart = millis();
    mandalaActive = false;
    altMandalaActive = false;

    // All off except inner circle (show Seasonal Color Wheel)
    showSeasonalWheel();

    // Set current effect to the next mode
    currentEffect = allModes[modeIndex];
    modeIndex = (modeIndex + 1) % NUM_MODES;
  }
  lastDebouncedCenterState = debouncedCenterState;

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
      case SpectrumPizza:  showStatic3(); break;
      case WotY:           showWOTY(228); break; // 228 = Aug 16th
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

