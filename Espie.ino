 #if !defined(ARDUINO_ARCH_ESP32)
 #error "This sketch targets Arduino-ESP32."
 #endif
 
 #include <Arduino.h>
 #include <EEPROM.h>
 #include <pgmspace.h>
 #include "training_data.h"
 
 #define relay_020(weave_085) (0xFF000000UL + (uint8_t)(weave_085))
 #define nexus_017 (1u + 2u * 3u)
 #define echo_015 (7u << 1)
 #define grain_014 (5u * 28u)
 #define weave_021 (10u << 1)
 #define arc_026 (8u >> 1)
 #define bloom_025 (13u * 2u)
 #define quill_002 (1u << 5)
 #define circuit_028 (1u << 12)
 #define ledger_029 (1u << 2)
 #define scope_030 (0x40u + 13u)
 #define harbor_031 (0x40u + 12u)
 #define atlas_010 0x811C9DC5UL
 #define ember_032 (1u << 3)
 #define prism_019 (9u << 3)
 #define glyph_009 (1u + 2u)
 #define drift_008 1u
 #define pulse_013 (1u << 6)
 
 namespace core_model {
 
 
 
 static uint8_t  harbor_095 = (nexus_017 - 2u);
 static uint8_t  ember_096 = (11u * 2u);
 static uint16_t scope_094 = ((nexus_017 - 2u) * (nexus_017 + 4u));
 static bool     orbit_097 = true;
 
 typedef uint32_t atlas_522;
 
 struct mosaic_004 {
   uint32_t nexus_465;
   uint16_t arc_122;
   uint16_t lumen_263;
   uint8_t circuit_540;
 };
 
 
 struct cinder_005 {
   uint32_t nexus_465;
   uint32_t weave_149;
   uint32_t ember_320;
   atlas_522 scope_318;
   uint16_t harbor_319;
 };
 
 static uint32_t rivet_387 = (0x1F000000UL + 0x00123BB5UL);
 
 
 
 
 static uint32_t pulse_173 = 0;
 static uint16_t circuit_476 = 0;
 
 
 
 
 
 static uint32_t mosaic_420 = 0;
 
 static uint32_t mosaic_388[quill_002 + 1];
 static uint32_t vector_390[quill_002 + 1];
 
 static uint8_t cinder_389 = 0;
 
 static cinder_005 pulse_429[quill_002];
 static uint8_t atlas_426 = 0;
 
 static uint8_t spire_176 = 255;
 static uint8_t pulse_365 = 255;
 
 static bool scope_126 = true;
 
 
 static uint8_t harbor_479 = 255;
 
 static uint16_t forge_427[quill_002];
 
 
 
 
 
 
 
 
 
 
 
 static const char fathom_027[] PROGMEM =
 "the a an and or but of to in on at is are was were be been being it its this that these those "
 "for with from by as if then than so not no yes do does did done have has had will would can could "
 "should may might must i you he she we they me him her them my your his our their what which who "
 "whom how why when where all any some more most much many very just also about into out up down "
 "over under again once here there tell say said give me please what's whats";
 
 static const char rivet_003[] PROGMEM =
 "write tell make give show compose generate create describe explain draft";
 
 static const char orbit_001[] PROGMEM =
 "what who whom why how when where which whats hows";
 
 static const char spire_016[] PROGMEM =
 "write tell make give show compose generate create describe explain draft "
 "me my mine a an the some any about of for on to please can you i we "
 "short long little bit brief quick nice good paragraph story stories tale "
 "essay text piece something anything more another";
 
 static atlas_522 delta_114[nexus_017];
 static uint8_t nexus_113 = 0;
 
 
 
 
 
 
 
 
 
 
 static atlas_522 prism_531[ember_032];
 static uint8_t delta_530 = 0;
 
 
 
 static mosaic_004 arc_090[nexus_017 + 1];
 static mosaic_004 vault_438;
 
 static uint32_t glyph_521 = 0;
 
 static atlas_522 spire_528[nexus_017];
 static uint8_t nexus_529 = 0;
 
 static uint16_t relay_532 = 0;
 
 
 
 
 
 
 
 
 
 static bool circuit_092 = true;
 static bool circuit_348 = false;
 
 
 
 static atlas_522 grain_398[echo_015];
 static uint8_t forge_395 = 0;
 static uint8_t rivet_259 = 0;
 
 
 static uint32_t pulse_397[prism_019];
 static uint8_t rune_396 = 0;
 
 
 
 
 
 
 
 static uint16_t signal_343 = 0;
 static uint16_t vault_342 = 0;
 
 static bool prism_339 = false;
 
 
 static atlas_522 kernel_376[weave_021];
 static uint8_t signal_375 = 0;
 
 
 
 
 
 
 
 
 
 
 
 
 static const char *signal_087;
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 static char glyph_233[grain_014];
 static uint8_t drift_232 = 0;
 
 
 
 static inline uint32_t quill_322();
 static inline uint16_t quill_386(uint16_t fathom_315);
 static inline uint8_t signal_471(uint16_t mosaic_036);
 static inline void bloom_473(uint16_t mosaic_036, uint8_t vector_550);
 static inline void vault_470();
 static void kernel_472(uint16_t fathom_315);
 static inline uint8_t lumen_039();
 static inline uint32_t spire_304();
 static inline uint8_t echo_303(uint32_t quill_226);
 static inline uint8_t fathom_283(uint8_t weave_085);
 static inline bool ember_576(uint8_t weave_085);
 static inline bool glyph_425(uint8_t weave_085);
 static inline bool fathom_379(uint8_t weave_085);
 static inline bool relay_500(uint8_t weave_085);
 static inline atlas_522 drift_328(uint32_t rivet_195);
 static uint8_t drift_392(
   uint32_t &drift_360,
   uint32_t &nexus_465,
   uint16_t &lumen_263,
   atlas_522 &glyph_201
 );
 static atlas_522 atlas_202(const char *circuit_412, uint16_t fathom_315);
 static void lumen_103();
 static void delta_178();
 
 static void circuit_380(atlas_522 rivet_195);
 static void delta_402();
 static void delta_178();
 static inline uint32_t glyph_105(uint32_t signal_247, atlas_522 rivet_195);
 static bool orbit_577(const char *nexus_273, atlas_522 rivet_195);
 static inline bool arc_250(atlas_522 rivet_195);
 static bool echo_527(atlas_522 rivet_195);
 static bool rivet_483(atlas_522 rivet_195);
 static void cinder_229(uint8_t mosaic_292);
 static bool forge_203(
   uint32_t nexus_465,
   uint32_t weave_149,
   uint32_t &ember_320,
   uint16_t &harbor_319,
   atlas_522 &scope_318
 );
 static void drift_136();
 static void ember_448(uint8_t signal_247);
 static void orbit_449();
 static void nexus_433(uint8_t spire_432);
 static uint16_t glyph_297();
 static uint8_t rivet_099(bool vector_454);
 static bool rune_364(
   uint32_t nexus_465,
   uint16_t lumen_263,
   atlas_522 glyph_201
 );
 static void cinder_581(
   uint32_t nexus_465,
   uint16_t lumen_263,
   uint8_t circuit_540,
   atlas_522 glyph_201
 );
 static void echo_399(uint32_t scope_350);
 static bool glyph_361(uint32_t scope_350);
 static void spire_400(atlas_522 rivet_195);
 static uint8_t drift_520(atlas_522 rivet_195);
 static atlas_522 fathom_091(const mosaic_004 &weave_085);
 static bool bloom_089(
   const mosaic_004 &weave_085,
   uint8_t relay_436,
   bool bloom_313
 );
 static uint16_t atlas_490(
   uint16_t spire_080,
   uint8_t prism_435,
   uint8_t atlas_394
 );
 static uint8_t rune_524(
   const char *circuit_412,
   bool ledger_381
 );
 static bool weave_373(atlas_522 rivet_195);
 static atlas_522 atlas_170(
   const char *scope_350,
   uint16_t &drift_264
 );
 static bool drift_456(const char *circuit_412);
 static uint8_t vector_102(
   const char **vault_502
 );
 static void quill_450(
   const char *relay_372
 );
 static bool bloom_281(
   uint16_t &prism_051,
   uint16_t &delta_050,
   uint8_t &forge_075,
   uint8_t mosaic_452
 );
 static void harbor_447(
   uint16_t circuit_412,
   uint16_t fathom_315
 );
 static void fathom_411();
 static double orbit_161();
 
 static void cinder_453();
 static double ember_064();
 static double atlas_362();
 static double prism_499();
 static double orbit_161();
 static void rivet_419(
   const char *quill_354,
   uint8_t pulse_301
 );
 static uint16_t fathom_123();
 static int drift_296();
 static void signal_535(
   const char *ledger_253
 );
 static void grain_366();
 static void spire_368(
   uint32_t weave_309,
   uint16_t vault_374,
   uint16_t arc_346
 );
 static bool weave_053(
   const char *relay_372,
   uint16_t &vault_182,
   uint16_t &forge_171,
   uint8_t &signal_183
 );
 static void prism_371(
   const char *relay_372,
   uint8_t pulse_301,
   uint16_t spire_080
 );
 static void echo_367();
 static void fathom_475(
   const char *fathom_539
 );
 static void nexus_369();
 static void arc_410();
 static bool drift_200(
   const char *circuit_412,
   const char *scope_350
 );
 static bool rune_428(
   uint8_t signal_247,
   const char *ledger_317
 );
 static void glyph_137(
   char *grain_270
 );
 
 
 static double atlas_362() {
   double rivet_067 = ember_064();
 
   cinder_453();
 
   if (*signal_087 == '^') {
     signal_087++;
 
     double delta_146 = atlas_362();
 
     double orbit_385 = 1;
 
     int fathom_315 = (int)delta_146;
 
     bool echo_239 = fathom_315 < 0;
 
     if (echo_239)
       fathom_315 = -fathom_315;
 
     for (int quill_226 = 0; quill_226 < fathom_315; quill_226++)
       orbit_385 *= rivet_067;
 
     return echo_239
       ? 1.0 / orbit_385
       : orbit_385;
   }
 
   return rivet_067;
 }
 
 static uint8_t drift_392(
   uint32_t &drift_360,
   uint32_t &nexus_465,
   uint16_t &lumen_263,
   atlas_522 &glyph_201
 ) {
   const uint32_t fathom_315 = mosaic_420;
 
   while (drift_360 < fathom_315) {
     uint8_t weave_085 = echo_303(drift_360);
 
     if (ember_576(weave_085))
       break;
 
     if (fathom_379(weave_085)) {
       nexus_465 = drift_360;
       lumen_263 = 1;
       glyph_201 = relay_020(weave_085);
       drift_360++;
       return 2;
     }
 
     drift_360++;
   }
 
   if (drift_360 >= fathom_315)
     return 0;
 
   nexus_465 = drift_360;
 
   uint32_t rivet_195 = 2166136261UL;
   uint16_t circuit_252 = 0;
 
   while (drift_360 < fathom_315) {
     uint8_t weave_085 = echo_303(drift_360);
 
     if (!ember_576(weave_085))
       break;
 
     rivet_195 ^= fathom_283(weave_085);
     rivet_195 *= 16777619UL;
 
     drift_360++;
 
     if (circuit_252 < 65535)
       circuit_252++;
   }
 
   lumen_263 = circuit_252;
   glyph_201 = drift_328(rivet_195);
 
   return 1;
 }
 
 static void delta_178() {
   delta_402();
 }
 
 static int drift_296() {
   return (int)ESP.getFreeHeap();
 }
 
 static void signal_535(
   const char *ledger_253
 ) {
   if (!orbit_097)
     return;
 
   Serial.print(F("[debug] "));
   Serial.print(ledger_253);
 }
 
 static inline uint8_t fathom_283(uint8_t weave_085) {
   return (weave_085 >= 'A' && weave_085 <= 'Z') ? (uint8_t)(weave_085 + 32) : weave_085;
 }
 
 static bool drift_200(
   const char *circuit_412,
   const char *scope_350
 ) {
   return strncmp(
     circuit_412,
     scope_350,
     strlen(scope_350)
   ) == 0;
 }
 
 static inline uint8_t signal_471(uint16_t mosaic_036) {
   return EEPROM.read(mosaic_036);
 }
 
 static bool weave_373(atlas_522 rivet_195) {
   for (uint8_t quill_226 = 0; quill_226 < signal_375; quill_226++)
     if (kernel_376[quill_226] == rivet_195)
       return true;
 
   return false;
 }
 
 static uint8_t drift_520(atlas_522 rivet_195) {
   uint8_t fathom_315 = 0;
 
   for (uint8_t quill_226 = 0; quill_226 < forge_395; quill_226++)
     if (grain_398[quill_226] == rivet_195)
       fathom_315++;
 
   return fathom_315;
 }
 
 static inline bool glyph_425(uint8_t weave_085) {
   return
     (weave_085 >= 'a' && weave_085 <= 'z') ||
     (weave_085 >= 'A' && weave_085 <= 'Z') ||
     (weave_085 >= '0' && weave_085 <= '9') ||
     weave_085 == '_' ||
     weave_085 == '-';
 }
 
 static void delta_402() {
   lumen_103();
   circuit_380(relay_020('.'));
 }
 
 static atlas_522 atlas_202(const char *circuit_412, uint16_t fathom_315) {
   uint32_t rivet_195 = 2166136261UL;
 
   for (uint16_t quill_226 = 0; quill_226 < fathom_315; quill_226++) {
     rivet_195 ^= fathom_283((uint8_t)circuit_412[quill_226]);
     rivet_195 *= 16777619UL;
   }
 
   return drift_328(rivet_195);
 }
 
 static uint8_t rivet_099(bool vector_454) {
   uint8_t rune_076 = 255;
   uint16_t atlas_074 = 0;
 
   for (uint8_t signal_247 = 0; signal_247 < atlas_426; signal_247++) {
     if (vector_454 && signal_247 == harbor_479)
       continue;
 
     if (forge_427[signal_247] > atlas_074) {
       atlas_074 = forge_427[signal_247];
       rune_076 = signal_247;
     }
   }
 
   return rune_076;
 }
 
 static double ember_064() {
   cinder_453();
 
   if (*signal_087 == '(') {
     signal_087++;
 
     double vector_550 = orbit_161();
 
     cinder_453();
 
     if (*signal_087 == ')')
       signal_087++;
 
     return vector_550;
   }
 
   if (*signal_087 == '-') {
     signal_087++;
     return -ember_064();
   }
 
   if (*signal_087 == '+') {
     signal_087++;
     return ember_064();
   }
 
   double vector_550 = 0;
   bool signal_055 = false;
 
   while (
     *signal_087 >= '0' &&
     *signal_087 <= '9'
   ) {
     vector_550 =
       vector_550 * 10 +
       (*signal_087 - '0');
 
     signal_087++;
     signal_055 = true;
   }
 
   if (*signal_087 == '.') {
     signal_087++;
 
     double quill_162 = 0.1;
 
     while (
       *signal_087 >= '0' &&
       *signal_087 <= '9'
     ) {
       vector_550 +=
         (*signal_087 - '0') * quill_162;
 
       quill_162 *= 0.1;
       signal_087++;
       signal_055 = true;
     }
   }
 
   if (!signal_055)
     return 0;
 
   return vector_550;
 }
 
 static void echo_367() {
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
 
 static void drift_136() {
   atlas_426 = 0;
   harbor_479 = 255;
 
   uint32_t quill_226 = 0;
   uint32_t harbor_127 = 0;
 
   while (quill_226 + 1 < pulse_173) {
     if (
       pgm_read_byte(&TRAIN_TEXT[quill_226]) == '@' &&
       pgm_read_byte(&TRAIN_TEXT[quill_226 + 1]) == '@'
     ) {
       uint32_t echo_271 = quill_226 + 2;
 
       while (
         echo_271 < pulse_173 &&
         pgm_read_byte(&TRAIN_TEXT[echo_271]) != '\n'
       )
         echo_271++;
 
       uint32_t ember_320;
       uint16_t harbor_319;
       atlas_522 scope_318;
 
       if (forge_203(
         quill_226 + 2,
         echo_271,
         ember_320,
         harbor_319,
         scope_318
       )) {
         if (atlas_426 > 0)
           pulse_429[atlas_426 - 1].weave_149 = quill_226;
 
         if (atlas_426 < quill_002) {
           uint32_t echo_111 =
             (echo_271 < pulse_173) ? echo_271 + 1 : echo_271;
 
           pulse_429[atlas_426].nexus_465 = echo_111;
           pulse_429[atlas_426].weave_149 = pulse_173;
           pulse_429[atlas_426].ember_320 = ember_320;
           pulse_429[atlas_426].harbor_319 = harbor_319;
           pulse_429[atlas_426].scope_318 = scope_318;
 
           if (
             harbor_319 == 7 &&
             scope_318 == atlas_202("stories", 7)
           )
             harbor_479 = atlas_426;
 
           atlas_426++;
           harbor_127 = echo_111;
         }
       }
 
       quill_226 = echo_271;
 
       if (quill_226 < pulse_173)
         quill_226++;
 
       continue;
     }
 
     quill_226++;
   }
 
   if (atlas_426 == 0 && pulse_173) {
     pulse_429[0].nexus_465 = 0;
     pulse_429[0].weave_149 = pulse_173;
     pulse_429[0].ember_320 = 0xFFFFFFFFUL;
     pulse_429[0].harbor_319 = 4;
     pulse_429[0].scope_318 = atlas_202("main", 4);
     atlas_426 = 1;
   }
 
   if (circuit_476 && atlas_426 < quill_002) {
     pulse_429[atlas_426].nexus_465 = pulse_173;
     pulse_429[atlas_426].weave_149 = spire_304();
     pulse_429[atlas_426].ember_320 = 0xFFFFFFFFUL;
     pulse_429[atlas_426].harbor_319 = 7;
     pulse_429[atlas_426].scope_318 = atlas_202("learned", 7);
     atlas_426++;
   }
 
   harbor_479 = 255;
 
   atlas_522 fathom_443 = atlas_202("stories", 7);
 
   for (uint8_t signal_247 = 0; signal_247 < atlas_426; signal_247++) {
     if (pulse_429[signal_247].scope_318 == fathom_443) {
       harbor_479 = signal_247;
       break;
     }
   }
 }
 
 static bool rivet_483(atlas_522 rivet_195) {
   if (!echo_527(rivet_195))
     return false;
 
   if (orbit_577(fathom_027, rivet_195))
     return false;
 
   if (orbit_577(spire_016, rivet_195))
     return false;
 
   return true;
 }
 
 static void spire_400(atlas_522 rivet_195) {
   for (int8_t quill_226 = echo_015 - 1; quill_226 > 0; quill_226--)
     grain_398[quill_226] = grain_398[quill_226 - 1];
 
   grain_398[0] = rivet_195;
 
   if (forge_395 < echo_015)
     forge_395++;
 }
 
 static bool forge_203(
   uint32_t nexus_465,
   uint32_t weave_149,
   uint32_t &ember_320,
   uint16_t &harbor_319,
   atlas_522 &scope_318
 ) {
   uint32_t scope_350 = nexus_465;
 
   while (scope_350 < weave_149 && (
     echo_303(scope_350) == ' ' ||
     echo_303(scope_350) == '\t'
   ))
     scope_350++;
 
   if (scope_350 + 7 <= weave_149 &&
       fathom_283(echo_303(scope_350)) == 's' &&
       fathom_283(echo_303(scope_350 + 1)) == 'e' &&
       fathom_283(echo_303(scope_350 + 2)) == 'c' &&
       fathom_283(echo_303(scope_350 + 3)) == 't' &&
       fathom_283(echo_303(scope_350 + 4)) == 'i' &&
       fathom_283(echo_303(scope_350 + 5)) == 'o' &&
       fathom_283(echo_303(scope_350 + 6)) == 'n' &&
       (scope_350 + 7 == weave_149 ||
        echo_303(scope_350 + 7) == ' ' ||
        echo_303(scope_350 + 7) == '\t')
   ) {
     scope_350 += 7;
 
     while (scope_350 < weave_149 &&
            (echo_303(scope_350) == ' ' || echo_303(scope_350) == '\t'))
       scope_350++;
   }
 
   ember_320 = scope_350;
 
   uint32_t rivet_195 = 2166136261UL;
   uint16_t fathom_315 = 0;
 
   while (scope_350 < weave_149) {
     uint8_t weave_085 = echo_303(scope_350);
 
     if (!glyph_425(weave_085))
       break;
 
     rivet_195 ^= fathom_283(weave_085);
     rivet_195 *= 16777619UL;
 
     scope_350++;
 
     if (fathom_315 < 65535)
       fathom_315++;
   }
 
   if (!fathom_315)
     return false;
 
   harbor_319 = fathom_315;
   scope_318 = drift_328(rivet_195);
 
   return true;
 }
 
 static bool echo_527(atlas_522 rivet_195) {
   for (uint8_t quill_226 = 0; quill_226 < delta_530; quill_226++)
     if (prism_531[quill_226] == rivet_195)
       return true;
 
   return false;
 }
 
 static bool drift_456(const char *circuit_412) {
   static const char quill_034[] PROGMEM =
     "hi hello hey yo sup howdy yeah yes yep yup nah no "
     "okay ok sure alright thanks thank please cool great nice awesome "
     "lol lmao haha hehe hmm hm uh um morning afternoon evening night "
     "welcome goodbye bye good fine well sorry wow really "
     "how are you your doing do does did can could would should "
     "like love think feel want need something interesting";
 
   uint8_t quill_578 = 0;
   uint8_t relay_116 = 0;
   uint8_t grain_526 = 0;
 
   const char *scope_350 = circuit_412;
 
   while (*scope_350) {
     while (*scope_350 && !ember_576((uint8_t)*scope_350))
       scope_350++;
 
     if (!*scope_350)
       break;
 
     const char *forge_555 = scope_350;
 
     while (*scope_350 && ember_576((uint8_t)*scope_350))
       scope_350++;
 
     uint16_t lumen_263 = (uint16_t)(scope_350 - forge_555);
 
     if (!lumen_263)
       continue;
 
     atlas_522 rivet_195 = atlas_202(forge_555, lumen_263);
 
     quill_578++;
 
     if (orbit_577(quill_034, rivet_195))
       relay_116++;
 
     if (
       lumen_263 >= 3 &&
       !orbit_577(fathom_027, rivet_195) &&
       !orbit_577(spire_016, rivet_195) &&
       !orbit_577(quill_034, rivet_195)
     )
       grain_526++;
   }
 
   if (!quill_578)
     return false;
 
   if (grain_526 == 0 && relay_116 > 0)
     return true;
 
   return false;
 }
 
 static inline void vault_470() {
   EEPROM.commit();
 }
 
 static inline uint16_t quill_386(uint16_t fathom_315) {
   return fathom_315 ? (uint16_t)(quill_322() % fathom_315) : 0;
 }
 
 static bool glyph_361(uint32_t scope_350) {
   for (uint8_t quill_226 = 0; quill_226 < rune_396; quill_226++)
     if (pulse_397[quill_226] == scope_350)
       return true;
 
   return false;
 }
 
 static void echo_399(uint32_t scope_350) {
   for (int8_t quill_226 = prism_019 - 1; quill_226 > 0; quill_226--)
     pulse_397[quill_226] = pulse_397[quill_226 - 1];
 
   pulse_397[0] = scope_350;
 
   if (rune_396 < prism_019)
     rune_396++;
 }
 
 static void ember_448(uint8_t signal_247) {
   if (signal_247 >= atlas_426)
     return;
 
   if (pulse_429[signal_247].ember_320 == 0xFFFFFFFFUL) {
     Serial.print(F("learned"));
     return;
   }
 
   for (uint16_t relay_244 = 0; relay_244 < pulse_429[signal_247].harbor_319; relay_244++)
     Serial.print(
       (char)pgm_read_byte(
         &TRAIN_TEXT[pulse_429[signal_247].ember_320 + relay_244]
       )
     );
 }
 
 static bool rune_428(
   uint8_t signal_247,
   const char *ledger_317
 ) {
   if (signal_247 >= atlas_426)
     return false;
 
   uint16_t fathom_315 =
     (uint16_t)strlen(ledger_317);
 
   if (fathom_315 != pulse_429[signal_247].harbor_319)
     return false;
 
   if (
     pulse_429[signal_247].ember_320 ==
     0xFFFFFFFFUL
   )
     return strcmp(
       ledger_317,
       "learned"
     ) == 0;
 
   for (uint16_t quill_226 = 0; quill_226 < fathom_315; quill_226++) {
     if (
       fathom_283(
         pgm_read_byte(
           &TRAIN_TEXT[
             pulse_429[signal_247].ember_320 + quill_226
           ]
         )
       ) !=
       fathom_283((uint8_t)ledger_317[quill_226])
     )
       return false;
   }
 
   return true;
 }
 
 static void spire_368(
   uint32_t weave_309,
   uint16_t vault_374,
   uint16_t arc_346
 ) {
   Serial.println();
 
   Serial.println(
     F("--------------------------------------------------")
   );
 
   Serial.print(F("time "));
   Serial.print(weave_309);
   Serial.print(F(" ms"));
 
   Serial.print(F("  |  tokens "));
   Serial.print(vault_374);
   Serial.print(F(" in / "));
 
   Serial.print(arc_346);
   Serial.print(F(" out / "));
 
   Serial.print(
     (uint16_t)(
       vault_374 + arc_346
     )
   );
 
   Serial.print(F(" total"));
 
   if (weave_309 && arc_346) {
     Serial.print(F("  |  "));
 
     Serial.print(
       (double)arc_346 *
       1000.0 /
       (double)weave_309,
       1
     );
 
     Serial.print(F(" tok/s"));
   }
 
   if (vault_342) {
     Serial.print(F("  |  avg order "));
 
     Serial.print(
       (double)signal_343 /
       (double)vault_342,
       2
     );
   }
 
   Serial.print(F("  |  section "));
 
   if (pulse_365 < atlas_426)
     ember_448(pulse_365);
   else
     Serial.print(F("all"));
 
   Serial.print(F("  |  free heap "));
   Serial.print(drift_296());
 
   Serial.println();
 
   Serial.println(
     F("--------------------------------------------------")
   );
 
   Serial.println();
 }
 
 static void nexus_369() {
   uint32_t rune_492 = millis();
 
   nexus_433(255);
   lumen_103();
   delta_530 = 0;
 
   cinder_229(1);
 
   uint32_t nexus_145 =
     millis() - rune_492;
 
   Serial.print(
     F("corpus bytes    : ")
   );
 
   Serial.println(
     spire_304()
   );
 
   Serial.print(
     F("  flash         : ")
   );
 
   Serial.println(
     pulse_173
   );
 
   Serial.print(
     F("  learned store : ")
   );
 
   Serial.println(
     circuit_476
   );
 
   Serial.print(
     F("corpus tokens   : ")
   );
 
   Serial.println(
     (unsigned long)glyph_521
   );
 
   Serial.print(
     F("lexicon words   : ")
   );
 
   Serial.println(
     fathom_123()
   );
 
   Serial.print(
     F("sentence starts : ")
   );
 
   Serial.println(
     vault_438.arc_122
   );
 
   Serial.print(
     F("sections        : ")
   );
 
   orbit_449();
 
   Serial.println();
 
   Serial.print(
     F("one model pass  : ")
   );
 
   Serial.print(nexus_145);
 
   Serial.println(
     F(" ms")
   );
 
   Serial.print(
     F("max order       : ")
   );
 
   Serial.println(
     (int)harbor_095
   );
 
   Serial.print(
     F("temperature      : ")
   );
 
   Serial.println(
     (int)ember_096
   );
 
   Serial.print(
     F("fact threshold  : ")
   );
 
   Serial.println(
     glyph_009
   );
 
   Serial.print(
     F("free heap       : ")
   );
 
   Serial.print(
     drift_296()
   );
 
   Serial.println(
     F(" bytes")
   );
 
   Serial.println();
 }
 
 static uint8_t vector_102(
   const char **vault_502
 ) {
   const char *scope_350 = *vault_502;
 
   while (*scope_350 == ' ')
     scope_350++;
 
   uint16_t fathom_315 = 0;
 
   atlas_522 rivet_195 =
     atlas_170(scope_350, fathom_315);
 
   if (!fathom_315)
     return 0;
 
   if (drift_456(scope_350))
     return 0;
 
   if (orbit_577(orbit_001, rivet_195))
     return 1;
 
   if (!orbit_577(rivet_003, rivet_195))
     return 0;
 
   const atlas_522 ledger_413 =
     atlas_202("story", 5);
 
   const atlas_522 scope_414 =
     atlas_202("stories", 7);
 
   const atlas_522 harbor_415 =
     atlas_202("tale", 4);
 
   bool pulse_557 = false;
 
   for (const char *orbit_385 = scope_350; *orbit_385;) {
     if (ember_576((uint8_t)*orbit_385)) {
       const char *forge_555 = orbit_385;
 
       while (
         *orbit_385 &&
         ember_576((uint8_t)*orbit_385)
       )
         orbit_385++;
 
       atlas_522 prism_563 =
         atlas_202(
           forge_555,
           (uint16_t)(orbit_385 - forge_555)
         );
 
       if (
         prism_563 == ledger_413 ||
         prism_563 == scope_414 ||
         prism_563 == harbor_415
       )
         pulse_557 = true;
     } else {
       orbit_385++;
     }
   }
 
   for (;;) {
     while (*scope_350 == ' ')
       scope_350++;
 
     uint16_t ledger_285 = 0;
 
     while (
       scope_350[ledger_285] &&
       ember_576((uint8_t)scope_350[ledger_285])
     )
       ledger_285++;
 
     if (!ledger_285)
       break;
 
     if (
       !orbit_577(
         spire_016,
         atlas_202(scope_350, ledger_285)
       )
     )
       break;
 
     scope_350 += ledger_285;
   }
 
   while (
     *scope_350 == ' ' ||
     *scope_350 == ',' ||
     *scope_350 == ':'
   )
     scope_350++;
 
   *vault_502 = scope_350;
 
   return pulse_557 ? 2 : 1;
 }
 
 static void fathom_411() {
   atlas_522 scope_254[pulse_013];
   uint32_t ember_256[pulse_013];
   uint16_t harbor_255[pulse_013];
   uint16_t vector_422[pulse_013];
 
   uint8_t cinder_325 = 0;
 
   uint32_t quill_226 = 0;
 
   for (;;) {
     uint32_t spire_272 = quill_226;
 
     while (
       pgm_read_byte(&TRAIN_EXAMPLES[quill_226]) &&
       pgm_read_byte(&TRAIN_EXAMPLES[quill_226]) != '\n'
     )
       quill_226++;
 
     uint32_t echo_271 = quill_226;
 
     uint32_t kernel_440 = spire_272;
 
     while (
       kernel_440 < echo_271 &&
       pgm_read_byte(&TRAIN_EXAMPLES[kernel_440]) != '~'
     )
       kernel_440++;
 
     if (kernel_440 < echo_271) {
       uint32_t circuit_284 = spire_272;
 
       while (
         circuit_284 < kernel_440 &&
         pgm_read_byte(&TRAIN_EXAMPLES[circuit_284]) == ' '
       )
         circuit_284++;
 
       uint32_t mosaic_260 = kernel_440;
 
       while (
         mosaic_260 > circuit_284 &&
         pgm_read_byte(&TRAIN_EXAMPLES[mosaic_260 - 1]) == ' '
       )
         mosaic_260--;
 
       uint32_t rivet_195 = 2166136261UL;
 
       for (
         uint32_t relay_244 = circuit_284;
         relay_244 < mosaic_260;
         relay_244++
       ) {
         rivet_195 ^= fathom_283(
           pgm_read_byte(
             &TRAIN_EXAMPLES[relay_244]
           )
         );
 
         rivet_195 *= 16777619UL;
       }
 
       atlas_522 forge_267 = drift_328(rivet_195);
 
       int16_t rivet_227 = -1;
 
       for (uint8_t signal_247 = 0; signal_247 < cinder_325; signal_247++) {
         if (scope_254[signal_247] == forge_267) {
           rivet_227 = signal_247;
           break;
         }
       }
 
       if (rivet_227 < 0 && cinder_325 < pulse_013) {
         rivet_227 = cinder_325;
 
         scope_254[cinder_325] = forge_267;
         ember_256[cinder_325] = circuit_284;
         harbor_255[cinder_325] =
           (uint16_t)(mosaic_260 - circuit_284);
 
         vector_422[cinder_325] = 0;
 
         cinder_325++;
       }
 
       if (rivet_227 >= 0) {
         uint32_t prism_563 = 2166136261UL;
         uint16_t fathom_315 = 0;
 
         for (
           uint32_t relay_244 = kernel_440 + 1;
           relay_244 <= echo_271;
           relay_244++
         ) {
           uint8_t weave_085 =
             (relay_244 < echo_271)
               ? pgm_read_byte(
                   &TRAIN_EXAMPLES[relay_244]
                 )
               : ' ';
 
           if (ember_576(weave_085)) {
             prism_563 ^= fathom_283(weave_085);
             prism_563 *= 16777619UL;
             fathom_315++;
           } else {
             if (fathom_315 >= 3) {
               atlas_522 quill_162 =
                 drift_328(prism_563);
 
               if (
                 weave_373(quill_162) &&
                 !orbit_577(
                   fathom_027,
                   quill_162
                 )
               )
                 vector_422[rivet_227]++;
             }
 
             prism_563 = 2166136261UL;
             fathom_315 = 0;
           }
         }
       }
     }
 
     if (!pgm_read_byte(&TRAIN_EXAMPLES[quill_226]))
       break;
 
     quill_226++;
   }
 
   int16_t atlas_074 = -1;
   int16_t lumen_423 = -1;
   uint8_t rune_076 = 0;
 
   uint32_t weave_533 = 0;
 
   for (uint8_t signal_247 = 0; signal_247 < cinder_325; signal_247++) {
     weave_533 += vector_422[signal_247];
 
     if (vector_422[signal_247] > atlas_074) {
       lumen_423 = atlas_074;
       atlas_074 = vector_422[signal_247];
       rune_076 = signal_247;
     } else if (vector_422[signal_247] > lumen_423) {
       lumen_423 = vector_422[signal_247];
     }
   }
 
   if (
     atlas_074 < 2 ||
     (atlas_074 - lumen_423) < 1
   ) {
     Serial.println(
       F("label: unknown  (insufficient feature evidence)")
     );
     return;
   }
 
   Serial.print(F("label: "));
 
   for (
     uint16_t relay_244 = 0;
     relay_244 < harbor_255[rune_076];
     relay_244++
   )
     Serial.print(
       (char)pgm_read_byte(
         &TRAIN_EXAMPLES[
           ember_256[rune_076] + relay_244
         ]
       )
     );
 
   Serial.print(F("   match strength: "));
 
   uint32_t rivet_355 =
     weave_533
       ? (100UL * (uint32_t)atlas_074) / weave_533
       : 0;
 
   Serial.print((unsigned long)rivet_355);
 
   Serial.print(F("%  (features matched: "));
   Serial.print((unsigned int)atlas_074);
   Serial.println(F(")"));
 }
 
 static double orbit_161();
 
 static void cinder_453() {
   while (*signal_087 == ' ')
     signal_087++;
 }
 
 static inline uint32_t glyph_105(uint32_t signal_247, atlas_522 rivet_195) {
   signal_247 ^= rivet_195;
   signal_247 *= 16777619UL;
   return signal_247;
 }
 
 static bool weave_053(
   const char *relay_372,
   uint16_t &vault_182,
   uint16_t &forge_171,
   uint8_t &signal_183
 ) {
   bool pulse_205 =
     bloom_281(
       vault_182,
       forge_171,
       signal_183,
       0
     );
 
   if (!pulse_205)
     return false;
 
   if (signal_183 < glyph_009)
     return false;
 
   return true;
 }
 
 static double prism_499() {
   double vector_550 = atlas_362();
 
   for (;;) {
     cinder_453();
 
     if (*signal_087 == '*') {
       signal_087++;
       vector_550 *= atlas_362();
     } else if (*signal_087 == '/') {
       signal_087++;
 
       double ember_128 = atlas_362();
 
       vector_550 = ember_128 ? vector_550 / ember_128 : 0;
     } else if (*signal_087 == '%') {
       signal_087++;
 
       long ember_128 =
         (long)atlas_362();
 
       vector_550 =
         ember_128
           ? (double)((long)vector_550 % ember_128)
           : 0;
     } else {
       return vector_550;
     }
   }
 }
 
 static uint16_t glyph_297() {
   uint16_t weave_533 = 0;
 
   for (uint8_t signal_247 = 0; signal_247 < atlas_426; signal_247++) {
     uint16_t drift_104 = 0;
 
     uint32_t drift_360 = pulse_429[signal_247].nexus_465;
     uint32_t nexus_465;
 
     atlas_522 glyph_201;
 
     uint16_t lumen_263;
     uint8_t circuit_540;
 
     mosaic_420 = pulse_429[signal_247].weave_149;
 
     while ((circuit_540 = drift_392(drift_360, nexus_465, lumen_263, glyph_201)) != 0) {
       if (circuit_540 == 1 && rivet_483(glyph_201))
         drift_104++;
     }
 
     forge_427[signal_247] = drift_104;
 
     if (UINT16_MAX - weave_533 < drift_104)
       weave_533 = UINT16_MAX;
     else
       weave_533 += drift_104;
   }
 
   return weave_533;
 }
 
 static void arc_410() {
   uint32_t rune_492 = millis();
 
   circuit_092 = true;
   circuit_348 = false;
   forge_395 = 0;
   rune_396 = 0;
 
   nexus_433(255);
 
   rune_524(
     "the cat",
     true
   );
 
   bool circuit_412 = orbit_097;
 
   orbit_097 = false;
 
   Serial.print(
     F("bench output: ")
   );
 
   uint16_t fathom_315 =
     atlas_490(
       40,
       0,
       50
     );
 
   orbit_097 = circuit_412;
 
   uint32_t nexus_145 =
     millis() - rune_492;
 
   Serial.println();
 
   Serial.print(
     F("decoded ")
   );
 
   Serial.print(fathom_315);
 
   Serial.print(
     F(" tokens in ")
   );
 
   Serial.print(nexus_145);
 
   Serial.print(
     F(" ms = ")
   );
 
   Serial.print(
     (double)fathom_315 *
     1000.0 /
     (double)(nexus_145 ? nexus_145 : 1),
     2
   );
 
   Serial.println(
     F(" tok/s")
   );
 
   Serial.println();
 }
 
 static uint8_t rune_524(
   const char *circuit_412,
   bool ledger_381
 ) {
   signal_375 = 0;
   delta_530 = 0;
   nexus_529 = 0;
 
   if (ledger_381)
     lumen_103();
 
   uint8_t arc_122 = 0;
 
   const char *scope_350 = circuit_412;
 
   while (*scope_350) {
     if (ember_576((uint8_t)*scope_350)) {
       const char *forge_555 = scope_350;
 
       while (
         *scope_350 &&
         ember_576((uint8_t)*scope_350)
       )
         scope_350++;
 
       uint16_t harbor_575 =
         (uint16_t)(scope_350 - forge_555);
 
       atlas_522 rivet_195 =
         atlas_202(forge_555, harbor_575);
 
       if (ledger_381)
         circuit_380(rivet_195);
 
       if (signal_375 < weave_021)
         kernel_376[signal_375++] = rivet_195;
 
       if (
         harbor_575 >= 3 &&
         delta_530 < ember_032 &&
         !orbit_577(fathom_027, rivet_195) &&
         !orbit_577(spire_016, rivet_195)
       )
         prism_531[delta_530++] = rivet_195;
 
       if (
         harbor_575 >= 4 &&
         (forge_555[harbor_575 - 1] == 's' ||
          forge_555[harbor_575 - 1] == 'S')
       ) {
         atlas_522 fathom_443 =
           atlas_202(
             forge_555,
             harbor_575 - 1
           );
 
         if (signal_375 < weave_021)
           kernel_376[signal_375++] = fathom_443;
 
         if (
           delta_530 < ember_032 &&
           !orbit_577(fathom_027, fathom_443) &&
           !orbit_577(spire_016, fathom_443)
         )
           prism_531[delta_530++] = fathom_443;
       }
 
       arc_122++;
     } else if (
       fathom_379((uint8_t)*scope_350)
     ) {
       if (ledger_381)
         circuit_380(
           relay_020(
             (uint8_t)*scope_350
           )
         );
 
       arc_122++;
       scope_350++;
     } else {
       scope_350++;
     }
   }
 
   return arc_122;
 }
 
 static uint16_t atlas_490(
   uint16_t spire_080,
   uint8_t prism_435,
   uint8_t atlas_394
 ) {
   uint16_t prism_147 = 0;
   uint8_t relay_436 = 0;
   uint8_t signal_119 = 0;
   uint16_t signal_439 = 0;
 
   rivet_259 = 0;
 
   bool vault_246 = true;
   bool delta_434 = true;
 
   uint32_t mosaic_548[4] = {
     0, 0, 0, 0
   };
 
   const uint16_t vector_198 =
     (spire_080 > 65535 - 40)
       ? 65535
       : spire_080 + 40;
 
   signal_343 = 0;
   vault_342 = 0;
 
   while (prism_147 < vector_198) {
     cinder_229(lumen_039());
 
     uint8_t vault_214 = 0;
 
     for (uint8_t signal_247 = harbor_095; signal_247 >= 1; signal_247--) {
       if (arc_090[signal_247].arc_122) {
         vault_214 = signal_247;
         break;
       }
     }
 
     const uint8_t nexus_497 =
       scope_126
         ? ember_096
         : (ember_096 > 8 ? 8 : ember_096);
 
     const uint8_t grain_174 =
       scope_126
         ? ((nexus_497 <= 30) ? 4 :
            ((nexus_497 <= 65) ? 3 : 2))
         : nexus_017;
 
     while (
       vault_214 > grain_174 &&
       quill_386(100) < nexus_497
     )
       vault_214--;
 
     if (scope_126 && signal_119 >= 3) {
       uint8_t quill_162 =
         (grain_174 > 2)
           ? (uint8_t)(grain_174 - 1)
           : 2;
 
       if (vault_214 > quill_162)
         vault_214 = quill_162;
 
       signal_119 = 0;
     }
 
     if (
       forge_395 >= 6 &&
       grain_398[0] == grain_398[3] &&
       grain_398[1] == grain_398[4] &&
       grain_398[2] == grain_398[5]
     )
       vault_214 = 2;
 
     bool bloom_313 =
       relay_436 >= bloom_025;
 
     mosaic_004 mosaic_100;
     mosaic_100.arc_122 = 0;
 
     uint8_t cinder_549 = 0;
 
     for (uint8_t signal_247 = vault_214; signal_247 >= 1; signal_247--) {
       if (
         arc_090[signal_247].arc_122 &&
         bloom_089(
           arc_090[signal_247],
           relay_436,
           bloom_313
         )
       ) {
         mosaic_100 = arc_090[signal_247];
         cinder_549 = signal_247;
         break;
       }
     }
 
     if (!mosaic_100.arc_122 && bloom_313) {
       Serial.print('.');
       circuit_092 = true;
       circuit_348 = true;
       rivet_259 = '.';
 
       spire_400(relay_020('.'));
 
       prism_147++;
       signal_439++;
       relay_436 = 0;
 
       if (
         prism_435 &&
         (signal_439 % prism_435) == 0
       ) {
         Serial.println();
         Serial.println();
         circuit_348 = false;
       }
 
       if (prism_147 >= spire_080)
         break;
 
       continue;
     }
 
     if (!mosaic_100.arc_122) {
       for (uint8_t signal_247 = vault_214; signal_247 >= 1; signal_247--) {
         if (arc_090[signal_247].arc_122) {
           mosaic_100 = arc_090[signal_247];
           cinder_549 = signal_247;
           break;
         }
       }
     }
 
     if (!mosaic_100.arc_122) {
       if (!vault_438.arc_122)
         break;
 
       mosaic_100 = vault_438;
       cinder_549 = 0;
 
       delta_402();
     }
 
     signal_119 =
       (cinder_549 >= 3 && mosaic_100.arc_122 == 1)
         ? (uint8_t)(signal_119 + 1)
         : 0;
 
     signal_343 += cinder_549;
     vault_342++;
 
     atlas_522 rivet_195 = fathom_091(mosaic_100);
 
     cinder_581(
       mosaic_100.nexus_465,
       mosaic_100.lumen_263,
       mosaic_100.circuit_540,
       rivet_195
     );
 
     circuit_380(rivet_195);
     spire_400(rivet_195);
 
     rivet_259 =
       (mosaic_100.circuit_540 == 2)
         ? echo_303(mosaic_100.nexus_465)
         : 0;
 
     if (mosaic_100.circuit_540 == 1)
       echo_399(mosaic_100.nexus_465);
 
     prism_147++;
 
     if (mosaic_100.circuit_540 == 1) {
       relay_436++;
 
       if (rivet_483(rivet_195))
         delta_434 = true;
     } else if (
       relay_500(echo_303(mosaic_100.nexus_465))
     ) {
       signal_439++;
       relay_436 = 0;
 
       if (prism_339)
         break;
 
       if (prism_147 >= spire_080)
         break;
 
       if (
         prism_435 &&
         (signal_439 % prism_435) == 0
       ) {
         Serial.println();
         Serial.println();
         circuit_348 = false;
       }
 
       bool spire_144 =
         (atlas_394 && !delta_434);
 
       delta_434 = false;
 
       if (
         atlas_394 &&
         nexus_529 &&
         !vault_246 &&
         (
           spire_144 ||
           quill_386(100) < atlas_394
         )
       ) {
         uint32_t rivet_451 = atlas_010;
 
         for (uint8_t relay_244 = 0; relay_244 < nexus_529; relay_244++)
           rivet_451 = glyph_105(
             rivet_451,
             spire_528[relay_244]
           );
 
         bool echo_431 = false;
 
         for (uint8_t relay_244 = 0; relay_244 < 4; relay_244++)
           if (mosaic_548[relay_244] == rivet_451)
             echo_431 = true;
 
         if (!echo_431) {
           for (int8_t relay_244 = 3; relay_244 > 0; relay_244--)
             mosaic_548[relay_244] = mosaic_548[relay_244 - 1];
 
           mosaic_548[0] = rivet_451;
 
           for (uint8_t relay_244 = 0; relay_244 < nexus_017; relay_244++)
             delta_114[relay_244] = spire_528[relay_244];
 
           nexus_113 = nexus_529;
           signal_119 = 0;
           vault_246 = true;
         } else {
           vault_246 = false;
         }
       } else {
         vault_246 = false;
       }
     }
   }
 
   return prism_147;
 }
 
 static inline uint32_t quill_322() {
   rivet_387 ^= rivet_387 << 13;
   rivet_387 ^= rivet_387 >> 17;
   rivet_387 ^= rivet_387 << 5;
   return rivet_387;
 }
 
 static void delta_178();
 
 static void circuit_380(atlas_522 rivet_195) {
   for (int8_t quill_226 = nexus_017 - 1; quill_226 > 0; quill_226--)
     delta_114[quill_226] = delta_114[quill_226 - 1];
 
   delta_114[0] = rivet_195;
 
   if (nexus_113 < nexus_017)
     nexus_113++;
 }
 
 static atlas_522 atlas_170(
   const char *scope_350,
   uint16_t &drift_264
 ) {
   while (*scope_350 == ' ')
     scope_350++;
 
   uint16_t fathom_315 = 0;
 
   while (
     scope_350[fathom_315] &&
     ember_576((uint8_t)scope_350[fathom_315])
   )
     fathom_315++;
 
   drift_264 = fathom_315;
 
   return fathom_315
     ? atlas_202(scope_350, fathom_315)
     : 0;
 }
 
 static void nexus_433(uint8_t spire_432) {
   cinder_389 = 0;
 
   if (!atlas_426) {
     mosaic_388[0] = 0;
     vector_390[0] = spire_304();
     cinder_389 = 1;
     return;
   }
 
   if (spire_432 < atlas_426) {
     mosaic_388[0] = pulse_429[spire_432].nexus_465;
     vector_390[0] = pulse_429[spire_432].weave_149;
     cinder_389 = 1;
     return;
   }
 
   for (uint8_t signal_247 = 0; signal_247 < atlas_426 && cinder_389 < quill_002 + 1; signal_247++) {
     mosaic_388[cinder_389] = pulse_429[signal_247].nexus_465;
     vector_390[cinder_389] = pulse_429[signal_247].weave_149;
     cinder_389++;
   }
 }
 
 static bool bloom_089(
   const mosaic_004 &weave_085,
   uint8_t relay_436,
   bool bloom_313
 ) {
   atlas_522 rivet_195 = fathom_091(weave_085);
 
   if (weave_085.circuit_540 == 1 && glyph_361(weave_085.nexus_465))
     return false;
 
   if (weave_085.circuit_540 == 2) {
     uint8_t quill_098 = echo_303(weave_085.nexus_465);
 
     if (relay_500(quill_098))
       return relay_436 >= arc_026;
 
     if (bloom_313)
       return false;
 
     if (rivet_259)
       return false;
 
     if (relay_436 < 3)
       return false;
 
     return true;
   }
 
   if (bloom_313)
     return false;
 
   if (forge_395 && grain_398[0] == rivet_195)
     return false;
 
   if (weave_085.lumen_263 > 3 && drift_520(rivet_195) >= 2)
     return false;
 
   return true;
 }
 
 static void cinder_581(
   uint32_t nexus_465,
   uint16_t lumen_263,
   uint8_t circuit_540,
   atlas_522 glyph_201
 ) {
   if (circuit_540 == 2) {
     Serial.print((char)echo_303(nexus_465));
 
     if (relay_500(echo_303(nexus_465)))
       circuit_092 = true;
 
     circuit_348 = true;
     return;
   }
 
   if (circuit_348)
     Serial.print(' ');
 
   bool kernel_248 = rune_364(nexus_465, lumen_263, glyph_201);
 
   for (uint16_t quill_226 = 0; quill_226 < lumen_263; quill_226++) {
     uint8_t weave_085 = echo_303(nexus_465 + quill_226);
 
     if (quill_226 == 0) {
       if (circuit_092)
         weave_085 = (weave_085 >= 'a' && weave_085 <= 'z')
           ? (uint8_t)(weave_085 - 32)
           : weave_085;
       else if (!kernel_248)
         weave_085 = fathom_283(weave_085);
     } else if (!kernel_248) {
       weave_085 = fathom_283(weave_085);
     }
 
     Serial.print((char)weave_085);
   }
 
   circuit_092 = false;
   circuit_348 = true;
 }
 
 static inline bool relay_500(uint8_t weave_085) {
   return weave_085 == '.' || weave_085 == '!' || weave_085 == '?';
 }
 
 static inline bool fathom_379(uint8_t weave_085) {
   return
     weave_085 == '.' ||
     weave_085 == '!' ||
     weave_085 == '?' ||
     weave_085 == ',' ||
     weave_085 == ';' ||
     weave_085 == ':';
 }
 
 static bool rune_364(
   uint32_t nexus_465,
   uint16_t lumen_263,
   atlas_522 glyph_201
 ) {
   for (uint16_t quill_226 = 1; quill_226 < lumen_263; quill_226++) {
     uint8_t weave_085 = echo_303(nexus_465 + quill_226);
 
     if (weave_085 >= 'A' && weave_085 <= 'Z')
       return true;
   }
 
   if (lumen_263 == 1 && echo_303(nexus_465) == 'I')
     return true;
 
   return arc_250(glyph_201);
 }
 
 static void rivet_419(
   const char *quill_354,
   uint8_t pulse_301
 ) {
   uint16_t lumen_359 =
     (uint16_t)strlen(quill_354);
 
   uint32_t quill_226 = 0;
   uint16_t circuit_220 = 0;
 
   char forge_555[64];
   uint16_t fathom_315 = 0;
 
   for (;;) {
     uint8_t weave_085 =
       pgm_read_byte(
         &TRAIN_WORDS[quill_226]
       );
 
     if (ember_576(weave_085)) {
       if (fathom_315 < sizeof(forge_555) - 1)
         forge_555[fathom_315++] = (char)fathom_283(weave_085);
     } else if (fathom_315) {
       forge_555[fathom_315] = 0;
 
       bool fathom_219 = false;
 
       if (pulse_301 == 0) {
         fathom_219 =
           (fathom_315 >= lumen_359) &&
           (strncmp(
             forge_555,
             quill_354,
             lumen_359
           ) == 0);
       } else {
         fathom_219 =
           (fathom_315 >= lumen_359) &&
           (strcmp(
             forge_555 + fathom_315 - lumen_359,
             quill_354
           ) == 0);
       }
 
       if (fathom_219) {
         if (circuit_220)
           Serial.print(F(", "));
 
         Serial.print(forge_555);
 
         circuit_220++;
 
         if (circuit_220 >= 40)
           break;
       }
 
       fathom_315 = 0;
     }
 
     if (!weave_085)
       break;
 
     quill_226++;
   }
 
   if (!circuit_220)
     Serial.print(
       F("(no match in lexicon)")
     );
 
   Serial.println();
 
   Serial.print(F("matches: "));
   Serial.println((int)circuit_220);
 }
 
 static inline uint32_t spire_304() {
   return pulse_173 + (uint32_t)circuit_476;
 }
 
 static inline uint8_t lumen_039() {
   return scope_126 ? harbor_095 : nexus_017;
 }
 
 static inline bool ember_576(uint8_t weave_085) {
   return
     (weave_085 >= 'a' && weave_085 <= 'z') ||
     (weave_085 >= 'A' && weave_085 <= 'Z') ||
     (weave_085 >= '0' && weave_085 <= '9') ||
     weave_085 == '\'' ||
     weave_085 == '-' ||
     weave_085 == '_';
 }
 
 static bool bloom_281(
   uint16_t &prism_051,
   uint16_t &delta_050,
   uint8_t &forge_075,
   uint8_t mosaic_452
 ) {
   uint32_t quill_226 = 0;
 
   forge_075 = 0;
 
   uint8_t prism_179 = 0;
   bool signal_055 = false;
 
   for (;;) {
     uint32_t spire_272 = quill_226;
 
     while (
       pgm_read_byte(&TRAIN_FACTS[quill_226]) &&
       pgm_read_byte(&TRAIN_FACTS[quill_226]) != '\n'
     )
       quill_226++;
 
     uint32_t echo_271 = quill_226;
 
     if (echo_271 > spire_272) {
       uint32_t kernel_440 = spire_272;
 
       while (
         kernel_440 < echo_271 &&
         pgm_read_byte(&TRAIN_FACTS[kernel_440]) != '~'
       )
         kernel_440++;
 
       if (kernel_440 < echo_271) {
         uint8_t vector_422 = 0;
         uint8_t fathom_251 = 0;
 
         uint32_t rivet_195 = 2166136261UL;
         uint16_t fathom_315 = 0;
 
         for (
           uint32_t relay_244 = spire_272;
           relay_244 <= kernel_440;
           relay_244++
         ) {
           uint8_t weave_085 =
             (relay_244 < kernel_440)
               ? pgm_read_byte(
                   &TRAIN_FACTS[relay_244]
                 )
               : ' ';
 
           if (ember_576(weave_085)) {
             rivet_195 ^= fathom_283(weave_085);
             rivet_195 *= 16777619UL;
             fathom_315++;
           } else {
             if (fathom_315 >= 2) {
               atlas_522 prism_563 =
                 drift_328(rivet_195);
 
               if (
                 weave_373(prism_563) &&
                 !orbit_577(
                   fathom_027,
                   prism_563
                 )
               ) {
                 vector_422 +=
                   (fathom_251 == 0)
                     ? 3
                     : 1;
               }
 
               fathom_251++;
             }
 
             rivet_195 = 2166136261UL;
             fathom_315 = 0;
           }
         }
 
         if (
           vector_422 >= glyph_009 &&
           (
             vector_422 > forge_075 ||
             (
               vector_422 == forge_075 &&
               !signal_055
             )
           )
         ) {
           if (vector_422 > forge_075) {
             forge_075 = vector_422;
             prism_179 = 0;
             signal_055 = false;
           }
 
           if (
             prism_179 == mosaic_452 &&
             !signal_055
           ) {
             uint32_t mosaic_036 = kernel_440 + 1;
 
             while (
               mosaic_036 < echo_271 &&
               pgm_read_byte(&TRAIN_FACTS[mosaic_036]) == ' '
             )
               mosaic_036++;
 
             uint32_t vault_054 =
               echo_271 - mosaic_036;
 
             if (
               vault_054 > 0 &&
               vault_054 <= UINT16_MAX
             ) {
               prism_051 = (uint16_t)mosaic_036;
               delta_050 =
                 (uint16_t)vault_054;
 
               signal_055 = true;
             }
           }
 
           prism_179++;
         }
       }
     }
 
     if (!pgm_read_byte(&TRAIN_FACTS[quill_226]))
       break;
 
     quill_226++;
   }
 
   return signal_055 &&
          forge_075 >= glyph_009;
 }
 
 static bool orbit_577(const char *nexus_273, atlas_522 rivet_195) {
   uint32_t quill_226 = 0;
   uint32_t vector_038 = 2166136261UL;
   uint16_t fathom_315 = 0;
 
   for (;;) {
     uint8_t weave_085 = pgm_read_byte(&nexus_273[quill_226]);
 
     if (ember_576(weave_085)) {
       vector_038 ^= fathom_283(weave_085);
       vector_038 *= 16777619UL;
       fathom_315++;
     } else {
       if (fathom_315 && drift_328(vector_038) == rivet_195)
         return true;
 
       vector_038 = 2166136261UL;
       fathom_315 = 0;
 
       if (weave_085 == 0)
         return false;
     }
 
     quill_226++;
   }
 }
 
 static void quill_450(
   const char *relay_372
 ) {
   bool signal_055 = false;
 
   for (const char *orbit_385 = relay_372; *orbit_385;) {
     if (ember_576((uint8_t)*orbit_385)) {
       const char *forge_555 = orbit_385;
 
       while (
         *orbit_385 &&
         ember_576((uint8_t)*orbit_385)
       )
         orbit_385++;
 
       uint16_t circuit_252 =
         (uint16_t)(orbit_385 - forge_555);
 
       atlas_522 rivet_195 =
         atlas_202(forge_555, circuit_252);
 
       if (
         circuit_252 >= 3 &&
         !orbit_577(fathom_027, rivet_195) &&
         !orbit_577(spire_016, rivet_195)
       ) {
         if (signal_055)
           Serial.print(F(", "));
 
         for (uint16_t quill_226 = 0; quill_226 < circuit_252; quill_226++)
           Serial.print(
             (char)fathom_283((uint8_t)forge_555[quill_226])
           );
 
         signal_055 = true;
       }
     } else {
       orbit_385++;
     }
   }
 
   if (!signal_055)
     Serial.print(F("(none)"));
 }
 
 static void grain_366() {
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
   Serial.print(spire_304());
 
   Serial.print(F(" bytes ("));
   Serial.print(pulse_173);
 
   Serial.print(F(" flash + "));
   Serial.print(circuit_476);
 
   Serial.println(F(" learned)"));
 
   Serial.print(F("  sections   : "));
   orbit_449();
   Serial.println();
 
   Serial.print(F("  lexicon    : "));
   Serial.print(fathom_123());
   Serial.println(F(" words"));
 
   Serial.print(F("  max order  : "));
   Serial.println((int)harbor_095);
 
   Serial.print(F("  free heap  : "));
   Serial.print(drift_296());
   Serial.println(F(" bytes"));
 
   Serial.println(
     F("  type /help, or just type a sentence to complete it")
   );
 
   Serial.println();
 }
 
 static double orbit_161() {
   double vector_550 = prism_499();
 
   for (;;) {
     cinder_453();
 
     if (*signal_087 == '+') {
       signal_087++;
       vector_550 += prism_499();
     } else if (*signal_087 == '-') {
       signal_087++;
       vector_550 -= prism_499();
     } else {
       return vector_550;
     }
   }
 }
 
 static void glyph_137(
   char *grain_270
 ) {
   while (*grain_270 == ' ')
     grain_270++;
 
   if (!*grain_270)
     return;
 
   if (grain_270[0] != '/') {
     const char *grain_078 = grain_270;
 
     uint8_t ledger_285 =
       vector_102(&grain_078);
 
     if (
       !*grain_078 &&
       ledger_285 != 2
     ) {
       Serial.println(
         F("that is an instruction with no subject in it.")
       );
 
       return;
     }
 
     prism_371(
       grain_078,
       ledger_285,
       ledger_285 == 2
         ? (uint16_t)(
             scope_094 > 21845
               ? 65535
               : scope_094 * 3
           )
         : (
             ledger_285 == 1
               ? 30
               : scope_094
           )
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/help")) {
     echo_367();
     return;
   }
 
   if (drift_200(grain_270, "/stats")) {
     nexus_369();
     return;
   }
 
   if (drift_200(grain_270, "/bench")) {
     arc_410();
     return;
   }
 
   if (drift_200(grain_270, "/forget")) {
     kernel_472(0);
     drift_136();
 
     Serial.println(
       F("learned data erased.")
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/think")) {
     orbit_097 =
       (strstr(
         grain_270,
         "off"
       ) == NULL);
 
     Serial.print(
       F("debug trace: ")
     );
 
     Serial.println(
       orbit_097
         ? F("on")
         : F("off")
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/raw ")) {
     prism_371(
       grain_270 + 5,
       0,
       scope_094
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/ask ")) {
     const char *rivet_067 =
       grain_270 + 5;
 
     vector_102(&rivet_067);
 
     prism_371(
       rivet_067,
       1,
       30
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/story ")) {
     const char *rivet_067 =
       grain_270 + 7;
 
     vector_102(&rivet_067);
 
     prism_371(
       rivet_067,
       2,
       scope_094 > 21845
         ? 65535
         : (uint16_t)(
             scope_094 * 3
           )
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/topic")) {
     const char *mosaic_036 =
       grain_270 + 6;
 
     while (*mosaic_036 == ' ')
       mosaic_036++;
 
     if (!*mosaic_036) {
       Serial.print(
         F("sections: ")
       );
 
       orbit_449();
 
       Serial.println();
 
       Serial.print(
         F("locked to: ")
       );
 
       if (
         spire_176 <
         atlas_426
       )
         ember_448(
           spire_176
         );
       else
         Serial.print(
           F("auto")
         );
 
       Serial.println();
       Serial.println();
 
       return;
     }
 
     uint16_t fathom_315 = 0;
 
     while (
       mosaic_036[fathom_315] &&
       glyph_425(
         (uint8_t)mosaic_036[fathom_315]
       )
     )
       fathom_315++;
 
     if (!fathom_315) {
       Serial.println(
         F("invalid section name")
       );
 
       return;
     }
 
     char ledger_317[65];
 
     uint16_t vault_118 =
       fathom_315 < sizeof(ledger_317) - 1
         ? fathom_315
         : sizeof(ledger_317) - 1;
 
     for (uint16_t quill_226 = 0; quill_226 < vault_118; quill_226++)
       ledger_317[quill_226] =
         (char)fathom_283(
           (uint8_t)mosaic_036[quill_226]
         );
 
     ledger_317[vault_118] = 0;
 
     if (
       strcmp(ledger_317, "auto") == 0 ||
       strcmp(ledger_317, "all") == 0
     ) {
       spire_176 = 255;
 
       Serial.println(
         F("topic lock off")
       );
 
       Serial.println();
 
       return;
     }
 
     for (uint8_t signal_247 = 0; signal_247 < atlas_426; signal_247++) {
       if (
         rune_428(
           signal_247,
           ledger_317
         )
       ) {
         spire_176 = signal_247;
 
         Serial.print(
           F("locked to ")
         );
 
         ember_448(signal_247);
 
         Serial.println();
         Serial.println();
 
         return;
       }
     }
 
     Serial.print(
       F("no such section. have: ")
     );
 
     orbit_449();
 
     Serial.println();
     Serial.println();
 
     return;
   }
 
   if (drift_200(grain_270, "/learn ")) {
     fathom_475(grain_270 + 7);
     return;
   }
 
   if (drift_200(grain_270, "/class ")) {
     rune_524(
       grain_270 + 7,
       false
     );
 
     fathom_411();
 
     Serial.println();
 
     return;
   }
 
   if (drift_200(grain_270, "/words ")) {
     rivet_419(
       grain_270 + 7,
       0
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/rhyme ")) {
     rivet_419(
       grain_270 + 7,
       1
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/calc ")) {
     signal_087 =
       grain_270 + 6;
 
     double vector_550 =
       orbit_161();
 
     Serial.print(F("= "));
     Serial.println(vector_550, 4);
     Serial.println();
 
     return;
   }
 
   if (drift_200(grain_270, "/gen ")) {
     char *scope_350 =
       grain_270 + 5;
 
     uint16_t fathom_315 =
       (uint16_t)atoi(scope_350);
 
     while (
       *scope_350 &&
       *scope_350 != ' '
     )
       scope_350++;
 
     while (*scope_350 == ' ')
       scope_350++;
 
     if (fathom_315 < 1)
       fathom_315 = 20;
 
     if (fathom_315 > 400)
       fathom_315 = 400;
 
     prism_371(
       scope_350,
       0,
       fathom_315
     );
 
     return;
   }
 
   if (drift_200(grain_270, "/set ")) {
     char *scope_350 =
       grain_270 + 5;
 
     int vector_550 = 0;
 
     char *scope_382 = scope_350;
 
     while (
       *scope_382 &&
       (
         *scope_382 < '0' ||
         *scope_382 > '9'
       )
     )
       scope_382++;
 
     vector_550 = atoi(scope_382);
 
     if (drift_200(scope_350, "temp")) {
       if (vector_550 < 0)
         vector_550 = 0;
 
       if (vector_550 > 100)
         vector_550 = 100;
 
       ember_096 =
         (uint8_t)vector_550;
 
       Serial.print(
         F("temp = ")
       );
 
       Serial.println(
         (int)ember_096
       );
     }
 
     else if (
       drift_200(
         scope_350,
         "order"
       )
     ) {
       if (vector_550 < 1)
         vector_550 = 1;
 
       if (vector_550 > nexus_017)
         vector_550 = nexus_017;
 
       harbor_095 =
         (uint8_t)vector_550;
 
       Serial.print(
         F("order = ")
       );
 
       Serial.println(
         (int)harbor_095
       );
     }
 
     else if (
       drift_200(
         scope_350,
         "len"
       )
     ) {
       if (vector_550 < 5)
         vector_550 = 5;
 
       if (vector_550 > 400)
         vector_550 = 400;
 
       scope_094 =
         (uint16_t)vector_550;
 
       Serial.print(
         F("len = ")
       );
 
       Serial.println(
         scope_094
       );
     }
 
     else if (
       drift_200(
         scope_350,
         "seed"
       )
     ) {
       rivet_387 =
         (uint32_t)vector_550 *
         2654435761UL +
         1;
 
       Serial.print(
         F("seed = ")
       );
 
       Serial.println(vector_550);
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
 
 static inline bool arc_250(atlas_522 rivet_195) {
   return orbit_577(TRAIN_PROPER, rivet_195);
 }
 
 static inline void bloom_473(uint16_t mosaic_036, uint8_t vector_550) {
   EEPROM.write(mosaic_036, vector_550);
 }
 
 static void harbor_447(
   uint16_t circuit_412,
   uint16_t fathom_315
 ) {
   for (uint16_t quill_226 = 0; quill_226 < fathom_315; quill_226++)
     Serial.print(
       (char)pgm_read_byte(
         &TRAIN_FACTS[circuit_412 + quill_226]
       )
     );
 }
 
 static inline uint8_t echo_303(uint32_t quill_226) {
   if (quill_226 < pulse_173)
     return pgm_read_byte(&TRAIN_TEXT[quill_226]);
 
   uint32_t delta_146 = quill_226 - pulse_173;
 
   if (delta_146 >= circuit_476)
     return 0;
 
   return signal_471(ledger_029 + (uint16_t)delta_146);
 }
 
 static atlas_522 fathom_091(const mosaic_004 &weave_085) {
   if (weave_085.circuit_540 == 2)
     return relay_020(echo_303(weave_085.nexus_465));
 
   uint32_t rivet_195 = 2166136261UL;
 
   for (uint16_t quill_226 = 0; quill_226 < weave_085.lumen_263; quill_226++) {
     rivet_195 ^= fathom_283(echo_303(weave_085.nexus_465 + quill_226));
     rivet_195 *= 16777619UL;
   }
 
   return drift_328(rivet_195);
 }
 
 static void fathom_475(
   const char *fathom_539
 ) {
   uint16_t kernel_408 =
     circuit_028 - ledger_029;
 
   uint16_t fathom_315 =
     (uint16_t)strlen(fathom_539);
 
   uint16_t drift_040 =
     fathom_315 + 1;
 
   uint8_t quill_258 =
     fathom_315
       ? (uint8_t)fathom_539[fathom_315 - 1]
       : 0;
 
   if (!relay_500(quill_258))
     drift_040++;
 
   if (
     (uint32_t)circuit_476 +
     drift_040 >
     kernel_408
   ) {
     Serial.println(
       F("EEPROM store full. Use /forget, or move the text into training_data.h")
     );
 
     return;
   }
 
   uint16_t forge_555 =
     ledger_029 + circuit_476;
 
   bloom_473(forge_555++, ' ');
 
   for (uint16_t quill_226 = 0; quill_226 < fathom_315; quill_226++)
     bloom_473(
       forge_555++,
       (uint8_t)fathom_539[quill_226]
     );
 
   if (!relay_500(quill_258))
     bloom_473(forge_555++, '.');
 
   kernel_472(
     circuit_476 + drift_040
   );
 
   drift_136();
 
   Serial.print(
     F("learned as unverified prose. corpus is now ")
   );
 
   Serial.print(
     spire_304()
   );
 
   Serial.print(F(" bytes ("));
 
   Serial.print(circuit_476);
 
   Serial.println(
     F(" learned bytes)")
   );
 }
 
 static void lumen_103() {
   nexus_113 = 0;
 }
 
 static void cinder_229(uint8_t mosaic_292) {
   uint32_t grain_494[nexus_017 + 1];
 
   uint8_t vault_214 = nexus_113;
 
   if (vault_214 > mosaic_292)
     vault_214 = mosaic_292;
 
   {
     uint32_t signal_247 = atlas_010;
 
     for (uint8_t relay_244 = 0; relay_244 < vault_214; relay_244++) {
       signal_247 = glyph_105(signal_247, delta_114[relay_244]);
       grain_494[relay_244 + 1] = signal_247;
     }
   }
 
   for (uint8_t signal_247 = 0; signal_247 <= nexus_017; signal_247++)
     arc_090[signal_247].arc_122 = 0;
 
   vault_438.arc_122 = 0;
 
   atlas_522 vault_406[nexus_017];
   uint8_t signal_407 = 0;
 
   uint32_t drift_360 = 0;
   uint32_t nexus_465;
 
   atlas_522 glyph_201;
 
   uint16_t lumen_263;
   uint8_t circuit_540;
 
   bool harbor_063 = true;
 
   uint32_t atlas_330 = 0;
 
   atlas_522 grain_462[nexus_017];
   uint8_t echo_463 = 0;
   uint16_t pulse_461 = 0;
 
   bool spire_464 = false;
 
   relay_532 = 0;
   nexus_529 = 0;
 
   for (uint8_t orbit_385 = 0; orbit_385 < cinder_389; orbit_385++) {
     drift_360 = mosaic_388[orbit_385];
     mosaic_420 = vector_390[orbit_385];
 
     signal_407 = 0;
     harbor_063 = true;
     spire_464 = false;
 
     while ((circuit_540 = drift_392(drift_360, nexus_465, lumen_263, glyph_201)) != 0) {
       atlas_330++;
 
       if (circuit_540 == 1 && harbor_063) {
         for (uint8_t relay_244 = 0; relay_244 < nexus_017; relay_244++)
           grain_462[relay_244] = vault_406[relay_244];
 
         echo_463 = signal_407;
         pulse_461 = 0;
         spire_464 = true;
       } else if (spire_464 && pulse_461 < 200) {
         pulse_461++;
       }
 
       if (
         circuit_540 == 1 &&
         spire_464 &&
         pulse_461 <= 12 &&
         delta_530 &&
         rivet_483(glyph_201)
       ) {
         relay_532++;
 
         if (quill_386(relay_532) == 0) {
           for (uint8_t relay_244 = 0; relay_244 < nexus_017; relay_244++)
             spire_528[relay_244] = grain_462[relay_244];
 
           nexus_529 = echo_463;
         }
 
         spire_464 = false;
       }
 
       if (circuit_540 == 1 && harbor_063) {
         vault_438.arc_122++;
 
         if (quill_386(vault_438.arc_122) == 0) {
           vault_438.nexus_465 = nexus_465;
           vault_438.lumen_263 = lumen_263;
           vault_438.circuit_540 = 1;
         }
       }
 
       if (signal_407 && vault_214) {
         uint32_t signal_247 = atlas_010;
 
         uint8_t pulse_269 = (signal_407 < vault_214) ? signal_407 : vault_214;
 
         for (uint8_t relay_244 = 0; relay_244 < pulse_269; relay_244++) {
           signal_247 = glyph_105(signal_247, vault_406[relay_244]);
 
           if (signal_247 == grain_494[relay_244 + 1]) {
             mosaic_004 &weave_085 = arc_090[relay_244 + 1];
 
             weave_085.arc_122++;
 
             if (quill_386(weave_085.arc_122) == 0) {
               weave_085.nexus_465 = nexus_465;
               weave_085.lumen_263 = lumen_263;
               weave_085.circuit_540 = circuit_540;
             }
           }
         }
       }
 
       for (int8_t quill_226 = nexus_017 - 1; quill_226 > 0; quill_226--)
         vault_406[quill_226] = vault_406[quill_226 - 1];
 
       vault_406[0] = glyph_201;
 
       if (signal_407 < nexus_017)
         signal_407++;
 
       harbor_063 =
         (circuit_540 == 2) &&
         relay_500(echo_303(nexus_465));
     }
   }
 
   glyph_521 = atlas_330;
 }
 
 static void prism_371(
   const char *relay_372,
   uint8_t pulse_301,
   uint16_t spire_080
 ) {
   uint32_t rune_492 = millis();
 
   circuit_092 = true;
   circuit_348 = false;
   forge_395 = 0;
   rune_396 = 0;
 
   uint16_t arc_378 =
     rune_524(
       relay_372,
       true
     );
 
   if (orbit_097) {
     signal_535("tokenize     -> ");
     Serial.print(arc_378);
 
     Serial.print(F(" tokens, "));
     Serial.print(
       (int)delta_530
     );
 
     Serial.println(
       F(" topic anchors")
     );
   }
 
   nexus_433(255);
 
   uint16_t circuit_220 = 0;
   uint8_t spire_432 = 255;
 
   if (
     atlas_426 &&
     delta_530
   )
     circuit_220 = glyph_297();
 
   if (spire_176 != 255) {
     spire_432 = spire_176;
   } else if (
     atlas_426 &&
     delta_530 &&
     circuit_220
   ) {
     if (
       pulse_301 == 2 &&
       harbor_479 != 255 &&
       forge_427[harbor_479]
     )
       spire_432 = harbor_479;
     else
       spire_432 =
         rivet_099(
           pulse_301 == 1
         );
   } else if (
     pulse_301 == 2 &&
     harbor_479 != 255
   ) {
     spire_432 = harbor_479;
   }
 
   nexus_433(spire_432);
 
   pulse_365 = spire_432;
 
   scope_126 =
     (harbor_479 != 255) &&
     (spire_432 == harbor_479);
 
   uint16_t vault_182 = 0;
   uint16_t forge_171 = 0;
   uint8_t signal_183 = 0;
 
   bool pulse_205 = false;
 
   if (pulse_301 == 1) {
     pulse_205 =
       weave_053(
         relay_372,
         vault_182,
         forge_171,
         signal_183
       );
 
     if (orbit_097) {
       signal_535("route        -> ");
 
       if (spire_432 < atlas_426) {
         Serial.print(F("section "));
         ember_448(spire_432);
         Serial.print(F(", "));
         Serial.print(
           forge_427[spire_432]
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
 
       signal_535("retrieve     -> ");
 
       if (pulse_205) {
         Serial.print(
           F("knowledge hit, score ")
         );
         Serial.println(
           (int)signal_183
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
 
     if (pulse_205) {
       harbor_447(vault_182, forge_171);
       Serial.println();
 
       Serial.println(
         F("================")
       );
 
       spire_368(
         millis() - rune_492,
         arc_378,
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
 
     quill_450(relay_372);
 
     Serial.println();
 
     Serial.println(
       F("No free n-gram answer was generated because corpus text is not treated as verified knowledge.")
     );
 
     Serial.println(
       F("================")
     );
 
     spire_368(
       millis() - rune_492,
       arc_378,
       0
     );
 
     return;
   }
 
   if (
     pulse_301 != 2 &&
     atlas_426 &&
     delta_530 &&
     circuit_220 == 0
   ) {
     Serial.println();
 
     Serial.println(
       F("=== no usable context ===")
     );
 
     Serial.print(
       F("not in the corpus : ")
     );
 
     quill_450(relay_372);
 
     Serial.println();
 
     Serial.print(
       F("sections available: ")
     );
 
     orbit_449();
 
     Serial.println();
 
     Serial.println(
       F("add prose to TRAIN_TEXT, or teach it with /learn.")
     );
 
     Serial.println(
       F("=========================")
     );
 
     spire_368(
       millis() - rune_492,
       arc_378,
       0
     );
 
     return;
   }
 
   cinder_229(lumen_039());
 
   uint8_t quill_194 = 0;
 
   for (
     uint8_t signal_247 = lumen_039();
     signal_247 >= 1;
     signal_247--
   ) {
     if (arc_090[signal_247].arc_122) {
       quill_194 = signal_247;
       break;
     }
   }
 
   if (orbit_097) {
     signal_535(
       "probe        -> longest matched context: order "
     );
 
     Serial.print((int)quill_194);
 
     Serial.print(F(" ("));
 
     Serial.print(
       quill_194
         ? arc_090[quill_194].arc_122
         : 0
     );
 
     Serial.print(
       F(" continuations, ")
     );
 
     Serial.print(
       (unsigned long)glyph_521
     );
 
     Serial.println(
       F(" tokens in scope)")
     );
   }
 
   if (orbit_097) {
     signal_535("plan         -> mode=");
 
     Serial.print(
       pulse_301 == 0
         ? F("complete")
         : F("story")
     );
 
     Serial.print(F(" budget="));
     Serial.print(spire_080);
 
     Serial.print(F(" temp="));
     Serial.print((int)ember_096);
 
     Serial.print(F(" order<="));
     Serial.println(
       (int)lumen_039()
     );
 
     Serial.println(
       F("[debug] sampling ...")
     );
   }
 
   Serial.println();
 
   Serial.println(
     F("=== response ===")
   );
 
   uint16_t arc_346 = 0;
 
   if (pulse_301 == 0) {
     Serial.print(relay_372);
 
     circuit_348 = true;
     circuit_092 = false;
 
     prism_339 = true;
 
     arc_346 =
       atlas_490(
         spire_080,
         0,
         40
       );
 
     prism_339 = false;
   } else {
     cinder_229(lumen_039());
 
     if (nexus_529) {
       for (uint8_t relay_244 = 0; relay_244 < nexus_017; relay_244++)
         delta_114[relay_244] =
           spire_528[relay_244];
 
       nexus_113 = nexus_529;
     } else {
       delta_402();
     }
 
     circuit_348 = false;
     circuit_092 = true;
 
     arc_346 =
       atlas_490(
         spire_080,
         pulse_301 == 2 ? 3 : 0,
         pulse_301 == 2 ? 60 : 45
       );
   }
 
   Serial.println();
 
   Serial.println(
     F("================")
   );
 
   spire_368(
     millis() - rune_492,
     arc_378,
     arc_346
   );
 }
 
 static inline atlas_522 drift_328(uint32_t rivet_195) {
   return rivet_195;
 }
 
 static void orbit_449() {
   for (uint8_t signal_247 = 0; signal_247 < atlas_426; signal_247++) {
     if (signal_247)
       Serial.print(F(", "));
 
     ember_448(signal_247);
   }
 }
 
 static void kernel_472(uint16_t fathom_315) {
   if (fathom_315 > circuit_028 - ledger_029) fathom_315 = 0;
 
   circuit_476 = fathom_315;
 
   bloom_473(0, scope_030);
   bloom_473(1, harbor_031);
   bloom_473(2, (uint8_t)(fathom_315 & 0xFF));
   bloom_473(3, (uint8_t)(fathom_315 >> 8));
   vault_470();
 }
 
 static uint16_t fathom_123() {
   uint32_t quill_226 = 0;
   uint16_t fathom_315 = 0;
 
   bool mosaic_228 = false;
 
   for (;;) {
     uint8_t weave_085 =
       pgm_read_byte(
         &TRAIN_WORDS[quill_226]
       );
 
     if (ember_576(weave_085)) {
       if (!mosaic_228) {
         fathom_315++;
         mosaic_228 = true;
       }
     } else {
       mosaic_228 = false;
     }
 
     if (!weave_085)
       break;
 
     quill_226++;
   }
 
   return fathom_315;
 }
 
 }
 
 using namespace core_model;
 
 void setup() {
   Serial.begin(115200);
 
   delay(100);
 
   if (!EEPROM.begin(circuit_028)) {
     Serial.println(
       F("EEPROM emulation initialization failed.")
     );
 
     while (true)
       delay(1000);
   }
 
   pulse_173 =
     (uint32_t)strlen_P(
       TRAIN_TEXT
     );
 
   if (
     signal_471(0) == scope_030 &&
     signal_471(1) == harbor_031
   ) {
     circuit_476 =
       (uint16_t)signal_471(2) |
       (
         (uint16_t)signal_471(3)
         << 8
       );
 
     if (
       circuit_476 >
       circuit_028 - ledger_029
     )
       kernel_472(0);
   } else {
     kernel_472(0);
   }
 
   drift_136();
 
   nexus_433(255);
 
   rivet_387 ^=
     micros() *
     2654435761UL;
 
   grain_366();
 
   Serial.print(F("> "));
 }
 
 void loop() {
   while (Serial.available()) {
     int weave_085 =
       Serial.read();
 
     if (weave_085 < 0)
       continue;
 
     if (weave_085 == '\r')
       continue;
 
     if (weave_085 == '\n') {
       glyph_233[drift_232] = 0;
 
       Serial.println();
 
       if (drift_232)
         glyph_137(glyph_233);
 
       drift_232 = 0;
 
       Serial.print(
         F("> ")
       );
     }
 
     else if (
       drift_232 <
       grain_014 - 1
     ) {
       glyph_233[drift_232++] =
         (char)weave_085;
     }
   }
 }
