// (c) Aharshi Deb 2026, Espie AI Language Model Version 0.1 , ESP32-S3
//Several problems have been fixed. this may be buggy.
//Use at own risk.
#if !defined(ARDUINO_ARCH_ESP32)
  #error "This sketch targets Arduino-ESP32."
#endif

#include <Arduino.h>
#include <pgmspace.h>
#include <EEPROM.h>

#include "training_data.h"

#define PUNCT_HASH(c) (0xFF000000UL + (uint8_t)(c))

#define MAX_ORDER       7
#define HIST_LEN        14
#define IN_BUF_LEN      140
#define MAX_PROMPT_TOK  20
#define MIN_SENT_WORDS  4
#define MAX_SENT_WORDS  26

static uint8_t  g_maxOrder = 5;
static uint8_t  g_temp = 22;
static uint16_t g_budget = 55;
static bool     g_showThink = true;

typedef uint32_t whash_t;

struct Cand {
  uint32_t start;
  uint16_t count;
  uint16_t len;
  uint8_t type;
};

#define MAX_SEG 32

struct Segment {
  uint32_t start;
  uint32_t end;
  uint32_t nameStart;
  whash_t nameHash;
  uint16_t nameLen;
};

static uint32_t g_rng = 0x1F123BB5UL;

static inline uint32_t rnd32() {
  g_rng ^= g_rng << 13;
  g_rng ^= g_rng >> 17;
  g_rng ^= g_rng << 5;
  return g_rng;
}

static inline uint16_t rndBelow(uint16_t n) {
  return n ? (uint16_t)(rnd32() % n) : 0;
}

#define EEP_SIZE 4096
#define EEP_HDR 4
#define EEP_M0 0x4D
#define EEP_M1 0x4C

static uint32_t g_flashLen = 0;
static uint16_t g_eepLen = 0;

static inline uint8_t eepRead(uint16_t a) {
  return EEPROM.read(a);
}

static inline void eepWrite(uint16_t a, uint8_t v) {
  EEPROM.write(a, v);
}

static inline void eepCommit() {
  EEPROM.commit();
}

static void eepSetLen(uint16_t n) {
  if (n > EEP_SIZE - EEP_HDR) n = 0;

  g_eepLen = n;

  eepWrite(0, EEP_M0);
  eepWrite(1, EEP_M1);
  eepWrite(2, (uint8_t)(n & 0xFF));
  eepWrite(3, (uint8_t)(n >> 8));
  eepCommit();
}

static uint32_t g_scanHi = 0;

static uint32_t g_rLo[MAX_SEG + 1];
static uint32_t g_rHi[MAX_SEG + 1];

static uint8_t g_rN = 0;

static Segment g_seg[MAX_SEG];
static uint8_t g_segN = 0;

static uint8_t g_forceSeg = 255;
static uint8_t g_lastSeg = 255;

static bool g_creative = true;

static inline uint8_t effOrder() {
  return g_creative ? g_maxOrder : MAX_ORDER;
}

static uint8_t g_storySeg = 255;

static uint16_t g_segHits[MAX_SEG];

static inline uint32_t corpusLen() {
  return g_flashLen + (uint32_t)g_eepLen;
}

static inline uint8_t corpusByte(uint32_t i) {
  if (i < g_flashLen)
    return pgm_read_byte(&TRAIN_TEXT[i]);

  uint32_t e = i - g_flashLen;

  if (e >= g_eepLen)
    return 0;

  return eepRead(EEP_HDR + (uint16_t)e);
}

static inline uint8_t lc(uint8_t c) {
  return (c >= 'A' && c <= 'Z') ? (uint8_t)(c + 32) : c;
}

static inline bool isWordChar(uint8_t c) {
  return
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= '0' && c <= '9') ||
    c == '\'' ||
    c == '-' ||
    c == '_';
}

static inline bool isSectionNameChar(uint8_t c) {
  return
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    (c >= '0' && c <= '9') ||
    c == '_' ||
    c == '-';
}

static inline bool isPunctChar(uint8_t c) {
  return
    c == '.' ||
    c == '!' ||
    c == '?' ||
    c == ',' ||
    c == ';' ||
    c == ':';
}

static inline bool isTerminal(uint8_t c) {
  return c == '.' || c == '!' || c == '?';
}

static inline whash_t foldHash(uint32_t h) {
  return h;
}

static uint8_t nextToken(
  uint32_t &pos,
  uint32_t &start,
  uint16_t &len,
  whash_t &hash
) {
  const uint32_t n = g_scanHi;

  while (pos < n) {
    uint8_t c = corpusByte(pos);

    if (isWordChar(c))
      break;

    if (isPunctChar(c)) {
      start = pos;
      len = 1;
      hash = PUNCT_HASH(c);
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
    uint8_t c = corpusByte(pos);

    if (!isWordChar(c))
      break;

    h ^= lc(c);
    h *= 16777619UL;

    pos++;

    if (l < 65535)
      l++;
  }

  len = l;
  hash = foldHash(h);

  return 1;
}

static whash_t hashRamWord(const char *s, uint16_t n) {
  uint32_t h = 2166136261UL;

  for (uint16_t i = 0; i < n; i++) {
    h ^= lc((uint8_t)s[i]);
    h *= 16777619UL;
  }

  return foldHash(h);
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

static whash_t g_ctx[MAX_ORDER];
static uint8_t g_ctxN = 0;

static void ctxReset() {
  g_ctxN = 0;
}

static void ctxSentenceStartFwd();

static void ctxPush(whash_t h) {
  for (int8_t i = MAX_ORDER - 1; i > 0; i--)
    g_ctx[i] = g_ctx[i - 1];

  g_ctx[0] = h;

  if (g_ctxN < MAX_ORDER)
    g_ctxN++;
}

static void ctxSentenceStart() {
  ctxReset();
  ctxPush(PUNCT_HASH('.'));
}

static void ctxSentenceStartFwd() {
  ctxSentenceStart();
}

#define KEY_SEED 2166136261UL

static inline uint32_t mixKey(uint32_t k, whash_t h) {
  k ^= h;
  k *= 16777619UL;
  return k;
}

static bool inWordList(const char *list, whash_t h) {
  uint32_t i = 0;
  uint32_t acc = 2166136261UL;
  uint16_t n = 0;

  for (;;) {
    uint8_t c = pgm_read_byte(&list[i]);

    if (isWordChar(c)) {
      acc ^= lc(c);
      acc *= 16777619UL;
      n++;
    } else {
      if (n && foldHash(acc) == h)
        return true;

      acc = 2166136261UL;
      n = 0;

      if (c == 0)
        return false;
    }

    i++;
  }
}

static inline bool isProperNoun(whash_t h) {
  return inWordList(TRAIN_PROPER, h);
}

#define MAX_TOPIC 8

static whash_t g_topicWord[MAX_TOPIC];
static uint8_t g_topicWordN = 0;

static bool isTopicWord(whash_t h) {
  for (uint8_t i = 0; i < g_topicWordN; i++)
    if (g_topicWord[i] == h)
      return true;

  return false;
}

static bool isStrongTopicWord(whash_t h) {
  if (!isTopicWord(h))
    return false;

  if (inWordList(STOPWORDS, h))
    return false;

  if (inWordList(META_WORDS, h))
    return false;

  return true;
}

static Cand g_cand[MAX_ORDER + 1];
static Cand g_startCand;

static uint32_t g_corpusTokens = 0;

static whash_t g_topicCtx[MAX_ORDER];
static uint8_t g_topicCtxN = 0;

static uint16_t g_topicHits = 0;

static void scanCorpus(uint8_t maxOrder) {
  uint32_t target[MAX_ORDER + 1];

  uint8_t hi = g_ctxN;

  if (hi > maxOrder)
    hi = maxOrder;

  {
    uint32_t k = KEY_SEED;

    for (uint8_t j = 0; j < hi; j++) {
      k = mixKey(k, g_ctx[j]);
      target[j + 1] = k;
    }
  }

  for (uint8_t k = 0; k <= MAX_ORDER; k++)
    g_cand[k].count = 0;

  g_startCand.count = 0;

  whash_t ring[MAX_ORDER];
  uint8_t ringN = 0;

  uint32_t pos = 0;
  uint32_t start;

  whash_t hash;

  uint16_t len;
  uint8_t type;

  bool atSentStart = true;

  uint32_t ntok = 0;

  whash_t ssRing[MAX_ORDER];
  uint8_t ssRingN = 0;
  uint16_t ssAge = 0;

  bool ssValid = false;

  g_topicHits = 0;
  g_topicCtxN = 0;

  for (uint8_t r = 0; r < g_rN; r++) {
    pos = g_rLo[r];
    g_scanHi = g_rHi[r];

    ringN = 0;
    atSentStart = true;
    ssValid = false;

    while ((type = nextToken(pos, start, len, hash)) != 0) {
      ntok++;

      if (type == 1 && atSentStart) {
        for (uint8_t j = 0; j < MAX_ORDER; j++)
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
        g_topicWordN &&
        isStrongTopicWord(hash)
      ) {
        g_topicHits++;

        if (rndBelow(g_topicHits) == 0) {
          for (uint8_t j = 0; j < MAX_ORDER; j++)
            g_topicCtx[j] = ssRing[j];

          g_topicCtxN = ssRingN;
        }

        ssValid = false;
      }

      if (type == 1 && atSentStart) {
        g_startCand.count++;

        if (rndBelow(g_startCand.count) == 0) {
          g_startCand.start = start;
          g_startCand.len = len;
          g_startCand.type = 1;
        }
      }

      if (ringN && hi) {
        uint32_t k = KEY_SEED;

        uint8_t lim = (ringN < hi) ? ringN : hi;

        for (uint8_t j = 0; j < lim; j++) {
          k = mixKey(k, ring[j]);

          if (k == target[j + 1]) {
            Cand &c = g_cand[j + 1];

            c.count++;

            if (rndBelow(c.count) == 0) {
              c.start = start;
              c.len = len;
              c.type = type;
            }
          }
        }
      }

      for (int8_t i = MAX_ORDER - 1; i > 0; i--)
        ring[i] = ring[i - 1];

      ring[0] = hash;

      if (ringN < MAX_ORDER)
        ringN++;

      atSentStart =
        (type == 2) &&
        isTerminal(corpusByte(start));
    }
  }

  g_corpusTokens = ntok;
}

static bool hashSectionName(
  uint32_t start,
  uint32_t end,
  uint32_t &nameStart,
  uint16_t &nameLen,
  whash_t &nameHash
) {
  uint32_t p = start;

  while (p < end && (
    corpusByte(p) == ' ' ||
    corpusByte(p) == '\t'
  ))
    p++;

  if (p + 7 <= end &&
      lc(corpusByte(p)) == 's' &&
      lc(corpusByte(p + 1)) == 'e' &&
      lc(corpusByte(p + 2)) == 'c' &&
      lc(corpusByte(p + 3)) == 't' &&
      lc(corpusByte(p + 4)) == 'i' &&
      lc(corpusByte(p + 5)) == 'o' &&
      lc(corpusByte(p + 6)) == 'n' &&
      (p + 7 == end ||
       corpusByte(p + 7) == ' ' ||
       corpusByte(p + 7) == '\t')
  ) {
    p += 7;

    while (p < end &&
           (corpusByte(p) == ' ' || corpusByte(p) == '\t'))
      p++;
  }

  nameStart = p;

  uint32_t h = 2166136261UL;
  uint16_t n = 0;

  while (p < end) {
    uint8_t c = corpusByte(p);

    if (!isSectionNameChar(c))
      break;

    h ^= lc(c);
    h *= 16777619UL;

    p++;

    if (n < 65535)
      n++;
  }

  if (!n)
    return false;

  nameLen = n;
  nameHash = foldHash(h);

  return true;
}

static void buildSegments() {
  g_segN = 0;
  g_storySeg = 255;

  uint32_t i = 0;
  uint32_t currentContent = 0;

  while (i + 1 < g_flashLen) {
    if (
      pgm_read_byte(&TRAIN_TEXT[i]) == '@' &&
      pgm_read_byte(&TRAIN_TEXT[i + 1]) == '@'
    ) {
      uint32_t lineEnd = i + 2;

      while (
        lineEnd < g_flashLen &&
        pgm_read_byte(&TRAIN_TEXT[lineEnd]) != '\n'
      )
        lineEnd++;

      uint32_t nameStart;
      uint16_t nameLen;
      whash_t nameHash;

      if (hashSectionName(
        i + 2,
        lineEnd,
        nameStart,
        nameLen,
        nameHash
      )) {
        if (g_segN > 0)
          g_seg[g_segN - 1].end = i;

        if (g_segN < MAX_SEG) {
          uint32_t contentStart =
            (lineEnd < g_flashLen) ? lineEnd + 1 : lineEnd;

          g_seg[g_segN].start = contentStart;
          g_seg[g_segN].end = g_flashLen;
          g_seg[g_segN].nameStart = nameStart;
          g_seg[g_segN].nameLen = nameLen;
          g_seg[g_segN].nameHash = nameHash;

          if (
            nameLen == 7 &&
            nameHash == hashRamWord("stories", 7)
          )
            g_storySeg = g_segN;

          g_segN++;
          currentContent = contentStart;
        }
      }

      i = lineEnd;

      if (i < g_flashLen)
        i++;

      continue;
    }

    i++;
  }

  if (g_segN == 0 && g_flashLen) {
    g_seg[0].start = 0;
    g_seg[0].end = g_flashLen;
    g_seg[0].nameStart = 0xFFFFFFFFUL;
    g_seg[0].nameLen = 4;
    g_seg[0].nameHash = hashRamWord("main", 4);
    g_segN = 1;
  }

  if (g_eepLen && g_segN < MAX_SEG) {
    g_seg[g_segN].start = g_flashLen;
    g_seg[g_segN].end = corpusLen();
    g_seg[g_segN].nameStart = 0xFFFFFFFFUL;
    g_seg[g_segN].nameLen = 7;
    g_seg[g_segN].nameHash = hashRamWord("learned", 7);
    g_segN++;
  }

  g_storySeg = 255;

  whash_t sh = hashRamWord("stories", 7);

  for (uint8_t k = 0; k < g_segN; k++) {
    if (g_seg[k].nameHash == sh) {
      g_storySeg = k;
      break;
    }
  }
}

static void printSegName(uint8_t k) {
  if (k >= g_segN)
    return;

  if (g_seg[k].nameStart == 0xFFFFFFFFUL) {
    Serial.print(F("learned"));
    return;
  }

  for (uint16_t j = 0; j < g_seg[k].nameLen; j++)
    Serial.print(
      (char)pgm_read_byte(
        &TRAIN_TEXT[g_seg[k].nameStart + j]
      )
    );
}

static void listSegments() {
  for (uint8_t k = 0; k < g_segN; k++) {
    if (k)
      Serial.print(F(", "));

    printSegName(k);
  }
}

static void setScan(uint8_t seg) {
  g_rN = 0;

  if (!g_segN) {
    g_rLo[0] = 0;
    g_rHi[0] = corpusLen();
    g_rN = 1;
    return;
  }

  if (seg < g_segN) {
    g_rLo[0] = g_seg[seg].start;
    g_rHi[0] = g_seg[seg].end;
    g_rN = 1;
    return;
  }

  for (uint8_t k = 0; k < g_segN && g_rN < MAX_SEG + 1; k++) {
    g_rLo[g_rN] = g_seg[k].start;
    g_rHi[g_rN] = g_seg[k].end;
    g_rN++;
  }
}

static uint16_t scoreSegments() {
  uint16_t total = 0;

  for (uint8_t k = 0; k < g_segN; k++) {
    uint16_t cnt = 0;

    uint32_t pos = g_seg[k].start;
    uint32_t start;

    whash_t hash;

    uint16_t len;
    uint8_t type;

    g_scanHi = g_seg[k].end;

    while ((type = nextToken(pos, start, len, hash)) != 0) {
      if (type == 1 && isStrongTopicWord(hash))
        cnt++;
    }

    g_segHits[k] = cnt;

    if (UINT16_MAX - total < cnt)
      total = UINT16_MAX;
    else
      total += cnt;
  }

  return total;
}

static uint8_t bestSegment(bool skipStory) {
  uint8_t bi = 255;
  uint16_t best = 0;

  for (uint8_t k = 0; k < g_segN; k++) {
    if (skipStory && k == g_storySeg)
      continue;

    if (g_segHits[k] > best) {
      best = g_segHits[k];
      bi = k;
    }
  }

  return bi;
}

static bool g_needCap = true;
static bool g_started = false;

static bool keepCase(
  uint32_t start,
  uint16_t len,
  whash_t hash
) {
  for (uint16_t i = 1; i < len; i++) {
    uint8_t c = corpusByte(start + i);

    if (c >= 'A' && c <= 'Z')
      return true;
  }

  if (len == 1 && corpusByte(start) == 'I')
    return true;

  return isProperNoun(hash);
}

static void emit(
  uint32_t start,
  uint16_t len,
  uint8_t type,
  whash_t hash
) {
  if (type == 2) {
    Serial.print((char)corpusByte(start));

    if (isTerminal(corpusByte(start)))
      g_needCap = true;

    g_started = true;
    return;
  }

  if (g_started)
    Serial.print(' ');

  bool keep = keepCase(start, len, hash);

  for (uint16_t i = 0; i < len; i++) {
    uint8_t c = corpusByte(start + i);

    if (i == 0) {
      if (g_needCap)
        c = (c >= 'a' && c <= 'z')
          ? (uint8_t)(c - 32)
          : c;
      else if (!keep)
        c = lc(c);
    } else if (!keep) {
      c = lc(c);
    }

    Serial.print((char)c);
  }

  g_needCap = false;
  g_started = true;
}

static whash_t g_hist[HIST_LEN];
static uint8_t g_histN = 0;
static uint8_t g_lastPunct = 0;

#define POS_HIST 72

static uint32_t g_posHist[POS_HIST];
static uint8_t g_posN = 0;

static void posPush(uint32_t p) {
  for (int8_t i = POS_HIST - 1; i > 0; i--)
    g_posHist[i] = g_posHist[i - 1];

  g_posHist[0] = p;

  if (g_posN < POS_HIST)
    g_posN++;
}

static bool posSeen(uint32_t p) {
  for (uint8_t i = 0; i < g_posN; i++)
    if (g_posHist[i] == p)
      return true;

  return false;
}

static void histPush(whash_t h) {
  for (int8_t i = HIST_LEN - 1; i > 0; i--)
    g_hist[i] = g_hist[i - 1];

  g_hist[0] = h;

  if (g_histN < HIST_LEN)
    g_histN++;
}

static uint8_t histCount(whash_t h) {
  uint8_t n = 0;

  for (uint8_t i = 0; i < g_histN; i++)
    if (g_hist[i] == h)
      n++;

  return n;
}

static whash_t candHash(const Cand &c) {
  if (c.type == 2)
    return PUNCT_HASH(corpusByte(c.start));

  uint32_t h = 2166136261UL;

  for (uint16_t i = 0; i < c.len; i++) {
    h ^= lc(corpusByte(c.start + i));
    h *= 16777619UL;
  }

  return foldHash(h);
}

static bool candOK(
  const Cand &c,
  uint8_t sentWords,
  bool mustEnd
) {
  whash_t h = candHash(c);

  if (c.type == 1 && posSeen(c.start))
    return false;

  if (c.type == 2) {
    uint8_t ch = corpusByte(c.start);

    if (isTerminal(ch))
      return sentWords >= MIN_SENT_WORDS;

    if (mustEnd)
      return false;

    if (g_lastPunct)
      return false;

    if (sentWords < 3)
      return false;

    return true;
  }

  if (mustEnd)
    return false;

  if (g_histN && g_hist[0] == h)
    return false;

  if (c.len > 3 && histCount(h) >= 2)
    return false;

  return true;
}

static uint16_t g_lastOrderSum = 0;
static uint16_t g_lastOrderN = 0;

static bool g_singleSentence = false;

static uint16_t generate(
  uint16_t budget,
  uint8_t sentPerPara,
  uint8_t reanchorPct
) {
  uint16_t emitted = 0;
  uint8_t sentWords = 0;
  uint8_t copyRun = 0;
  uint16_t sentences = 0;

  g_lastPunct = 0;

  bool justAnchored = true;
  bool sentHadTopic = true;

  uint32_t usedAnchor[4] = {
    0, 0, 0, 0
  };

  const uint16_t hardCap =
    (budget > 65535 - 40)
      ? 65535
      : budget + 40;

  g_lastOrderSum = 0;
  g_lastOrderN = 0;

  while (emitted < hardCap) {
    scanCorpus(effOrder());

    uint8_t hi = 0;

    for (uint8_t k = g_maxOrder; k >= 1; k--) {
      if (g_cand[k].count) {
        hi = k;
        break;
      }
    }

    const uint8_t temp =
      g_creative
        ? g_temp
        : (g_temp > 8 ? 8 : g_temp);

    const uint8_t floorOrder =
      g_creative
        ? ((temp <= 30) ? 4 :
           ((temp <= 65) ? 3 : 2))
        : MAX_ORDER;

    while (
      hi > floorOrder &&
      rndBelow(100) < temp
    )
      hi--;

    if (g_creative && copyRun >= 3) {
      uint8_t f =
        (floorOrder > 2)
          ? (uint8_t)(floorOrder - 1)
          : 2;

      if (hi > f)
        hi = f;

      copyRun = 0;
    }

    if (
      g_histN >= 6 &&
      g_hist[0] == g_hist[3] &&
      g_hist[1] == g_hist[4] &&
      g_hist[2] == g_hist[5]
    )
      hi = 2;

    bool mustEnd =
      sentWords >= MAX_SENT_WORDS;

    Cand chosen;
    chosen.count = 0;

    uint8_t usedOrder = 0;

    for (uint8_t k = hi; k >= 1; k--) {
      if (
        g_cand[k].count &&
        candOK(
          g_cand[k],
          sentWords,
          mustEnd
        )
      ) {
        chosen = g_cand[k];
        usedOrder = k;
        break;
      }
    }

    if (!chosen.count && mustEnd) {
      Serial.print('.');
      g_needCap = true;
      g_started = true;
      g_lastPunct = '.';

      histPush(PUNCT_HASH('.'));

      emitted++;
      sentences++;
      sentWords = 0;

      if (
        sentPerPara &&
        (sentences % sentPerPara) == 0
      ) {
        Serial.println();
        Serial.println();
        g_started = false;
      }

      if (emitted >= budget)
        break;

      continue;
    }

    if (!chosen.count) {
      for (uint8_t k = hi; k >= 1; k--) {
        if (g_cand[k].count) {
          chosen = g_cand[k];
          usedOrder = k;
          break;
        }
      }
    }

    if (!chosen.count) {
      if (!g_startCand.count)
        break;

      chosen = g_startCand;
      usedOrder = 0;

      ctxSentenceStart();
    }

    copyRun =
      (usedOrder >= 3 && chosen.count == 1)
        ? (uint8_t)(copyRun + 1)
        : 0;

    g_lastOrderSum += usedOrder;
    g_lastOrderN++;

    whash_t h = candHash(chosen);

    emit(
      chosen.start,
      chosen.len,
      chosen.type,
      h
    );

    ctxPush(h);
    histPush(h);

    g_lastPunct =
      (chosen.type == 2)
        ? corpusByte(chosen.start)
        : 0;

    if (chosen.type == 1)
      posPush(chosen.start);

    emitted++;

    if (chosen.type == 1) {
      sentWords++;

      if (isStrongTopicWord(h))
        sentHadTopic = true;
    } else if (
      isTerminal(corpusByte(chosen.start))
    ) {
      sentences++;
      sentWords = 0;

      if (g_singleSentence)
        break;

      if (emitted >= budget)
        break;

      if (
        sentPerPara &&
        (sentences % sentPerPara) == 0
      ) {
        Serial.println();
        Serial.println();
        g_started = false;
      }

      bool drifted =
        (reanchorPct && !sentHadTopic);

      sentHadTopic = false;

      if (
        reanchorPct &&
        g_topicCtxN &&
        !justAnchored &&
        (
          drifted ||
          rndBelow(100) < reanchorPct
        )
      ) {
        uint32_t sig = KEY_SEED;

        for (uint8_t j = 0; j < g_topicCtxN; j++)
          sig = mixKey(
            sig,
            g_topicCtx[j]
          );

        bool seen = false;

        for (uint8_t j = 0; j < 4; j++)
          if (usedAnchor[j] == sig)
            seen = true;

        if (!seen) {
          for (int8_t j = 3; j > 0; j--)
            usedAnchor[j] = usedAnchor[j - 1];

          usedAnchor[0] = sig;

          for (uint8_t j = 0; j < MAX_ORDER; j++)
            g_ctx[j] = g_topicCtx[j];

          g_ctxN = g_topicCtxN;
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

static whash_t g_promptHash[MAX_PROMPT_TOK];
static uint8_t g_promptN = 0;

static uint8_t loadPrompt(
  const char *s,
  bool pushCtx
) {
  g_promptN = 0;
  g_topicWordN = 0;
  g_topicCtxN = 0;

  if (pushCtx)
    ctxReset();

  uint8_t count = 0;

  const char *p = s;

  while (*p) {
    if (isWordChar((uint8_t)*p)) {
      const char *w = p;

      while (
        *p &&
        isWordChar((uint8_t)*p)
      )
        p++;

      uint16_t wl =
        (uint16_t)(p - w);

      whash_t h =
        hashRamWord(w, wl);

      if (pushCtx)
        ctxPush(h);

      if (g_promptN < MAX_PROMPT_TOK)
        g_promptHash[g_promptN++] = h;

      if (
        wl >= 3 &&
        g_topicWordN < MAX_TOPIC &&
        !inWordList(STOPWORDS, h) &&
        !inWordList(META_WORDS, h)
      )
        g_topicWord[g_topicWordN++] = h;

      if (
        wl >= 4 &&
        (w[wl - 1] == 's' ||
         w[wl - 1] == 'S')
      ) {
        whash_t sh =
          hashRamWord(
            w,
            wl - 1
          );

        if (g_promptN < MAX_PROMPT_TOK)
          g_promptHash[g_promptN++] = sh;

        if (
          g_topicWordN < MAX_TOPIC &&
          !inWordList(STOPWORDS, sh) &&
          !inWordList(META_WORDS, sh)
        )
          g_topicWord[g_topicWordN++] = sh;
      }

      count++;
    } else if (
      isPunctChar((uint8_t)*p)
    ) {
      if (pushCtx)
        ctxPush(
          PUNCT_HASH(
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

static bool promptHas(whash_t h) {
  for (uint8_t i = 0; i < g_promptN; i++)
    if (g_promptHash[i] == h)
      return true;

  return false;
}

static whash_t firstWordHash(
  const char *p,
  uint16_t &lenOut
) {
  while (*p == ' ')
    p++;

  uint16_t n = 0;

  while (
    p[n] &&
    isWordChar((uint8_t)p[n])
  )
    n++;

  lenOut = n;

  return n
    ? hashRamWord(p, n)
    : 0;
}

static bool isConversationPrompt(const char *s) {
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
    while (*p && !isWordChar((uint8_t)*p))
      p++;

    if (!*p)
      break;

    const char *w = p;

    while (*p && isWordChar((uint8_t)*p))
      p++;

    uint16_t len = (uint16_t)(p - w);

    if (!len)
      continue;

    whash_t h = hashRamWord(w, len);

    words++;

    if (inWordList(WORDS, h))
      conversational++;

    if (
      len >= 3 &&
      !inWordList(STOPWORDS, h) &&
      !inWordList(META_WORDS, h) &&
      !inWordList(WORDS, h)
    )
      topic++;
  }

  if (!words)
    return false;

  if (topic == 0 && conversational > 0)
    return true;

  return false;
}

static uint8_t detectMode(
  const char **text
) {
  const char *p = *text;

  while (*p == ' ')
    p++;

  uint16_t n = 0;

  whash_t h =
    firstWordHash(p, n);

  if (!n)
    return 0;

  if (isConversationPrompt(p))
    return 0;

  if (inWordList(ASK_WORDS, h))
    return 1;

  if (!inWordList(CMD_VERBS, h))
    return 0;

  const whash_t s1 =
    hashRamWord("story", 5);

  const whash_t s2 =
    hashRamWord("stories", 7);

  const whash_t s3 =
    hashRamWord("tale", 4);

  bool wantsStory = false;

  for (const char *r = p; *r;) {
    if (isWordChar((uint8_t)*r)) {
      const char *w = r;

      while (
        *r &&
        isWordChar((uint8_t)*r)
      )
        r++;

      whash_t wh =
        hashRamWord(
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
      isWordChar((uint8_t)p[m])
    )
      m++;

    if (!m)
      break;

    if (
      !inWordList(
        META_WORDS,
        hashRamWord(p, m)
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

static void printTopicWords(
  const char *prompt
) {
  bool any = false;

  for (const char *r = prompt; *r;) {
    if (isWordChar((uint8_t)*r)) {
      const char *w = r;

      while (
        *r &&
        isWordChar((uint8_t)*r)
      )
        r++;

      uint16_t l =
        (uint16_t)(r - w);

      whash_t h =
        hashRamWord(w, l);

      if (
        l >= 3 &&
        !inWordList(STOPWORDS, h) &&
        !inWordList(META_WORDS, h)
      ) {
        if (any)
          Serial.print(F(", "));

        for (uint16_t i = 0; i < l; i++)
          Serial.print(
            (char)lc((uint8_t)w[i])
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

#define FACT_MIN_SCORE 3
#define FACT_MIN_MARGIN 1

static bool findFact(
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

          if (isWordChar(c)) {
            h ^= lc(c);
            h *= 16777619UL;
            n++;
          } else {
            if (n >= 2) {
              whash_t wh =
                foldHash(h);

              if (
                promptHas(wh) &&
                !inWordList(
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
          score >= FACT_MIN_SCORE &&
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
         bestScore >= FACT_MIN_SCORE;
}

static void printFact(
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

#define MAX_LABELS 64

static void classify() {
  whash_t labelHash[MAX_LABELS];
  uint32_t labelStart[MAX_LABELS];
  uint16_t labelLen[MAX_LABELS];
  uint16_t score[MAX_LABELS];

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
        h ^= lc(
          pgm_read_byte(
            &TRAIN_EXAMPLES[j]
          )
        );

        h *= 16777619UL;
      }

      whash_t lh = foldHash(h);

      int16_t idx = -1;

      for (uint8_t k = 0; k < nl; k++) {
        if (labelHash[k] == lh) {
          idx = k;
          break;
        }
      }

      if (idx < 0 && nl < MAX_LABELS) {
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

          if (isWordChar(c)) {
            wh ^= lc(c);
            wh *= 16777619UL;
            n++;
          } else {
            if (n >= 3) {
              whash_t f =
                foldHash(wh);

              if (
                promptHas(f) &&
                !inWordList(
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

static const char *g_cp;

static double parseExpr();

static void skipSp() {
  while (*g_cp == ' ')
    g_cp++;
}

static double parseAtom() {
  skipSp();

  if (*g_cp == '(') {
    g_cp++;

    double v = parseExpr();

    skipSp();

    if (*g_cp == ')')
      g_cp++;

    return v;
  }

  if (*g_cp == '-') {
    g_cp++;
    return -parseAtom();
  }

  if (*g_cp == '+') {
    g_cp++;
    return parseAtom();
  }

  double v = 0;
  bool any = false;

  while (
    *g_cp >= '0' &&
    *g_cp <= '9'
  ) {
    v =
      v * 10 +
      (*g_cp - '0');

    g_cp++;
    any = true;
  }

  if (*g_cp == '.') {
    g_cp++;

    double f = 0.1;

    while (
      *g_cp >= '0' &&
      *g_cp <= '9'
    ) {
      v +=
        (*g_cp - '0') * f;

      f *= 0.1;
      g_cp++;
      any = true;
    }
  }

  if (!any)
    return 0;

  return v;
}

static double parsePow() {
  double b = parseAtom();

  skipSp();

  if (*g_cp == '^') {
    g_cp++;

    double e = parsePow();

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

static double parseTerm() {
  double v = parsePow();

  for (;;) {
    skipSp();

    if (*g_cp == '*') {
      g_cp++;
      v *= parsePow();
    } else if (*g_cp == '/') {
      g_cp++;

      double d = parsePow();

      v = d ? v / d : 0;
    } else if (*g_cp == '%') {
      g_cp++;

      long d =
        (long)parsePow();

      v =
        d
          ? (double)((long)v % d)
          : 0;
    } else {
      return v;
    }
  }
}

static double parseExpr() {
  double v = parseTerm();

  for (;;) {
    skipSp();

    if (*g_cp == '+') {
      g_cp++;
      v += parseTerm();
    } else if (*g_cp == '-') {
      g_cp++;
      v -= parseTerm();
    } else {
      return v;
    }
  }
}

static void lexScan(
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

    if (isWordChar(c)) {
      if (n < sizeof(w) - 1)
        w[n++] = (char)lc(c);
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

static uint16_t lexCount() {
  uint32_t i = 0;
  uint16_t n = 0;

  bool in = false;

  for (;;) {
    uint8_t c =
      pgm_read_byte(
        &TRAIN_WORDS[i]
      );

    if (isWordChar(c)) {
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

static int freeRam() {
  return (int)ESP.getFreeHeap();
}

static void thinkLine(
  const char *label
) {
  if (!g_showThink)
    return;

  Serial.print(F("[debug] "));
  Serial.print(label);
}

static void banner() {
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
  Serial.print(corpusLen());

  Serial.print(F(" bytes ("));
  Serial.print(g_flashLen);

  Serial.print(F(" flash + "));
  Serial.print(g_eepLen);

  Serial.println(F(" learned)"));

  Serial.print(F("  sections   : "));
  listSegments();
  Serial.println();

  Serial.print(F("  lexicon    : "));
  Serial.print(lexCount());
  Serial.println(F(" words"));

  Serial.print(F("  max order  : "));
  Serial.println((int)g_maxOrder);

  Serial.print(F("  free heap  : "));
  Serial.print(freeRam());
  Serial.println(F(" bytes"));

  Serial.println(
    F("  type /help, or just type a sentence to complete it")
  );

  Serial.println();
}

static void report(
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

  if (g_lastOrderN) {
    Serial.print(F("  |  avg order "));

    Serial.print(
      (double)g_lastOrderSum /
      (double)g_lastOrderN,
      2
    );
  }

  Serial.print(F("  |  section "));

  if (g_lastSeg < g_segN)
    printSegName(g_lastSeg);
  else
    Serial.print(F("all"));

  Serial.print(F("  |  free heap "));
  Serial.print(freeRam());

  Serial.println();

  Serial.println(
    F("--------------------------------------------------")
  );

  Serial.println();
}

static bool answerQuestionFromFact(
  const char *prompt,
  uint16_t &fs,
  uint16_t &fl,
  uint8_t &fscore
) {
  bool haveFact =
    findFact(
      fs,
      fl,
      fscore,
      0
    );

  if (!haveFact)
    return false;

  if (fscore < FACT_MIN_SCORE)
    return false;

  return true;
}

static void runInference(
  const char *prompt,
  uint8_t mode,
  uint16_t budget
) {
  uint32_t t0 = millis();

  g_needCap = true;
  g_started = false;
  g_histN = 0;
  g_posN = 0;

  uint16_t ptok =
    loadPrompt(
      prompt,
      true
    );

  if (g_showThink) {
    thinkLine("tokenize     -> ");
    Serial.print(ptok);

    Serial.print(F(" tokens, "));
    Serial.print(
      (int)g_topicWordN
    );

    Serial.println(
      F(" topic anchors")
    );
  }

  setScan(255);

  uint16_t hits = 0;
  uint8_t seg = 255;

  if (
    g_segN &&
    g_topicWordN
  )
    hits = scoreSegments();

  if (g_forceSeg != 255) {
    seg = g_forceSeg;
  } else if (
    g_segN &&
    g_topicWordN &&
    hits
  ) {
    if (
      mode == 2 &&
      g_storySeg != 255 &&
      g_segHits[g_storySeg]
    )
      seg = g_storySeg;
    else
      seg =
        bestSegment(
          mode == 1
        );
  } else if (
    mode == 2 &&
    g_storySeg != 255
  ) {
    seg = g_storySeg;
  }

  setScan(seg);

  g_lastSeg = seg;

  g_creative =
    (g_storySeg != 255) &&
    (seg == g_storySeg);

  /*
    Answer mode is now strictly factual.
    It never falls through to free n-gram generation.
  */

  uint16_t fs = 0;
  uint16_t fl = 0;
  uint8_t fscore = 0;

  bool haveFact = false;

  if (mode == 1) {
    haveFact =
      answerQuestionFromFact(
        prompt,
        fs,
        fl,
        fscore
      );

    if (g_showThink) {
      thinkLine("route        -> ");

      if (seg < g_segN) {
        Serial.print(F("section "));
        printSegName(seg);
        Serial.print(F(", "));
        Serial.print(
          g_segHits[seg]
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

      thinkLine("retrieve     -> ");

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
      printFact(fs, fl);
      Serial.println();

      Serial.println(
        F("================")
      );

      report(
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

    printTopicWords(prompt);

    Serial.println();

    Serial.println(
      F("No free n-gram answer was generated because corpus text is not treated as verified knowledge.")
    );

    Serial.println(
      F("================")
    );

    report(
      millis() - t0,
      ptok,
      0
    );

    return;
  }

  if (
    mode != 2 &&
    g_segN &&
    g_topicWordN &&
    hits == 0
  ) {
    Serial.println();

    Serial.println(
      F("=== no usable context ===")
    );

    Serial.print(
      F("not in the corpus : ")
    );

    printTopicWords(prompt);

    Serial.println();

    Serial.print(
      F("sections available: ")
    );

    listSegments();

    Serial.println();

    Serial.println(
      F("add prose to TRAIN_TEXT, or teach it with /learn.")
    );

    Serial.println(
      F("=========================")
    );

    report(
      millis() - t0,
      ptok,
      0
    );

    return;
  }

  scanCorpus(effOrder());

  uint8_t grounded = 0;

  for (
    uint8_t k = effOrder();
    k >= 1;
    k--
  ) {
    if (g_cand[k].count) {
      grounded = k;
      break;
    }
  }

  if (g_showThink) {
    thinkLine(
      "probe        -> longest matched context: order "
    );

    Serial.print((int)grounded);

    Serial.print(F(" ("));

    Serial.print(
      grounded
        ? g_cand[grounded].count
        : 0
    );

    Serial.print(
      F(" continuations, ")
    );

    Serial.print(
      (unsigned long)g_corpusTokens
    );

    Serial.println(
      F(" tokens in scope)")
    );
  }

  if (g_showThink) {
    thinkLine("plan         -> mode=");

    Serial.print(
      mode == 0
        ? F("complete")
        : F("story")
    );

    Serial.print(F(" budget="));
    Serial.print(budget);

    Serial.print(F(" temp="));
    Serial.print((int)g_temp);

    Serial.print(F(" order<="));
    Serial.println(
      (int)effOrder()
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

    g_started = true;
    g_needCap = false;

    g_singleSentence = true;

    outTok =
      generate(
        budget,
        0,
        40
      );

    g_singleSentence = false;
  } else {
    scanCorpus(effOrder());

    if (g_topicCtxN) {
      for (uint8_t j = 0; j < MAX_ORDER; j++)
        g_ctx[j] =
          g_topicCtx[j];

      g_ctxN = g_topicCtxN;
    } else {
      ctxSentenceStart();
    }

    g_started = false;
    g_needCap = true;

    outTok =
      generate(
        budget,
        mode == 2 ? 3 : 0,
        mode == 2 ? 60 : 45
      );
  }

  Serial.println();

  Serial.println(
    F("================")
  );

  report(
    millis() - t0,
    ptok,
    outTok
  );
}

static void help() {
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

static void doLearn(
  const char *txt
) {
  uint16_t room =
    EEP_SIZE - EEP_HDR;

  uint16_t n =
    (uint16_t)strlen(txt);

  uint16_t add =
    n + 1;

  uint8_t last =
    n
      ? (uint8_t)txt[n - 1]
      : 0;

  if (!isTerminal(last))
    add++;

  if (
    (uint32_t)g_eepLen +
    add >
    room
  ) {
    Serial.println(
      F("EEPROM store full. Use /forget, or move the text into training_data.h")
    );

    return;
  }

  uint16_t w =
    EEP_HDR + g_eepLen;

  eepWrite(w++, ' ');

  for (uint16_t i = 0; i < n; i++)
    eepWrite(
      w++,
      (uint8_t)txt[i]
    );

  if (!isTerminal(last))
    eepWrite(w++, '.');

  eepSetLen(
    g_eepLen + add
  );

  buildSegments();

  Serial.print(
    F("learned as unverified prose. corpus is now ")
  );

  Serial.print(
    corpusLen()
  );

  Serial.print(F(" bytes ("));

  Serial.print(g_eepLen);

  Serial.println(
    F(" learned bytes)")
  );
}

static void stats() {
  uint32_t t0 = millis();

  setScan(255);
  ctxReset();
  g_topicWordN = 0;

  scanCorpus(1);

  uint32_t dt =
    millis() - t0;

  Serial.print(
    F("corpus bytes    : ")
  );

  Serial.println(
    corpusLen()
  );

  Serial.print(
    F("  flash         : ")
  );

  Serial.println(
    g_flashLen
  );

  Serial.print(
    F("  learned store : ")
  );

  Serial.println(
    g_eepLen
  );

  Serial.print(
    F("corpus tokens   : ")
  );

  Serial.println(
    (unsigned long)g_corpusTokens
  );

  Serial.print(
    F("lexicon words   : ")
  );

  Serial.println(
    lexCount()
  );

  Serial.print(
    F("sentence starts : ")
  );

  Serial.println(
    g_startCand.count
  );

  Serial.print(
    F("sections        : ")
  );

  listSegments();

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
    (int)g_maxOrder
  );

  Serial.print(
    F("temperature      : ")
  );

  Serial.println(
    (int)g_temp
  );

  Serial.print(
    F("fact threshold  : ")
  );

  Serial.println(
    FACT_MIN_SCORE
  );

  Serial.print(
    F("free heap       : ")
  );

  Serial.print(
    freeRam()
  );

  Serial.println(
    F(" bytes")
  );

  Serial.println();
}

static void bench() {
  uint32_t t0 = millis();

  g_needCap = true;
  g_started = false;
  g_histN = 0;
  g_posN = 0;

  setScan(255);

  loadPrompt(
    "the cat",
    true
  );

  bool s = g_showThink;

  g_showThink = false;

  Serial.print(
    F("bench output: ")
  );

  uint16_t n =
    generate(
      40,
      0,
      50
    );

  g_showThink = s;

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

static bool startsWith(
  const char *s,
  const char *p
) {
  return strncmp(
    s,
    p,
    strlen(p)
  ) == 0;
}

static bool sectionNameEquals(
  uint8_t k,
  const char *name
) {
  if (k >= g_segN)
    return false;

  uint16_t n =
    (uint16_t)strlen(name);

  if (n != g_seg[k].nameLen)
    return false;

  if (
    g_seg[k].nameStart ==
    0xFFFFFFFFUL
  )
    return strcmp(
      name,
      "learned"
    ) == 0;

  for (uint16_t i = 0; i < n; i++) {
    if (
      lc(
        pgm_read_byte(
          &TRAIN_TEXT[
            g_seg[k].nameStart + i
          ]
        )
      ) !=
      lc((uint8_t)name[i])
    )
      return false;
  }

  return true;
}

static void handleLine(
  char *line
) {
  while (*line == ' ')
    line++;

  if (!*line)
    return;

  if (line[0] != '/') {
    const char *body = line;

    uint8_t m =
      detectMode(&body);

    if (
      !*body &&
      m != 2
    ) {
      Serial.println(
        F("that is an instruction with no subject in it.")
      );

      return;
    }

    runInference(
      body,
      m,
      m == 2
        ? (uint16_t)(
            g_budget > 21845
              ? 65535
              : g_budget * 3
          )
        : (
            m == 1
              ? 30
              : g_budget
          )
    );

    return;
  }

  if (startsWith(line, "/help")) {
    help();
    return;
  }

  if (startsWith(line, "/stats")) {
    stats();
    return;
  }

  if (startsWith(line, "/bench")) {
    bench();
    return;
  }

  if (startsWith(line, "/forget")) {
    eepSetLen(0);
    buildSegments();

    Serial.println(
      F("learned data erased.")
    );

    return;
  }

  if (startsWith(line, "/think")) {
    g_showThink =
      (strstr(
        line,
        "off"
      ) == NULL);

    Serial.print(
      F("debug trace: ")
    );

    Serial.println(
      g_showThink
        ? F("on")
        : F("off")
    );

    return;
  }

  if (startsWith(line, "/raw ")) {
    runInference(
      line + 5,
      0,
      g_budget
    );

    return;
  }

  if (startsWith(line, "/ask ")) {
    const char *b =
      line + 5;

    detectMode(&b);

    runInference(
      b,
      1,
      30
    );

    return;
  }

  if (startsWith(line, "/story ")) {
    const char *b =
      line + 7;

    detectMode(&b);

    runInference(
      b,
      2,
      g_budget > 21845
        ? 65535
        : (uint16_t)(
            g_budget * 3
          )
    );

    return;
  }

  if (startsWith(line, "/topic")) {
    const char *a =
      line + 6;

    while (*a == ' ')
      a++;

    if (!*a) {
      Serial.print(
        F("sections: ")
      );

      listSegments();

      Serial.println();

      Serial.print(
        F("locked to: ")
      );

      if (
        g_forceSeg <
        g_segN
      )
        printSegName(
          g_forceSeg
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
      isSectionNameChar(
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
        (char)lc(
          (uint8_t)a[i]
        );

    name[copy] = 0;

    if (
      strcmp(name, "auto") == 0 ||
      strcmp(name, "all") == 0
    ) {
      g_forceSeg = 255;

      Serial.println(
        F("topic lock off")
      );

      Serial.println();

      return;
    }

    for (uint8_t k = 0; k < g_segN; k++) {
      if (
        sectionNameEquals(
          k,
          name
        )
      ) {
        g_forceSeg = k;

        Serial.print(
          F("locked to ")
        );

        printSegName(k);

        Serial.println();
        Serial.println();

        return;
      }
    }

    Serial.print(
      F("no such section. have: ")
    );

    listSegments();

    Serial.println();
    Serial.println();

    return;
  }

  if (startsWith(line, "/learn ")) {
    doLearn(line + 7);
    return;
  }

  if (startsWith(line, "/class ")) {
    loadPrompt(
      line + 7,
      false
    );

    classify();

    Serial.println();

    return;
  }

  if (startsWith(line, "/words ")) {
    lexScan(
      line + 7,
      0
    );

    return;
  }

  if (startsWith(line, "/rhyme ")) {
    lexScan(
      line + 7,
      1
    );

    return;
  }

  if (startsWith(line, "/calc ")) {
    g_cp =
      line + 6;

    double v =
      parseExpr();

    Serial.print(F("= "));
    Serial.println(v, 4);
    Serial.println();

    return;
  }

  if (startsWith(line, "/gen ")) {
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

    runInference(
      p,
      0,
      n
    );

    return;
  }

  if (startsWith(line, "/set ")) {
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

    if (startsWith(p, "temp")) {
      if (v < 0)
        v = 0;

      if (v > 100)
        v = 100;

      g_temp =
        (uint8_t)v;

      Serial.print(
        F("temp = ")
      );

      Serial.println(
        (int)g_temp
      );
    }

    else if (
      startsWith(
        p,
        "order"
      )
    ) {
      if (v < 1)
        v = 1;

      if (v > MAX_ORDER)
        v = MAX_ORDER;

      g_maxOrder =
        (uint8_t)v;

      Serial.print(
        F("order = ")
      );

      Serial.println(
        (int)g_maxOrder
      );
    }

    else if (
      startsWith(
        p,
        "len"
      )
    ) {
      if (v < 5)
        v = 5;

      if (v > 400)
        v = 400;

      g_budget =
        (uint16_t)v;

      Serial.print(
        F("len = ")
      );

      Serial.println(
        g_budget
      );
    }

    else if (
      startsWith(
        p,
        "seed"
      )
    ) {
      g_rng =
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

static char g_in[IN_BUF_LEN];
static uint8_t g_inN = 0;

void setup() {
  Serial.begin(115200);

  delay(100);

  if (!EEPROM.begin(EEP_SIZE)) {
    Serial.println(
      F("EEPROM emulation initialization failed.")
    );

    while (true)
      delay(1000);
  }

  /*
    ESP32 supports a larger flash corpus than the old
    uint16_t implementation. All corpus offsets are now
    32-bit, so TRAIN_TEXT is no longer silently truncated
    at 65535 bytes.
  */
  g_flashLen =
    (uint32_t)strlen_P(
      TRAIN_TEXT
    );

  if (
    eepRead(0) == EEP_M0 &&
    eepRead(1) == EEP_M1
  ) {
    g_eepLen =
      (uint16_t)eepRead(2) |
      (
        (uint16_t)eepRead(3)
        << 8
      );

    if (
      g_eepLen >
      EEP_SIZE - EEP_HDR
    )
      eepSetLen(0);
  } else {
    eepSetLen(0);
  }

  buildSegments();

  setScan(255);

  g_rng ^=
    micros() *
    2654435761UL;

  banner();

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
      g_in[g_inN] = 0;

      Serial.println();

      if (g_inN)
        handleLine(g_in);

      g_inN = 0;

      Serial.print(
        F("> ")
      );
    }

    else if (
      g_inN <
      IN_BUF_LEN - 1
    ) {
      g_in[g_inN++] =
        (char)c;
    }
  }
}
