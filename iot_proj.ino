#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D2         // RFID SDA
#define RST_PIN D4        // RFID RST
#define LOCK_PIN D0       // Relay IN (for solenoid)
#define led D1
#define buz D8

MFRC522 mfrc522(SS_PIN, RST_PIN);
byte knownUID[] = {0xE3, 0x33, 0xEB, 0x2C};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(led,OUTPUT);
  pinMode(buz,OUTPUT);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, HIGH);  // Default: Locked (HIGH = relay off)

  Serial.println("Ready. Scan your RFID card...");
}

void loop() {
  // Default state: lock the door
  digitalWrite(LOCK_PIN, HIGH);  // Relay off (solenoid locked)
  digitalWrite(led, 0);
  digitalWrite(buz, 0);
  // Wait for a card
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  // Print UID
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Check UID
  if (compareUID(mfrc522.uid.uidByte, knownUID, mfrc522.uid.size)) {
    Serial.println("✅ Access Granted - Unlocking for 5 seconds");
    digitalWrite(LOCK_PIN, LOW);  // Relay ON (solenoid unlock)
    digitalWrite(led,1);
    delay(5000);                  // Stay unlocked
  } else {
    Serial.println("❌ Access Denied - Staying locked");
    digitalWrite(buz,1);
    delay(2000);
  }

  // Stop communication with card
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(500);  // Small delay before next scan
}

bool compareUID(byte* scannedUID, byte* validUID, byte len) {
  for (byte i = 0; i < len; i++) {
    if (scannedUID[i] != validUID[i]) return false;
  }
  return true;
}
