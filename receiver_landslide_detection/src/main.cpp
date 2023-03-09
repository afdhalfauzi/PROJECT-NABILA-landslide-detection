#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
#include <WiFi.h>
#include <CTBot.h>

SSD1306 display(0x3c, 4, 15);
CTBot myBot;
TBMessage tMessage;

#define SS 18
#define RST 14
#define DI0 26
#define BAND 433E6

String wifiSSID = "BMZimages";
String wifiPassword = "bennamazarina";

String token = "6032383267:AAHCrdtB9kWulR80w7c2kV-mApcKyGP-t40";
int moisture, gyro;
String reply, status, prevStatus = "NORMAL";

void connectWifi();
void loginTelegram();
String setStatus(int percentage, int gyro);
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
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS, RST, DI0);

  if (!LoRa.begin(BAND))
  {
    display.drawString(64, 25, "Starting LoRa failed!");
    while (1)
      ;
  }
  Serial.println("LoRa Initial OK!");
  display.drawString(64, 5, "LoRa Initializing OK!");
  display.display();

  connectWifi();
  loginTelegram();
}
void loop()
{
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet. ");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0, "Status:");
    display.drawString(64, 33, "Kelembaban Tanah:");
    display.display();

    while (LoRa.available())
    {
      String data = LoRa.readString();
      moisture = data.substring(0, data.indexOf(",")).toInt();
      gyro = data.substring(data.indexOf(",") + 1).toInt();
      status = setStatus(moisture, gyro);
      if (status != prevStatus)
      {
        reply = "";
        reply += "ALERT \n";
        reply += "Status : " + status + "\n";
        reply += "Soil Moisture : " + String(moisture) + "%";
        prevStatus = status;
        myBot.sendMessage(tMessage.sender.id, reply);
      }
      display.setFont(ArialMT_Plain_16);
      display.drawString(64, 13, status);
      display.drawString(64, 45, String(moisture) + "%");
      display.display();
    }

    if (myBot.getNewMessage(tMessage))
    {
      if (tMessage.messageType == CTBotMessageText)
      {
        if (tMessage.text.equalsIgnoreCase("/start"))
        {
          reply = "";
          reply += "Hi, welcome to Landslide Detection Bot\n";
          reply += "Command List as below : \n";
          reply += "Get Status : /get_status \n";
          reply += "Get Soil Moisture : /get_moist \n";
        }
        else if (tMessage.text.equalsIgnoreCase("/get_status"))
        {
          reply = "";
          reply += "Status : " + status;
        }
        else if (tMessage.text.equalsIgnoreCase("/get_moist"))
        {
          reply = "";
          reply += "Soil Moisture : " + String(moisture) + "%";
        }
        else
        {
          reply = "";
          reply += "Hi, welcome to Landslide Detection Bot\n";
          reply += "Command is not valid, please use /start to get Command List! : \n";
        }
        myBot.sendMessage(tMessage.sender.id, reply);
      }
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

void connectWifi()
{
  Serial.println("Connecting To Wifi");
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  display.drawString(64, 25, "WiFi Connected");
  display.display();
}

void loginTelegram()
{
  while (!myBot.testConnection())
  {
    myBot.setTelegramToken(token);
    delay(1000);
  }
  display.drawString(64, 45, "Telegram Connection OK!");
  display.display();
}