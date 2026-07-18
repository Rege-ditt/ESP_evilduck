#include "sd_card.h"
#include "config.h"
#include "debug.h"

#if SD_ENABLED
#include <SD_MMC.h>

namespace {
bool mounted = false;

String normalizePath(const String &in) {
  if (!in.length()) return "/";
  String p = in;
  if (p[0] != '/') p = "/" + p;
  return p;
}

bool isScriptName(const String &name) {
  return name.endsWith(".txt") || name.endsWith(".duck");
}
}

namespace sd_card {

bool begin() {
  mounted = false;
  
  // Ініціалізація SDMMC з пінами з config.h
  // setPins(clk, cmd, d0, d1, d2, d3) для 4-bit або (clk, cmd, d0) для 1-bit
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  
  // Початок SDMMC 1-bit режиму
  if (!SD_MMC.begin("/sdcard", true)) {  // true = 1-bit режим
    debug::error("SD card mount failed (SDMMC 1-bit)");
    return false;
  }
  
  mounted = true;
  if (!SD_MMC.exists("/scripts")) {
    SD_MMC.mkdir("/scripts");
  }
  
  debug::info("SD card mounted (SDMMC 1-bit)");
  return true;
}

bool ready() { return mounted; }

size_t totalBytes() {
  if (!mounted) return 0;
  return SD_MMC.cardSize();
}

size_t usedBytes() {
  if (!mounted) return 0;
  return SD_MMC.usedBytes();
}

size_t freeBytes() {
  if (!mounted) return 0;
  return SD_MMC.cardSize() - SD_MMC.usedBytes();
}

bool exists(const String &path) {
  if (!mounted) return false;
  return SD_MMC.exists(normalizePath(path));
}

bool mkdir(const String &path) {
  if (!mounted) return false;
  return SD_MMC.mkdir(normalizePath(path));
}

bool remove(const String &path) {
  if (!mounted) return false;
  return SD_MMC.remove(normalizePath(path));
}

bool rename(const String &from, const String &to) {
  if (!mounted) return false;
  return SD_MMC.rename(normalizePath(from), normalizePath(to));
}

File openRead(const String &path) {
  if (!mounted) return File();
  return SD_MMC.open(normalizePath(path), FILE_READ);
}

String readFile(const String &path) {
  if (!mounted) return "";
  File f = openRead(path);
  if (!f) return "";
  String out = f.readString();
  f.close();
  return out;
}

bool writeFile(const String &path, const String &content) {
  if (!mounted) return false;
  File f = SD_MMC.open(normalizePath(path), FILE_WRITE);
  if (!f) return false;
  size_t n = f.print(content);
  f.close();
  return n == content.length();
}

bool appendFile(const String &path, const String &content) {
  if (!mounted) return false;
  File f = SD_MMC.open(normalizePath(path), FILE_APPEND);
  if (!f) return false;
  size_t n = f.print(content);
  f.close();
  return n == content.length();
}

String listDir(const String &path) {
  String output;
  if (!mounted) return output;
  File root = SD_MMC.open(normalizePath(path));
  if (!root || !root.isDirectory()) return output;
  File entry = root.openNextFile();
  while (entry) {
    output += String(entry.name());
    if (entry.isDirectory()) output += "/\n";
    else output += " (" + String(entry.size()) + ")\n";
    entry = root.openNextFile();
  }
  root.close();
  return output;
}

String listScriptsJson() {
  String json = "[";
  if (!mounted) {
    json += "]";
    return json;
  }
  bool first = true;
  auto scan = [&](const char *dirPath) {
    File root = SD_MMC.open(dirPath);
    if (!root || !root.isDirectory()) return;
    File entry = root.openNextFile();
    while (entry) {
      if (!entry.isDirectory()) {
        String name = String(entry.name());
        int slash = name.lastIndexOf('/');
        if (slash >= 0) name = name.substring(slash + 1);
        if (isScriptName(name)) {
          if (!first) json += ",";
          first = false;
          json += "\"sd:" + name + "\"";
        }
      }
      entry = root.openNextFile();
    }
    root.close();
  };
  scan("/");
  scan("/scripts");
  json += "]";
  return json;
}

}

#else

namespace sd_card {
bool begin() { return false; }
bool ready() { return false; }
size_t totalBytes() { return 0; }
size_t usedBytes() { return 0; }
size_t freeBytes() { return 0; }
bool exists(const String &) { return false; }
bool mkdir(const String &) { return false; }
bool remove(const String &) { return false; }
bool rename(const String &, const String &) { return false; }
String readFile(const String &) { return ""; }
bool writeFile(const String &, const String &) { return false; }
bool appendFile(const String &, const String &) { return false; }
String listDir(const String &) { return ""; }
String listScriptsJson() { return "[]"; }
File openRead(const String &) { return File(); }
}

#endif