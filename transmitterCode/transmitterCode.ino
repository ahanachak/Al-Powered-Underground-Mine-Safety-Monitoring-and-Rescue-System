#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <LoRa.h>

// =====================================================
// PIN CONFIGURATION
// =====================================================

// BME280
#define BME280_ADDRESS 0x76

// MQ-2 Digital Output
#define MQ2_DO 3

// RA-02 LoRa
#define LORA_DIO0 2
#define LORA_RST  9
#define LORA_CS   10

// LoRa frequency
#define LORA_FREQUENCY 433E6

// =====================================================
// OBJECTS
// =====================================================

Adafruit_BME280 bme;

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("======================================");
  Serial.println("   MINE RESCUE ROVER - UNO Q");
  Serial.println("   SENSOR + LORA TRANSMITTER");
  Serial.println("======================================");

  // ---------------------------------------------------
  // MQ-2
  // ---------------------------------------------------

  pinMode(MQ2_DO, INPUT);

  Serial.println("MQ-2 initialized.");

  // ---------------------------------------------------
  // BME280
  // ---------------------------------------------------

  Serial.println("Initializing BME280...");

  if (!bme.begin(BME280_ADDRESS))
  {
    Serial.println("ERROR: BME280 NOT FOUND!");
    Serial.println("Check wiring/address.");

    while (1)
    {
      delay(1000);
    }
  }

  Serial.println("BME280 OK.");

  // ---------------------------------------------------
  // LORA
  // ---------------------------------------------------

  Serial.println("Initializing RA-02...");

  LoRa.setPins(
    LORA_CS,
    LORA_RST,
    LORA_DIO0
  );

  if (!LoRa.begin(LORA_FREQUENCY))
  {
    Serial.println("ERROR: LoRa initialization FAILED!");

    while (1)
    {
      delay(1000);
    }
  }

  // Explicitly configure LoRa parameters
  // so transmitter and receiver match.

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  // CRC must be enabled on BOTH sides.
  LoRa.enableCrc();

  LoRa.setTxPower(17);

  Serial.println("RA-02 LoRa OK.");

  Serial.println();
  Serial.println("SYSTEM READY.");
  Serial.println();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ---------------------------------------------------
  // READ BME280
  // ---------------------------------------------------

  float temperature = bme.readTemperature();
  float humidity    = bme.readHumidity();
  float pressure    = bme.readPressure() / 100.0F;

  // ---------------------------------------------------
  // READ MQ-2
  // ---------------------------------------------------

  int gasDigital = digitalRead(MQ2_DO);

  String gasStatus;

  // Most MQ-2 modules output LOW when the
  // potentiometer threshold is crossed.
  if (gasDigital == LOW)
  {
    gasStatus = "ALERT";
  }
  else
  {
    gasStatus = "SAFE";
  }

  // ---------------------------------------------------
  // DISPLAY LOCALLY
  // ---------------------------------------------------

  Serial.println("--------------------------------------");

  Serial.print("Temperature : ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("Pressure    : ");
  Serial.print(pressure, 1);
  Serial.println(" hPa");

  Serial.print("Gas Status  : ");
  Serial.println(gasStatus);

  // ---------------------------------------------------
  // CREATE LORA PACKET
  // ---------------------------------------------------

  String packet =
    "TEMP=" + String(temperature, 1) +
    ",HUM=" + String(humidity, 1) +
    ",PRESS=" + String(pressure, 1) +
    ",GAS=" + gasStatus;

  Serial.print("LoRa TX: ");
  Serial.println(packet);

  // ---------------------------------------------------
  // TRANSMIT
  // ---------------------------------------------------

  LoRa.beginPacket();

  LoRa.print(packet);

  LoRa.endPacket();

  Serial.println("LoRa packet sent.");

  delay(2000);
}