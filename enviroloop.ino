/*******************************
ESP32 Water Quality Monitoring
POST: pH, TDS, Turbidity, Buzzer
*******************************/
 
#include <WiFi.h>
#include <HTTPClient.h>
 
/* ---------- WIFI ---------- */
const char* ssid = "iot";
const char* password = "12345678";
String postURL = "http://iotcloud22.in/4570/post_value.php";
 
WiFiClient client;
HTTPClient http;
 
/* ---------- PINS ---------- */
#define PH_PIN     34
#define TURB_PIN   32
#define TDS_PIN    35
#define BUZZER     25
 
/* ---------- SETUP ---------- */
void setup() {
  Serial.begin(115200);
 
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
 
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
 
  /* WiFi */
  Serial.print("Connecting WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
 
  Serial.println("=== Water Quality Monitoring Started ===");
}
 
/* ---------- POST FUNCTION ---------- */
void sendToServer(float ph, float tds, float turb, int buzzer)
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi NOT connected → POST skipped");
    return;
  }
 
  String postData =
      "value1=" + String(ph, 2) +
      "&value3=" + String(tds, 0) +
      "&value5=" + String(turb, 1) +
      "&value7=" + String(buzzer);
 
  Serial.println("POST DATA:");
  Serial.println(postData);
 
  http.begin(client, postURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
 
  int code = http.POST(postData);
  Serial.print("POST CODE: ");
  Serial.println(code);
 
  http.end();
}
 
/* ---------- LOOP ---------- */
void loop() {
 
  /* -------- READ ADC -------- */
  int phRaw   = analogRead(PH_PIN);
  int turbRaw = analogRead(TURB_PIN);
  int tdsRaw  = analogRead(TDS_PIN);
 
  /* -------- ADC → VOLTAGE -------- */
  float vPH   = phRaw   * (3.3 / 4095.0);
  float vTurb = turbRaw * (3.3 / 4095.0);
  float vTDS  = tdsRaw  * (3.3 / 4095.0);
 
  /* -------- pH CALCULATION -------- */
  float pH;
 
  if (vPH <= 0.5)
    pH = vPH * 21.5;
  else if (vPH <= 1.0)
    pH = 1.75 + (vPH - 0.5) * (5.8 - 1.75) / 0.5;
  else if (vPH <= 2.5)
    pH = 6.5 + (vPH - 1.65) * (7.5 - 6.5) / 0.85;
  else
    pH = 10.5 + (vPH - 3.0) * (11.5 - 10.5) / 0.3;
 
  if (pH < 0) pH = 0;
 
  /* -------- TURBIDITY (NTU) -------- */
  float turbidity = (3.3 - vTurb) * (100.0 / 3.3);
  if (turbidity < 0) turbidity = 0;
 
  /* -------- TDS (ppm) -------- */
  float tds = (vTDS / 3.3) * 150;
  if (tds < 0) tds = 0;
 
  /* -------- BUZZER LOGIC -------- */
  bool buzzerOn = false;
 
  // SAFE TDS = 80–90 ppm
  if (tds < 60 || tds > 90) buzzerOn = true;
 
  // SAFE Turbidity = 45–60 NTU
  if (turbidity < 45 || turbidity > 60) buzzerOn = true;
 
  digitalWrite(BUZZER, buzzerOn ? HIGH : LOW);
 
  /* -------- SERIAL OUTPUT -------- */
  Serial.println("----------------------------");
  Serial.print("pH Value        : "); Serial.println(pH, 2);
  Serial.print("Turbidity NTU   : "); Serial.println(turbidity, 1);
  Serial.print("TDS (ppm)       : "); Serial.println(tds, 0);
  Serial.print("Buzzer Status   : ");
  Serial.println(buzzerOn ? "ON ❌" : "OFF ✅");
 
  /* -------- POST -------- */
  sendToServer(
    pH,
    tds,
    turbidity,
    buzzerOn ? 1 : 0
  );
 
  delay(2000);
}
