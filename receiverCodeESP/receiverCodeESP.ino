#include <SPI.h>
#include <LoRa.h>

// =====================================================
// RA-02 / SX1278 CONNECTION
// =====================================================

#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23

#define LORA_CS    5
#define LORA_RST   14
#define LORA_DIO0  26

#define LORA_FREQUENCY 433E6

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("======================================");
  Serial.println("   MINE RESCUE ROVER");
  Serial.println("   ESP32 LORA RECEIVER");
  Serial.println("======================================");

  // ---------------------------------------------------
  // SPI
  // ---------------------------------------------------

  SPI.begin(
    LORA_SCK,
    LORA_MISO,
    LORA_MOSI,
    LORA_CS
  );

  // ---------------------------------------------------
  // LORA
  // ---------------------------------------------------

  LoRa.setPins(
    LORA_CS,
    LORA_RST,
    LORA_DIO0
  );

  Serial.println("Initializing LoRa...");

  if (!LoRa.begin(LORA_FREQUENCY))
  {
    Serial.println("ERROR: LoRa initialization FAILED!");

    while (1)
    {
      delay(1000);
    }
  }

  // Must exactly match UNO Q transmitter.

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  LoRa.enableCrc();

  Serial.println("LoRa receiver initialized.");
  Serial.println("Waiting for packets...");
  Serial.println();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  int packetSize = LoRa.parsePacket();

  if (packetSize)
  {
    String receivedPacket = "";

    while (LoRa.available())
    {
      receivedPacket += (char)LoRa.read();
    }

    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();

    // -------------------------------------------------
    // SERIAL MONITOR
    // -------------------------------------------------

    Serial.println("--------------------------------------");

    Serial.print("LoRa RX: ");
    Serial.println(receivedPacket);

    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.println(" dBm");

    Serial.print("SNR: ");
    Serial.print(snr);
    Serial.println(" dB");

    // -------------------------------------------------
    // EXTRACT SENSOR VALUES
    // -------------------------------------------------

    float temperature = NAN;
    float humidity = NAN;
    float pressure = NAN;

    String gas = "UNKNOWN";

    int tempIndex = receivedPacket.indexOf("TEMP=");
    int humIndex  = receivedPacket.indexOf(",HUM=");
    int pressIndex = receivedPacket.indexOf(",PRESS=");
    int gasIndex = receivedPacket.indexOf(",GAS=");

    if (
      tempIndex >= 0 &&
      humIndex >= 0 &&
      pressIndex >= 0 &&
      gasIndex >= 0
    )
    {
      String tempString =
        receivedPacket.substring(
          tempIndex + 5,
          humIndex
        );

      String humString =
        receivedPacket.substring(
          humIndex + 5,
          pressIndex
        );

      String pressureString =
        receivedPacket.substring(
          pressIndex + 7,
          gasIndex
        );

      String gasString =
        receivedPacket.substring(
          gasIndex + 5
        );

      temperature = tempString.toFloat();
      humidity = humString.toFloat();
      pressure = pressureString.toFloat();

      gas = gasString;
    }

    // -------------------------------------------------
    // OUTPUT MACHINE-READABLE JSON
    // -------------------------------------------------

    Serial.print("{");

    Serial.print("\"temp\":");
    Serial.print(temperature, 1);

    Serial.print(",\"humidity\":");
    Serial.print(humidity, 1);

    Serial.print(",\"pressure\":");
    Serial.print(pressure, 1);

    Serial.print(",\"gas\":\"");
    Serial.print(gas);
    Serial.print("\"");

    Serial.print(",\"rssi\":");
    Serial.print(rssi);

    Serial.print(",\"snr\":");
    Serial.print(snr, 1);

    Serial.println("}");
  }
}