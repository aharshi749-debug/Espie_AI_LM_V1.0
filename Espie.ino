//(c) Aharshi Deb 2026
// All rights reserved
// Warning: use at your own risk

#if !defined(ARDUINO_ARCH_ESP32)
  #error "This sketch targets Arduino-ESP32."
#endif

#include "training_data.h"
#include <EEPROM.h>
#include <pgmspace.h>
#include <Arduino.h>

#define PX_MARK(c) (0xFF000000UL + (uint8_t)(c))

#define NGRAM_DEPTH       7
#define MEMORY_DEPTH        14
#define LINE_CAP      140
#define QUERY_CAP  20
#define SENT_MIN  4
#define SENT_MAX  26

static uint8_t  cfgOrder = 5;
static uint8_t  cfgTemp = 22;
static uint16_t cfgBudget = 55;
static bool     cfgTrace = true;

typedef uint32_t token_hash_t;

struct CandidateSlot {
  uint32_t start;
  uint16_t count;
  uint16_t len;
  uint8_t type;
};

#define BAND_LIMIT 32

struct CorpusSection {
  uint32_t start;
  uint32_t end;
  uint32_t nameStart;
  token_hash_t nameHash;
  uint16_t nameLen;
};

static uint32_t randomState = 0x1F123BB5UL;

static inline uint32_t nextRandom32() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  return randomState;
}

static inline uint16_t randomBelow(uint16_t n) {
  return n ? (uint16_t)(nextRandom32() % n) : 0;
}

#define STORE_CAP 4096
#define STORE_HEAD 4
#define STORE_SIG_A 0x4D
#define STORE_SIG_B 0x4C

static uint32_t flashBytes = 0;
static uint16_t storedBytes = 0;

static inline uint8_t storageRead(uint16_t a) {
  return EEPROM.read(a);
}

static inline void storageWrite(uint16_t a, uint8_t v) {
  EEPROM.write(a, v);
}

static inline void storageCommit() {
  EEPROM.commit();
}

static void storageSetLength(uint16_t n) {
  if (n > STORE_CAP - STORE_HEAD) n = 0;

  storedBytes = n;

  storageWrite(0, STORE_SIG_A);
  storageWrite(1, STORE_SIG_B);
  storageWrite(2, (uint8_t)(n & 0xFF));
  storageWrite(3, (uint8_t)(n >> 8));
  storageCommit();
}

static uint32_t scanLimit = 0;

static uint32_t rangeBegin[BAND_LIMIT + 1];
static uint32_t rangeEnd[BAND_LIMIT + 1];

static uint8_t rangeCount = 0;

static CorpusSection sections[BAND_LIMIT];
static uint8_t sectionCount = 0;

static uint8_t forcedSection = 255;
static uint8_t previousSection = 255;

static bool creativeMode = true;

static inline uint8_t activeOrder() {
  return creativeMode ? cfgOrder : NGRAM_DEPTH;
}

static uint8_t storySection = 255;

static uint16_t sectionHits[BAND_LIMIT];

static inline uint32_t modelLength() {
  return flashBytes + (uint32_t)storedBytes;
}

static inline uint8_t modelByte(uint32_t i) {
  if (i < flashBytes)
    return pgm_read_byte(&TRAIN_TEXT[i]);

  uint32_t e = i - flashBytes;

  if (e >= storedBytes)
    return 0;

  return storageRead(STORE_HEAD + (uint16_t)e);
}

static inline uint8_t lowerAscii(uint8_t c) {
  return (c >= 'A' && c <= 'Z') ? (uint8_t)(c + 32) : c;
}

static inline bool wordChar(uint8_t c) {
  return
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= '0' && c <= '9') ||
    c == '\'' ||
    c == '-' ||
    c == '_';
}

static inline bool sectionChar(uint8_t c) {
  return
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= '0' && c <= '9') ||
    c == '_' ||
    c == '-';
}

static inline bool punctChar(uint8_t c) {
  return
    c == '.' ||
    c == '!' ||
    c == '?' ||
    c == ',' ||
    c == ';' ||
    c == ':';
}

static inline bool terminalChar(uint8_t c) {
  return c == '.' || c == '!' || c == '?';
}

static inline token_hash_t normalizeHash(uint32_t h) {
  return h;
}

static uint8_t readToken(
  uint32_t &pos,
  uint32_t &start,
  uint16_t &len,
  token_hash_t &hash
) {
  const uint32_t n = scanLimit;

  while (pos < n) {
    uint8_t c = modelByte(pos);

    if (wordChar(c))
      break;

    if (punctChar(c)) {
      start = pos;
      len = 1;
      hash = PX_MARK(c);
      pos++;
      return 2;
    }

    pos++;
  }

  if (pos >= n)
    return 0;

  start = pos;

  uint32_t h = 2166136261UL;
  uint16_t l = 0;

  while (pos < n) {
    uint8_t c = modelByte(pos);

    if (!wordChar(c))
      break;

    h ^= lowerAscii(c);
    h *= 16777619UL;

    pos++;

    if (l < 65535)
      l++;
  }

  len = l;
  hash = normalizeHash(h);

  return 1;
}

static token_hash_t hashInputWord(const char *s, uint16_t n) {
  uint32_t h = 2166136261UL;

  for (uint16_t i = 0; i < n; i++) {
    h ^= lowerAscii((uint8_t)s[i]);
    h *= 16777619UL;
  }

  return normalizeHash(h);
}

static const char STOPWORDS[] PROGMEM =
"the a an and or but of to in on at is are was were be been being it its this that these those "
"for with from by as if then than so not no yes do does did done have has had will would can could "
"should may might must i you he she we they me him her them my your his our their what which who "
"whom how why when where all any some more most much many very just also about into out up down "
"over under again once here there tell say said give me please what's whats";

static const char CMD_VERBS[] PROGMEM =
"write tell make give show compose generate create describe explain draft";

static const char ASK_WORDS[] PROGMEM =
"what who whom why how when where which whats hows";

static const char META_WORDS[] PROGMEM =
"write tell make give show compose generate create describe explain draft "
"me my mine a an the some any about of for on to please can you i we "
"short long little bit brief quick nice good paragraph story stories tale "
"essay text piece something anything more another";

static token_hash_t contextStack[NGRAM_DEPTH];
static uint8_t contextDepth = 0;

static void clearContext() {
  contextDepth = 0;
}

static void forwardSentenceReset();

static void pushContext(token_hash_t h) {
  for (int8_t i = NGRAM_DEPTH - 1; i > 0; i--)
    contextStack[i] = contextStack[i - 1];

  contextStack[0] = h;

  if (contextDepth < NGRAM_DEPTH)
    contextDepth++;
}

static void resetAtSentence() {
  clearContext();
  pushContext(PX_MARK('.'));
}

static void forwardSentenceReset() {
  resetAtSentence();
}

#define HASH_ORIGIN 2166136261UL

static inline uint32_t combineHash(uint32_t k, token_hash_t h) {
  k ^= h;
  k *= 16777619UL;
  return k;
}

static bool wordListContains(const char *list, token_hash_t h) {
  uint32_t i = 0;
  uint32_t acc = 2166136261UL;
  uint16_t n = 0;

  for (;;) {
    uint8_t c = pgm_read_byte(&list[i]);

    if (wordChar(c)) {
      acc ^= lowerAscii(c);
      acc *= 16777619UL;
      n++;
    } else {
      if (n && normalizeHash(acc) == h)
        return true;

      acc = 2166136261UL;
      n = 0;

      if (c == 0)
        return false;
    }

    i++;
  }
}

static inline bool knownName(token_hash_t h) {
  return wordListContains(TRAIN_PROPER, h);
}

#define TOPIC_CAP 8

static token_hash_t topicKeys[TOPIC_CAP];
static uint8_t topicKeyCount = 0;

static bool topicContains(token_hash_t h) {
  for (uint8_t i = 0; i < topicKeyCount; i++)
    if (topicKeys[i] == h)
      return true;

  return false;
}

static bool strongTopic(token_hash_t h) {
  if (!topicContains(h))
    return false;

  if (wordListContains(STOPWORDS, h))
    return false;

  if (wordListContains(META_WORDS, h))
    return false;

  return true;
}

static CandidateSlot candidateTable[NGRAM_DEPTH + 1];
static CandidateSlot sentenceStart;

static uint32_t tokenTotal = 0;

static token_hash_t topicContext[NGRAM_DEPTH];
static uint8_t topicContextDepth = 0;

static uint16_t topicMatchCount = 0;

static void indexModel(uint8_t maxOrder) {
  uint32_t target[NGRAM_DEPTH + 1];

  uint8_t hi = contextDepth;

  if (hi > maxOrder)
    hi = maxOrder;

  {
    uint32_t k = HASH_ORIGIN;

    for (uint8_t j = 0; j < hi; j++) {
      k = combineHash(k, contextStack[j]);
      target[j + 1] = k;
    }
  }

  for (uint8_t k = 0; k <= NGRAM_DEPTH; k++)
    candidateTable[k].count = 0;

  sentenceStart.count = 0;

  token_hash_t ring[NGRAM_DEPTH];
  uint8_t ringN = 0;

  uint32_t pos = 0;
  uint32_t start;

  token_hash_t hash;

  uint16_t len;
  uint8_t type;

  bool atSentStart = true;

  uint32_t ntok = 0;

  token_hash_t ssRing[NGRAM_DEPTH];
  uint8_t ssRingN = 0;
  uint16_t ssAge = 0;

  bool ssValid = false;

  topicMatchCount = 0;
  topicContextDepth = 0;

  for (uint8_t r = 0; r < rangeCount; r++) {
    pos = rangeBegin[r];
    scanLimit = rangeEnd[r];

    ringN = 0;
    atSentStart = true;
    ssValid = false;

    while ((type = readToken(pos, start, len, hash)) != 0) {
      ntok++;

      if (type == 1 && atSentStart) {
        for (uint8_t j = 0; j < NGRAM_DEPTH; j++)
          ssRing[j] = ring[j];

        ssRingN = ringN;
        ssAge = 0;
        ssValid = true;
      } else if (ssValid && ssAge < 200) {
        ssAge++;
      }

      if (
        type == 1 &&
        ssValid &&
        ssAge <= 12 &&
        topicKeyCount &&
        strongTopic(hash)
      ) {
        topicMatchCount++;

        if (randomBelow(topicMatchCount) == 0) {
          for (uint8_t j = 0; j < NGRAM_DEPTH; j++)
            topicContext[j] = ssRing[j];

          topicContextDepth = ssRingN;
        }

        ssValid = false;
      }

      if (type == 1 && atSentStart) {
        sentenceStart.count++;

        if (randomBelow(sentenceStart.count) == 0) {
          sentenceStart.start = start;
          sentenceStart.len = len;
          sentenceStart.type = 1;
        }
      }

      if (ringN && hi) {
        uint32_t k = HASH_ORIGIN;

        uint8_t lim = (ringN < hi) ? ringN : hi;

        for (uint8_t j = 0; j < lim; j++) {
          k = combineHash(k, ring[j]);

          if (k == target[j + 1]) {
            CandidateSlot &c = candidateTable[j + 1];

            c.count++;

            if (randomBelow(c.count) == 0) {
              c.start = start;
              c.len = len;
              c.type = type;
            }
          }
        }
      }

      for (int8_t i = NGRAM_DEPTH - 1; i > 0; i--)
        ring[i] = ring[i - 1];

      ring[0] = hash;

      if (ringN < NGRAM_DEPTH)
        ringN++;

      atSentStart =
        (type == 2) &&
        terminalChar(modelByte(start));
    }
  }

  tokenTotal = ntok;
}

static bool hashSection(
  uint32_t start,
  uint32_t end,
  uint32_t &nameStart,
  uint16_t &nameLen,
  token_hash_t &nameHash
) {
  uint32_t p = start;

  while (p < end && (
    modelByte(p) == ' ' ||
    modelByte(p) == '\t'
  ))
    p++;

  if (p + 7 <= end &&
      lowerAscii(modelByte(p)) == 's' &&
      lowerAscii(modelByte(p + 1)) == 'e' &&
      lowerAscii(modelByte(p + 2)) == 'c' &&
      lowerAscii(modelByte(p + 3)) == 't' &&
      lowerAscii(modelByte(p + 4)) == 'i' &&
      lowerAscii(modelByte(p + 5)) == 'o' &&
      lowerAscii(modelByte(p + 6)) == 'n' &&
      (p + 7 == end ||
       modelByte(p + 7) == ' ' ||
       modelByte(p + 7) == '\t')
  ) {
    p += 7;

    while (p < end &&
           (modelByte(p) == ' ' || modelByte(p) == '\t'))
      p++;
  }

  nameStart = p;

  uint32_t h = 2166136261UL;
  uint16_t n = 0;

  while (p < end) {
    uint8_t c = modelByte(p);

    if (!sectionChar(c))
      break;

    h ^= lowerAscii(c);
    h *= 16777619UL;

    p++;

    if (n < 65535)
      n++;
  }

  if (!n)
    return false;

  nameLen = n;
  nameHash = normalizeHash(h);

  return true;
}

static void discoverSections() {
  sectionCount = 0;
  storySection = 255;

  uint32_t i = 0;
  uint32_t currentContent = 0;

  while (i + 1 < flashBytes) {
    if (
      pgm_read_byte(&TRAIN_TEXT[i]) == '@' &&
      pgm_read_byte(&TRAIN_TEXT[i + 1]) == '@'
    ) {
      uint32_t lineEnd = i + 2;

      while (
        lineEnd < flashBytes &&
        pgm_read_byte(&TRAIN_TEXT[lineEnd]) != '\n'
      )
        lineEnd++;

      uint32_t nameStart;
      uint16_t nameLen;
      token_hash_t nameHash;

      if (hashSection(
        i + 2,
        lineEnd,
        nameStart,
        nameLen,
        nameHash
      )) {
        if (sectionCount > 0)
          sections[sectionCount - 1].end = i;

        if (sectionCount < BAND_LIMIT) {
          uint32_t contentStart =
            (lineEnd < flashBytes) ? lineEnd + 1 : lineEnd;

          sections[sectionCount].start = contentStart;
          sections[sectionCount].end = flashBytes;
          sections[sectionCount].nameStart = nameStart;
          sections[sectionCount].nameLen = nameLen;
          sections[sectionCount].nameHash = nameHash;

          if (
            nameLen == 7 &&
            nameHash == hashInputWord("stories", 7)
          )
            storySection = sectionCount;

          sectionCount++;
          currentContent = contentStart;
        }
      }

      i = lineEnd;

      if (i < flashBytes)
        i++;

      continue;
    }

    i++;
  }

  if (sectionCount == 0 && flashBytes) {
    sections[0].start = 0;
    sections[0].end = flashBytes;
    sections[0].nameStart = 0xFFFFFFFFUL;
    sections[0].nameLen = 4;
    sections[0].nameHash = hashInputWord("main", 4);
    sectionCount = 1;
  }

  if (storedBytes && sectionCount < BAND_LIMIT) {
    sections[sectionCount].start = flashBytes;
    sections[sectionCount].end = modelLength();
    sections[sectionCount].nameStart = 0xFFFFFFFFUL;
    sections[sectionCount].nameLen = 7;
    sections[sectionCount].nameHash = hashInputWord("learned", 7);
    sectionCount++;
  }

  storySection = 255;

  token_hash_t sh = hashInputWord("stories", 7);

  for (uint8_t k = 0; k < sectionCount; k++) {
    if (sections[k].nameHash == sh) {
      storySection = k;
      break;
    }
  }
}

static void showSectionName(uint8_t k) {
  if (k >= sectionCount)
    return;

  if (sections[k].nameStart == 0xFFFFFFFFUL) {
    Serial.print(F("learned"));
    return;
  }

  for (uint16_t j = 0; j < sections[k].nameLen; j++)
    Serial.print(
      (char)pgm_read_byte(
        &TRAIN_TEXT[sections[k].nameStart + j]
      )
    );
}

static void showSections() {
  for (uint8_t k = 0; k < sectionCount; k++) {
    if (k)
      Serial.print(F(", "));

    showSectionName(k);
  }
}

static void selectScan(uint8_t seg) {
  rangeCount = 0;

  if (!sectionCount) {
    rangeBegin[0] = 0;
    rangeEnd[0] = modelLength();
    rangeCount = 1;
    return;
  }

  if (seg < sectionCount) {
    rangeBegin[0] = sections[seg].start;
    rangeEnd[0] = sections[seg].end;
    rangeCount = 1;
    return;
  }

  for (uint8_t k = 0; k < sectionCount && rangeCount < BAND_LIMIT + 1; k++) {
    rangeBegin[rangeCount] = sections[k].start;
    rangeEnd[rangeCount] = sections[k].end;
    rangeCount++;
  }
}

static uint16_t measureSections() {
  uint16_t total = 0;

  for (uint8_t k = 0; k < sectionCount; k++) {
    uint16_t cnt = 0;

    uint32_t pos = sections[k].start;
    uint32_t start;

    token_hash_t hash;

    uint16_t len;
    uint8_t type;

    scanLimit = sections[k].end;

    while ((type = readToken(pos, start, len, hash)) != 0) {
      if (type == 1 && strongTopic(hash))
        cnt++;
    }

    sectionHits[k] = cnt;

    if (UINT16_MAX - total < cnt)
      total = UINT16_MAX;
    else
      total += cnt;
  }

  return total;
}

static uint8_t chooseSection(bool skipStory) {
  uint8_t bi = 255;
  uint16_t best = 0;

  for (uint8_t k = 0; k < sectionCount; k++) {
    if (skipStory && k == storySection)
      continue;

    if (sectionHits[k] > best) {
      best = sectionHits[k];
      bi = k;
    }
  }

  return bi;
}

static bool capitalizeNext = true;
static bool outputStarted = false;

static bool preserveCase(
  uint32_t start,
  uint16_t len,
  token_hash_t hash
) {
  for (uint16_t i = 1; i < len; i++) {
    uint8_t c = modelByte(start + i);

    if (c >= 'A' && c <= 'Z')
      return true;
  }

  if (len == 1 && modelByte(start) == 'I')
    return true;

  return knownName(hash);
}

static void writeToken(
  uint32_t start,
  uint16_t len,
  uint8_t type,
  token_hash_t hash
) {
  if (type == 2) {
    Serial.print((char)modelByte(start));

    if (terminalChar(modelByte(start)))
      capitalizeNext = true;

    outputStarted = true;
    return;
  }

  if (outputStarted)
    Serial.print(' ');

  bool keep = preserveCase(start, len, hash);

  for (uint16_t i = 0; i < len; i++) {
    uint8_t c = modelByte(start + i);

    if (i == 0) {
      if (capitalizeNext)
        c = (c >= 'a' && c <= 'z')
          ? (uint8_t)(c - 32)
          : c;
      else if (!keep)
        c = lowerAscii(c);
    } else if (!keep) {
      c = lowerAscii(c);
    }

    Serial.print((char)c);
  }

  capitalizeNext = false;
  outputStarted = true;
}

static token_hash_t recentTokens[MEMORY_DEPTH];
static uint8_t recentCount = 0;
static uint8_t lastPunctuation = 0;

#define POSITION_MEMORY 72

static uint32_t recentPositions[POSITION_MEMORY];
static uint8_t recentPositionCount = 0;

static void rememberPosition(uint32_t p) {
  for (int8_t i = POSITION_MEMORY - 1; i > 0; i--)
    recentPositions[i] = recentPositions[i - 1];

  recentPositions[0] = p;

  if (recentPositionCount < POSITION_MEMORY)
    recentPositionCount++;
}

static bool positionKnown(uint32_t p) {
  for (uint8_t i = 0; i < recentPositionCount; i++)
    if (recentPositions[i] == p)
      return true;

  return false;
}

static void rememberToken(token_hash_t h) {
  for (int8_t i = MEMORY_DEPTH - 1; i > 0; i--)
    recentTokens[i] = recentTokens[i - 1];

  recentTokens[0] = h;

  if (recentCount < MEMORY_DEPTH)
    recentCount++;
}

static uint8_t tokenFrequency(token_hash_t h) {
  uint8_t n = 0;

  for (uint8_t i = 0; i < recentCount; i++)
    if (recentTokens[i] == h)
      n++;

  return n;
}

static token_hash_t candidateTokenHash(const CandidateSlot &c) {
  if (c.type == 2)
    return PX_MARK(modelByte(c.start));

  uint32_t h = 2166136261UL;

  for (uint16_t i = 0; i < c.len; i++) {
    h ^= lowerAscii(modelByte(c.start + i));
    h *= 16777619UL;
  }

  return normalizeHash(h);
}

static bool candidateAllowed(
  const CandidateSlot &c,
  uint8_t sentWords,
  bool mustEnd
) {
  token_hash_t h = candidateTokenHash(c);

  if (c.type == 1 && positionKnown(c.start))
    return false;

  if (c.type == 2) {
    uint8_t ch = modelByte(c.start);

    if (terminalChar(ch))
      return sentWords >= SENT_MIN;

    if (mustEnd)
      return false;

    if (lastPunctuation)
      return false;

    if (sentWords < 3)
      return false;

    return true;
  }

  if (mustEnd)
    return false;

  if (recentCount && recentTokens[0] == h)
    return false;

  if (c.len > 3 && tokenFrequency(h) >= 2)
    return false;

  return true;
}

static uint16_t orderScoreSum = 0;
static uint16_t orderSampleCount = 0;

static bool oneSentenceMode = false;

static uint16_t synthesize(
  uint16_t budget,
  uint8_t sentPerPara,
  uint8_t reanchorPct
) {
  uint16_t emitted = 0;
  uint8_t sentWords = 0;
  uint8_t copyRun = 0;
  uint16_t sentences = 0;

  lastPunctuation = 0;

  bool justAnchored = true;
  bool sentHadTopic = true;

  uint32_t usedAnchor[4] = {
    0, 0, 0, 0
  };

  const uint16_t hardCap =
    (budget > 65535 - 40)
      ? 65535
      : budget + 40;

  orderScoreSum = 0;
  orderSampleCount = 0;

  while (emitted < hardCap) {
    indexModel(activeOrder());

    uint8_t hi = 0;

    for (uint8_t k = cfgOrder; k >= 1; k--) {
      if (candidateTable[k].count) {
        hi = k;
        break;
      }
    }

    const uint8_t temp =
      creativeMode
        ? cfgTemp
        : (cfgTemp > 8 ? 8 : cfgTemp);

    const uint8_t floorOrder =
      creativeMode
        ? ((temp <= 30) ? 4 :
           ((temp <= 65) ? 3 : 2))
        : NGRAM_DEPTH;

    while (
      hi > floorOrder &&
      randomBelow(100) < temp
    )
      hi--;

    if (creativeMode && copyRun >= 3) {
      uint8_t f =
        (floorOrder > 2)
          ? (uint8_t)(floorOrder - 1)
          : 2;

      if (hi > f)
        hi = f;

      copyRun = 0;
    }

    if (
      recentCount >= 6 &&
      recentTokens[0] == recentTokens[3] &&
      recentTokens[1] == recentTokens[4] &&
      recentTokens[2] == recentTokens[5]
    )
      hi = 2;

    bool mustEnd =
      sentWords >= SENT_MAX;

    CandidateSlot chosen;
    chosen.count = 0;

    uint8_t usedOrder = 0;

    for (uint8_t k = hi; k >= 1; k--) {
      if (
        candidateTable[k].count &&
        candidateAllowed(
          candidateTable[k],
          sentWords,
          mustEnd
        )
      ) {
        chosen = candidateTable[k];
        usedOrder = k;
        break;
      }
    }

    if (!chosen.count && mustEnd) {
      Serial.print('.');
      capitalizeNext = true;
      outputStarted = true;
      lastPunctuation = '.';

      rememberToken(PX_MARK('.'));

      emitted++;
      sentences++;
      sentWords = 0;

      if (
        sentPerPara &&
        (sentences % sentPerPara) == 0
      ) {
        Serial.println();
        Serial.println();
        outputStarted = false;
      }

      if (emitted >= budget)
        break;

      continue;
    }

    if (!chosen.count) {
      for (uint8_t k = hi; k >= 1; k--) {
        if (candidateTable[k].count) {
          chosen = candidateTable[k];
          usedOrder = k;
          break;
        }
      }
    }

    if (!chosen.count) {
      if (!sentenceStart.count)
        break;

      chosen = sentenceStart;
      usedOrder = 0;

      resetAtSentence();
    }

    copyRun =
      (usedOrder >= 3 && chosen.count == 1)
        ? (uint8_t)(copyRun + 1)
        : 0;

    orderScoreSum += usedOrder;
    orderSampleCount++;

    token_hash_t h = candidateTokenHash(chosen);

    writeToken(
      chosen.start,
      chosen.len,
      chosen.type,
      h
    );

    pushContext(h);
    rememberToken(h);

    lastPunctuation =
      (chosen.type == 2)
        ? modelByte(chosen.start)
        : 0;

    if (chosen.type == 1)
      rememberPosition(chosen.start);

    emitted++;

    if (chosen.type == 1) {
      sentWords++;

      if (strongTopic(h))
        sentHadTopic = true;
    } else if (
      terminalChar(modelByte(chosen.start))
    ) {
      sentences++;
      sentWords = 0;

      if (oneSentenceMode)
        break;

      if (emitted >= budget)
        break;

      if (
        sentPerPara &&
        (sentences % sentPerPara) == 0
      ) {
        Serial.println();
        Serial.println();
        outputStarted = false;
      }

      bool drifted =
        (reanchorPct && !sentHadTopic);

      sentHadTopic = false;

      if (
        reanchorPct &&
        topicContextDepth &&
        !justAnchored &&
        (
          drifted ||
          randomBelow(100) < reanchorPct
        )
      ) {
        uint32_t sig = HASH_ORIGIN;

        for (uint8_t j = 0; j < topicContextDepth; j++)
          sig = combineHash(
            sig,
            topicContext[j]
          );

        bool seen = false;

        for (uint8_t j = 0; j < 4; j++)
          if (usedAnchor[j] == sig)
            seen = true;

        if (!seen) {
          for (int8_t j = 3; j > 0; j--)
            usedAnchor[j] = usedAnchor[j - 1];

          usedAnchor[0] = sig;

          for (uint8_t j = 0; j < NGRAM_DEPTH; j++)
            contextStack[j] = topicContext[j];

          contextDepth = topicContextDepth;
          copyRun = 0;
          justAnchored = true;
        } else {
          justAnchored = false;
        }
      } else {
        justAnchored = false;
      }
    }
  }

  return emitted;
}

static token_hash_t promptTokens[QUERY_CAP];
static uint8_t promptTokenCount = 0;

static uint8_t tokenizePrompt(
  const char *s,
  bool pushCtx
) {
  promptTokenCount = 0;
  topicKeyCount = 0;
  topicContextDepth = 0;

  if (pushCtx)
    clearContext();

  uint8_t count = 0;

  const char *p = s;

  while (*p) {
    if (wordChar((uint8_t)*p)) {
      const char *w = p;

      while (
        *p &&
        wordChar((uint8_t)*p)
      )
        p++;

      uint16_t wl =
        (uint16_t)(p - w);

      token_hash_t h =
        hashInputWord(w, wl);

      if (pushCtx)
        pushContext(h);

      if (promptTokenCount < QUERY_CAP)
        promptTokens[promptTokenCount++] = h;

      if (
        wl >= 3 &&
        topicKeyCount < TOPIC_CAP &&
        !wordListContains(STOPWORDS, h) &&
        !wordListContains(META_WORDS, h)
      )
        topicKeys[topicKeyCount++] = h;

      if (
        wl >= 4 &&
        (w[wl - 1] == 's' ||
         w[wl - 1] == 'S')
      ) {
        token_hash_t sh =
          hashInputWord(
            w,
            wl - 1
          );

        if (promptTokenCount < QUERY_CAP)
          promptTokens[promptTokenCount++] = sh;

        if (
          topicKeyCount < TOPIC_CAP &&
          !wordListContains(STOPWORDS, sh) &&
          !wordListContains(META_WORDS, sh)
        )
          topicKeys[topicKeyCount++] = sh;
      }

      count++;
    } else if (
      punctChar((uint8_t)*p)
    ) {
      if (pushCtx)
        pushContext(
          PX_MARK(
            (uint8_t)*p
          )
        );

      count++;
      p++;
    } else {
      p++;
    }
  }

  return count;
}

static bool promptContains(token_hash_t h) {
  for (uint8_t i = 0; i < promptTokenCount; i++)
    if (promptTokens[i] == h)
      return true;

  return false;
}

static token_hash_t firstPromptHash(
  const char *p,
  uint16_t &lenOut
) {
  while (*p == ' ')
    p++;

  uint16_t n = 0;

  while (
    p[n] &&
    wordChar((uint8_t)p[n])
  )
    n++;

  lenOut = n;

  return n
    ? hashInputWord(p, n)
    : 0;
}

static bool socialPrompt(const char *s) {
  static const char WORDS[] PROGMEM =
    "hi hello hey yo sup howdy yeah yes yep yup nah no "
    "okay ok sure alright thanks thank please cool great nice awesome "
    "lol lmao haha hehe hmm hm uh um morning afternoon evening night "
    "welcome goodbye bye good fine well sorry wow really "
    "how are you your doing do does did can could would should "
    "like love think feel want need something interesting";

  uint8_t words = 0;
  uint8_t conversational = 0;
  uint8_t topic = 0;

  const char *p = s;

  while (*p) {
    while (*p && !wordChar((uint8_t)*p))
      p++;

    if (!*p)
      break;

    const char *w = p;

    while (*p && wordChar((uint8_t)*p))
      p++;

    uint16_t len = (uint16_t)(p - w);

    if (!len)
      continue;

    token_hash_t h = hashInputWord(w, len);

    words++;

    if (wordListContains(WORDS, h))
      conversational++;

    if (
      len >= 3 &&
      !wordListContains(STOPWORDS, h) &&
      !wordListContains(META_WORDS, h) &&
      !wordListContains(WORDS, h)
    )
      topic++;
  }

  if (!words)
    return false;

  if (topic == 0 && conversational > 0)
    return true;

  return false;
}

static uint8_t classifyInput(
  const char **text
) {
  const char *p = *text;

  while (*p == ' ')
    p++;

  uint16_t n = 0;

  token_hash_t h =
    firstPromptHash(p, n);

  if (!n)
    return 0;

  if (socialPrompt(p))
    return 0;

  if (wordListContains(ASK_WORDS, h))
    return 1;

  if (!wordListContains(CMD_VERBS, h))
    return 0;

  const token_hash_t s1 =
    hashInputWord("story", 5);

  const token_hash_t s2 =
    hashInputWord("stories", 7);

  const token_hash_t s3 =
    hashInputWord("tale", 4);

  bool wantsStory = false;

  for (const char *r = p; *r;) {
    if (wordChar((uint8_t)*r)) {
      const char *w = r;

      while (
        *r &&
        wordChar((uint8_t)*r)
      )
        r++;

      token_hash_t wh =
        hashInputWord(
          w,
          (uint16_t)(r - w)
        );

      if (
        wh == s1 ||
        wh == s2 ||
        wh == s3
      )
        wantsStory = true;
    } else {
      r++;
    }
  }

  for (;;) {
    while (*p == ' ')
      p++;

    uint16_t m = 0;

    while (
      p[m] &&
      wordChar((uint8_t)p[m])
    )
      m++;

    if (!m)
      break;

    if (
      !wordListContains(
        META_WORDS,
        hashInputWord(p, m)
      )
    )
      break;

    p += m;
  }

  while (
    *p == ' ' ||
    *p == ',' ||
    *p == ':'
  )
    p++;

  *text = p;

  return wantsStory ? 2 : 1;
}

static void showTopics(
  const char *prompt
) {
  bool any = false;

  for (const char *r = prompt; *r;) {
    if (wordChar((uint8_t)*r)) {
      const char *w = r;

      while (
        *r &&
        wordChar((uint8_t)*r)
      )
        r++;

      uint16_t l =
        (uint16_t)(r - w);

      token_hash_t h =
        hashInputWord(w, l);

      if (
        l >= 3 &&
        !wordListContains(STOPWORDS, h) &&
        !wordListContains(META_WORDS, h)
      ) {
        if (any)
          Serial.print(F(", "));

        for (uint16_t i = 0; i < l; i++)
          Serial.print(
            (char)lowerAscii((uint8_t)w[i])
          );

        any = true;
      }
    } else {
      r++;
    }
  }

  if (!any)
    Serial.print(F("(none)"));
}

#define FACT_SCORE_MIN 3
#define FACT_MARGIN_MIN 1

static bool lookupFact(
  uint16_t &ansStart,
  uint16_t &ansLen,
  uint8_t &bestScore,
  uint8_t skip
) {
  uint32_t i = 0;

  bestScore = 0;

  uint8_t found = 0;
  bool any = false;

  for (;;) {
    uint32_t lineStart = i;

    while (
      pgm_read_byte(&TRAIN_FACTS[i]) &&
      pgm_read_byte(&TRAIN_FACTS[i]) != '\n'
    )
      i++;

    uint32_t lineEnd = i;

    if (lineEnd > lineStart) {
      uint32_t sep = lineStart;

      while (
        sep < lineEnd &&
        pgm_read_byte(&TRAIN_FACTS[sep]) != '~'
      )
        sep++;

      if (sep < lineEnd) {
        uint8_t score = 0;
        uint8_t kw = 0;

        uint32_t h = 2166136261UL;
        uint16_t n = 0;

        for (
          uint32_t j = lineStart;
          j <= sep;
          j++
        ) {
          uint8_t c =
            (j < sep)
              ? pgm_read_byte(
                  &TRAIN_FACTS[j]
                )
              : ' ';

          if (wordChar(c)) {
            h ^= lowerAscii(c);
            h *= 16777619UL;
            n++;
          } else {
            if (n >= 2) {
              token_hash_t wh =
                normalizeHash(h);

              if (
                promptContains(wh) &&
                !wordListContains(
                  STOPWORDS,
                  wh
                )
              ) {
                score +=
                  (kw == 0)
                    ? 3
                    : 1;
              }

              kw++;
            }

            h = 2166136261UL;
            n = 0;
          }
        }

        if (
          score >= FACT_SCORE_MIN &&
          (
            score > bestScore ||
            (
              score == bestScore &&
              !any
            )
          )
        ) {
          if (score > bestScore) {
            bestScore = score;
            found = 0;
            any = false;
          }

          if (
            found == skip &&
            !any
          ) {
            uint32_t a = sep + 1;

            while (
              a < lineEnd &&
              pgm_read_byte(&TRAIN_FACTS[a]) == ' '
            )
              a++;

            uint32_t answerLength =
              lineEnd - a;

            if (
              answerLength > 0 &&
              answerLength <= UINT16_MAX
            ) {
              ansStart = (uint16_t)a;
              ansLen =
                (uint16_t)answerLength;

              any = true;
            }
          }

          found++;
        }
      }
    }

    if (!pgm_read_byte(&TRAIN_FACTS[i]))
      break;

    i++;
  }

  return any &&
         bestScore >= FACT_SCORE_MIN;
}

static void showFact(
  uint16_t s,
  uint16_t n
) {
  for (uint16_t i = 0; i < n; i++)
    Serial.print(
      (char)pgm_read_byte(
        &TRAIN_FACTS[s + i]
      )
    );
}

#define LABEL_CAP 64

static void runClassifier() {
  token_hash_t labelHash[LABEL_CAP];
  uint32_t labelStart[LABEL_CAP];
  uint16_t labelLen[LABEL_CAP];
  uint16_t score[LABEL_CAP];

  uint8_t nl = 0;

  uint32_t i = 0;

  for (;;) {
    uint32_t lineStart = i;

    while (
      pgm_read_byte(&TRAIN_EXAMPLES[i]) &&
      pgm_read_byte(&TRAIN_EXAMPLES[i]) != '\n'
    )
      i++;

    uint32_t lineEnd = i;

    uint32_t sep = lineStart;

    while (
      sep < lineEnd &&
      pgm_read_byte(&TRAIN_EXAMPLES[sep]) != '~'
    )
      sep++;

    if (sep < lineEnd) {
      uint32_t ls = lineStart;

      while (
        ls < sep &&
        pgm_read_byte(&TRAIN_EXAMPLES[ls]) == ' '
      )
        ls++;

      uint32_t le = sep;

      while (
        le > ls &&
        pgm_read_byte(&TRAIN_EXAMPLES[le - 1]) == ' '
      )
        le--;

      uint32_t h = 2166136261UL;

      for (
        uint32_t j = ls;
        j < le;
        j++
      ) {
        h ^= lowerAscii(
          pgm_read_byte(
            &TRAIN_EXAMPLES[j]
          )
        );

        h *= 16777619UL;
      }

      token_hash_t lh = normalizeHash(h);

      int16_t idx = -1;

      for (uint8_t k = 0; k < nl; k++) {
        if (labelHash[k] == lh) {
          idx = k;
          break;
        }
      }

      if (idx < 0 && nl < LABEL_CAP) {
        idx = nl;

        labelHash[nl] = lh;
        labelStart[nl] = ls;
        labelLen[nl] =
          (uint16_t)(le - ls);

        score[nl] = 0;

        nl++;
      }

      if (idx >= 0) {
        uint32_t wh = 2166136261UL;
        uint16_t n = 0;

        for (
          uint32_t j = sep + 1;
          j <= lineEnd;
          j++
        ) {
          uint8_t c =
            (j < lineEnd)
              ? pgm_read_byte(
                  &TRAIN_EXAMPLES[j]
                )
              : ' ';

          if (wordChar(c)) {
            wh ^= lowerAscii(c);
            wh *= 16777619UL;
            n++;
          } else {
            if (n >= 3) {
              token_hash_t f =
                normalizeHash(wh);

              if (
                promptContains(f) &&
                !wordListContains(
                  STOPWORDS,
                  f
                )
              )
                score[idx]++;
            }

            wh = 2166136261UL;
            n = 0;
          }
        }
      }
    }

    if (!pgm_read_byte(&TRAIN_EXAMPLES[i]))
      break;

    i++;
  }

  int16_t best = -1;
  int16_t second = -1;
  uint8_t bi = 0;

  uint32_t total = 0;

  for (uint8_t k = 0; k < nl; k++) {
    total += score[k];

    if (score[k] > best) {
      second = best;
      best = score[k];
      bi = k;
    } else if (score[k] > second) {
      second = score[k];
    }
  }

  if (
    best < 2 ||
    (best - second) < 1
  ) {
    Serial.println(
      F("label: unknown  (insufficient feature evidence)")
    );
    return;
  }

  Serial.print(F("label: "));

  for (
    uint16_t j = 0;
    j < labelLen[bi];
    j++
  )
    Serial.print(
      (char)pgm_read_byte(
        &TRAIN_EXAMPLES[
          labelStart[bi] + j
        ]
      )
    );

  Serial.print(F("   match strength: "));

  uint32_t pct =
    total
      ? (100UL * (uint32_t)best) / total
      : 0;

  Serial.print((unsigned long)pct);

  Serial.print(F("%  (features matched: "));
  Serial.print((unsigned int)best);
  Serial.println(F(")"));
}

static const char *calcCursor;

static double expression();

static void skipSpaces() {
  while (*calcCursor == ' ')
    calcCursor++;
}

static double atom() {
  skipSpaces();

  if (*calcCursor == '(') {
    calcCursor++;

    double v = expression();

    skipSpaces();

    if (*calcCursor == ')')
      calcCursor++;

    return v;
  }

  if (*calcCursor == '-') {
    calcCursor++;
    return -atom();
  }

  if (*calcCursor == '+') {
    calcCursor++;
    return atom();
  }

  double v = 0;
  bool any = false;

  while (
    *calcCursor >= '0' &&
    *calcCursor <= '9'
  ) {
    v =
      v * 10 +
      (*calcCursor - '0');

    calcCursor++;
    any = true;
  }

  if (*calcCursor == '.') {
    calcCursor++;

    double f = 0.1;

    while (
      *calcCursor >= '0' &&
      *calcCursor <= '9'
    ) {
      v +=
        (*calcCursor - '0') * f;

      f *= 0.1;
      calcCursor++;
      any = true;
    }
  }

  if (!any)
    return 0;

  return v;
}

static double power() {
  double b = atom();

  skipSpaces();

  if (*calcCursor == '^') {
    calcCursor++;

    double e = power();

    double r = 1;

    int n = (int)e;

    bool inv = n < 0;

    if (inv)
      n = -n;

    for (int i = 0; i < n; i++)
      r *= b;

    return inv
      ? 1.0 / r
      : r;
  }

  return b;
}

static double term() {
  double v = power();

  for (;;) {
    skipSpaces();

    if (*calcCursor == '*') {
      calcCursor++;
      v *= power();
    } else if (*calcCursor == '/') {
      calcCursor++;

      double d = power();

      v = d ? v / d : 0;
    } else if (*calcCursor == '%') {
      calcCursor++;

      long d =
        (long)power();

      v =
        d
          ? (double)((long)v % d)
          : 0;
    } else {
      return v;
    }
  }
}

static double expression() {
  double v = term();

  for (;;) {
    skipSpaces();

    if (*calcCursor == '+') {
      calcCursor++;
      v += term();
    } else if (*calcCursor == '-') {
      calcCursor++;
      v -= term();
    } else {
      return v;
    }
  }
}

static void scanLexeme(
  const char *pat,
  uint8_t mode
) {
  uint16_t pn =
    (uint16_t)strlen(pat);

  uint32_t i = 0;
  uint16_t hits = 0;

  char w[64];
  uint16_t n = 0;

  for (;;) {
    uint8_t c =
      pgm_read_byte(
        &TRAIN_WORDS[i]
      );

    if (wordChar(c)) {
      if (n < sizeof(w) - 1)
        w[n++] = (char)lowerAscii(c);
    } else if (n) {
      w[n] = 0;

      bool hit = false;

      if (mode == 0) {
        hit =
          (n >= pn) &&
          (strncmp(
            w,
            pat,
            pn
          ) == 0);
      } else {
        hit =
          (n >= pn) &&
          (strcmp(
            w + n - pn,
            pat
          ) == 0);
      }

      if (hit) {
        if (hits)
          Serial.print(F(", "));

        Serial.print(w);

        hits++;

        if (hits >= 40)
          break;
      }

      n = 0;
    }

    if (!c)
      break;

    i++;
  }

  if (!hits)
    Serial.print(
      F("(no match in lexicon)")
    );

  Serial.println();

  Serial.print(F("matches: "));
  Serial.println((int)hits);
}

static uint16_t countLexemes() {
  uint32_t i = 0;
  uint16_t n = 0;

  bool in = false;

  for (;;) {
    uint8_t c =
      pgm_read_byte(
        &TRAIN_WORDS[i]
      );

    if (wordChar(c)) {
      if (!in) {
        n++;
        in = true;
      }
    } else {
      in = false;
    }

    if (!c)
      break;

    i++;
  }

  return n;
}

static int measureFreeMemory() {
  return (int)ESP.getFreeHeap();
}

static void traceLine(
  const char *label
) {
  if (!cfgTrace)
    return;

  Serial.print(F("[debug] "));
  Serial.print(label);
}

static void printBanner() {
  Serial.println();

  Serial.println(
    F("+--------------------------------------------------+")
  );

  Serial.println(
    F("|  Espie v1.0 - n-gram language model on ESP32-S3 |")
  );

  Serial.println(
    F("+--------------------------------------------------+")
  );

  Serial.print(F("  corpus     : "));
  Serial.print(modelLength());

  Serial.print(F(" bytes ("));
  Serial.print(flashBytes);

  Serial.print(F(" flash + "));
  Serial.print(storedBytes);

  Serial.println(F(" learned)"));

  Serial.print(F("  sections   : "));
  showSections();
  Serial.println();

  Serial.print(F("  lexicon    : "));
  Serial.print(countLexemes());
  Serial.println(F(" words"));

  Serial.print(F("  max order  : "));
  Serial.println((int)cfgOrder);

  Serial.print(F("  free heap  : "));
  Serial.print(measureFreeMemory());
  Serial.println(F(" bytes"));

  Serial.println(
    F("  type /help, or just type a sentence to complete it")
  );

  Serial.println();
}

static void printReport(
  uint32_t ms,
  uint16_t promptTok,
  uint16_t outTok
) {
  Serial.println();

  Serial.println(
    F("--------------------------------------------------")
  );

  Serial.print(F("time "));
  Serial.print(ms);
  Serial.print(F(" ms"));

  Serial.print(F("  |  tokens "));
  Serial.print(promptTok);
  Serial.print(F(" in / "));

  Serial.print(outTok);
  Serial.print(F(" out / "));

  Serial.print(
    (uint16_t)(
      promptTok + outTok
    )
  );

  Serial.print(F(" total"));

  if (ms && outTok) {
    Serial.print(F("  |  "));

    Serial.print(
      (double)outTok *
      1000.0 /
      (double)ms,
      1
    );

    Serial.print(F(" tok/s"));
  }

  if (orderSampleCount) {
    Serial.print(F("  |  avg order "));

    Serial.print(
      (double)orderScoreSum /
      (double)orderSampleCount,
      2
    );
  }

  Serial.print(F("  |  section "));

  if (previousSection < sectionCount)
    showSectionName(previousSection);
  else
    Serial.print(F("all"));

  Serial.print(F("  |  free heap "));
  Serial.print(measureFreeMemory());

  Serial.println();

  Serial.println(
    F("--------------------------------------------------")
  );

  Serial.println();
}

static bool answerFromFact(
  const char *prompt,
  uint16_t &fs,
  uint16_t &fl,
  uint8_t &fscore
) {
  bool haveFact =
    lookupFact(
      fs,
      fl,
      fscore,
      0
    );

  if (!haveFact)
    return false;

  if (fscore < FACT_SCORE_MIN)
    return false;

  return true;
}

static void processInput(
  const char *prompt,
  uint8_t mode,
  uint16_t budget
) {
  uint32_t t0 = millis();

  capitalizeNext = true;
  outputStarted = false;
  recentCount = 0;
  recentPositionCount = 0;

  uint16_t ptok =
    tokenizePrompt(
      prompt,
      true
    );

  if (cfgTrace) {
    traceLine("tokenize     -> ");
    Serial.print(ptok);

    Serial.print(F(" tokens, "));
    Serial.print(
      (int)topicKeyCount
    );

    Serial.println(
      F(" topic anchors")
    );
  }

  selectScan(255);

  uint16_t hits = 0;
  uint8_t seg = 255;

  if (
    sectionCount &&
    topicKeyCount
  )
    hits = measureSections();

  if (forcedSection != 255) {
    seg = forcedSection;
  } else if (
    sectionCount &&
    topicKeyCount &&
    hits
  ) {
    if (
      mode == 2 &&
      storySection != 255 &&
      sectionHits[storySection]
    )
      seg = storySection;
    else
      seg =
        chooseSection(
          mode == 1
        );
  } else if (
    mode == 2 &&
    storySection != 255
  ) {
    seg = storySection;
  }

  selectScan(seg);

  previousSection = seg;

  creativeMode =
    (storySection != 255) &&
    (seg == storySection);

  uint16_t fs = 0;
  uint16_t fl = 0;
  uint8_t fscore = 0;

  bool haveFact = false;

  if (mode == 1) {
    haveFact =
      answerFromFact(
        prompt,
        fs,
        fl,
        fscore
      );

    if (cfgTrace) {
      traceLine("route        -> ");

      if (seg < sectionCount) {
        Serial.print(F("section "));
        showSectionName(seg);
        Serial.print(F(", "));
        Serial.print(
          sectionHits[seg]
        );
        Serial.print(
          F(" topic hits")
        );
      } else {
        Serial.print(
          F("no strong topic section")
        );
      }

      Serial.println(
        F(", factual mode")
      );

      traceLine("retrieve     -> ");

      if (haveFact) {
        Serial.print(
          F("knowledge hit, score ")
        );
        Serial.println(
          (int)fscore
        );
      } else {
        Serial.println(
          F("no sufficiently strong knowledge hit")
        );
      }
    }

    Serial.println();

    Serial.println(
      F("=== response ===")
    );

    if (haveFact) {
      showFact(fs, fl);
      Serial.println();

      Serial.println(
        F("================")
      );

      printReport(
        millis() - t0,
        ptok,
        0
      );

      return;
    }

    Serial.println(
      F("I don't have a sufficiently supported fact for that.")
    );

    Serial.print(
      F("Known topic terms: ")
    );

    showTopics(prompt);

    Serial.println();

    Serial.println(
      F("No free n-gram answer was generated because corpus text is not treated as verified knowledge.")
    );

    Serial.println(
      F("================")
    );

    printReport(
      millis() - t0,
      ptok,
      0
    );

    return;
  }

  if (
    mode != 2 &&
    sectionCount &&
    topicKeyCount &&
    hits == 0
  ) {
    Serial.println();

    Serial.println(
      F("=== no usable context ===")
    );

    Serial.print(
      F("not in the corpus : ")
    );

    showTopics(prompt);

    Serial.println();

    Serial.print(
      F("sections available: ")
    );

    showSections();

    Serial.println();

    Serial.println(
      F("add prose to TRAIN_TEXT, or teach it with /learn.")
    );

    Serial.println(
      F("=========================")
    );

    printReport(
      millis() - t0,
      ptok,
      0
    );

    return;
  }

  indexModel(activeOrder());

  uint8_t grounded = 0;

  for (
    uint8_t k = activeOrder();
    k >= 1;
    k--
  ) {
    if (candidateTable[k].count) {
      grounded = k;
      break;
    }
  }

  if (cfgTrace) {
    traceLine(
      "probe        -> longest matched context: order "
    );

    Serial.print((int)grounded);

    Serial.print(F(" ("));

    Serial.print(
      grounded
        ? candidateTable[grounded].count
        : 0
    );

    Serial.print(
      F(" continuations, ")
    );

    Serial.print(
      (unsigned long)tokenTotal
    );

    Serial.println(
      F(" tokens in scope)")
    );
  }

  if (cfgTrace) {
    traceLine("plan         -> mode=");

    Serial.print(
      mode == 0
        ? F("complete")
        : F("story")
    );

    Serial.print(F(" budget="));
    Serial.print(budget);

    Serial.print(F(" temp="));
    Serial.print((int)cfgTemp);

    Serial.print(F(" order<="));
    Serial.println(
      (int)activeOrder()
    );

    Serial.println(
      F("[debug] sampling ...")
    );
  }

  Serial.println();

  Serial.println(
    F("=== response ===")
  );

  uint16_t outTok = 0;

  if (mode == 0) {
    Serial.print(prompt);

    outputStarted = true;
    capitalizeNext = false;

    oneSentenceMode = true;

    outTok =
      synthesize(
        budget,
        0,
        40
      );

    oneSentenceMode = false;
  } else {
    indexModel(activeOrder());

    if (topicContextDepth) {
      for (uint8_t j = 0; j < NGRAM_DEPTH; j++)
        contextStack[j] =
          topicContext[j];

      contextDepth = topicContextDepth;
    } else {
      resetAtSentence();
    }

    outputStarted = false;
    capitalizeNext = true;

    outTok =
      synthesize(
        budget,
        mode == 2 ? 3 : 0,
        mode == 2 ? 60 : 45
      );
  }

  Serial.println();

  Serial.println(
    F("================")
  );

  printReport(
    millis() - t0,
    ptok,
    outTok
  );
}

static void printHelp() {
  Serial.println(
    F("Espie commands")
  );

  Serial.println(
    F("  <any text>        continue the text (base-model completion)")
  );

  Serial.println(
    F("  /ask <question>   answer only from verified knowledge")
  );

  Serial.println(
    F("  /raw <text>       force literal completion, no intent detection")
  );

  Serial.println(
    F("  /topic <name>     lock generation to one corpus section")
  );

  Serial.println(
    F("  /story <topic>    write a multi-paragraph story")
  );

  Serial.println(
    F("  /gen <n> <text>   continue <text> for exactly n tokens")
  );

  Serial.println(
    F("  /class <text>     classify text against trained examples")
  );

  Serial.println(
    F("  /calc <expr>      arithmetic: + - * / % ^ and parentheses")
  );

  Serial.println(
    F("  /words <prefix>   list lexicon words starting with prefix")
  );

  Serial.println(
    F("  /rhyme <ending>   list lexicon words ending with that string")
  );

  Serial.println(
    F("  /learn <text>     append unverified prose to learned corpus")
  );

  Serial.println(
    F("  /forget           erase everything learned via /learn")
  );

  Serial.println(
    F("  /set temp <0-100> higher = more creative")
  );

  Serial.println(
    F("  /set order <1-7>  context window length")
  );

  Serial.println(
    F("  /set len <n>      default generation budget in tokens")
  );

  Serial.println(
    F("  /set seed <n>     RNG seed, for reproducible output")
  );

  Serial.println(
    F("  /think on|off     show or hide the inference debug trace")
  );

  Serial.println(
    F("  /stats            model and memory statistics")
  );

  Serial.println(
    F("  /bench            measure decode speed")
  );

  Serial.println();
}

static void storeLearning(
  const char *txt
) {
  uint16_t room =
    STORE_CAP - STORE_HEAD;

  uint16_t n =
    (uint16_t)strlen(txt);

  uint16_t add =
    n + 1;

  uint8_t last =
    n
      ? (uint8_t)txt[n - 1]
      : 0;

  if (!terminalChar(last))
    add++;

  if (
    (uint32_t)storedBytes +
    add >
    room
  ) {
    Serial.println(
      F("EEPROM store full. Use /forget, or move the text into training_data.h")
    );

    return;
  }

  uint16_t w =
    STORE_HEAD + storedBytes;

  storageWrite(w++, ' ');

  for (uint16_t i = 0; i < n; i++)
    storageWrite(
      w++,
      (uint8_t)txt[i]
    );

  if (!terminalChar(last))
    storageWrite(w++, '.');

  storageSetLength(
    storedBytes + add
  );

  discoverSections();

  Serial.print(
    F("learned as unverified prose. corpus is now ")
  );

  Serial.print(
    modelLength()
  );

  Serial.print(F(" bytes ("));

  Serial.print(storedBytes);

  Serial.println(
    F(" learned bytes)")
  );
}

static void printStats() {
  uint32_t t0 = millis();

  selectScan(255);
  clearContext();
  topicKeyCount = 0;

  indexModel(1);

  uint32_t dt =
    millis() - t0;

  Serial.print(
    F("corpus bytes    : ")
  );

  Serial.println(
    modelLength()
  );

  Serial.print(
    F("  flash         : ")
  );

  Serial.println(
    flashBytes
  );

  Serial.print(
    F("  learned store : ")
  );

  Serial.println(
    storedBytes
  );

  Serial.print(
    F("corpus tokens   : ")
  );

  Serial.println(
    (unsigned long)tokenTotal
  );

  Serial.print(
    F("lexicon words   : ")
  );

  Serial.println(
    countLexemes()
  );

  Serial.print(
    F("sentence starts : ")
  );

  Serial.println(
    sentenceStart.count
  );

  Serial.print(
    F("sections        : ")
  );

  showSections();

  Serial.println();

  Serial.print(
    F("one model pass  : ")
  );

  Serial.print(dt);

  Serial.println(
    F(" ms")
  );

  Serial.print(
    F("max order       : ")
  );

  Serial.println(
    (int)cfgOrder
  );

  Serial.print(
    F("temperature      : ")
  );

  Serial.println(
    (int)cfgTemp
  );

  Serial.print(
    F("fact threshold  : ")
  );

  Serial.println(
    FACT_SCORE_MIN
  );

  Serial.print(
    F("free heap       : ")
  );

  Serial.print(
    measureFreeMemory()
  );

  Serial.println(
    F(" bytes")
  );

  Serial.println();
}

static void runBenchmark() {
  uint32_t t0 = millis();

  capitalizeNext = true;
  outputStarted = false;
  recentCount = 0;
  recentPositionCount = 0;

  selectScan(255);

  tokenizePrompt(
    "the cat",
    true
  );

  bool s = cfgTrace;

  cfgTrace = false;

  Serial.print(
    F("bench output: ")
  );

  uint16_t n =
    synthesize(
      40,
      0,
      50
    );

  cfgTrace = s;

  uint32_t dt =
    millis() - t0;

  Serial.println();

  Serial.print(
    F("decoded ")
  );

  Serial.print(n);

  Serial.print(
    F(" tokens in ")
  );

  Serial.print(dt);

  Serial.print(
    F(" ms = ")
  );

  Serial.print(
    (double)n *
    1000.0 /
    (double)(dt ? dt : 1),
    2
  );

  Serial.println(
    F(" tok/s")
  );

  Serial.println();
}

static bool hasPrefix(
  const char *s,
  const char *p
) {
  return strncmp(
    s,
    p,
    strlen(p)
  ) == 0;
}

static bool sectionMatches(
  uint8_t k,
  const char *name
) {
  if (k >= sectionCount)
    return false;

  uint16_t n =
    (uint16_t)strlen(name);

  if (n != sections[k].nameLen)
    return false;

  if (
    sections[k].nameStart ==
    0xFFFFFFFFUL
  )
    return strcmp(
      name,
      "learned"
    ) == 0;

  for (uint16_t i = 0; i < n; i++) {
    if (
      lowerAscii(
        pgm_read_byte(
          &TRAIN_TEXT[
            sections[k].nameStart + i
          ]
        )
      ) !=
      lowerAscii((uint8_t)name[i])
    )
      return false;
  }

  return true;
}

static void dispatchLine(
  char *line
) {
  while (*line == ' ')
    line++;

  if (!*line)
    return;

  if (line[0] != '/') {
    const char *body = line;

    uint8_t m =
      classifyInput(&body);

    if (
      !*body &&
      m != 2
    ) {
      Serial.println(
        F("that is an instruction with no subject in it.")
      );

      return;
    }

    processInput(
      body,
      m,
      m == 2
        ? (uint16_t)(
            cfgBudget > 21845
              ? 65535
              : cfgBudget * 3
          )
        : (
            m == 1
              ? 30
              : cfgBudget
          )
    );

    return;
  }

  if (hasPrefix(line, "/help")) {
    printHelp();
    return;
  }

  if (hasPrefix(line, "/stats")) {
    printStats();
    return;
  }

  if (hasPrefix(line, "/bench")) {
    runBenchmark();
    return;
  }

  if (hasPrefix(line, "/forget")) {
    storageSetLength(0);
    discoverSections();

    Serial.println(
      F("learned data erased.")
    );

    return;
  }

  if (hasPrefix(line, "/think")) {
    cfgTrace =
      (strstr(
        line,
        "off"
      ) == NULL);

    Serial.print(
      F("debug trace: ")
    );

    Serial.println(
      cfgTrace
        ? F("on")
        : F("off")
    );

    return;
  }

  if (hasPrefix(line, "/raw ")) {
    processInput(
      line + 5,
      0,
      cfgBudget
    );

    return;
  }

  if (hasPrefix(line, "/ask ")) {
    const char *b =
      line + 5;

    classifyInput(&b);

    processInput(
      b,
      1,
      30
    );

    return;
  }

  if (hasPrefix(line, "/story ")) {
    const char *b =
      line + 7;

    classifyInput(&b);

    processInput(
      b,
      2,
      cfgBudget > 21845
        ? 65535
        : (uint16_t)(
            cfgBudget * 3
          )
    );

    return;
  }

  if (hasPrefix(line, "/topic")) {
    const char *a =
      line + 6;

    while (*a == ' ')
      a++;

    if (!*a) {
      Serial.print(
        F("sections: ")
      );

      showSections();

      Serial.println();

      Serial.print(
        F("locked to: ")
      );

      if (
        forcedSection <
        sectionCount
      )
        showSectionName(
          forcedSection
        );
      else
        Serial.print(
          F("auto")
        );

      Serial.println();
      Serial.println();

      return;
    }

    uint16_t n = 0;

    while (
      a[n] &&
      sectionChar(
        (uint8_t)a[n]
      )
    )
      n++;

    if (!n) {
      Serial.println(
        F("invalid section name")
      );

      return;
    }

    char name[65];

    uint16_t copy =
      n < sizeof(name) - 1
        ? n
        : sizeof(name) - 1;

    for (uint16_t i = 0; i < copy; i++)
      name[i] =
        (char)lowerAscii(
          (uint8_t)a[i]
        );

    name[copy] = 0;

    if (
      strcmp(name, "auto") == 0 ||
      strcmp(name, "all") == 0
    ) {
      forcedSection = 255;

      Serial.println(
        F("topic lock off")
      );

      Serial.println();

      return;
    }

    for (uint8_t k = 0; k < sectionCount; k++) {
      if (
        sectionMatches(
          k,
          name
        )
      ) {
        forcedSection = k;

        Serial.print(
          F("locked to ")
        );

        showSectionName(k);

        Serial.println();
        Serial.println();

        return;
      }
    }

    Serial.print(
      F("no such section. have: ")
    );

    showSections();

    Serial.println();
    Serial.println();

    return;
  }

  if (hasPrefix(line, "/learn ")) {
    storeLearning(line + 7);
    return;
  }

  if (hasPrefix(line, "/class ")) {
    tokenizePrompt(
      line + 7,
      false
    );

    runClassifier();

    Serial.println();

    return;
  }

  if (hasPrefix(line, "/words ")) {
    scanLexeme(
      line + 7,
      0
    );

    return;
  }

  if (hasPrefix(line, "/rhyme ")) {
    scanLexeme(
      line + 7,
      1
    );

    return;
  }

  if (hasPrefix(line, "/calc ")) {
    calcCursor =
      line + 6;

    double v =
      expression();

    Serial.print(F("= "));
    Serial.println(v, 4);
    Serial.println();

    return;
  }

  if (hasPrefix(line, "/gen ")) {
    char *p =
      line + 5;

    uint16_t n =
      (uint16_t)atoi(p);

    while (
      *p &&
      *p != ' '
    )
      p++;

    while (*p == ' ')
      p++;

    if (n < 1)
      n = 20;

    if (n > 400)
      n = 400;

    processInput(
      p,
      0,
      n
    );

    return;
  }

  if (hasPrefix(line, "/set ")) {
    char *p =
      line + 5;

    int v = 0;

    char *q = p;

    while (
      *q &&
      (
        *q < '0' ||
        *q > '9'
      )
    )
      q++;

    v = atoi(q);

    if (hasPrefix(p, "temp")) {
      if (v < 0)
        v = 0;

      if (v > 100)
        v = 100;

      cfgTemp =
        (uint8_t)v;

      Serial.print(
        F("temp = ")
      );

      Serial.println(
        (int)cfgTemp
      );
    }

    else if (
      hasPrefix(
        p,
        "order"
      )
    ) {
      if (v < 1)
        v = 1;

      if (v > NGRAM_DEPTH)
        v = NGRAM_DEPTH;

      cfgOrder =
        (uint8_t)v;

      Serial.print(
        F("order = ")
      );

      Serial.println(
        (int)cfgOrder
      );
    }

    else if (
      hasPrefix(
        p,
        "len"
      )
    ) {
      if (v < 5)
        v = 5;

      if (v > 400)
        v = 400;

      cfgBudget =
        (uint16_t)v;

      Serial.print(
        F("len = ")
      );

      Serial.println(
        cfgBudget
      );
    }

    else if (
      hasPrefix(
        p,
        "seed"
      )
    ) {
      randomState =
        (uint32_t)v *
        2654435761UL +
        1;

      Serial.print(
        F("seed = ")
      );

      Serial.println(v);
    }

    else {
      Serial.println(
        F("unknown setting")
      );
    }

    Serial.println();

    return;
  }

  Serial.println(
    F("unknown command, try /help")
  );
}

static char inputLine[LINE_CAP];
static uint8_t inputLength = 0;

void setup() {
  Serial.begin(115200);

  delay(100);

  if (!EEPROM.begin(STORE_CAP)) {
    Serial.println(
      F("EEPROM emulation initialization failed.")
    );

    while (true)
      delay(1000);
  }

  flashBytes =
    (uint32_t)strlen_P(
      TRAIN_TEXT
    );

  if (
    storageRead(0) == STORE_SIG_A &&
    storageRead(1) == STORE_SIG_B
  ) {
    storedBytes =
      (uint16_t)storageRead(2) |
      (
        (uint16_t)storageRead(3)
        << 8
      );

    if (
      storedBytes >
      STORE_CAP - STORE_HEAD
    )
      storageSetLength(0);
  } else {
    storageSetLength(0);
  }

  discoverSections();

  selectScan(255);

  randomState ^=
    micros() *
    2654435761UL;

  printBanner();

  Serial.print(F("> "));
}

void loop() {
  while (Serial.available()) {
    int c =
      Serial.read();

    if (c < 0)
      continue;

    if (c == '\r')
      continue;

    if (c == '\n') {
      inputLine[inputLength] = 0;

      Serial.println();

      if (inputLength)
        dispatchLine(inputLine);

      inputLength = 0;

      Serial.print(
        F("> ")
      );
    }

    else if (
      inputLength <
      LINE_CAP - 1
    ) {
      inputLine[inputLength++] =
        (char)c;
    }
  }
}
