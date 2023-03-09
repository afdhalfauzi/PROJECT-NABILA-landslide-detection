#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "MPU9250.h"

#define MOIST_PIN A7

MPU9250 mpu;
bool readGyro();
int readMoisture();
float pitch, roll;
int moist_value;
String message;

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    delay(2000);

    pinMode(MOIST_PIN, INPUT);
    if (!mpu.setup(0x68))
    { // change to your own address
        while (1)
        {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }
    if (!LoRa.begin(433E6))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
}

void loop()
{
    if (mpu.update())
    {
        static uint32_t prev_ms = millis();
        if (millis() > prev_ms + 1000)
        {
            message = String(readMoisture()) + "," + String(readGyro());
            LoRa.beginPacket();
            LoRa.print(message);
            LoRa.endPacket();
            prev_ms = millis();
        }
    }
}

bool readGyro()
{
    pitch = mpu.getPitch();
    roll = mpu.getRoll();
    Serial.print("Pitch, Roll: ");
    Serial.print(pitch, 2);
    Serial.print(", ");
    Serial.println(roll, 2);
    if (pitch > 50 || pitch < -50 || roll > 50 || roll < -50)
        return 1;
    else
        return 0;
}

int readMoisture()
{
    Serial.print("\t Humidity: ");
    Serial.println(analogRead(MOIST_PIN));
    moist_value = analogRead(MOIST_PIN);
    return map(moist_value, 1023, 0, 0, 100);
}