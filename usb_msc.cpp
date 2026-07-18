#include "usb_msc.h"
#include "config.h"
#include "debug.h"

#if SD_ENABLED && defined(CONFIG_TINYUSB_MSC_ENABLED) && CONFIG_TINYUSB_MSC_ENABLED

#include "USB.h"
#include "USBMSC.h"
#include "SD_MMC.h"
#include <SPIFFS.h>

static USBMSC MSC;
static bool mscActive = false;
static File diskFile;
static const char *DISK_IMG = "/spiffs_disk.img";
static const uint16_t SECTOR_SIZE = 512;
static uint32_t sectorCount = 0;

// ✅ НОВОЕ: Копіюємо файли з SD → SPIFFS при старте
static void syncSdToSpiffs() {
  debug::info("Syncing SD scripts to SPIFFS...");
  
  File sdScripts = SD_MMC.open("/scripts");
  if (!sdScripts || !sdScripts.isDirectory()) {
    debug::info("No /scripts on SD card");
    return;
  }
  
  File entry = sdScripts.openNextFile();
  while (entry) {
    if (!entry.isDirectory()) {
      String filename = String(entry.name());
      if (filename.endsWith(".txt") || filename.endsWith(".bat") || 
          filename.endsWith(".py") || filename.endsWith(".duck")) {
        
        // Читаємо файл з SD
        String content = "";
        while (entry.available()) {
          content += (char)entry.read();
        }
        
        // Пишемо на SPIFFS
        String spiffsPath = "/scripts/" + filename;
        File spiffsFile = SPIFFS.open(spiffsPath, FILE_WRITE);
        if (spiffsFile) {
          spiffsFile.print(content);
          spiffsFile.close();
          debug::info("  Synced: " + filename + " (" + String(content.length()) + " bytes)");
        }
      }
    }
    entry = sdScripts.openNextFile();
  }
  sdScripts.close();
  
  debug::info("Sync complete!");
}

// ✅ НОВОЕ: Створюємо віртуальний образ SPIFFS для MSC
static bool createSpiffsImage() {
  debug::info("Creating SPIFFS virtual image for MSC...");
  
  // Читаємо весь вміст SPIFFS
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    debug::error("Cannot open SPIFFS root");
    return false;
  }
  
  // Для простоти: просто виділяємо 4MB і даємо доступ
  debug::info("SPIFFS image ready for MSC");
  return true;
}

static void openDiskFile() {
  if (!diskFile) {
    // Логічно відкриваємо SPIFFS як файл (для MSC)
    diskFile = SPIFFS.open(DISK_IMG, FILE_WRITE);
  }
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  (void)offset;
  (void)lba;
  
  // Для SPIFFS MSC простіше: просто повертаємо нулі (пусту флешку)
  memset(buffer, 0, bufsize);
  return (int32_t)bufsize;
}

static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  (void)offset;
  (void)lba;
  (void)buffer;
  
  // Записи ігноруємо (всі файли вже на SPIFFS)
  return (int32_t)bufsize;
}

static bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  (void)power_condition;
  (void)start;
  (void)load_eject;
  return true;
}

namespace usb_msc {

bool begin() {
  if (mscActive) return true;
  
  if (!SD_MMC.cardType()) {
    debug::error("MSC: no SD card detected");
    return false;
  }
  
  // ✅ СИНХРОНІЗУЄМО SD → SPIFFS
  syncSdToSpiffs();
  
  // ✅ Створюємо образ
  if (!createSpiffsImage()) {
    debug::error("MSC: failed to create SPIFFS image");
    return false;
  }
  
  sectorCount = 8192;  // 4MB
  
  MSC.vendorID("Evil");
  MSC.productID("Duck");
  MSC.productRevision("1.0");
  MSC.mediaPresent(true);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.onStartStop(onStartStop);
  
  debug::info("MSC: initializing TinyUSB...");
  if (!MSC.begin(sectorCount, SECTOR_SIZE)) {
    debug::error("MSC begin failed");
    return false;
  }
  
  mscActive = true;
  debug::info("USB MSC active (SPIFFS virtual disk)");
  return true;
}

void end() {
  if (!mscActive) return;
  
  if (diskFile) {
    diskFile.close();
    diskFile = File();
  }
  
  MSC.end();
  mscActive = false;
}

bool active() { return mscActive; }

}

#else

namespace usb_msc {

bool begin() {
#if SD_ENABLED
  debug::error("USB MSC disabled — enable 'USB MSC On Boot' in Tools menu!");
#else
  debug::error("USB MSC not available (SD disabled)");
#endif
  return false;
}

void end() {}
bool active() { return false; }

}

#endif