#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"

SSD1306 display(0x3c, 4, 15);

#define SS 18
#define RST 14
#define DI0 26
#define BAND 433E6

void splitString(String data);
String setStatus(int percentage, int gyro);
int moisture, gyro;

void setup()
{
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  Serial.begin(115200);
  while (!Serial)
    ; // if just the the basic function, must connect to a computer
  delay(1000);
  Serial.println("LoRa Receiver");
  display.drawString(64, 5, "LoRa Receiver");
  display.display();
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND))
  {
    display.drawString(64, 25, "Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Initial OK!");
  display.drawString(64, 25, "LoRa Initializing OK!");
  display.display();
}
void loop()
{
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packets
    Serial.print("Received packet. ");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, "Status:");
    display.drawString(64, 33, "Kelembaban Tanah:");
    display.display();
    // read packet
    while (LoRa.available())
    {
      String data = LoRa.readString();
      Serial.print(data);
      moisture = data.substring(0, data.indexOf(",")).toInt();
      gyro = data.substring(data.indexOf(",") + 1).toInt();
      display.setFont(ArialMT_Plain_16);
      display.drawString(64, 13, setStatus(moisture, gyro));
      display.drawString(64, 45, String(moisture) + "%");
      display.display();
    }
  }
}

String setStatus(int percentage, int gyro)
{
  if (percentage >= 60 && gyro == 1)
    return "LONGSOR";
  else if (percentage >= 60 && gyro == 0)
    return "BAHAYA";
  else if (percentage >= 45)
    return "SIAGA";
  else if (percentage >= 30)
    return "WASPADA";
  else if (percentage >= 0)
    return "NORMAL";
  else
    return "NO DATA";
}
