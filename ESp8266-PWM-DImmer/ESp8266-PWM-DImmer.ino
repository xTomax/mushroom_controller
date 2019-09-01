#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "ESP8266Dimmer.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN D3
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);
char data[100];

String resp = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
<style>.slidecontainer {width: 100%;}.slider { -webkit-appearance: none;width: 100%;height: 25px;\
background: #d3d3d3;outline: none;opacity: 0.7;-webkit-transition: .2s;transition: opacity .2s;}.slider:hover {opacity: 1;}\
.slider::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 25px;height: 25px;background: #4CAF50;cursor: pointer;}\
.slider::-moz-range-thumb {width: 25px;height: 25px;background: #4CAF50;cursor: pointer;}\
.button3{display:inline-block;padding:0.5em 1.2em;margin:0 0.5em 0.5em 0;border-radius:2em;box-sizing: border-box;text-decoration:none;font-family:'Roboto',sans-serif;\
font-weight:300;color:#FFFFFF;background-color:#4eb5f1;text-align:center;transition: all 0.2s;}button3:hover{background-color:#4095c6;}@media all and (max-width:30em){\
.button3{display:block;margin:0.2em auto;}}\ 
</style></head><body>\
<h1>Selecionar percentagem</h1><FORM action=\"/\" method=\"post\"><div class=\"slidecontainer\"><input type=\"range\" min=\"1\" max=\"100\" value=\"50\" class=\"slider\" name=\"inputRange\" id=\"myRange\">\
<p>Value: <span id=\"demo\"></span></p><INPUT type=\"submit\" class=\"button3\" value=\"Enviar\"></FORM></div>\
<FORM action=\"/ON\" method=\"post\">\
<INPUT type=\"submit\" class=\"button3\" value=\"ON\"></FORM>\
<FORM action=\"/OFF\" method=\"post\">\
<INPUT type=\"submit\" class=\"button3\" value=\"OFF\"></FORM>";

char dhtInfo[] = "<p>Temp: %.2f</p></br><p>Humidity: %.2f</p>";

String lastPart = "<script>var slider = document.getElementById(\"myRange\");var output = document.getElementById(\"demo\");\
output.innerHTML = slider.value;slider.oninput = function() {output.innerHTML = this.value;}</script></body></html>";

unsigned long startedAt = millis();
void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();

    wifiManager.autoConnect("BootAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    Serial.print("After waiting ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited/1000);
    Serial.print(" secs in setup() connection result is ");
    Serial.println(connRes);
    if (WiFi.status()!=WL_CONNECTED)
    {
        Serial.println("failed to connect, finishing setup anyway");
    } 
    else
    {
      Serial.print("local ip: ");
      Serial.println(WiFi.localIP());
    }

    
    server.on("/", handleRootPath);    //Associate the handler function to the path
    server.on("/ON", handleON);    //Associate the handler function to the path
    server.on("/OFF", handleOFF);    //Associate the handler function to the path
    server.onNotFound(handleNotFound);
    server.begin();                    //Start the server
    Serial.println("Server listening on port 80");
    initDimmer();
    dht.begin();
}

void loop() {
    server.handleClient();         //Handling of incoming requests  
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}
void handleOFF(){
  setOff();
  server.send(200, "text/html", resp);
}
void handleON(){
  setOn();
  server.send(200, "text/html", resp);
}
void handleSubmit()
{
  String LEDvalue;
  if (!server.hasArg("inputRange")) return returnFail("BAD ARGS");
  //TODO: Alterar aqui o valor do pwm!!
  setDimmerVal(server.arg("inputRange").toInt());
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  sprintf(data, dhtInfo, t, h);
  server.send(200, "text/html", resp + String(data) + lastPart);
}

void handleRootPath() {
  if (server.hasArg("inputRange")) {
    handleSubmit();
  }
  else {
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    sprintf(data, dhtInfo, t, h);
    server.send(200, "text/html", resp + String(data) + lastPart);
  }
}
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/html", message);
}
