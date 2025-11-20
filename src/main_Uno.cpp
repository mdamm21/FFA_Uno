#include <Servo.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>

// Definition für den Servo
#define servoPin 5

// Servo-Objekt erstellen
Servo myServo;

// RTC-Objekt erstellen
RTC_DS3231 rtc;

// Variable, um die empfangene Distanz zu speichern
long distance = -1;

void setup() {
  // Serielle Kommunikation starten
  Serial.begin(115200);
  Serial.println("Setup gestartet");

  // Servo initialisieren
  myServo.attach(servoPin);
  Serial.println("Servo angehängt");

  // Initiale Position des Servos auf 0 Grad setzen
  myServo.write(0);
  Serial.println("Servo auf 0 Grad gesetzt");
  delay(1000); // Warten, bis der Servo initialisiert ist

  // RTC initialisieren
  if (!rtc.begin()) {
    Serial.println("RTC konnte nicht initialisiert werden!");
    while (1); // Stoppe das Programm, wenn die RTC nicht funktioniert
  }

  if (rtc.lostPower()) {
    Serial.println("RTC hat die Stromversorgung verloren, Zeit wird neu eingestellt!");
    // Setze die Zeit nur einmal, wenn die RTC die Stromversorgung verloren hat
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Setup abgeschlossen");
}

void loop() {
  // Überprüfen, ob serielle Daten verfügbar sind
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    // Wenn der Befehl "move" ist, überprüfe die Distanz und bewege den Servo
    if (command.startsWith("move")) {
      // Extrahiere die Distanz aus dem Befehl
      int separatorIndex = command.indexOf(' ');
      if (separatorIndex > 0) {
        String distanceStr = command.substring(separatorIndex + 1);
        distance = distanceStr.toInt();
        Serial.print("Empfangene Distanz: ");
        Serial.println(distance);
      } else {
        distance = -1; // Ungültige Distanz
        Serial.println("Ungültige Distanz");
      }
    }
  }

  DateTime now = rtc.now();

  // Debug-Ausgabe der aktuellen Zeit
  Serial.print("Aktuelle Zeit: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());

  // Überprüfen, ob es 18:00 Uhr lokal ist
  if (now.hour() == 18 && now.minute() == 0 && now.second() == 0) {
    if (distance != -1) {
      // Servo eine 1/4 Umdrehung vorwärts (90 Grad)
      myServo.write(90);
      Serial.println("Servo auf 90 Grad gesetzt");
      delay(1000); // Kurze Pause, um die Bewegung zu gewährleisten

      // Verweildauer im geöffneten Zustand basierend auf der gemessenen Entfernung
      int openDuration;
      if (distance <= 30) {
        openDuration = map(distance, 1, 30, 1000, 5000); // Verweildauer zwischen 1 und 5 Sekunden
        Serial.print("Verweildauer: ");
        Serial.println(openDuration);
      } else {
        openDuration = 1000; // Standardverweildauer, wenn keine gültige Messung
        Serial.println("Standard Verweildauer: 1000ms");
      }

      delay(openDuration);

      // Servo in die Ausgangsposition zurück (0 Grad)
      myServo.write(0);
      Serial.println("Servo auf 0 Grad zurückgesetzt");
      delay(1000); // Kurze Pause, um die Bewegung zu gewährleisten

      // Setze die Distanz zurück, um sicherzustellen, dass die Aktion nur einmal pro Tag ausgeführt wird
      distance = -1;
    }
  }

  delay(1000); // Loop einmal pro Sekunde prüfen
}
