#include <Canbus.h>  // don't forget to include these
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <SD.h>
#include <SPI.h>

#define SD_CS 9 // SparkFun CAN Bus Shield SD card CS pin

File logFile;

void setup()
{
  Serial.begin(9600);
  delay(1000);

  pinMode(10, OUTPUT);  // CAN CS
  pinMode(SD_CS,OUTPUT);

  digitalWrite(10, LOW);  // Let CAN library control it
  digitalWrite(SD_CS,HIGH); //Deselect SD

  //Initialise MCP2515 CAN controller at the specified speed
  if(!Canbus.init(CANSPEED_500)) {
    Serial.println("CAN Init FAIL");
    while(1);
  }
  Serial.println("CAN Init OK");

  // Initialize SD Card
  if(!SD.begin(SD_CS)) {
    Serial.println("SD Init Failed!");
    while(1);
  }
  Serial.println("SD Init OK");

  logFile = SD.open("wheel.csv", FILE_WRITE);

  if (!logFile) {
    Serial.println("File open failed");
    while(1);
  }

  logFile.println("Time_ms,CAN_ID,DLC,B0,B1,B2,B3,B4,B5,B6,B7");
  logFile.flush();

  Serial.println("Logging started...");
}

void loop()
{
  static unsigned long lastFlush = 0;
  tCAN message;

  if (mcp2515_check_message())
  {
    if (mcp2515_get_message(&message))
    {
      logFile.print(millis());
      logFile.print(",");
      logFile.print(message.id, HEX);
      logFile.print(",");
      logFile.print(message.header.length);

      for (int i = 0; i < 8; i++)
      {
        logFile.print(",");
        logFile.print(message.data[i], HEX);
      }

      logFile.println();

      //Prints to Serial
      Serial.print(millis());
      Serial.print(", ID:");
      Serial.print(message.id, HEX);
      Serial.print(" DLC:");
      Serial.print(message.header.length);
      for (int i = 0; i < message.header.length; i++){
        Serial.print(" ");
        Serial.print(message.data[i], HEX);
      }
      Serial.println();

            // Flush every 250 ms (not every frame)
      if (millis() - lastFlush > 200)
      {
        logFile.flush();
        lastFlush = millis();
      }
    }
  }
}