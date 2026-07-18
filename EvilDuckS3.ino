#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "debug.h"
#include "storage.h"
#include "settings.h"
#include "webserver.h"
#include "duckscript.h"
#include "com.h"
#include "cli.h"
#include "hid_backend.h"
#include "keyboard_locale.h"
#include "led.h"
#include "attackmode.h"
#include "script_library.h"
#include "usb_msc.h"

static void applyIdleLedColor() {
  if (settings::getDisableLed()) {
    led::setColor(0, 0, 0);
    return;
  }
  uint32_t color = settings::getLedColor();
  led::setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

// ✅ НОВОЕ: Синхронизировать SPIFFS → MSC при старте
static void syncSpiffsToMSC() {
  debug::info("Syncing SPIFFS scripts to MSC...");
  // Коли файли записані в SPIFFS, вони автоматично видні через MSC
  // Тому нічого не потрібно робити — просто MSC читатиме з SPIFFS
}

void setup() {
  delay(500);
  Serial.begin(115200);
  delay(100);
  
  debug::begin();
  debug::info("Booting EvilDuck...");

  led::begin();
  led::setColor(40, 0, 0);
  delay(120);
  led::setColor(0, 40, 0);
  delay(120);
  led::setColor(0, 0, 40);
  
  hid_backend::begin();
  keyboard_locale::begin();
  attackmode::begin();

  debug::info("Initializing storage...");
  storage::begin();
  
  debug::info("Installing script library...");
  script_library::installIfNeeded();
  
  debug::info("Loading settings...");
  settings::load();
  applyIdleLedColor();
  
  debug::info("Initializing USB MSC...");
  if (usb_msc::begin()) {
    debug::info("USB MSC initialized successfully");
  } else {
    debug::info("USB MSC initialization failed");
  }
  
  syncSpiffsToMSC();
  
  debug::info("Starting CLI...");
  cli::begin();

  debug::info("Starting webserver...");
  webserver::begin();

  debug::info("Setup complete!");
  
  if (settings::autorunEnabled()) {
    String s = settings::getAutorun();
    if (s.length()) {
      uint32_t delayMs = settings::pickAutorunDelayMs();
      if (delayMs == 0) delayMs = settings::getStealthMode() ? 8000 : 3000;
      debug::info("Autorun in " + String(delayMs) + "ms: " + s);
      delay(delayMs);
      duckscript::run(s);
    }
  }
}

void loop() {
  duckscript::update();
  com::update();
  cli::update();
  delay(1);
}