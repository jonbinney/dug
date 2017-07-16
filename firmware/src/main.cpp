#include <Arduino.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

// Hack so that we don't have to include the quotes around preprocessor
// definitions when we pass them in at the commandline.
// See http://stackoverflow.com/questions/6671698/adding-quotes-to-argument-in-c-preprocessor
#define STRINGIFY2(S) #S
#define STRINGIFY(S) STRINGIFY2(S)

#if defined(WIFI_SSID)
const char wifi_ssid[] = STRINGIFY(WIFI_SSID);
#else
#error WIFI_SSID must be defined!
#endif

#if defined(WIFI_PASSWORD)
const char wifi_password[] = STRINGIFY(WIFI_PASSWORD);
#else
#error WIFI_PASSWORD must be defined!
#endif

int wifi_status = WL_IDLE_STATUS;

// Port on which to listen for UDP control packets.
const unsigned int control_port = 3434;

// Port to send status packets to.
const unsigned int remote_status_port = 3434;

char control_packet_buffer[255];
char status_packet_buffer[255];

WiFiUDP udp;

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  while ( wifi_status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifi_ssid);
    wifi_status = WiFi.begin(wifi_ssid, wifi_password);

    delay(1000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();

  udp.begin(control_port);
}

void loop()
{

  int packet_size = udp.parsePacket();
  if (packet_size)
  {
    Serial.print("Received packet of size ");
    Serial.println(packet_size);
    Serial.print("From ");
    IPAddress remote_ip = udp.remoteIP();
    Serial.print(remote_ip);
    Serial.print(", port ");
    Serial.println(udp.remotePort());

    int len = udp.read(control_packet_buffer, 254);
    if (len > 0) control_packet_buffer[len] = 0;
    Serial.println("Contents:");
    Serial.println(control_packet_buffer);

    // Send back a status packet.
    udp.beginPacket(udp.remoteIP(), remote_status_port);
    udp.write(status_packet_buffer, 10);
    udp.endPacket();
  }
}
