#include <Arduino.h>

// KMITL FIGHT BOARD: ต่อหัวสัญญาณ LDR เข้ากับ GPI34
constexpr uint8_t LDR_PIN = 34;
constexpr uint8_t SAMPLE_COUNT = 16;
constexpr unsigned long SAMPLE_INTERVAL_MS = 500;

// หลังกรอกข้อมูลจริงใน calibration.xlsx ให้คัดลอกค่าสัมประสิทธิ์มาใส่
// แล้วเปลี่ยน CALIBRATION_READY เป็น 1
#define CALIBRATION_READY 0
#define USE_QUADRATIC_MODEL 0

constexpr float LINEAR_M = 0.0f;
constexpr float LINEAR_B = 0.0f;

constexpr float POLY_A = 0.0f;
constexpr float POLY_B = 0.0f;
constexpr float POLY_C = 0.0f;

uint16_t readAveragedAdc() {
  uint32_t total = 0;

  for (uint8_t i = 0; i < SAMPLE_COUNT; i++) {
    total += analogRead(LDR_PIN);
    delay(2);
  }

  return static_cast<uint16_t>(total / SAMPLE_COUNT);
}

float adcToLux(uint16_t adc) {
#if USE_QUADRATIC_MODEL
  return POLY_A * adc * adc + POLY_B * adc + POLY_C;
#else
  return LINEAR_M * adc + LINEAR_B;
#endif
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  Serial.println("time_ms,adc,voltage_v,lux");
}

void loop() {
  static unsigned long previousSampleTime = 0;

  if (millis() - previousSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }

  previousSampleTime = millis();

  const uint16_t adc = readAveragedAdc();
  const float voltage = adc * 3.3f / 4095.0f;

  Serial.print(previousSampleTime);
  Serial.print(',');
  Serial.print(adc);
  Serial.print(',');
  Serial.print(voltage, 3);
  Serial.print(',');

#if CALIBRATION_READY
  float lux = adcToLux(adc);
  if (lux < 0) {
    lux = 0;
  }
  Serial.println(lux, 2);
#else
  Serial.println("NA");
#endif
}
