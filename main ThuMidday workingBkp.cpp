#include <Arduino.h>
#include <FastLED.h>
// #include <time.h>  // Add this for time functions

const int adminSensorPin = 2;    // GPIO2 input 
const int centerSensorPin = 4;   // GPIO3 input (swapped)
// const int freeSensorPin = 3;     // GPIO4 input (swapped)
// const int edgeSensorPin = 7;     // GPIO4 input (new/unused)

const int indicateYellow = 8;     // GPIO8 output (yellow indicator)
const int indicateWhite = 9;      // GPIO9 output (white indicator) 
const int indicateBlue = 10;      // GPIO10 output (blue indicator)
const int indicateGreen = 20;     // GPIO21 output (green indicator, renamed from indicatePin)

#define LED_PIN     21     // GPIO20 output pin for addressable LEDs
#define NUM_LEDS    184    // Updated total LED count; was 150 in v1 with numbers 0,1,2,3,4,37 unused
#define BRIGHTNESS  64
const uint8_t brightnessLevels[] = {12, 25, 64, 191}; // 5%, 10%, 25%, 75%
const uint8_t NUM_BRIGHTNESS_LEVELS = sizeof(brightnessLevels) / sizeof(brightnessLevels[0]);
uint8_t brightnessIndex = 3; // Start at a reasonable default (e.g., 64)

#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
// const unsigned long DYNAMIC_PHASE_DURATION = 300000; // 5 minutes in milliseconds
const unsigned long DYNAMIC_PHASE_DURATION = 30000; // 30 seconds in milliseconds

// --- Effect Mode Enum ---
enum EffectMode {  /// Define the effect modes
  STATIC0,         // Static Mode #0: Candy-corn
  STATIC1,         // Static Mode #1: MVP - Minimally Viable Mandala
  FoxyYB,          // Static Mode #2: Foxy YellowBlue
  BlueCardinals,   // Blue Cardinals (semi-Dynamic)
  BREATHING,       // All pixels Breathing blue
  TWINKLE,         // Twinkle effect for random pixels
  SeasonalWheel,   // mini Seasonal Color Wheel (Static)
  CenterBurst,     // Center burst effect
  RainbowFade,     // Rainbow fade effect
  SpiralFill,      // Spiral fill effect
  SpectrumPizza,   // Static Mode #3: Spectrum Pizza
  WotY,            // full Wheel of the Year (Static)
  FlowerOutline,   // Flower Outline (Static)
  RadarSweep,      // Radar Sweep (Dynamic)
  DynamicFlower,   // Dynamic Flower (Dynamic) - v2's default
  FragileSpokes    // Fragile Spokes (Static)
};

// --- Menu mode arrays ---
const EffectMode mainModes[] = {
  WotY,
  DynamicFlower,
  TWINKLE
  // Add your main frequently-used modes here
};
const uint8_t NUM_MAIN_MODES = sizeof(mainModes) / sizeof(mainModes[0]);
const EffectMode subModes[] = {
  FragileSpokes,
  SpiralFill,
  FoxyYB
  // Add less-used modes here
};
const uint8_t NUM_SUB_MODES = sizeof(subModes) / sizeof(subModes[0]);
uint8_t menuLevel = 0; // 0 = main, 1 = submenu
uint8_t modeIndex = 0;
EffectMode currentEffect = mainModes[0];

// // Deprecated old allModes[] list:
// const EffectMode allModes[] = {
//   // RadarSweep,    // full dynamic
//   // FlowerOutline, // thin static
//   DynamicFlower, // thin dynamic
//   // FragileSpokes, // thin static
//   WotY,         // full static (Wheel of the Year)
//   // BlueCardinals, // full semi-dynamic
//   // CenterBurst,   // full dynamic
//   // BREATHING,     // full dynamic
//   // SpiralFill,    // full dynamic

//   // STATIC0,       // full static (Candy-corn)
//   // FoxyYB,        // full static (Foxy YellowBlue)
//   // SpectrumPizza, // full static (Spectrum Pizza)
  

//   // STATIC1,       // full static (MVP)
//   // SeasonalWheel, // mini static 
//   // TWINKLE,       // mini dynamic
//   // RainbowFade    // full dynamic
// };
// const uint8_t NUM_MODES = sizeof(allModes) / sizeof(allModes[0]);
// uint8_t modeIndex = 0;
// EffectMode currentEffect = DynamicFlower; // Start with Dynamic Flower

// --- Palette definitions ---
// Muse palette: White, Orange(Yellow), OrangeRed(Orange), Red, Magenta, Blue, Cyan, Chartreuse
const CRGB palette[8] = {
  CRGB::White, CRGB::Orange, CRGB::OrangeRed, CRGB::Red,
  CRGB::Magenta, CRGB::Blue, CRGB::Cyan, CRGB::Chartreuse
};
// WotY palette: white Winter, blue Imbloc, chartreuse Spring, cyan Beltane,
//               yellow Summer, magenta Lammas, orange Autumn, red Samhain.
  // Feast   | Date   | Day # | Palette Color
  // ------- | ------ | ----- | -------------
  // Imbolc  | 1 Feb  | 32    | Blue
  // Ostara  | 20 Mar | 79    | Chartreuse
  // Beltane | 1 May  | 121   | Cyan
  // Litha   | 21 Jun | 172   | Orange (Yellow)
  // Lammas  | 1 Aug  | 213   | Magenta
  // Mabon   | 22 Sep | 265   | OrangeRed (Orange)
  // Samhain | 31 Oct | 304   | Red
  // Yule    | 21 Dec | 355   | White
const CRGB yearPalette[8] = {
  CRGB::White, CRGB::Blue, CRGB::Chartreuse, CRGB::Cyan,
  CRGB::Orange, CRGB::Magenta, CRGB::OrangeRed, CRGB::Red
};

CRGB leds[NUM_LEDS];

// --- Deprecated old pixel exceptions ---
// Define excepted pixels - now none are excepted (all 184 are used)
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

// --- Mapping arrays and build functions ---
// Base (sub)circles definitions:
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

// 9 concentric Rings definitions:
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
// 16 radial arrays (spokes), of cardinals and secondaries
// Cardinals (even): 9 pixels each
const int radial0[9]  = { innerCircle[0], circle2[0], circle3_cardinals[0], circle4_cardinals[0], circle5_cardinals[0], circle6_cardinals_in[0], circle6_cardinals_out[0], circle7_cardinals[0], circle8_cardinals[0] };
const int radial2[9]  = { innerCircle[1], circle2[1], circle3_cardinals[1], circle4_cardinals[1], circle5_cardinals[1], circle6_cardinals_in[1], circle6_cardinals_out[1], circle7_cardinals[1], circle8_cardinals[1] };
const int radial4[9]  = { innerCircle[2], circle2[2], circle3_cardinals[2], circle4_cardinals[2], circle5_cardinals[2], circle6_cardinals_in[2], circle6_cardinals_out[2], circle7_cardinals[2], circle8_cardinals[2] };
const int radial6[9]  = { innerCircle[3], circle2[3], circle3_cardinals[3], circle4_cardinals[3], circle5_cardinals[3], circle6_cardinals_in[3], circle6_cardinals_out[3], circle7_cardinals[3], circle8_cardinals[3] };
const int radial8[9]  = { innerCircle[4], circle2[4], circle3_cardinals[4], circle4_cardinals[4], circle5_cardinals[4], circle6_cardinals_in[4], circle6_cardinals_out[4], circle7_cardinals[4], circle8_cardinals[4] };
const int radial10[9] = { innerCircle[5], circle2[5], circle3_cardinals[5], circle4_cardinals[5], circle5_cardinals[5], circle6_cardinals_in[5], circle6_cardinals_out[5], circle7_cardinals[5], circle8_cardinals[5] };
const int radial12[9] = { innerCircle[6], circle2[6], circle3_cardinals[6], circle4_cardinals[6], circle5_cardinals[6], circle6_cardinals_in[6], circle6_cardinals_out[6], circle7_cardinals[6], circle8_cardinals[6] };
const int radial14[9] = { innerCircle[7], circle2[7], circle3_cardinals[7], circle4_cardinals[7], circle5_cardinals[7], circle6_cardinals_in[7], circle6_cardinals_out[7], circle7_cardinals[7], circle8_cardinals[7] };
// Secondaries (odd): 4 pixels each
const int radial1[4]  = { circle3_secondaries[0], circle4_secondaries[0], circle6_secondaries[0], circle8_secondaries[0] };
const int radial3[4]  = { circle3_secondaries[1], circle4_secondaries[1], circle6_secondaries[1], circle8_secondaries[1] };
const int radial5[4]  = { circle3_secondaries[2], circle4_secondaries[2], circle6_secondaries[2], circle8_secondaries[2] };
const int radial7[4]  = { circle3_secondaries[3], circle4_secondaries[3], circle6_secondaries[3], circle8_secondaries[3] };
const int radial9[4]  = { circle3_secondaries[4], circle4_secondaries[4], circle6_secondaries[4], circle8_secondaries[4] };
const int radial11[4] = { circle3_secondaries[5], circle4_secondaries[5], circle6_secondaries[5], circle8_secondaries[5] };
const int radial13[4] = { circle3_secondaries[6], circle4_secondaries[6], circle6_secondaries[6], circle8_secondaries[6] };
const int radial15[4] = { circle3_secondaries[7], circle4_secondaries[7], circle6_secondaries[7], circle8_secondaries[7] };

// Array of pointers to the 16 radial arrays
const int* radials[] = {
  radial0, radial1, radial2, radial3, radial4, radial5, radial6, radial7,
  radial8, radial9, radial10, radial11, radial12, radial13, radial14, radial15
};
const int radialSizes[] = {9,4,9,4,9,4,9,4,9,4,9,4,9,4,9,4};
const int numRadials = sizeof(radials) / sizeof(radials[0]);

// buildSectorMapping() function for the 16 angular slices:
const int sectorLens[16] = {14,9,14,9,14,9,14,9,14,9,14,9,14,9,14,9};
int sectorPixels[16][14]; // Use max size for all sectors

void buildSectorMapping() {
  for (int s = 0; s < 16; s++) {
    int len = 0;
    int idx = s / 2; // Map 16 sectors to 8 indices

    // Add radial pixels
    for (int i = 0; i < radialSizes[s]; i++)
      sectorPixels[s][len++] = radials[s][i];

    // Add the 5 off-radial pixels between this radial and the next, counterclockwise
    // Right ones for cardinals (even sectors), Left ones for secondaries (odd sectors)
    if (s % 2 == 0) { // Cardinal sector
      sectorPixels[s][len++] = circle7_splitRight[idx];
      sectorPixels[s][len++] = circle6_splitRight[idx];
      sectorPixels[s][len++] = circle5_splitRight[idx];
      sectorPixels[s][len++] = circle8_secondRight[idx];
      sectorPixels[s][len++] = circle7_secondRight[idx];
    } else { // Secondary sector
      sectorPixels[s][len++] = circle7_secondLeft[idx];
      sectorPixels[s][len++] = circle8_secondLeft[idx];
      sectorPixels[s][len++] = circle5_splitLeft[idx];
      sectorPixels[s][len++] = circle6_splitLeft[idx];
      sectorPixels[s][len++] = circle7_splitLeft[idx];
    }
  }
}
// Build left and right curves for each sector:
int leftCurves[8][6]; // 8 curves, 6 pixels each
void buildLeftCurves() {
  for (int s = 0; s < 8; s++) {
    leftCurves[s][0] = circle3_secondaries[s];
    leftCurves[s][1] = circle4_secondaries[s];
    leftCurves[s][2] = circle5_splitLeft[s];
    leftCurves[s][3] = circle6_splitLeft[s];
    leftCurves[s][4] = circle7_splitLeft[s];
    leftCurves[s][5] = circle8_cardinals[(s + 1) % 8]; // last cardinal of next sector
  }
}

int rightCurves[8][6]; // 8 curves, 6 pixels each
void buildRightCurves() {
  for (int s = 0; s < 8; s++) {
    rightCurves[s][0] = circle3_secondaries[s];
    rightCurves[s][1] = circle4_secondaries[s];
    rightCurves[s][2] = circle5_splitRight[s];
    rightCurves[s][3] = circle6_splitRight[s];
    rightCurves[s][4] = circle7_splitRight[s];
    rightCurves[s][5] = circle8_cardinals[s]; // last cardinal of same sector
  }
}

// --- Debouncing variables ---
static bool lastAdminState = LOW;
static bool lastCenterState = LOW;
// static bool lastFreeState = LOW;
// static bool lastEdgeState = LOW;
static unsigned long lastAdminChange = 0;
static unsigned long lastCenterChange = 0;
// static unsigned long lastFreeChange = 0;
// static unsigned long lastEdgeChange = 0;
const unsigned long debounceDelay = 200; // 200ms debounce
static bool debouncedAdminState = LOW;
static bool debouncedCenterState = LOW;
// static bool debouncedFreeState = LOW;
// static bool debouncedEdgeState = LOW;

// --- Other state variables ---
unsigned long bootTime = 0;
unsigned long lastCenterTouch = 0;
bool mandalaActive = true;
bool altMandalaActive = false;
unsigned long lastEffectUpdate = 0;
uint8_t effectOffset = 0;
bool burstActive = false;           // Center burst effect state
bool showingColorWheel = false;     // Seasonal Color Wheel state
unsigned long colorWheelStart = 0;  // Timer for color wheel animation
bool firstSpiralRun = true;         // Flag to track first run of Spiral Fill effect

// --- All effects Mode functions ---
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

void showWOTY(int dayOfYear) { // Wheel of the Year with 16-sector resolution
  dayOfYear = dayOfYear % 365;
  // Each sector is ~22.8 days (365/16)
  int seasonOffset = (dayOfYear + 10) / 23; // +10 to align Yule
  seasonOffset = seasonOffset % 16; // Keep in 0-15 range

  for (int s = 0; s < 16; s++) {
    // Rotate the palette based on the season
    int paletteIndex = ((21 - s + seasonOffset)/2) % 8; // Use 8-color yearPalette, repeat every 8
    CRGB color = yearPalette[paletteIndex];
    for (int i = 0; i < sectorLens[s]; i++) {
      leds[sectorPixels[s][i]] = color;
    }
  }
  FastLED.show();
}

void showSeasonalWheel(int numberOfRings) { // Show the seasonal color wheel
  // All off except inner circle (showing the Seasonal Color Wheel)
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  for (int i = 0; i < numberOfRings; i++) {
    for (int j = 0; j < 8; j++) {
      CRGB color = yearPalette[(10 - j) % 8]; // Use the Wheel of the Year's palette
      leds[radials[j*2][i]] = color;
    }
  }
  // // Show the seasonal color wheel in the inner circle
  // leds[NUM_LEDS-4] = CRGB::White;      // Our palette 0
  // leds[NUM_LEDS-3] = CRGB::Blue;       // Our palette 5
  // leds[NUM_LEDS-2] = CRGB::Chartreuse; // Our palette 7
  // leds[NUM_LEDS-1] = CRGB::Cyan;       // Our palette 6
  // leds[NUM_LEDS-8] = CRGB::Orange;     // Our palette 1
  // leds[NUM_LEDS-7] = CRGB::Magenta;    // Our palette 4
  // leds[NUM_LEDS-6] = CRGB::OrangeRed;  // Our palette 2
  // leds[NUM_LEDS-5] = CRGB::Red;        // Our palette 3
  FastLED.show();
}

void showFlowerOutline(int octalRotations) {  // The _split curves form a flower outline
  for (int c = 0; c < 8; c++) {
    int p = (c + 7) % 8; // curve in the preceding octant
    CRGB color = palette[c];
    for (int i = 0; i < 6; i++) {
      leds[leftCurves[(p+octalRotations)%8][i]] = color;
      // Special case: for last right curve (c==7), skip i==0 and i==1
      if (!(c == 7 && (i == 0 || i == 1))) {
        leds[rightCurves[(c+octalRotations)%8][i]] = color;
      }
    }
    // leds[ring1[c]] = color;
    // leds[ring2[c]] = color;
  }
  FastLED.show();
}

void showFragileSpokes(int octalRotations) {
  static int erasedRadial = -1;           // Which radial is being erased/redrawn (-1 = none)
  static int redrawStep = 0;              // Progress of redraw (0 = not started)
  static unsigned long lastUpdate = 0;
  const unsigned long redrawInterval = 100; // ms between redraw steps
  // const int octalRotations = 2; // 90 degrees rotation (2 octants)

  // Draw all cardinals in palette colors, except the one being erased/redrawn
  for (int r = 0; r < 8; r++) {
    int idx = ((r + octalRotations) * 2) % 16;
    if (erasedRadial != r) {
      CRGB color = palette[r % 8];
      // CRGB color = palette[r % 2 + 2];
      for (int i = 0; i < radialSizes[idx]; i++) {
        leds[radials[idx][i]] = color;
      }
      // Draw the arrow curves
      for (int i = 2; i <= 4; i++) {
        leds[rightCurves[(r + octalRotations) % 8][i]] = color;
        leds[leftCurves[(r + octalRotations + 7) % 8][i]] = color;
      }
    }
  }
  FastLED.show();
  // // Erase condition: on center sensor touch (debounced)
  // static bool lastDebouncedCenterState = LOW;
  // if (erasedRadial == -1 && debouncedCenterState == HIGH && lastDebouncedCenterState == LOW) {
  //   erasedRadial = random(0, 8); // Pick a random cardinal (0,2,4,...14)
  //   int octant = (erasedRadial + octalRotations) % 8;
  //   int idx = (octant * 2) % 16;
  //   for (int i = 0; i < radialSizes[idx]; i++) {
  //     leds[radials[idx][i]] = CRGB::Black;
  //   }
  //   for (int i = 2; i <= 4; i++) {
  //     leds[rightCurves[octant][i]] = CRGB::Black;
  //     leds[leftCurves[(octant + 7) % 8][i]] = CRGB::Black;
  //   }
  //   redrawStep = 0;
  //   FastLED.show();
  //   lastUpdate = millis();
  //   lastDebouncedCenterState = debouncedCenterState;
  //   return;
  // }
  // lastDebouncedCenterState = debouncedCenterState;

  // // Redraw the erased radial from outside inwards
  // if (erasedRadial != -1 && millis() - lastUpdate > redrawInterval) {
  //   lastUpdate = millis();
  //   int octant = (erasedRadial + octalRotations) % 8;
  //   int idx = (octant * 2) % 16;
  //   CRGB color = palette[erasedRadial % 8]; 
  //   // CRGB color = palette[erasedRadial % 2 + 2]; 
  //   int len = radialSizes[idx];

  //   // Redraw the main radial pixel
  //   if (redrawStep < len) {
  //     leds[radials[idx][len - 1 - redrawStep]] = color;
  //   }

  //   // Redraw arrowhead pixels only for the first two redraw steps
  //   if (redrawStep >= 0 && redrawStep < 3) {
  //     int curveIdx = 4 - redrawStep; // 4, 3 for redrawStep 0, 1
  //     leds[rightCurves[octant][curveIdx]] = color;
  //     leds[leftCurves[(octant + 7) % 8][curveIdx]] = color;
  //   }

  //   redrawStep++;
  //   FastLED.show();

  //   if (redrawStep >= len) {
  //     // Done redrawing, reset for next cycle
  //     erasedRadial = -1;
  //   }
  // }
}
// Dynamic Mode functions:
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

  static uint8_t ringBrightness[9] = {0};     // Track brightness for each ring
  static uint8_t innerPauseBrightness = 255;  // Track inner ring brightness during pause

  const unsigned long expandInterval = 80;    // Faster expansion
  const unsigned long contractInterval = 250; // Slower contraction
  const unsigned long pauseOuter = 800;       // Pause at outermost ring
  const unsigned long pauseInner = 800;       // Pause at innermost ring
  const uint8_t fadeStep = 8;                 // Smaller step for slower fade

  const uint8_t ringPalette[] = {0, 1, 1, 2, 2, 3, 3, 4, 4};
  // Start burst effect
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
      innerPauseBrightness = 255; // Reset for next time
    } else {
      // During contracted pause, show only the innermost ring
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      if (!pauseAtOuter) {
        // Fade inner ring down to target brightness during pause
        if (innerPauseBrightness > 128) {
          innerPauseBrightness = max(128, innerPauseBrightness - fadeStep);
        }
        for (int i = 0; i < ringSizes[0]; i++) {
          CRGB base = palette[ringPalette[0]];
          leds[rings[0][i]] = CRGB(
            (base.r * innerPauseBrightness) / 255,
            (base.g * innerPauseBrightness) / 255,
            (base.b * innerPauseBrightness) / 255
          );
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

  // If expanding, use expandInterval; if contracting, use contractInterval
  unsigned long interval = burstOut ? expandInterval : contractInterval;
  if (millis() - burstStart >= interval) {
    burstStart = millis(); // Reset burst start time
    if (burstOut) { // Expanding
      burstStep++;
      if (burstStep > 8) { // reached outermost, pause before contracting
        burstStep = 8;
        inPause = true;
        pauseAtOuter = true;
        pauseStart = millis();
      }
    } else { // Contracting
      if (burstStep > 0) {
        burstStep--;
      } else if (burstStep == 0) { // reached innermost, pause before expanding
        inPause = true;
        pauseAtOuter = false;
        pauseStart = millis();
      }
    }
  }
}
// Isa's input on speed : both 20 and 40 have their merits
void showRainbowFade(uint16_t fadeSpeed) { // Rainbow fade effect across all rings
  static uint8_t baseHue = 0;
  static unsigned long lastUpdate = 0;
  // Update every fadeSpeed ms for smooth animation
  if (millis() - lastUpdate > fadeSpeed) {
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
// Isa's opinion on chaotic spiral palette mixing : not great because 4 colors is too kunterbunt
void showSpiralFill(const CRGB* activePalette, uint8_t numColors) { // Spiral fill effect
  static unsigned long lastUpdate = 0;
  static int colorIndex = 0;
  static int pixelIndex = 0;

  // Wipe to black only at the onset of the mode
  if (firstSpiralRun == true) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    firstSpiralRun = false;
    pixelIndex = 0;
    colorIndex = 0;
    return;
  }

  // Animate spiral fill
  if (millis() - lastUpdate > 40) { // Update spiral pixels every 40 ms
    lastUpdate = millis();
    if (pixelIndex < NUM_LEDS) {
      leds[NUM_LEDS - 1 - pixelIndex] = activePalette[colorIndex]; // Fill from the (inner) end of the light pebbles string
      pixelIndex++;
      FastLED.show();
    } else {
      // Finished this color, move to next color
      colorIndex++;
      if (colorIndex >= numColors) colorIndex = 0;
      pixelIndex = 0;
      // Do not wipe to black, just start drawing next color on top
    }
  }
}
void showRadarSweep(uint8_t tailLength) {
  static unsigned long lastUpdate = 0;
  static int sweepIndex = 0;
  // const uint8_t tailLength = 10; // Number of fading tail sectors
  const uint8_t fadeStep = 128 / tailLength;  // Amount to fade per tail step

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Sweep timing
  if (millis() - lastUpdate > 175) { // 175 ms per sectorsweep
    lastUpdate = millis();
    sweepIndex = (sweepIndex + 1) % 16; // 16 sectors
    // sweepIndex = (sweepIndex - 1 + 16) % 16; // clockwise
  }

  // Tail: fill all pixels in the trailing sectors
  for (int t = 1; t < tailLength; t++) { // Start at 1 so leading edge is handled separately
    int tailIdx = (sweepIndex - t + 16) % 16;
    // int tailIdx = (sweepIndex + t) % 16;
    uint8_t brightness = 128 - (t * fadeStep);
    CRGB color = CRGB(
      (palette[7].r * brightness) / 255,
      (palette[7].g * brightness) / 255,
      (palette[7].b * brightness) / 255
    );
    for (int i = 0; i < sectorLens[tailIdx]; i++) {
      leds[sectorPixels[tailIdx][i]] = color;
    }
  }

  // Leading edge: only the radial line in white
  for (int i = 0; i < radialSizes[sweepIndex]; i++) {
    leds[radials[sweepIndex][i]] = CRGB::White;
  }

  FastLED.show();
}
void showDynamicFlower(int octalRotations) {
  static int animStep = 0;
  static unsigned long lastUpdate = 0;
  const int curveLen = 6; // Length of each curve
  const int petalLen = 11; // Length of each petal outline
  const int totalSteps = 8 * petalLen; // 8 petals
  const unsigned long animInterval = 250;  // ms between animation steps

  // Clear all LEDs for outline
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Draw all curves in color
  for (int c = 0; c < 8; c++) {
    int p = (c + 7) % 8;
    CRGB color = palette[c];
    for (int i = 0; i < curveLen; i++) {
      leds[leftCurves[(p + octalRotations) % 8][i]] = color;
      if (!(c == 7 && (i == 0 || i == 1))) {
        leds[rightCurves[(c + octalRotations) % 8][i]] = color;
      }
    }
  }

  // Animate the black pixel
  int phase = animStep / petalLen; // which petal
  int pos   = animStep % petalLen; // which pixel
  int curveIdx = (phase + octalRotations) % 8;

  // Advance animation step if pos==5 
  //  to avoid doubling the wait on the petal's inner pixel
  if (pos == 5) {  
    animStep = (animStep + 3) % totalSteps;
    // Recalculate phase and pos after skipping
    phase = animStep / petalLen;
    pos   = animStep % petalLen;
    curveIdx = (phase + octalRotations) % 8;
  }

  // Only update animation step if enough time has passed
  if (millis() - lastUpdate > animInterval) {
    lastUpdate = millis();
    animStep = (animStep + 1) % totalSteps;
  }

  // Animate one pixel to black
  if (pos < 6) {    // Down right curve
    leds[rightCurves[curveIdx][(curveLen - 1 - pos) % curveLen]] = CRGB::Black;
  } else {          // Up left curve
    leds[leftCurves[curveIdx][(curveLen - 6 + pos) % curveLen]] = CRGB::Black;
  }

  FastLED.show();
}

void restoreMandalaState() {
  showSeasonalWheel(9);
  FastLED.show();
}

void setup() {
  bootTime = millis();
  // Serial.begin(115200); // Start serial for debugging
  buildSectorMapping(); // Build the sector mapping
  buildLeftCurves();
  buildRightCurves();
  pinMode(adminSensorPin, INPUT);
  pinMode(centerSensorPin, INPUT);
  // pinMode(freeSensorPin, INPUT_PULLUP);
  // pinMode(edgeSensorPin, INPUT_PULLDOWN);
  pinMode(indicateGreen, OUTPUT);    // Renamed from indicatePin
  pinMode(indicateBlue, OUTPUT);
  pinMode(indicateWhite, OUTPUT);
  pinMode(indicateYellow, OUTPUT);
  
  digitalWrite(indicateGreen, LOW);  // Start green off
  digitalWrite(indicateBlue, LOW);   // Start blue off
  digitalWrite(indicateWhite, LOW); // Start white on (default)
  digitalWrite(indicateYellow, LOW); // Start yellow off
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightnessLevels[brightnessIndex]);
  FastLED.clear(); // Clear all LEDs at startup

  // --- setup() dev checks --
  // delay(500); // Initial setup visualization

  // // Visualize the 16 sectors with distinct colors in setup()
  // for (int s = 0; s < 16; s++) {
  //   CRGB color = palette[s % 8]; // Use palette colors, repeat every 8
  //   for (int i = 0; i < sectorLens[s]; i++) {
  //     leds[sectorPixels[s][i]] = color;
  //   }
  // }
  // // Visualize the radial sections with distinct colors in setup()
  // for (int r = 0; r < numRadials; r++) {
  //   CRGB color = palette[r % 4];
  //   for (int i = 0; i < radialSizes[r]; i++) {
  //     leds[radials[r][i]] = color;
  //   }
  // }

  // // Loop through each ring group
  // for (int r = 0; r < numRings; r++) { 
  //   CRGB ringColor = palette[r % 7]; // Cycle through palette with mod 7
  //   // Color all pixels in this ring
  //   for (int i = 2; i < ringSizes[ringIndex]; i++) {
  //     leds[rings[ringIndex][i]] = ringColor;
  //   }
  // }

  // // Loop through each group of circles
  // for (int group = 0; group < numCircles; group++) { 
  //   leds[circles[group][3]] = palette[group % 8]; // Assign color from palette based on group index
  // }

  // showFlowerOutline(2); // 2 octal rotations bring White to the top
  // showStatic3(); // Show Static Mode #3 (Spectrum Pizza)
  // showWOTY(231); // Show Wheel of the Year oriented to today's day number in the year
  // showStatic2(); // Show Static Mode #2 (Foxy YellowBlue)
  // showStatic1(); // Show Static Mode #1 (MVP - Minimally Viable Mandala)
  // showStatic0(); // Show Static Mode #0 (Candy-corn)
  // showSeasonalWheel(8); // Show the Seasonal Color Wheel
  
  showFragileSpokes(2); // Show Fragile Spokes effect with white on top
  FastLED.show();
}

// --- Main loop ---
void loop() {
  if (millis() - bootTime < 1000) return;

  // --- loop() dev checks --
  // showRainbowFade(40); // 40 ms transition speed
  // // kunterbunt happy chaos spiral:
  // showSpiralFill(yearPalette, 8); // spiralFill all colors in yearPalette
  // showSpiralFill(palette, 8);     // spiralFill first 7 colors in palette
  // showRadarSweep(7);
  // showDynamicFlower(2); // 2 octal rotations bring White to the top
  
  // Read raw sensor states
  int adminSensorState = digitalRead(adminSensorPin);
  int centerSensorState = digitalRead(centerSensorPin);
  // int freeSensorState = digitalRead(freeSensorPin);
  // int edgeSensorState = digitalRead(edgeSensorPin);

  // --- sensors debouncing logic ---
  // Admin sensor debouncing
  if (adminSensorState != lastAdminState) lastAdminChange = millis();
  if ((millis() - lastAdminChange) > debounceDelay) {
    if (adminSensorState != debouncedAdminState) debouncedAdminState = adminSensorState;
  }
  lastAdminState = adminSensorState;

  // Center sensor debouncing
  if (centerSensorState != lastCenterState) lastCenterChange = millis();
  if ((millis() - lastCenterChange) > debounceDelay) {
    if (centerSensorState != debouncedCenterState) debouncedCenterState = centerSensorState;
  }
  lastCenterState = centerSensorState;

  // Indicator logic
  digitalWrite(indicateBlue, debouncedCenterState == HIGH ? HIGH : LOW);
  digitalWrite(indicateGreen, debouncedAdminState == HIGH ? HIGH : LOW);
  bool anySensorHigh = (debouncedAdminState == HIGH || debouncedCenterState == HIGH);
  digitalWrite(indicateWhite, anySensorHigh ? HIGH : LOW);
  digitalWrite(indicateYellow, !anySensorHigh ? HIGH : LOW);
  
  // Brightness control (ramp up/down on long touch)
  static bool rampUp = true; // Start with ramping up
  static uint8_t rampBrightness = brightnessLevels[brightnessIndex];
  const uint8_t minBrightness = 12;   // 5%
  const uint8_t maxBrightness = 191;  // 75%
  const uint8_t rampStep = 2;         // Change per update
  static unsigned long lastRampUpdate = 0;
  const unsigned long rampInterval = 60; // ms between brightness steps
  static bool lastDebouncedAdminState = LOW;
  if (debouncedAdminState == HIGH) {
    // Ramp brightness while held
    if (millis() - lastRampUpdate > rampInterval) {
      lastRampUpdate = millis();
      if (rampUp && rampBrightness < maxBrightness) {
        rampBrightness = (uint8_t)min((int)maxBrightness, (int)rampBrightness + (int)rampStep);
        FastLED.setBrightness(rampBrightness);
        FastLED.show();
      } else if (!rampUp && rampBrightness > minBrightness) {
        rampBrightness = (uint8_t)max((int)minBrightness, (int)rampBrightness - (int)rampStep);
        FastLED.setBrightness(rampBrightness);
        FastLED.show();
      }
    }
  }
  // Alternate ramp direction on each new touch
  if (debouncedAdminState == HIGH && lastDebouncedAdminState == LOW) {
    rampUp = !rampUp; // Switch direction each time a new touch starts
  }
  lastDebouncedAdminState = debouncedAdminState;

  // TEMPORARY: Disable loop functionality after this point for testing
  static bool disableLoop = false;
  if (disableLoop) return; // Exit loop immediately

  // --- Menu switching logic ---
  static unsigned long centerTouchStart = 0;
  static bool menuVisualizing = false;
  static unsigned long menuVisualizeStart = 0;
  static bool longTap = false;

  if (menuVisualizing) {
    // Show the correct menu visualization
    showSeasonalWheel(longTap ? ((menuLevel + 1) % 2 + 1) : (menuLevel + 1));
    if (millis() - menuVisualizeStart >= 1000) {
      menuVisualizing = false;
      if (longTap) {
        // Long tap: increment menu level and reset mode
        menuLevel = (menuLevel + 1) % 2;
        modeIndex = 0;
        currentEffect = (menuLevel == 0) ? mainModes[modeIndex] : subModes[modeIndex];
      } else {
        // Short tap: increment mode within current menu
        if (menuLevel == 0) {
          modeIndex = (modeIndex + 1) % NUM_MAIN_MODES;
          currentEffect = mainModes[modeIndex];
        } else {
          modeIndex = (modeIndex + 1) % NUM_SUB_MODES;
          currentEffect = subModes[modeIndex];
        }
      }
      altMandalaActive = true;
      lastCenterTouch = millis();
      centerTouchStart = 0;
      longTap = false;
    }
    return;
  }

  if (debouncedCenterState == HIGH) {
    if (centerTouchStart == 0) {
      centerTouchStart = millis();
    }
    if (!menuVisualizing && millis() - centerTouchStart > 1000) {
      // Long tap detected: visualize next menu level
      menuVisualizing = true;
      menuVisualizeStart = millis();
      longTap = true;
      showSeasonalWheel((menuLevel + 1) % 2 + 1);
    }
  } else {
    if (centerTouchStart != 0 && !menuVisualizing) {
      // Short tap detected: visualize current menu level
      menuVisualizing = true;
      menuVisualizeStart = millis();
      longTap = false;
      showSeasonalWheel(menuLevel + 1);
    }
    centerTouchStart = 0;
  }

  // --- Effect mode cycling ---
  static EffectMode lastEffect = STATIC1;
  if (altMandalaActive && millis() - lastCenterTouch <= DYNAMIC_PHASE_DURATION) {
    // If we just switched to SpiralFill, reset the spiral
    if (currentEffect == SpiralFill && lastEffect != SpiralFill) firstSpiralRun = true;

    switch (currentEffect) {
      case DynamicFlower:  showDynamicFlower(2); break; // 2 octal rotations bring White to the top
      case FoxyYB:         showStatic2(); break;
      case SpectrumPizza:  showStatic3(); break;
      case FragileSpokes:  showFragileSpokes(2); break; // 2 octal rotations bring White to the top
      case WotY:           showWOTY(243); break; // 243 = Aug 31th
      case TWINKLE:        showTwinkle(); break;
      // case RadarSweep:     showRadarSweep(7); break; // 7 sectors tail length
      // case FlowerOutline:  showFlowerOutline(); break; // thin static
      // case STATIC1:        showStatic1(); break;
      // case STATIC0:        showStatic0(); break;
      // case BlueCardinals:  showBlueCardinals(); break;
      // case BREATHING:      showBreathing(); break;
      // case SeasonalWheel:  showSeasonalWheel(1); break;
      // case CenterBurst:    showCenterBurst(); break;
      // case RainbowFade:    showRainbowFade(40); break; // 40 ms (slower) fade speed
      case SpiralFill:     showSpiralFill(palette, 7); break; // normal palette excluding chartreuse
      default:             showDynamicFlower(2); break;
    }
    lastEffect = currentEffect;
  } else {
    burstActive = false; // Reset burst effect state when leaving dynamic phase
    showStatic1(); // Always show DynamicFlower outside altMandala phase
  }

  // Restore mandala state after phase duration
  if (!mandalaActive && altMandalaActive && millis() - lastCenterTouch > DYNAMIC_PHASE_DURATION) {
    mandalaActive = true;
    altMandalaActive = false;
    restoreMandalaState();
  }

}

