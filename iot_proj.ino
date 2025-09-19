#define BLYNK_TEMPLATE_ID "TMPL3qvrx_wEH"
#define BLYNK_TEMPLATE_NAME "openTheDoor"
#define BLYNK_AUTH_TOKEN "EhVmO9uSlbOrSAZ4a5c5WhOYt5f_WFPI"
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>  // For getting time via NTP

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>




// WiFi Credentials
char ssid[] = "Vivo v40";
char pass[] = "abinayat092";

// Pin Definitions
#define SS_PIN D2         // RFID SDA
#define RST_PIN D4        // RFID RST
#define LOCK_PIN D0       // Relay for solenoid
#define led D1            // Access LED
#define buz D8            // Buzzer for denied

MFRC522 mfrc522(SS_PIN, RST_PIN);
byte knownUID[] = {0xE3, 0x33, 0xEB, 0x2C};

bool remoteUnlock = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  pinMode(led, OUTPUT);
  pinMode(buz, OUTPUT);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, HIGH);  // Locked by default

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
 configTime(19800, 0, "pool.ntp.org");  // GMT+5:30 (IST). Change offset if needed


  Serial.println("Ready. Scan your RFID card...");
}

void loop() {
  Blynk.run();

  // Remote unlock via Blynk switch
  if (remoteUnlock) {
    digitalWrite(LOCK_PIN, LOW);   // Unlock
    digitalWrite(led, HIGH);
    return;  // Skip RFID check if remotely unlocked
  } else {
    digitalWrite(LOCK_PIN, HIGH);  // Locked
    digitalWrite(led, LOW);
  }

  digitalWrite(buz, LOW);  // Default: buzzer off

  // Wait for a card
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Show UID
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Check UID
  if (compareUID(mfrc522.uid.uidByte, knownUID, mfrc522.uid.size)) {
    Serial.println("✅ Access Granted - Unlocking for 5 seconds");
    Blynk.virtualWrite(V0, "✅ Abinaya! unlocked the door at " + getCurrentTime());
    digitalWrite(LOCK_PIN, LOW);  // Unlock
    digitalWrite(led, HIGH);
    delay(5000);                  // Stay unlocked
  } else {
    Serial.println("❌ Access Denied - Staying locked");
    Blynk.virtualWrite(V0, "❌ Unauthorized card scanned at " + getCurrentTime());

    digitalWrite(buz, HIGH);
    delay(2000);
  }

  // Stop communication
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(500);  // Small gap before next scan
}
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "⏱ time error";
  }
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

// UID checker
bool compareUID(byte* scannedUID, byte* validUID, byte len) {
  for (byte i = 0; i < len; i++) {
    if (scannedUID[i] != validUID[i]) return false;
  }
  return true;
}

// Blynk Switch Handler (V1)
BLYNK_WRITE(V1) {
  int value = param.asInt();
  remoteUnlock = (value == 1);

if (remoteUnlock) {
  Serial.println("🔓 Door remotely unlocked (switch ON)");
  Blynk.virtualWrite(V0, "🔓 Door remotely unlocked at " + getCurrentTime());
} else {
  Serial.println("🔒 Door remotely locked (switch OFF)");
  Blynk.virtualWrite(V0, "🔒 Door remotely locked at " + getCurrentTime());
}
}
