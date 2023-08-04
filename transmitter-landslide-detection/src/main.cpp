#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <MPU6500_WE.h>

#define MOIST_PIN A7

MPU6500_WE mpu = MPU6500_WE(0x68);
bool isFell();
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
    if (!mpu.init())
    { // change to your own address
        while (1)
        {
            Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
            delay(5000);
        }
    }
    Serial.println("Position you MPU6500 flat and don't move it - calibrating...");
    delay(1000);
    mpu.autoOffsets();
    Serial.println("Done!");

    if (!LoRa.begin(433E6))
    {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }
}

void loop()
{
    if (isFell())
    {
        message = String(readMoisture()) + "," + String(isFell());
        LoRa.beginPacket();
        LoRa.print(message);
        LoRa.endPacket();
        Serial.println("LONGSOR LONGSOR");
    }
    static uint32_t prev_ms = millis();
    if (millis() > prev_ms + 1000)
    {
        message = String(readMoisture()) + "," + String(isFell());
        LoRa.beginPacket();
        LoRa.print(message);
        LoRa.endPacket();
        Serial.println(message);
        prev_ms = millis();
    }
}

bool isFell()
{
    xyzFloat gyr = mpu.getGyrValues();
    // pitch = mpu.getPitch();
    // roll = mpu.getRoll();
    // Serial.print("Pitch, Roll: ");
    // Serial.print(pitch, 2);
    // Serial.print(", ");
    // Serial.println(roll, 2);
    // if (pitch > 50 || pitch < -50 || roll > 50 || roll < -50)
    //     return 1;
    if (gyr.x > 100 || gyr.x < -100 || gyr.y > 100 || gyr.y < -100 || gyr.z > 100 || gyr.z < -100)
    {
        return 1;
    }
    else
        return 0;
}

int readMoisture()
{
    Serial.print("\t Humidity: ");
    Serial.println(analogRead(MOIST_PIN));
    moist_value = analogRead(MOIST_PIN);
    return map(moist_value, 1023, 100, 0, 100);
}