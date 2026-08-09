#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <Adafruit_Sensor.h>

String item = "0";
const char* ssid = "Ruquiya";//change to your own WIFI name
const char* password = "244466666";// change the WIFI password to your own

WiFiServer server(80);
volatile int wifi_mark = 0, wifi_time = 0;
volatile int RGB_RED = 0, RGB_GREEN = 0, RGB_BLUE = 0;
bool wifimode_flag = 1;
//WIFI configuration

#include <HardwareSerial.h>//calling libraries for hard serial ports
volatile int radio;//variables for storing serial voice signals
volatile int radio_mark = 0, radio_time = 0;
//configuration of the voice module

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel rgb_display_16 = Adafruit_NeoPixel(10, 16, NEO_GRB + NEO_KHZ800);
volatile int buttun;
bool LED_flag = 0;
//configuration of variables for buttons and RGB

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
const int i2cAddress = 0x27; //  I2C address of LCD1602
const int numRows = 2;      // number of LCD1602 rows
const int numCols = 16;     // number of columns of LCD1602
hd44780_I2Cexp lcd(i2cAddress, numRows, numCols); // create the LCD1602 object
#include <DHT.h>
DHT dht25(25, 11);
//configuration of LCD screen and temperature/humidity module

#include <ESP32_Servo.h>
Servo servo_17;
const int window_close = 60;
const int window_open = 135;
bool window_flag = 0;
//window servo configuration

#include <MFRC522_I2C.h>
MFRC522_I2C mfrc522(0x28, -1);
String rfid_str = "";
Servo servo_18;
const int door_close = 5;
const int door_open = 105;
//RFID module configuration and door configuration

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  delay(1000);

  Serial2.begin(115200, SERIAL_8N1, 13, 27); //configure rx to be 13 pins, tx to be 27 pins, and the serial port name of the voice module is Serial2
  radio = 0;//store the data from the voice serial port into the radio variable
  //initialization of the serial port of the voice module

  pinMode(14, INPUT);
  pinMode(32, INPUT);
  pinMode(5, OUTPUT);
  //initialization of human infrared, photosensitive sensors, and LED pins

  pinMode(26, INPUT);
  pinMode(23, OUTPUT);
  rgb_display_16.begin();
  buttun = 0;
  //initialization of pushbuttons, laser lights, and abyssal lights (light strips)

  Wire.begin();
  lcd.begin(numCols, numRows); // initialize LCD1602
  lcd.backlight();              // turn on the backlight
  delay(500);
  lcd.clear();
  pinMode(25, INPUT);
  dht25.begin();
  // initialization of LCD screen and temperature/humidity module

  mfrc522.PCD_Init();
  servo_18.attach(18, 500, 2500);
  servo_18.write(door_close);
  delay(500);
  //initialization of gate servo and  RFID initialization

  pinMode(33, INPUT);
  servo_17.attach(17, 500, 2500);
  servo_17.write(window_open);
  delay(500);
  //initialization of raindrop sensor and window servos

  delay(2000);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print(WiFi.localIP());
    server.begin();
    Serial.println("TCP server started");
    MDNS.addService("http", "tcp", 80);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Wifi failed!");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("Wifi failed!");
  }
  //WIFI initialization
}

void loop()
{
  wifi_controll();
  radio_controll();
  if ((wifi_mark == 0) && (radio_mark == 0))
  {
    LED_dark();
    two_light();
    LCD_TEMP_HUMI();
    raindrop_windows();
    RFID_door();
  }
  //no data from wifi detected here.
  delay(50);
}

/****** human night light program 001******/
void LED_dark()
{
  if ((digitalRead(14) == 1) || (analogRead(32) > 3000)) {
    digitalWrite(5, HIGH);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("LED_on");
    delay(1000);
    LED_flag = 1;
  } else if ( LED_flag == 1) {
    LED_flag = 0;
    digitalWrite(5, LOW);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("LED_off");
  }
}
/****** human night light program 001******/

/****** button abyssal light and laser program 002 ******/
void two_light()
{
  if (digitalRead(26) == 0)
  {
    while (digitalRead(26) == 0)
    {
      delay(200);
    }
    buttun++;
    if ((long) (buttun) % (long) (2) == 1)
    {
      digitalWrite(23, HIGH);
      rgb_display_16.setBrightness(100);//configure Brightness
      rgb_all(10, 0, 0, 200); //configure the color
      rgb_display_16.show();//letting the configuration take effect
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("RGB_on");
    }
    else if ((long) (buttun) % (long) (2) == 0)
    {
      digitalWrite(23, LOW);
      rgb_display_16.setBrightness(0);
      rgb_display_16.show();
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("RGB_off");
    }
  }
}
void rgb_all(int j, int rgb_r, int rgb_g, int rgb_b)
{
  for (int i = 1; i <= j; i++)
  {
    rgb_display_16.setPixelColor((i) - 1, (((rgb_r & 0xffffff) << 16) | ((rgb_g & 0xffffff) << 8) | rgb_b));
  }
}
//simplifying the control program for abyssal lamps using program subfunctions
/****** button abyssal light and laser program 002 ******/

/****** raindrop window opening program 003******/
void raindrop_windows()
{
  if (analogRead(33) > 3000)
  {
    servo_17.write(window_close);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("window_close");
    delay(1000);
    window_flag = 1;
  } else if (window_flag == 1)
  {
    window_flag = 0;
    servo_17.write(window_open);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("window_open");
  }
}
/****** raindrop window opening program 003******/

/******LCD temperature and humidity display 004******/
void LCD_TEMP_HUMI()
{
  lcd.setCursor(1 - 1, 1 - 1);
  lcd.print("T:");
  lcd.setCursor(3 - 1, 1 - 1);
  lcd.print(dht25.readTemperature());
  lcd.setCursor(9 - 1, 1 - 1);
  lcd.print("H:");
  lcd.setCursor(11 - 1, 1 - 1);
  lcd.print(dht25.readHumidity());
}
/******LCD temperature and humidity display 004******/

/******RFID door opener program 005******/
void RFID_door()
{
  if ( ! mfrc522.PICC_IsNewCardPresent() || ! mfrc522.PICC_ReadCardSerial() )
  {
    delay(50);
    return;
  }
  rfid_str = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    rfid_str = rfid_str + String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println(rfid_str);
  servo_18.write(door_open);
  lcd.setCursor(1 - 1, 2 - 1);
  lcd.print("door open  ");
  lcd.setCursor(1 - 1, 2 - 1);
  lcd.print("door_open");
  digitalWrite(19, HIGH);
  delay(2000);
  servo_18.write(door_close);
  lcd.setCursor(1 - 1, 2 - 1);
  lcd.print("               ");
  lcd.setCursor(1 - 1, 2 - 1);
  lcd.print("door_close");
  digitalWrite(19, LOW);
}
/******RFID door opener program 005******/

/*****wifi control program 006*****/
void wifi_controll()
{
  if ((wifi_time != 0) && ((millis() - wifi_time) > 2000))
  {
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("auto_mode");
    Serial.println("auto_mode");
    wifi_mark = 0;
    wifi_time = 0;
  }
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
  while (client.connected() && !client.available())
  {
    delay(1);
  }
  String req = client.readStringUntil('\r');
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1)
  {
    Serial.print("Invalid request: ");
    Serial.println(req);
    return;
  }
  req = req.substring(addr_start + 1, addr_end);
  item = req;
  wifi_mark = 1;
  wifi_time = millis();
  if ( wifimode_flag == 1 )
  {
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("wifi_mode");
    wifimode_flag = 0;
  }
  Serial.println(item);
  String s;
  if (req == "/")  //browser accesses address can read the information sent by 'the client.println(s);'
  {
    IPAddress ip = WiFi.localIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>Hello from ESP32 at ";
    s += ipStr;
    s += "</html>\r\n\r\n";
    Serial.println("Sending 200");
    client.println(s);  //send the string S, you can read the information when visiting the address of E smart home using a browser
  }
  if (req == "/Test")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += "Test_OK";
    s += "r\n\r\n";
    client.println(s);
  }
  if (req == "/LED_on") //browser accesses the ip address/led/on
  {
    digitalWrite(5, HIGH);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("LED_on");
    client.println("turn on the LED");
  }
  if (req == "/LED_off")
  {
    digitalWrite(5, LOW);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("LED_off");
    client.println("turn off the LED");
  }
  if (req == "/laser_on")
  {
    digitalWrite(23, HIGH);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("laser_on");
    client.println("turn on the jiguang");
  }
  if (req == "/laser_off")
  {
    digitalWrite(23, LOW);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("laser_off");
    client.println("turn off the jiguang");
  }
  if (req == "/RGB_on")
  {
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("RGB_on");
    client.println("turn on the shenyuan");
  }
  if (req == "/RGB_off")
  {
    rgb_display_16.setBrightness(0);
    rgb_display_16.show();
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("RGB_off");
    client.println("turn off the shenyuan");
  }
  if (hasNumber(req) == 1)
  {
    Serial.println("hasNumber(req)==1");
    char result[10];
    extractCharacters(req, result);
    Serial.println(result);
    if (strcmp(result, "RGB_B") == 0)
    {
      RGB_BLUE = extractNumbers(req);
      Serial.println("result==RGB_B");
    }
    else if (strcmp(result, "RGB_G") == 0)
    {
      RGB_GREEN = extractNumbers(req);
      Serial.println("result==RGB_G");
    }
    else if (strcmp(result, "RGB_R") == 0)
    {
      RGB_RED = extractNumbers(req);
      Serial.println("result==RGB_R");
    }
    rgb_display_16.setBrightness(100);
    rgb_all(10, RGB_RED, RGB_GREEN, RGB_BLUE);
    rgb_display_16.show();
    Serial.println(RGB_GREEN);
    Serial.println(RGB_BLUE);
    Serial.println(RGB_RED);
  }
  if (req == "/door_on")
  {
    servo_18.write(door_open);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("door_open");
    client.println("turn on the door");
  }
  if (req == "/door_off")
  {
    servo_18.write(door_close);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("door_close");
    client.println("turn off the door");
  }
  if (req == "/window_on")
  {
    servo_17.write(window_open);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("window_open");
    client.println("turn on the window");
  }
  if (req == "/window_off")
  {
    servo_17.write(window_close);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("window_close");
    client.println("turn off the window");
  }
  if (req == "/bee_on")
  {
    digitalWrite(19, HIGH);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("bee_on");
    client.println("turn on the bee");
  }
  if (req == "/bee_off")
  {
    digitalWrite(19, LOW);
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("bee_off");
    client.println("turn off the bee");
  }
  if (req == "/raindrop_req")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += "raindrop_" + String(analogRead(33));
    s += "\r\n\r\n";
    client.println(s);
  }
  if (req == "/brightness_req")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += String("brightness_") + String(analogRead(32));
    s += "\r\n\r\n";
    client.println(s);
  }
  if (req == "/PIR_req")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += "PIR_" + String(digitalRead(14));
    s += "\r\n\r\n";
    client.println(s);
  }
  if (req == "/temperature_req")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += "temperature_" + String(dht25.readTemperature());
    s += "\r\n\r\n";
    client.println(s);
  }
  if (req == "/humidity_req")
  {
    s = "HTTP/1.1 200 0K\r\nContent-Type: text \r\n\r\n ";
    s += "humidity_" + String(dht25.readHumidity());
    s += "\r\n\r\n";
    client.println(s);
  }
  client.stop();
}

int hasNumber(String str)
{
  int i = 0;
  while (str[i] != '\0')
  {
    if (str[i] >= '0' && str[i] <= '9')
    {
      return 1;  // contains numbers
    }
    i++;
  }
  return 0;// does not contain numbers
}

int extractNumbers(String str)
{
  //  iterate over each character in the string
  int i = 0, numbers = 0;
  while (str[i] != '\0')
  {
    //if the character is a number, it is converted to an integer and added to numbers
    if (str[i] >= '0' && str[i] <= '9')
    {
      numbers = (numbers * 10) + (str[i] - '0');
    }
    i++;
  }
  return numbers;
}

void extractCharacters(String str, char *result)
{
  int startIndex = 0;
  int endIndex = 0;
  // find character start position
  while (str[startIndex] != '/' && str[startIndex] != '\0')
  {
    startIndex++;
  }
  // find end-of-character position
  endIndex = startIndex + 1;
  while (str[endIndex] != '/' && str[endIndex] != '\0')
  {
    endIndex++;
  }
  // copy the character part to the result string
  int i;
  for (i = startIndex + 1; i < endIndex; i++)
  {
    result[i - startIndex - 1] = str[i];
  }
  result[i - startIndex - 1] = '\0';
}
/*****wifi control program 006*****/

/***** voice control program 007*****/
void radio_controll()
{
  if ((radio_time != 0) && ((millis() - radio_time) > 2000))
  {
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("auto_mode");
    Serial.println("auto_mode");
    radio_mark = 0;
    radio_time = 0;
  }
  if (Serial2.available() > 0) //whether the voice serial port is receiving data
  {
    radio = Serial2.read();//storing data from the voice serial port into the radio variable
    Serial.println(radio, HEX); //use the serial port on the computer side to send the value in hexadecimal form to the computer's serial monitor
    radio_mark = 1;
    radio_time = millis();
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("               ");
    lcd.setCursor(1 - 1, 2 - 1);
    lcd.print("radio_mode");
    Serial.println("radio_mode");
    if (radio == 0x02)
    {
      servo_18.write(door_open);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("door_open");
    }
    if (radio == 0x03)
    {
      servo_18.write(door_close);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("door_close");
    }
    if (radio == 0x04)
    {
      servo_17.write(window_open);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("window_open");
    }
    if (radio == 0x05)
    {
      servo_17.write(window_close);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("window_close");
    }
    if (radio == 0x06)
    {
      digitalWrite(19, HIGH);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("bee_on");
    }
    if (radio == 0x07)
    {
      digitalWrite(19, LOW);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("bee_off");
    }
    if (radio == 0x08)
    {
      digitalWrite(5, HIGH);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("LED_on");
    }
    if (radio == 0x09)
    {
      digitalWrite(5, LOW);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("LED_off");
    }
    if (radio == 0x0A)
    {
      digitalWrite(23, HIGH);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("laser_on");
    }
    if (radio == 0x0B)
    {
      digitalWrite(23, LOW);
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("laser_off");
    }
    if (radio == 0x0C)
    {
      rgb_display_16.setBrightness(100);//configuration brightness
      rgb_all(10, 0, 0, 200); //configuration color
      rgb_display_16.show();//letting the configuration take effect
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("RGB_on");
    }
    if (radio == 0x0D)
    {
      rgb_display_16.setBrightness(0);//configuration brightness
      rgb_display_16.show();//letting the configuration take effect
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("               ");
      lcd.setCursor(1 - 1, 2 - 1);
      lcd.print("RGB_off");
    }
  }
}
/***** voice control program 007*****/