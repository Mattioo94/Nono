
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h> 

//================================================================================================
// NETWORK
//================================================================================================

const char* ssid = "Nono";
const char* password = "12345678";

ESP8266WebServer server(80);

//================================================================================================
// SERVOS
//================================================================================================

Servo servoH; // HORIZONTAL
Servo servoV; // VERTICAL

//================================================================================================
// GPIO's
//================================================================================================

int LEFT_PWM_PIN = 4;
int LEFT_GPIO_PIN = 5;

int RIGHT_PWM_PIN = 13;
int RIGHT_GPIO_PIN = 15;

int SERVO_HORIZONTAL_PIN = 14;
int SERVO_VERTICAL_PIN = 16;

//================================================================================================
// CURRENT VALUES
//================================================================================================

int LEFT_VALUE = 0;
int RIGHT_VALUE = 0;
int COURSE_VALUE = 1;

int SERVO_HORIZONTAL_VALUE = 90;
int SERVO_VERTICAL_VALUE = 90;

//================================================================================================
// SETUP
//================================================================================================

void setup(void)
{
  pinMode(LEFT_PWM_PIN, OUTPUT);
  pinMode(LEFT_GPIO_PIN, OUTPUT);
  
  pinMode(RIGHT_PWM_PIN, OUTPUT);
  pinMode(RIGHT_GPIO_PIN, OUTPUT);
  
  digitalWrite(LEFT_PWM_PIN, LOW);
  digitalWrite(LEFT_GPIO_PIN, LOW);
  
  digitalWrite(RIGHT_PWM_PIN, LOW);
  digitalWrite(RIGHT_GPIO_PIN, LOW);

  servoH.attach(SERVO_HORIZONTAL_PIN);
  servoV.attach(SERVO_VERTICAL_PIN);

  //----------------------------------------------------------------------------------------------

  servoH.write(SERVO_HORIZONTAL_VALUE);
  servoV.write(SERVO_VERTICAL_VALUE);
  
  server.on("/", HTTP_POST, []()
  {
    if(server.hasArg("plain"))
    {  
      DynamicJsonBuffer bufor;
      JsonObject& json = bufor.parseObject(server.arg("plain"));

      if(json.success())
      {
        Configure(json);
      }
    }
  });
  
  server.begin();

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  WiFi.softAP(ssid, password);

  Serial.begin(115200);
  Serial.println("\n\nWiFi signal strength (RSSI): " + String(WiFi.RSSI()) + " dBm");
}

//================================================================================================
// LOOP
//================================================================================================

void loop(void)
{
  server.handleClient();
}

//================================================================================================
// JSON REQUEST - TRYING UPDATE VALUES OF PWM's
//================================================================================================

void Configure(JsonObject& json)
{
  if(json.containsKey("Nono") && json["Nono"].is<JsonArray&>())
  { 
    JsonArray& CONFIG = json["Nono"].asArray();
    if(CONFIG.size() == 5 && CONFIG[0].is<int>() && CONFIG[1].is<int>() && CONFIG[2].is<int>() && CONFIG[3].is<int>() && CONFIG[4].is<int>())
    {
      int LEFT = CONFIG[0];
      int RIGHT = CONFIG[1];
      int COURSE = CONFIG[2];

      int SERVO_HORIZONTAL = CONFIG[3];
      int SERVO_VERTICAL = CONFIG[4];
      
      if(LEFT >= 0 && LEFT <= 1024 && RIGHT >= 0 && RIGHT <= 1024 && COURSE >= 0 && COURSE <= 1 && SERVO_HORIZONTAL >= 0 && SERVO_HORIZONTAL <= 180 && SERVO_VERTICAL >= 0 && SERVO_VERTICAL <= 180)
      {
        LEFT_VALUE = LEFT;
        RIGHT_VALUE = RIGHT;
        COURSE_VALUE = COURSE;

        SERVO_HORIZONTAL_VALUE = SERVO_HORIZONTAL;
        SERVO_VERTICAL_VALUE = SERVO_VERTICAL;

        digitalWrite(RIGHT_GPIO_PIN, COURSE_VALUE == 0 ? LOW : HIGH);
        digitalWrite(LEFT_GPIO_PIN, COURSE_VALUE == 0 ? LOW : HIGH);

        analogWrite(RIGHT_PWM_PIN, COURSE_VALUE == 0 ? LEFT_VALUE : 1024 - LEFT_VALUE);
        analogWrite(LEFT_PWM_PIN, COURSE_VALUE == 0 ? RIGHT_VALUE : 1024 - RIGHT_VALUE);

        servoH.write(SERVO_HORIZONTAL_VALUE);
        servoV.write(SERVO_VERTICAL_VALUE);

        String json = "{\"Nono\":[" + String(LEFT_VALUE) + ", " + String(RIGHT_VALUE) + ", " + String(COURSE_VALUE) + ", " + String(SERVO_HORIZONTAL_VALUE) + ", " + String(SERVO_VERTICAL_VALUE) + "]}";  
        server.send( 200, "text/json", json );
      }
    }
  }
}
