#ifndef DATA_UTILS_H
#define DATA_UTILS_H

#include <stdint.h>
#include <string.h>     // strncpy
#include <Arduino.h>    // para Serial y tipos Arduino

// --- CONTROL DE DEPURACIÓN SERIAL ---
#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
  #define DEBUG_PRINT(x)        Serial.print(x)
  #define DEBUG_PRINTLN(x)      Serial.println(x)
  #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif
// ------------------------------------

// ===== Escalas de empaquetado (tag -> ancla -> concentrador) =====
static constexpr float TEMP_SCALE   = 100.0f;   // °C  *100
static constexpr float HUM_SCALE    = 100.0f;   // %RH *100
static constexpr float ACC_SCALE    = 1000.0f;  // g   *1000
static constexpr float GYR_SCALE    = 100.0f;   // dps *100
static constexpr float MAG_SCALE    = 10.0f;    // uT  *10
static constexpr float MDIR_SCALE   = 10.0f;    // deg *10

// ===== Helpers de casteo seguro =====
static inline int16_t clamp_i16(float v) {
  long s = lroundf(v);
  if (s >  32767L) return  32767;
  if (s < -32768L) return -32768;
  return (int16_t)s;
}
static inline uint16_t clamp_u16(float v) {
  long s = lroundf(v);
  if (s >  65535L) return  65535U;
  if (s <  0L)     return  0U;
  return (uint16_t)s;
}

// ============================================================================
// PAQUETE EMPAQUETADO: lo que envía CADA ANCLA al CONCENTRADOR por TWR (range)
// Incluye: identificación, correlación, distancia medida y (opcional) calidad.
// Además, incluye los datos de timestamp+IMU que entrega el TAG (enteros con
// escala), para que el concentrador los persista junto a la posición calculada.
// ============================================================================
#pragma pack(push, 1)
typedef struct AnchorRangeReport_t {
    // ---- Identificación / correlación (ToF/TWR) ----
    uint16_t anchor_saddr;   // ID corto DW1000 de la ancla
    uint32_t tag_uid;        // ID/EUI del tag (o hash)
    uint16_t seq;            // correlación del evento (mismo blink/round)
    float    range_m;        // distancia ToF calibrada (metros)
    uint32_t t_ms;           // (opcional) timestamp local de la ancla (ms)

    // ---- Calidad recepción (opcional; 0 si no se usa) ----
    uint16_t rxpacc;
    uint16_t std_noise;
    uint16_t fp_ampl1, fp_ampl2, fp_ampl3;
    uint16_t cir_pwr;

    // ---- Config PHY (opcional; 0 si no se usa) ----
    uint8_t  uwb_ch, uwb_prf, uwb_pcode, uwb_drate;

    // ---- Timestamp de captura en el TAG ----
    uint16_t year;
    uint8_t  month, day, hour, minute, second;
    uint16_t millis;

    // ---- Sensores del TAG (enteros con escala) ----
    int16_t  temp;           // °C   *100
    uint16_t hum;            // %RH  *100
    int16_t  aX, aY, aZ, aSQ; // g   *1000
    int16_t  gX, gY, gZ;     // dps  *100
    int16_t  mX, mY, mZ;     // uT   *10
    uint16_t mDir;           // deg  *10
    char     etiqueta[4];    // p.ej. "HB","BV"
} AnchorRangeReport_t;
#pragma pack(pop)

// ============================================================================
// Estructura decodificada (para uso interno en el CONCENTRADOR)
// Tipos "friendlies" (float) y mismos campos para trazabilidad
// ============================================================================
typedef struct DecodedAnchorReport_t {
    // Identificación / correlación
    uint16_t anchor_saddr;
    uint32_t tag_uid;
    uint16_t seq;
    float    range_m;
    uint32_t t_ms;

    // Calidad (opc)
    uint16_t rxpacc;
    uint16_t std_noise;
    uint16_t fp_ampl1, fp_ampl2, fp_ampl3;
    uint16_t cir_pwr;

    // Config PHY (opc)
    uint8_t  uwb_ch, uwb_prf, uwb_pcode, uwb_drate;

    // Timestamp TAG
    uint16_t year;
    uint8_t  month, day, hour, minute, second;
    uint16_t millis;

    // Sensores TAG (float ya escalados)
    float temp, hum;
    float aX, aY, aZ, aSQ;
    float gX, gY, gZ;
    float mX, mY, mZ, mDir;
    char  etiqueta[4];  // se mantiene fijo en 4 chars + '\0'
} DecodedAnchorReport_t;

// ============================================================================
// UNPACK: AnchorRangeReport_t -> DecodedAnchorReport_t
// ============================================================================
inline DecodedAnchorReport_t unpack_anchor_report(const AnchorRangeReport_t& p) {
    DecodedAnchorReport_t d = {};

    // Identificación / correlación
    d.anchor_saddr = p.anchor_saddr;
    d.tag_uid      = p.tag_uid;
    d.seq          = p.seq;
    d.range_m      = p.range_m;
    d.t_ms         = p.t_ms;

    // Calidad
    d.rxpacc    = p.rxpacc;
    d.std_noise = p.std_noise;
    d.fp_ampl1  = p.fp_ampl1;
    d.fp_ampl2  = p.fp_ampl2;
    d.fp_ampl3  = p.fp_ampl3;
    d.cir_pwr   = p.cir_pwr;

    // PHY
    d.uwb_ch    = p.uwb_ch;
    d.uwb_prf   = p.uwb_prf;
    d.uwb_pcode = p.uwb_pcode;
    d.uwb_drate = p.uwb_drate;

    // Timestamp del TAG
    d.year   = p.year;
    d.month  = p.month;   d.day    = p.day;
    d.hour   = p.hour;    d.minute = p.minute; d.second = p.second;
    d.millis = p.millis;

    // Sensores (des-escalado a float)
    d.temp =  p.temp / TEMP_SCALE;
    d.hum  =  p.hum  / HUM_SCALE;

    d.aX   =  p.aX   / ACC_SCALE;  d.aY = p.aY / ACC_SCALE;  d.aZ = p.aZ / ACC_SCALE;  d.aSQ = p.aSQ / ACC_SCALE;
    d.gX   =  p.gX   / GYR_SCALE;  d.gY = p.gY / GYR_SCALE;  d.gZ = p.gZ / GYR_SCALE;
    d.mX   =  p.mX   / MAG_SCALE;  d.mY = p.mY / MAG_SCALE;  d.mZ = p.mZ / MAG_SCALE;
    d.mDir =  p.mDir / MDIR_SCALE;

    strncpy(d.etiqueta, p.etiqueta, sizeof(d.etiqueta));
    d.etiqueta[sizeof(d.etiqueta)-1] = '\0';

    return d;
}

// ============================================================================
// PACK: DecodedAnchorReport_t -> AnchorRangeReport_t
// (Aplica escalas y saturación para mantener rangos válidos)
// ============================================================================
inline AnchorRangeReport_t pack_anchor_report(const DecodedAnchorReport_t& d) {
    AnchorRangeReport_t p = {};

    // Identificación / correlación
    p.anchor_saddr = d.anchor_saddr;
    p.tag_uid      = d.tag_uid;
    p.seq          = d.seq;
    p.range_m      = d.range_m;
    p.t_ms         = d.t_ms;

    // Calidad
    p.rxpacc    = d.rxpacc;
    p.std_noise = d.std_noise;
    p.fp_ampl1  = d.fp_ampl1;
    p.fp_ampl2  = d.fp_ampl2;
    p.fp_ampl3  = d.fp_ampl3;
    p.cir_pwr   = d.cir_pwr;

    // PHY
    p.uwb_ch    = d.uwb_ch;
    p.uwb_prf   = d.uwb_prf;
    p.uwb_pcode = d.uwb_pcode;
    p.uwb_drate = d.uwb_drate;

    // Timestamp del TAG
    p.year   = d.year;
    p.month  = d.month;   p.day    = d.day;
    p.hour   = d.hour;    p.minute = d.minute; p.second = d.second;
    p.millis = d.millis;

    // Sensores (aplica escala + saturación)
    p.temp = clamp_i16(d.temp * TEMP_SCALE);
    p.hum  = clamp_u16(d.hum  * HUM_SCALE);

    p.aX   = clamp_i16(d.aX * ACC_SCALE);
    p.aY   = clamp_i16(d.aY * ACC_SCALE);
    p.aZ   = clamp_i16(d.aZ * ACC_SCALE);
    p.aSQ  = clamp_i16(d.aSQ * ACC_SCALE);

    p.gX   = clamp_i16(d.gX * GYR_SCALE);
    p.gY   = clamp_i16(d.gY * GYR_SCALE);
    p.gZ   = clamp_i16(d.gZ * GYR_SCALE);

    p.mX   = clamp_i16(d.mX * MAG_SCALE);
    p.mY   = clamp_i16(d.mY * MAG_SCALE);
    p.mZ   = clamp_i16(d.mZ * MAG_SCALE);

    p.mDir = clamp_u16(d.mDir * MDIR_SCALE);

    strncpy(p.etiqueta, d.etiqueta, sizeof(p.etiqueta) - 1);
    p.etiqueta[sizeof(p.etiqueta) - 1] = '\0';

    return p;
}

#endif // DATA_UTILS_H