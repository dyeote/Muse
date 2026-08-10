// muse_time.h  ──  Wi-Fi time sync for muse-touch-lights (ESP32-C3, v6)
//
// Wi-Fi is switched ON only long enough to fetch the real time from the
// internet, then switched OFF again. Once the time is set the ESP32 keeps
// ticking on its own internal clock, so your Clock visual reads the time
// with the radio fully off — no WS2812B interference, low power.
//
// ─── HOW TO USE ──────────────────────────────────────────────────────
//   1. Edit the three lines marked EDIT ME below.
//   2. #include "muse_time.h"   ONCE, near the top of your main sketch.
//   3. setup():  call  museTimeBegin();
//   4. loop():   call  museTimeTick();      // re-syncs itself on schedule
//   5. Clock mode:
//          int h, m, s;
//          if (museGetTime(h, m, s)) {
//              // ... map h / m / s onto the mandala ...
//          }
// ─────────────────────────────────────────────────────────────────────

#pragma once
#include <WiFi.h>
#include "time.h"

// ═══════════════════ EDIT ME ═════════════════════════════════════════
static const char* MUSE_WIFI_SSID = "Hallownest";
static const char* MUSE_WIFI_PASS = "prettyplease";
static const char* MUSE_TZ        = "CET-1CEST";   // your timezone — see note below
// ═════════════════════════════════════════════════════════════════════

static const char*    MUSE_NTP_SERVER      = "pool.ntp.org";
static const uint32_t MUSE_WIFI_TIMEOUT_MS = 10000;             // give up connecting after 10 s
static const uint32_t MUSE_RESYNC_MS       = 60UL*60UL*1000UL;  // re-sync once an hour
static const uint32_t MUSE_RETRY_MS        = 30UL*1000UL;       // retry every 30 s until first success
static const uint8_t  MUSE_MAX_ATTEMPTS    = 2;                 // give up after this many failed attempts

static bool     _museTimeSet  = false;
static bool     _museGaveUp   = false;  // stops retrying after MUSE_MAX_ATTEMPTS failures
static uint8_t  _museAttempts = 0;
static uint32_t _museLastSync = 0;


// Turn Wi-Fi on, fetch the time, turn Wi-Fi off. Returns true on success.
inline bool museSyncTimeNow() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(MUSE_WIFI_SSID, MUSE_WIFI_PASS);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < MUSE_WIFI_TIMEOUT_MS) {
        delay(100);
    }

    bool ok = false;
    if (WiFi.status() == WL_CONNECTED) {
        configTzTime(MUSE_TZ, MUSE_NTP_SERVER);   // kick off the NTP request
        struct tm t;
        if (getLocalTime(&t, 5000)) {             // wait up to 5 s for a reply
            ok = true;
            _museTimeSet = true;
        }
    }

    // Radio OFF — keeps LED timing clean and saves power.
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return ok;
}

// Call once inside setup().
inline void museTimeBegin() {
    _museAttempts = 1;
    museSyncTimeNow();
    if (!_museTimeSet && _museAttempts >= MUSE_MAX_ATTEMPTS) _museGaveUp = true;
    _museLastSync = millis();
}


// Call every loop(). Re-syncs on a schedule; does nothing in between.
inline void museTimeTick() {
    if (!_museTimeSet && _museGaveUp) return;   // already gave up, wait for reboot
    uint32_t interval = _museTimeSet ? MUSE_RESYNC_MS : MUSE_RETRY_MS;
    if (millis() - _museLastSync >= interval) {
        _museLastSync = millis();
        if (!_museTimeSet) _museAttempts++;
        museSyncTimeNow();
        if (!_museTimeSet && _museAttempts >= MUSE_MAX_ATTEMPTS) _museGaveUp = true;
    }
}


// Read the current local time. Returns false until the first sync succeeds.
inline bool museGetTime(int& hour, int& minute, int& second) {
    if (!_museTimeSet) return false;
    struct tm t;
    if (!getLocalTime(&t)) return false;   // returns instantly once time is set
    hour   = t.tm_hour;   // 0–23
    minute = t.tm_min;    // 0–59
    second = t.tm_sec;    // 0–59
    return true;
}