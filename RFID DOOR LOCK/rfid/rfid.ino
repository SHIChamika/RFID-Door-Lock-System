#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// Pin Definitions
#define SS_PIN 10
#define RST_PIN 9
#define GREEN_LED 7
#define RED_LED 8
#define BUZZER 5
#define SERVO_PIN 6
#define LCD_ADDRESS 0x27  // Change to 0x3F if needed

// Objects
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorLockServo;
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

// Authorized Card UID (Replace with your UID)
byte authorizedUID[] = {0xA1, 0xB2, 0xC3, 0xD4}; // Replace with actual UID

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    
    doorLockServo.attach(SERVO_PIN);
    doorLockServo.write(0); // Locked position

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Scan Your Card");

    Serial.println("System Ready. Scan your RFID card...");
}

void loop() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        return;
    }

    Serial.print("Card UID: ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Card Detected!");

    bool accessGranted = true;

    for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        if (rfid.uid.uidByte[i] != authorizedUID[i]) {
            accessGranted = false;
        }
    }
    Serial.println();

    if (accessGranted) {
        Serial.println("✅ Access Granted!");
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(RED_LED, LOW);
        tone(BUZZER, 1000, 200); // Short beep

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted!");
        lcd.setCursor(0, 1);
        lcd.print("Door Unlocked");

        doorLockServo.write(90); // Unlock door
        delay(5000);             // Keep it open for 5 seconds
        doorLockServo.write(0);  // Lock door again

        digitalWrite(GREEN_LED, LOW);
    } else {
        Serial.println("❌ Access Denied!");
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        tone(BUZZER, 1000, 500); // Long beep

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied!");
        lcd.setCursor(0, 1);
        lcd.print("Try Again!");

        delay(2000);
        digitalWrite(RED_LED, LOW); // Turn off after delay
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan Your Card");

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}