// (c) Aharshi Deb, 2026
// This is Espie V2.0
// If you are compiling this then you have agreed to the rules at the bottom of README.md.
#if !defined(ARDUINO_ARCH_ESP32)
#error "Espie V2.0 requires Arduino-ESP32."
#endif

#include <Arduino.h>
#include <pgmspace.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "training_data.h"

namespace EspieV2 {

static constexpr uint16_t INPUT_CAP = 240;
static constexpr uint8_t TOKEN_CAP = 64;
static constexpr uint16_t RESPONSE_CAP = 160;
static constexpr uint8_t TOP_K = 28;
static constexpr uint8_t RECENT_CAP = 16;
static constexpr uint16_t DEFAULT_LENGTH = 32;
static constexpr uint16_t FACT_WORD_CAP = 24;
static constexpr uint8_t FACT_FUZZY_MIN = 70;
static constexpr uint8_t MAX_NAME_WORDS = 4;

struct Runtime {
  float hidden[RNN_HIDDEN_DIM];
  uint16_t recent[RECENT_CAP];
  uint8_t recentCount;
  uint8_t temperature;
  uint16_t length;
  uint32_t rng;
  bool trace;
  uint32_t tokens;
  uint32_t replies;
  char userName[32];
  bool hasName;
};

static Runtime rt = {
  {0}, {0}, 0, 52, DEFAULT_LENGTH, 0x1F123BB5UL, false, 0, 0, {0}, false
};

static char line[INPUT_CAP];
static uint16_t lineLen = 0;
static char response[RESPONSE_CAP];

static uint32_t rng32() {
  uint32_t x = rt.rng;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rt.rng = x ? x : 0xA341316CUL;
  return rt.rng;
}

static float randomUnit() {
  return (float)(rng32() & 0x00FFFFFFUL) / 16777216.0f;
}

static void clearState() {
  memset(rt.hidden, 0, sizeof(rt.hidden));
  memset(rt.recent, 0, sizeof(rt.recent));
  rt.recentCount = 0;
}

static void flashCopy(char *dst, size_t cap, const char *src) {
  if (!cap) return;
  size_t n = 0;
  while (n + 1 < cap) {
    char c = (char)pgm_read_byte(src + n);
    if (!c) break;
    dst[n++] = c;
  }
  dst[n] = 0;
}

static bool isWordChar(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') ||
         c == '-' || c == '+';
}

static char lowerAscii(char c) {
  if (c >= 'A' && c <= 'Z') return (char)(c + 32);
  return c;
}

static void normalizeText(const char *src, char *dst, size_t cap) {
  if (!cap) return;
  size_t w = 0;
  bool space = true;
  for (size_t i = 0; src[i] && w + 1 < cap; ++i) {
    char c = lowerAscii(src[i]);
    if (c == '\'' || c == '"' || c == '`') continue;
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
      dst[w++] = c;
      space = false;
      continue;
    }
    if (c == '+' || c == '-') {
      if (!space && w + 1 < cap) dst[w++] = ' ';
      space = true;
      continue;
    }
    if (!space && w + 1 < cap) {
      dst[w++] = ' ';
      space = true;
    }
  }
  while (w && dst[w - 1] == ' ') --w;
  dst[w] = 0;
}

static void normalizeQuestion(const char *src, char *dst, size_t cap) {
  char prepared[INPUT_CAP];
  size_t pw = 0;
  for (size_t i = 0; src[i] && pw + 1 < sizeof(prepared); ++i) {
    if ((src[i] == 'c' || src[i] == 'C') && src[i + 1] == '+' && src[i + 2] == '+') {
      const char *rep = "c plus plus";
      size_t n = strlen(rep);
      if (pw + n >= sizeof(prepared)) break;
      memcpy(prepared + pw, rep, n); pw += n; i += 2;
    } else {
      prepared[pw++] = src[i];
    }
  }
  prepared[pw] = 0;
  char base[INPUT_CAP];
  normalizeText(prepared, base, sizeof(base));

  const char *prefixes[] = {
    "can you tell me ", "could you tell me ", "please tell me ",
    "can you explain ", "could you explain ", "please explain ",
    "tell me "
  };
  const uint8_t prefixCount = sizeof(prefixes) / sizeof(prefixes[0]);
  for (uint8_t i = 0; i < prefixCount; ++i) {
    size_t n = strlen(prefixes[i]);
    if (!strncmp(base, prefixes[i], n)) {
      memmove(base, base + n, strlen(base + n) + 1);
      break;
    }
  }

  char rebuilt[INPUT_CAP];
  size_t w = 0;
  const char *p = base;
  while (*p && w + 1 < sizeof(rebuilt)) {
    while (*p == ' ') ++p;
    if (!*p) break;
    const char *b = p;
    while (*p && *p != ' ') ++p;
    size_t n = (size_t)(p - b);
    const char *replacement = nullptr;
    size_t rn = 0;

    if (n == 5 && !strncmp(b, "whats", 5)) { replacement = "what is"; rn = 7; }
    else if (n == 4 && !strncmp(b, "hows", 4)) { replacement = "how is"; rn = 6; }
    else if (n == 4 && !strncmp(b, "wheres", 4)) { replacement = "where is"; rn = 8; }
    else if (n == 4 && !strncmp(b, "whos", 4)) { replacement = "who is"; rn = 6; }
    else if (n == 5 && !strncmp(b, "whys", 5)) { replacement = "why is"; rn = 6; }
    else if (n == 5 && !strncmp(b, "howre", 5)) { replacement = "how are"; rn = 7; }
    else if (n == 5 && !strncmp(b, "youre", 5)) { replacement = "you are"; rn = 7; }
    else if (n == 2 && !strncmp(b, "im", 2)) { replacement = "i am"; rn = 4; }
    else if (n == 3 && !strncmp(b, "its", 3)) { replacement = "it is"; rn = 5; }

    if (w) rebuilt[w++] = ' ';
    if (replacement) {
      if (w + rn >= sizeof(rebuilt)) rn = sizeof(rebuilt) - w - 1;
      memcpy(rebuilt + w, replacement, rn); w += rn;
    } else {
      if (w + n >= sizeof(rebuilt)) n = sizeof(rebuilt) - w - 1;
      memcpy(rebuilt + w, b, n); w += n;
    }
  }
  rebuilt[w] = 0;
  if (!cap) return;
  strncpy(dst, rebuilt, cap - 1);
  dst[cap - 1] = 0;
}

static uint16_t findToken(const char *word) {
  for (uint16_t i = 0; i < RNN_VOCAB_SIZE; ++i) {
    char tmp[24];
    flashCopy(tmp, sizeof(tmp), RNN_VOCAB[i]);
    if (!strcmp(tmp, word)) return i;
  }
  return 0;
}

static uint8_t tokenize(const char *src, uint16_t *ids, uint8_t cap) {
  uint8_t count = 0;
  char word[32];
  uint8_t n = 0;

  for (uint16_t i = 0;; ++i) {
    char c = src[i];
    bool end = !c || !isWordChar(c);
    if (!end && (size_t)n + 1 < sizeof(word)) {
      word[n++] = lowerAscii(c);
      continue;
    }

    if (n) {
      word[n] = 0;
      if (count < cap) ids[count++] = findToken(word);
      n = 0;
    }

    if (!c) break;

    if (strchr(".,!?;:%()", c)) {
      char p[2] = {c, 0};
      if (count < cap) ids[count++] = findToken(p);
    }
  }
  return count;
}

static void stepRnn(uint16_t token) {
  float next[RNN_HIDDEN_DIM];

  for (uint16_t h = 0; h < RNN_HIDDEN_DIM; ++h) {
    float s = pgm_read_float(&RNN_BH[h]);
    for (uint16_t e = 0; e < RNN_EMBED_DIM; ++e) {
      float x = pgm_read_float(&RNN_EMBED[(uint32_t)token * RNN_EMBED_DIM + e]);
      s += pgm_read_float(&RNN_WXH[(uint32_t)h * RNN_EMBED_DIM + e]) * x;
    }
    for (uint16_t k = 0; k < RNN_HIDDEN_DIM; ++k) {
      s += pgm_read_float(&RNN_WHH[(uint32_t)h * RNN_HIDDEN_DIM + k]) * rt.hidden[k];
    }
    next[h] = tanhf(s);
  }
  memcpy(rt.hidden, next, sizeof(next));
}

static void remember(uint16_t id) {
  if (rt.recentCount < RECENT_CAP) {
    rt.recent[rt.recentCount++] = id;
  } else {
    memmove(rt.recent, rt.recent + 1, sizeof(uint16_t) * (RECENT_CAP - 1));
    rt.recent[RECENT_CAP - 1] = id;
  }
}

static uint8_t recentCount(uint16_t id) {
  uint8_t n = 0;
  for (uint8_t i = 0; i < rt.recentCount; ++i) if (rt.recent[i] == id) ++n;
  return n;
}

static bool isForbidden(uint16_t id) {
  return id == 0 || id == 1 || id == 3 || id == 4;
}

static bool isTerminal(uint16_t id) {
  char t[8];
  flashCopy(t, sizeof(t), RNN_VOCAB[id]);
  return !strcmp(t, ".") || !strcmp(t, "!") || !strcmp(t, "?");
}

static uint16_t sampleNext() {
  struct Pick { uint16_t id; float score; } top[TOP_K];
  uint8_t used = 0;
  float temperature = (float)rt.temperature / 100.0f;

  for (uint16_t id = 0; id < RNN_VOCAB_SIZE; ++id) {
    if (isForbidden(id)) continue;
    float s = pgm_read_float(&RNN_BO[id]);
    for (uint16_t h = 0; h < RNN_HIDDEN_DIM; ++h) {
      s += pgm_read_float(&RNN_WHO[(uint32_t)id * RNN_HIDDEN_DIM + h]) * rt.hidden[h];
    }

    uint8_t repeats = recentCount(id);
    if (repeats) s -= 0.20f * repeats;
    s /= temperature;

    if (used < TOP_K) {
      top[used++] = {id, s};
    } else if (s > top[used - 1].score) {
      top[used - 1] = {id, s};
    } else continue;

    uint8_t p = used - 1;
    while (p && top[p].score > top[p - 1].score) {
      Pick z = top[p - 1]; top[p - 1] = top[p]; top[p] = z; --p;
    }
  }

  if (!used) return 0;
  float peak = top[0].score;
  float total = 0;
  float weights[TOP_K];
  for (uint8_t i = 0; i < used; ++i) {
    weights[i] = expf(top[i].score - peak);
    total += weights[i];
  }
  float r = randomUnit() * total;
  for (uint8_t i = 0; i < used; ++i) {
    if (r <= weights[i]) return top[i].id;
    r -= weights[i];
  }
  return top[0].id;
}

static void appendText(char *out, size_t cap, size_t &used, const char *token) {
  if (!token || !*token) return;
  bool punct = strchr(".,!?;:%)", token[0]) != nullptr;
  bool open = token[0] == '(';
  size_t n = strlen(token);

  if (punct) {
    while (used && out[used - 1] == ' ') --used;
    if (used + n + 2 >= cap) return;
    memcpy(out + used, token, n); used += n;
    out[used++] = ' '; out[used] = 0;
    return;
  }

  if (!open && used && out[used - 1] != ' ') {
    if (used + 1 >= cap) return;
    out[used++] = ' ';
  }
  if (used + n + 1 >= cap) return;
  memcpy(out + used, token, n); used += n; out[used] = 0;
}

static bool generateReply(const char *prompt, char *out, size_t cap) {
  uint16_t ids[TOKEN_CAP];
  uint8_t n = tokenize(prompt, ids, TOKEN_CAP - 2);

  // Persistent conversation: deliberately do NOT clearState() here.
  uint16_t userId = findToken("<user>");
  uint16_t espieId = findToken("<espie>");
  stepRnn(userId); rt.tokens++;
  for (uint8_t i = 0; i < n; ++i) { stepRnn(ids[i]); rt.tokens++; }
  stepRnn(espieId); rt.tokens++;

  out[0] = 0;
  size_t used = 0;
  bool emitted = false;

  for (uint16_t step = 0; step < rt.length; ++step) {
    uint16_t id = sampleNext();
    if (id == 2) break;
    if (isForbidden(id)) continue;

    char token[24];
    flashCopy(token, sizeof(token), RNN_VOCAB[id]);
    if (!strcmp(token, "<unk>") || !strcmp(token, "<user>") || !strcmp(token, "<espie>")) continue;

    appendText(out, cap, used, token);
    if (used >= cap - 2) break;
    emitted = true;
    remember(id);

    if (isTerminal(id)) break;
    stepRnn(id); rt.tokens++;
  }

  while (used && out[used - 1] == ' ') out[--used] = 0;
  if (!emitted) return false;

  if (out[used - 1] != '.' && out[used - 1] != '!' && out[used - 1] != '?') {
    if (used + 1 < cap) { out[used++] = '.'; out[used] = 0; }
  }
  return true;
}

static uint8_t wordsOf(const char *s, char words[][24], uint8_t cap) {
  char normalized[INPUT_CAP];
  normalizeQuestion(s, normalized, sizeof(normalized));
  uint8_t count = 0;
  const char *p = normalized;
  while (*p && count < cap) {
    while (*p == ' ') ++p;
    if (!*p) break;
    const char *b = p;
    while (*p && *p != ' ') ++p;
    size_t n = (size_t)(p - b);
    if (n >= 24) n = 23;
    memcpy(words[count], b, n); words[count][n] = 0;
    ++count;
  }
  return count;
}

static bool wordPresent(char words[][24], uint8_t n, const char *needle) {
  for (uint8_t i = 0; i < n; ++i) if (!strcmp(words[i], needle)) return true;
  return false;
}

static uint8_t commonWords(char a[][24], uint8_t an, char b[][24], uint8_t bn) {
  uint8_t hit = 0;
  for (uint8_t i = 0; i < bn; ++i) if (wordPresent(a, an, b[i])) ++hit;
  return hit;
}

static int factLookup(const char *input, char *answer, size_t answerCap, uint8_t *outHit = nullptr, int *outIndex = nullptr) {
  char query[INPUT_CAP];
  normalizeQuestion(input, query, sizeof(query));
  if (!*query) return -1;

  char qw[FACT_WORD_CAP][24];
  uint8_t qn = wordsOf(query, qw, FACT_WORD_CAP);
  int best = -1;
  int bestScore = -1000000;
  uint8_t bestHit = 0;

  for (uint8_t i = 0; i < RNN_MAX_FACTS; ++i) {
    char rawKey[48];
    flashCopy(rawKey, sizeof(rawKey), RNN_FACT_KEYS[i]);
    if (!rawKey[0]) continue;
    char key[INPUT_CAP];
    normalizeQuestion(rawKey, key, sizeof(key));
    if (!strcmp(query, key)) {
      best = i;
      bestScore = 1000000;
      { char exactWords[FACT_WORD_CAP][24]; bestHit = wordsOf(key, exactWords, FACT_WORD_CAP); }
      break;
    }
    char kw[FACT_WORD_CAP][24];
    uint8_t kn = wordsOf(key, kw, FACT_WORD_CAP);
    if (!kn || !qn) continue;
    uint8_t hit = commonWords(qw, qn, kw, kn);
    int score = (int)hit * 1000 - (int)(kn - hit) * 300 - abs((int)qn - (int)kn) * 30;
    uint8_t coverage = (uint8_t)(((uint16_t)hit * 100) / kn);
    bool plausible = (hit == kn) || (kn >= 2 && coverage >= FACT_FUZZY_MIN);
    if (plausible && score > bestScore) {
      bestScore = score;
      best = i;
      bestHit = hit;
    }
  }

  if (outHit) *outHit = bestHit;
  if (outIndex) *outIndex = best;
  if (best < 0) return -1;
  flashCopy(answer, answerCap, RNN_FACT_ANSWERS[best]);
  return best;
}

static bool findFact(const char *input) {
  char answer[192];
  uint8_t hit = 0;
  int index = -1;
  int found = factLookup(input, answer, sizeof(answer), &hit, &index);
  if (rt.trace) {
    char query[INPUT_CAP];
    normalizeQuestion(input, query, sizeof(query));
    Serial.print(F("Espie: [fact] normalized: ")); Serial.println(query);
    Serial.print(F("Espie: [fact] matches: ")); Serial.println(hit);
    Serial.print(F("Espie: [fact] index: ")); Serial.println(index);
  }
  if (found < 0) return false;
  Serial.print(F("Espie: ")); Serial.println(answer);
  ++rt.replies;
  return true;
}

static void printBanner() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("RNN Chatbot"));
  Serial.println(F("ESP32-S3 local neural inference"));
  Serial.println(F("========================================"));
  Serial.print(F("Espie: model v2.0 experimental | vocab ")); Serial.print(RNN_VOCAB_SIZE);
  Serial.print(F(" | hidden ")); Serial.println(RNN_HIDDEN_DIM);
  Serial.println(F("Espie: type /help for commands."));
  Serial.println();
}

static void help() {
  Serial.println(F("Espie: /help"));
  Serial.println(F("Espie: /ask <question>   verified fact lookup"));
  Serial.println(F("Espie: /talk <text>      force RNN generation"));
  Serial.println(F("Espie: /calc <expr>      calculator"));
  Serial.println(F("Espie: /topic <text>     topic classifier"));
  Serial.println(F("Espie: /info              runtime/model info"));
  Serial.println(F("Espie: /clear             reset RNN hidden state"));
  Serial.println(F("Espie: /forget            erase stored name"));
  Serial.println(F("Espie: /config temp N     temperature 30-125 (0.30-1.25)"));
  Serial.println(F("Espie: /config len N      response tokens 8-64"));
  Serial.println(F("Espie: /config seed N     RNG seed"));
  Serial.println(F("Espie: /debug on|off      diagnostic output"));
  Serial.println(F("Espie: /about              model architecture"));
}

struct Calc { const char *p; };
static void skip(Calc &c) { while (*c.p == ' ' || *c.p == '\t') ++c.p; }
static double expr(Calc &c);
static double atom(Calc &c) {
  skip(c);
  if (*c.p == '(') { ++c.p; double v = expr(c); skip(c); if (*c.p == ')') ++c.p; return v; }
  char *end = nullptr; double v = strtod(c.p, &end); if (end == c.p) return NAN; c.p = end; return v;
}
static double power(Calc &c) {
  skip(c); bool neg = false; if (*c.p == '+' || *c.p == '-') { neg = *c.p == '-'; ++c.p; }
  double a = atom(c); if (neg) a = -a; skip(c);
  if (*c.p == '^') { ++c.p; a = pow(a, power(c)); }
  return a;
}
static double term(Calc &c) {
  double a = power(c);
  for (;;) { skip(c); char op=*c.p; if(op!='*'&&op!='/'&&op!='%') break; ++c.p; double b=power(c); if(op=='*') a*=b; else if(op=='/') a=(b==0)?NAN:a/b; else a=(b==0)?NAN:fmod(a,b); }
  return a;
}
static double expr(Calc &c) {
  double a=term(c);
  for (;;) { skip(c); char op=*c.p; if(op!='+'&&op!='-') break; ++c.p; double b=term(c); if(op=='+') a+=b; else a-=b; }
  return a;
}
static void doCalc(const char *s) {
  Calc c{s}; double v=expr(c); skip(c);
  if(!isfinite(v)||*c.p){Serial.println(F("Espie: calculator error."));return;}
  Serial.print(F("Espie: ")); Serial.println(v,6);
}

static void classify(const char *s) {
  static const char *labels[] = {"conversation","programming","embedded","language model","artificial intelligence","math","science","debugging","hardware"};
  static const char *keys[] = {"hello hi hey thanks goodbye chat", "code compiler function variable pointer program c++ python", "esp32 gpio uart spi i2c microcontroller firmware", "language model rnn token corpus embedding softmax", "ai artificial intelligence neural machine learning inference", "calculate number equation math plus minus multiply divide", "science physics chemistry biology energy gravity water", "bug error crash debug test failure fix", "cpu memory voltage current circuit chip processor"};
  char q[INPUT_CAP]; normalizeQuestion(s, q, sizeof(q));
  char qw[FACT_WORD_CAP][24]; uint8_t qn = wordsOf(q, qw, FACT_WORD_CAP);
  int best = -1, bestScore = 0;
  for (uint8_t i = 0; i < 9; ++i) {
    char kw[FACT_WORD_CAP][24]; uint8_t kn = wordsOf(keys[i], kw, FACT_WORD_CAP);
    int score = commonWords(qw, qn, kw, kn);
    if (score > bestScore) { bestScore = score; best = i; }
  }
  Serial.print(F("Espie: class: ")); Serial.println(best < 0 ? "general" : labels[best]);
}

static void stats() {
  Serial.println(F("Espie: v2.0 runtime"));
  Serial.print(F("Espie: vocabulary : ")); Serial.println(RNN_VOCAB_SIZE);
  Serial.print(F("Espie: embedding  : ")); Serial.println(RNN_EMBED_DIM);
  Serial.print(F("Espie: hidden     : ")); Serial.println(RNN_HIDDEN_DIM);
  Serial.print(F("Espie: temperature: ")); Serial.println(rt.temperature);
  Serial.print(F("Espie: length     : ")); Serial.println(rt.length);
  Serial.print(F("Espie: tokens     : ")); Serial.println(rt.tokens);
  Serial.print(F("Espie: replies    : ")); Serial.println(rt.replies);
  Serial.print(F("Espie: free heap  : ")); Serial.println(ESP.getFreeHeap());
}

static void about() {
  Serial.println(F("Espie: v2.0 experimental Elman RNN chatbot."));
  Serial.println(F("Espie: token -> embedding -> recurrent hidden state -> dense logits -> top-k softmax sampling."));
  Serial.println(F("Espie: hidden state persists across chat messages until /clear."));
  Serial.println(F("Espie: /ask uses a separate normalized verified-fact layer."));
  Serial.println(F("Espie: every generated response stops at the first sentence terminator."));
  Serial.println(F("Espie: model data is compiled into training_data.h; no EEPROM is used."));
}

static void configCommand(const char *arg) {
  if (!strncmp(arg, "temp ", 5)) {
    int v = atoi(arg + 5);
    if (v < 30) v = 30;
    if (v > 125) v = 125;
    rt.temperature = (uint8_t)v;
    Serial.print(F("Espie: temperature = 0."));
    if (v < 100) Serial.print('0');
    Serial.println(v);
    return;
  }
  if (!strncmp(arg, "len ", 4)) {
    int v = atoi(arg + 4);
    if (v < 8) v = 8;
    if (v > 64) v = 64;
    rt.length = (uint16_t)v;
    Serial.print(F("Espie: response length = ")); Serial.println(v);
    return;
  }
  if (!strncmp(arg, "seed ", 5)) {
    rt.rng = strtoul(arg + 5, nullptr, 10);
    if (!rt.rng) rt.rng = 0x1F123BB5UL;
    Serial.println(F("Espie: seed updated."));
    return;
  }
  Serial.println(F("Espie: usage /config temp 30-125 | /config len 8-64 | /config seed N"));
}

static bool captureName(const char *prompt) {
  char q[INPUT_CAP];
  normalizeQuestion(prompt, q, sizeof(q));
  const char *prefix = "my name is ";
  if (strncmp(q, prefix, strlen(prefix)) != 0) return false;
  const char *name = q + strlen(prefix);
  if (!*name) return false;

  uint8_t words = 0;
  for (const char *p = name; *p;) {
    while (*p == ' ') ++p;
    if (!*p) break;
    ++words;
    while (*p && *p != ' ') ++p;
  }
  if (!words || words > MAX_NAME_WORDS) return false;

  size_t n = strlen(name);
  if (n >= sizeof(rt.userName)) n = sizeof(rt.userName) - 1;
  memcpy(rt.userName, name, n);
  rt.userName[n] = 0;
  rt.hasName = true;
  return true;
}

static bool answerNameQuestion(const char *prompt) {
  if (!rt.hasName) return false;
  char q[INPUT_CAP];
  normalizeQuestion(prompt, q, sizeof(q));
  if (!strcmp(q, "what is my name") ||
      !strcmp(q, "what is my name again") ||
      !strcmp(q, "do you know my name") ||
      !strcmp(q, "tell me my name")) {
    Serial.print(F("Espie: Your name is "));
    Serial.print(rt.userName);
    Serial.println(F("."));
    ++rt.replies;
    return true;
  }
  return false;
}

static bool isQuestionLike(const char *prompt) {
  char q[INPUT_CAP];
  normalizeQuestion(prompt, q, sizeof(q));
  const char *heads[] = {"what ", "why ", "how ", "who ", "where ", "when ", "which ", "can ", "could ", "does ", "do ", "is ", "are ", "tell me ", "explain "};
  for (uint8_t i = 0; i < sizeof(heads) / sizeof(heads[0]); ++i) {
    if (!strncmp(q, heads[i], strlen(heads[i]))) return true;
  }
  size_t n = strlen(q);
  return n && (q[n - 1] == '?' || q[n - 1] == '.');
}

static void respond(const char *prompt, bool forceRnn = false) {
  if (!prompt || !*prompt) return;

  if (!forceRnn && captureName(prompt)) {
    Serial.println(F("Espie: Nice to meet you."));
    ++rt.replies;
    return;
  }
  if (!forceRnn && answerNameQuestion(prompt)) return;

  char factAnswer[192];
  if (!forceRnn && isQuestionLike(prompt) && factLookup(prompt, factAnswer, sizeof(factAnswer)) >= 0) {
    if (rt.trace) Serial.println(F("Espie: [fact] automatic verified match"));
    Serial.print(F("Espie: ")); Serial.println(factAnswer);
    ++rt.replies;
    return;
  }

  bool ok = false;
  for (uint8_t a = 0; a < 4 && !ok; ++a) {
    ok = generateReply(prompt, response, sizeof(response));
    if (!ok) rt.rng ^= 0x9E3779B9UL + a;
  }
  if (!ok) {
    Serial.println(F("Espie: I could not form a response."));
    return;
  }
  Serial.print(F("Espie: ")); Serial.println(response);
  ++rt.replies;
}

static bool commandMatch(const char *input, const char *name, const char **arg = nullptr) {
  size_t n = strlen(name);
  if (strncmp(input, name, n) != 0) return false;
  if (input[n] == 0) { if (arg) *arg = input + n; return true; }
  if (input[n] == ' ') { if (arg) *arg = input + n + 1; return true; }
  return false;
}

static void command(const char *input) {
  const char *arg = nullptr;
  if (commandMatch(input, "/help")) { help(); return; }
  if (commandMatch(input, "/ask", &arg)) {
    if (!*arg || !findFact(arg)) Serial.println(F("Espie: I do not have a verified answer for that."));
    return;
  }
  if (commandMatch(input, "/talk", &arg)) { if (*arg) respond(arg, true); return; }
  if (commandMatch(input, "/calc", &arg)) { if (*arg) doCalc(arg); return; }
  if (commandMatch(input, "/topic", &arg)) { if (*arg) classify(arg); return; }
  if (commandMatch(input, "/info")) { stats(); return; }
  if (commandMatch(input, "/clear")) {
    clearState();
    Serial.println(F("Espie: RNN conversation memory reset."));
    return;
  }
  if (commandMatch(input, "/forget")) {
    rt.userName[0] = 0;
    rt.hasName = false;
    Serial.println(F("Espie: profile memory cleared."));
    return;
  }
  if (commandMatch(input, "/about")) { about(); return; }
  if (commandMatch(input, "/config", &arg)) { configCommand(arg); return; }
  if (commandMatch(input, "/debug", &arg)) {
    if (!strcmp(arg, "on")) rt.trace = true;
    else if (!strcmp(arg, "off")) rt.trace = false;
    else { Serial.println(F("Espie: usage /debug on|off")); return; }
    Serial.println(rt.trace ? F("Espie: trace on.") : F("Espie: trace off."));
    return;
  }
  Serial.println(F("Espie: unknown command. Type /help."));
}

static void processLine() {
  while(lineLen&&(line[lineLen-1]==' '||line[lineLen-1]=='\t'))line[--lineLen]=0;
  char *p=line;while(*p==' '||*p=='\t')++p;if(!*p)return;
  if(*p=='/'){command(p);return;}
  if(rt.trace){uint16_t ids[TOKEN_CAP];uint8_t n=tokenize(p,ids,TOKEN_CAP);Serial.print(F("Espie: [rnn] input tokens: "));Serial.println(n);}
  respond(p);
}
}

void setup() {
  Serial.begin(115200);
  delay(250);
  EspieV2::clearState();
  EspieV2::printBanner();
}

void loop() {
  while (Serial.available()) {
    char c=(char)Serial.read();
    if(c=='\r')continue;
    if(c=='\n'){
      EspieV2::line[EspieV2::lineLen]=0;
      EspieV2::processLine();
      EspieV2::lineLen=0;
      continue;
    }
    if(EspieV2::lineLen+1<EspieV2::INPUT_CAP)EspieV2::line[EspieV2::lineLen++]=c;
  }
}
