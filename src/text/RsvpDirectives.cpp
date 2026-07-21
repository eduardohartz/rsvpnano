#include "text/RsvpDirectives.h"

#include "board/BoardStorage.h"
#include <cstring>

#include "storage/fs/StoragePaths.h"
#include "text/LatinText.h"

namespace RsvpText {

using namespace StoragePaths;

constexpr size_t kMaxChapterTitleChars = 64;
// Directive lines longer than this keep only their prefix; the converter writes
// unbounded "@title ..." lines, so an oversized line must not abort the scan.
constexpr size_t kMaxDirectiveLineChars = 160;

bool prefixHasBoundary(const String &lowered, const char *prefix) {
  const size_t prefixLength = std::strlen(prefix);
  if (!lowered.startsWith(prefix)) {
    return false;
  }
  if (lowered.length() == prefixLength) {
    return true;
  }

  const char next = lowered[prefixLength];
  const uint8_t nextValue = LatinText::byteValue(next);
  return (nextValue <= ' ' && !LatinText::isWordCharacter(nextValue)) ||
         next == ':' || next == '.' || next == '-';
}

String stripBom(String text) {
  trimAsciiWhitespace(text);
  if (text.length() >= 3 && static_cast<uint8_t>(text[0]) == 0xEF &&
      static_cast<uint8_t>(text[1]) == 0xBB &&
      static_cast<uint8_t>(text[2]) == 0xBF) {
    text.remove(0, 3);
    trimAsciiWhitespace(text);
  }
  return text;
}

bool chapterTitleFromLine(const String &line, String &title) {
  String trimmed = normalizeDisplayText(stripBom(line));
  trimAsciiWhitespace(trimmed);
  if (trimmed.isEmpty() || trimmed.length() > kMaxChapterTitleChars) {
    return false;
  }

  if (trimmed.startsWith("#")) {
    size_t prefixLength = 0;
    while (prefixLength < trimmed.length() && trimmed[prefixLength] == '#') {
      ++prefixLength;
    }
    title = trimmed.substring(prefixLength);
    trimAsciiWhitespace(title);
    return !title.isEmpty();
  }

  String lowered = trimmed;
  lowered.toLowerCase();
  if (prefixHasBoundary(lowered, "chapter") ||
      prefixHasBoundary(lowered, "part") ||
      prefixHasBoundary(lowered, "book")) {
    title = trimmed;
    return true;
  }

  return false;
}

String directiveValue(const String &line, const char *directive) {
  String value = line.substring(std::strlen(directive));
  trimAsciiWhitespace(value);
  if (!value.isEmpty() &&
      (value[0] == ':' || value[0] == '-' || value[0] == '.')) {
    value.remove(0, 1);
    trimAsciiWhitespace(value);
  }
  return normalizeDisplayText(value);
}

RsvpDirectiveValues readRsvpDirectiveValues(const String &path) {
  RsvpDirectiveValues values;
  if (!hasRsvpExtension(path)) {
    return values;
  }

  File file = Board::Storage::filesystem().open(path);
  if (!file || file.isDirectory()) {
    if (file) {
      file.close();
    }
    return values;
  }

  // Returns true when scanning should stop.
  auto processLine = [&values](const String &rawLine) {
    String trimmed = stripBom(rawLine);
    if (trimmed.isEmpty()) {
      return false;
    }

    String lowered = trimmed;
    lowered.toLowerCase();
    if (values.title.isEmpty() && prefixHasBoundary(lowered, "@title")) {
      values.title = directiveValue(trimmed, "@title");
    } else if (values.author.isEmpty() &&
               prefixHasBoundary(lowered, "@author")) {
      values.author = directiveValue(trimmed, "@author");
    } else if (!trimmed.startsWith("@")) {
      return true;
    }

    return !values.title.isEmpty() && !values.author.isEmpty();
  };

  String line;
  line.reserve(kMaxChapterTitleChars + 16);
  bool lineOverflow = false;
  bool stopped = false;
  while (file.available()) {
    const char c = static_cast<char>(file.read());
    if (c == '\r') {
      continue;
    }

    if (c != '\n') {
      if (!lineOverflow) {
        line += c;
        lineOverflow = line.length() > kMaxDirectiveLineChars;
      }
      continue;
    }

    if (processLine(line)) {
      stopped = true;
      break;
    }
    line = "";
    lineOverflow = false;
  }
  if (!stopped && !line.isEmpty()) {
    processLine(line);
  }

  file.close();
  return values;
}

String readRsvpDirectiveValue(const String &path, const char *directive) {
  if (!hasRsvpExtension(path)) {
    return "";
  }

  File file = Board::Storage::filesystem().open(path);
  if (!file || file.isDirectory()) {
    if (file) {
      file.close();
    }
    return "";
  }

  String result;
  // Returns true when scanning should stop.
  auto processLine = [&result, directive](const String &rawLine) {
    String trimmed = stripBom(rawLine);
    if (trimmed.isEmpty()) {
      return false;
    }

    String lowered = trimmed;
    lowered.toLowerCase();
    if (prefixHasBoundary(lowered, directive)) {
      result = directiveValue(trimmed, directive);
      return true;
    }

    return !trimmed.startsWith("@");
  };

  String line;
  line.reserve(kMaxChapterTitleChars + 16);
  bool lineOverflow = false;
  bool stopped = false;
  while (file.available()) {
    const char c = static_cast<char>(file.read());
    if (c == '\r') {
      continue;
    }

    if (c != '\n') {
      if (!lineOverflow) {
        line += c;
        lineOverflow = line.length() > kMaxDirectiveLineChars;
      }
      continue;
    }

    if (processLine(line)) {
      stopped = true;
      break;
    }
    line = "";
    lineOverflow = false;
  }
  if (!stopped && !line.isEmpty()) {
    processLine(line);
  }

  file.close();
  return result;
}

// Tokenization.

} // namespace RsvpText
