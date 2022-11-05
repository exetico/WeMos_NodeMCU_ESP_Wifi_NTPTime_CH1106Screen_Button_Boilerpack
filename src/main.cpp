#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <NTPClient.h>   // https://github.com/arduino-libraries/NTPClient
#include <WiFiUdp.h> // https://github.com/esp8266/Arduino (Core Arduino Lib for ESP8266)
#include <ezButton.h> // https://github.com/ArduinoGetStarted/button

// VARS
//-- WIFI
bool __WIFI_CONNECTED = false;
int __TMP_WIFI_RECONNECT_ATTEMPTS = 0;

char __CASES[7][12] = {"CaseNoNo", "Case1", "Case2", "Case3", "Case4", "Case5", "Case6"};

//-- TMP
String __TMP_JSON_RESPONSE_ORIGIN = "";
int __TMP_JSON_RESPONSE_ORIGIN_FIRST_INT = 0;
String __TMP_JSON_RESPONSE_URL = "";
String __TMP_JSON_RESPONSE_HOST = "";
int __TMP_USER_INPUT_1 = 0;
char const *__TMP_USER_INPUT_2 = "";
String __TMP_USER_INPUT_3 = "";

//-- CONFIG
bool __FETCHING_CONFIG = false;
bool __SCREEN_INITATED = 0;

//-- TIME
int __THIS_TIME = 0;
int __TIME_HH = 0;
int __TIME_MM = 0;
int __TIME_SS = 0;
int __TIME_EPOCH = 0;
String __TIME_FORMATTED = "";

//-- BUTTONS
ezButton button(2); // GPIO2 = D4
unsigned long lastCount = 0;
unsigned long count = 0;
unsigned long buttonIsPressed = 0;

unsigned long lastButtonPressTargetMs = 4000;
unsigned long lastButtonPressMillis = millis();
unsigned long lastButtonPressWasLongPress = 0;

// OLED SCREEN
#include <U8g2lib.h>

// Objects
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

// Time
const long utcOffsetInSeconds = 3600;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Download files
#include <WiFiClient.h>
WiFiClientSecure client;
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

// Define path for source file
char const *HTTPS_PATH = "/get";
char const *HTTPS_START = "https://";
char const *CONFIG_HOST = "httpbin.org";

String CONFIG_URL = String(HTTPS_START) + String(CONFIG_HOST) + String(HTTPS_PATH);
const char *URL_COMPLETE = CONFIG_URL.c_str(); // std::string::c_str() will return a non-modifiable standard C character array version of the provided string

// WIFI - WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wm;

void screenPrintText(String text, unsigned long position, unsigned long line, unsigned long cleanScreen)
{
  if (!__SCREEN_INITATED)
  {
    __SCREEN_INITATED = 1;
    // Screen begin
    u8x8.begin();
    u8x8.setFont(u8x8_font_victoriabold8_r);
  }

  if (cleanScreen)
  {
    u8x8.clearDisplay();
  }

  String stringTxt = text;
  u8x8.drawString(position, line, stringTxt.c_str());
}

void logInfo(const String &txt1, const String &txt2)
{
  Serial.println(" ");
  Serial.print(String(txt1));
  Serial.print(String(txt2));
  Serial.println(" ");
}

bool fetchJsonFile(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    delay(300);

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure(); // This IS INSECURE, so please consider to actually validate the HTTPS connection, if you're transfering or requesting something important!
    HTTPClient https;

    String url = CONFIG_URL;
    if (https.begin(*client, CONFIG_URL))
    { // HTTPS
      Serial.println("[HTTPS] GET...");
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server?
        if (httpCode == HTTP_CODE_OK)
        {
          String payload = https.getString();
          Serial.println(String("[HTTPS] Received payload: ") + payload);

          DynamicJsonDocument doc(1024);

          // Deserialize json and create json object
          DeserializationError error = deserializeJson(doc, payload);
          JsonObject obj = doc.as<JsonObject>();

          if (error)
          {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            return 1;
          }

          // Save a few of the values, note that I'm defining them as const char * types
          __TMP_JSON_RESPONSE_ORIGIN = (const char *)obj["origin"];
          __TMP_JSON_RESPONSE_URL = (const char *)obj["url"];
          __TMP_JSON_RESPONSE_HOST = (const char *)obj["headers"]["Host"];

          // Save first number of origin as int
          int NUMBER_LENGTH = 1;
          char FIRST_INT_CHAR[NUMBER_LENGTH];
          strncpy(FIRST_INT_CHAR, (const char *)obj["origin"], NUMBER_LENGTH);
          __TMP_JSON_RESPONSE_ORIGIN_FIRST_INT = atoi(FIRST_INT_CHAR);

          // Print result
          logInfo("__TMP_JSON_RESPONSE_ORIGIN ", __TMP_JSON_RESPONSE_ORIGIN);
          logInfo("__TMP_JSON_RESPONSE_URL ", __TMP_JSON_RESPONSE_URL);
          logInfo("__TMP_JSON_RESPONSE_HOST ", __TMP_JSON_RESPONSE_HOST);
          logInfo("__TMP_JSON_RESPONSE_ORIGIN_FIRST_INT ", String(__TMP_JSON_RESPONSE_ORIGIN_FIRST_INT));
          Serial.println("Is FIRST_INT really a integer? Yep!..." + String(__TMP_JSON_RESPONSE_ORIGIN_FIRST_INT + 1));

          return 0;
        }
      }
      else
      {
        Serial.printf("[HTTPS] GET... failed, error: %s\n\r", https.errorToString(httpCode).c_str());

        return 0;
      }

      https.end();
    }
    else
    {
      Serial.printf("[HTTPS] Unable to connect\n\r");
    }

    return 0;
  }
  else
  {
    __TMP_WIFI_RECONNECT_ATTEMPTS = __TMP_WIFI_RECONNECT_ATTEMPTS + 1;

    if (__TMP_WIFI_RECONNECT_ATTEMPTS > 5)
    {
      // Restart chip
      Serial.println("No wifi. Restart in 2sec");
      screenPrintText("No wifi", 0, 0, 1);
      screenPrintText("Restart in 2sec", 0, 0, 0);
      delay(2000); // Wait a bit
      ESP.restart();
    }

    Serial.println("Wifi Error");
    Serial.println("Trying to turn WiFi Off");
    WiFi.mode(WIFI_OFF); // Disable WiFi
    Serial.println("Set wifi to STA-mode again");
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.println("Wifi Error - Trying to wifi.begin() again");
    WiFi.begin();                   // Start WiFi again
    delay(2000);                    // Wait a bit
    Serial.println(WiFi.localIP()); // Get IP if any
    Serial.println(WiFi.status());  // Get current connection status
    delay(2000);
    return 1;
  }
}

void getJsonConfig(void)
{
  __FETCHING_CONFIG = true;
  while (__FETCHING_CONFIG)
  {
    __FETCHING_CONFIG = fetchJsonFile();
  }
}

// SETUP
void setup()
{
  // Txt
  screenPrintText("Starting...", 0, 0, 1);

  // Buttons
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  button.setCountMode(COUNT_FALLING);

  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(300);

  // Screen
  // initialize with the I2C addr 0x3C
  Serial.println("Screen begin");
  screenPrintText("Hello world...", 0, 0, 1);
  Serial.println("Screen end");
  delay(300);
  u8x8.clearDisplay();

  // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("AutoConnectAP", "boilerpack"); // password protected ap

  if (!res)
  {
    Serial.println("Failed to connect");
    screenPrintText("Wifi Error...", 0, 0, 1);
    // ESP.restart();
  }

  if (res)
  {
    __WIFI_CONNECTED = true;

    // Download json
    Serial.println("JSON Download Begin");
    screenPrintText("JSON DL Begin", 0, 0, 1);
    getJsonConfig();
    if (__TMP_JSON_RESPONSE_ORIGIN && __TMP_JSON_RESPONSE_URL && __TMP_JSON_RESPONSE_HOST)
    {
      Serial.println("JSON Download OK");
    }
    Serial.println("JSON Download End");
    screenPrintText("JSON DL End", 0, 0, 1);
  }
  else
  {
    Serial.println("Configuration portal running");
    screenPrintText("No WiFi", 0, 0, 1);
    screenPrintText("-Restart", 0, 1, 0);
    screenPrintText("-Change SSID", 0, 2, 0);
    screenPrintText("", 0, 5, 0);
    screenPrintText("Didn't create hotspot", 0, 6, 0);
  }
}

// Delays
boolean delayButWithoutDelay(unsigned long time)
{
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  static unsigned long previousmillis = 0;
  unsigned long currentmillis = millis();
  if (currentmillis - previousmillis >= time)
  {
    previousmillis = currentmillis;
    return true;
  }
  return false;
}

boolean delayButWithoutDelay(unsigned long &since, unsigned long time)
{
  // return false if we're still "delaying", true if time ms has passed.
  // this should look a lot like "blink without delay"
  unsigned long currentmillis = millis();
  if (currentmillis - since >= time)
  {
    since = currentmillis;
    return true;
  }
  return false;
}


String command;
void loop()
{
  // Read serial
  if (Serial.available())
  {
    command = Serial.readStringUntil('\n');
    char *commandStr = const_cast<char *>(command.c_str());

    // Print command
    logInfo("Recieved serial command: ", commandStr);

    // Trigger command by typing i followed by your integer
    String command1Identifier = "c";
    if (command.indexOf(command1Identifier) == (0))
    {
      Serial.println(__CASES[1]);
      // Save number defined after "1i". Note that's there's logic to inform the user about invalid inputs
      String strippedCommand = command;
      strippedCommand.replace(command1Identifier, "");
      long number = atol(strippedCommand.c_str());
      __TMP_USER_INPUT_1 = number;
      Serial.println(__TMP_USER_INPUT_1 + 1); // Test if it's parsed a int
      screenPrintText(String(__TMP_USER_INPUT_1), 0, 1, 1);
    }

    // Trigger command by typing 2c, followed by the string you want to save
    String command2Identifier = "2c";
    if (command.indexOf(command2Identifier) == (0))
    {
      Serial.println(__CASES[2]);
      // Remember that __TMP_USER_INPUT_2 is the "char *" type.
      // We'll need to convert the input to our defined type, by creating a char array
      String substringCommand = command.substring(command2Identifier.length(),command.length());
      char strCmd2_array[substringCommand.length()];
      substringCommand.toCharArray(strCmd2_array, substringCommand.length());
      __TMP_USER_INPUT_2 = strCmd2_array;
      Serial.println("2c " + String(__TMP_USER_INPUT_2)); // Combine text and print
      Serial.print("2c ");
      Serial.print(__TMP_USER_INPUT_2); // Print without combining
      screenPrintText(__TMP_USER_INPUT_2, 0, 1, 1);
    }

    // Trigger command by typing 3c, followed by the string you want to save
    if (command.indexOf("3c") == (0))
    {
      Serial.println(__CASES[3]);
      __TMP_USER_INPUT_3 = command; // std::string::c_str() will return a non-modifiable standard C character array version of the provided string
      Serial.println("3c " + command);
      screenPrintText(__TMP_USER_INPUT_3, 0, 1, 1);
      Serial.println(__CASES[1]);
    }
  }

  if (wm.getConfigPortalActive())
  {
    Serial.println("No wifi - Case1....");
    screenPrintText("No wifi Case1", 0, 0, 1);
  }

  if (wm.getWebPortalActive())
  {
    Serial.println("No wifi - Case2....");
    screenPrintText("No wifi Case2", 0, 0, 1);
  }

  // Button counts
  button.loop(); // MUST call the loop() function first

  if (button.isPressed())
  {
    Serial.println("The button is pressed");
    buttonIsPressed = 1;
    // createColor.whiteBoth();
  }

  if (button.isReleased())
  {
    Serial.println("The button is released");
    buttonIsPressed = 0;
    // createColor.off();
  }

  if (lastButtonPressWasLongPress)
  {
    screenPrintText("Long press :)...", 0, 2, 0);
  }

  count = button.getCount();

  if (count != lastCount)
  {
    Serial.println(count);

    int countIn6 = count % 6 + 1;

    lastButtonPressMillis = millis();

    switch (countIn6)
    {
    case 1:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case1", 0, 2, 0);
      break;

    case 2:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case2", 0, 2, 0);
      break;

    case 3:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case3", 0, 2, 0);
      break;

    case 4:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case4", 0, 2, 0);
      break;

    case 5:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case5", 0, 2, 0);
      break;

    case 6:
      Serial.println(__CASES[countIn6]);
      screenPrintText("Action: Case6", 0, 2, 0);
      break;
    }

    lastCount = count;
  }

  // Loop, different timeperiods
  static unsigned long buttontime, clocktime;

  if (delayButWithoutDelay(buttontime, 500))
  {
    // Check if button press has been too long time ago
    unsigned long currentButtonmillis = millis();
    if (button.getCount() > 0 && currentButtonmillis - lastButtonPressMillis >= lastButtonPressTargetMs)
    {
      // Reset count
      button.resetCount();
      Serial.print("ResetButtonCount");

      // Set state of longpress button
      if (buttonIsPressed)
        lastButtonPressWasLongPress = 1;
    }
  }
  // Every 1000
  if (delayButWithoutDelay(clocktime, 1000))
  {
    // Update time
    timeClient.update();
    __TIME_HH = timeClient.getHours();
    __TIME_MM = timeClient.getMinutes();
    __TIME_SS = timeClient.getSeconds();
    __TIME_EPOCH = timeClient.getEpochTime();
    __TIME_FORMATTED = timeClient.getFormattedTime();
  }
}
