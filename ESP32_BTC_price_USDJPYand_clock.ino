/********************************************
     ESP32 ADD Realtime Clock(JST)
     Prints current time on real time
     Forked from https://github.com/joysfera/esp32-btc
     Forked by kitora12 in '21 JAN 4
     Rebuild by kitora12 in '22 JAN 8
     https://github.com/kitora12/esp32-btc
     released under the GNU GPL

     Tested board manager: Arduino core for the ESP32 2.0.2
     Tested boards: ESP32 DevModule, NODE32S
*********************************************/

/*BASE*/
/********************************************
    ESP32 LCD TFT / OLED display demo
    Prints current time and BTC price in USD
    written by Petr Stehlik in 2019/07/25
    released under the GNU GPL
    https://github.com/joysfera/
*********************************************/

#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <NTPClient.h>  // https://github.com/arduino-libraries/NTPClient
#include <Tasker.h>     // https://github.com/joysfera/arduino-tasker


# include <Wire.h>
# include <Adafruit_GFX.h>
# include <Adafruit_SSD1306.h>


WiFiClientSecure client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Tasker tasker;

Adafruit_SSD1306 disp(128, 64, &Wire, -1);


const char* ssid     = "*********"; // your network SSID (name of wifi network)
const char* password = "*********"; // your network password
const char* server = "api.coinbase.com";  // Server URL

int btcvalue = 888888;

void setup()
{
    Serial.begin(115200);
    delay(100);
    pinMode(2,OUTPUT);//GPIO2 BLUE LED@Hiletgo esp-32
    digitalWrite(2,HIGH);
    delay(1000);
    digitalWrite(2,LOW);

    Wire.begin(21, 22); //SDA SCL
    if (!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 init failed"));
        for (;;) ; // no display no fun
    }
    disp.setTextColor(WHITE);

    disp.print("Connecting to WiFi");
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        disp.print('.');
        Serial.print('.');
        // wait 1 second for re-trying
        delay(1000);
    }
    Serial.println(" OK::::::");
    Serial.println(ssid);

    timeClient.begin();
    timeClient.setTimeOffset(3600 * 9); //  JST 3600 * 9 (1h *9 )  
                                        //  CST 3600 * 2
    timeClient.update();

    displayClock();
    getBTC();

}

int posX1 = 129;
int posX2 = 129;
int posTIME =1;
int posLRSW=1;
int posTIMESPEEDCOUNT=0;
//move L =1  move R=0



void loop()
{
    timeClient.update();                  // keep time up-to-date 
    tasker.loop();

  
    if (digitalRead(0) == LOW)            // if button on GPIO0 is pressed then
        tasker.setTimeout(displayClock, 250); // update display almost instantly
}

void displayClock(void)
{
disp.clearDisplay();

//TIME Scroll
posTIMESPEEDCOUNT++;
if(posTIMESPEEDCOUNT>100){ //Speed Control

  switch(posLRSW){
    case 0:
    posTIME--;
        if(posTIME<0){
        posLRSW=1;
        }
    break;
  
    case 1:
    posTIME++;
        if(posTIME>30){
        posLRSW=0;
        }
    break;
    }

  
posTIMESPEEDCOUNT=0;
}
//Serial.println((String)"POSITION:" + posTIME + " LorR:" + posLRSW);

//Price Scroll
    posX1--;
    if(posX1<=0){
      posX1=129;
      }
    if(posX2<0 && posX2<12){
      posX2--;
      }

    //first disp. With clock 
    disp.setTextWrap(0);
    disp.setTextSize(2);
    disp.setCursor(1 + posTIME , 1);
    disp.print(timeClient.getFormattedTime());
    disp.setCursor(1 + posX1,20);
    disp.print("BTC");
    disp.setTextSize(3);
    disp.setCursor(1 + posX1,40);
    disp.print('$');
    disp.print(btcvalue);

    //second disp. No clock.

    disp.setTextSize(2);
    //disp.setCursor(posX1-128, 1);
    //disp.print(timeClient.getFormattedTime());
    disp.setCursor(posX1-128,20);
    disp.print("BTC");
    disp.setTextSize(3);
    disp.setCursor(posX1-128,40);
    disp.print('$');
    disp.print(btcvalue);    
    disp.display();

    tasker.setTimeout(displayClock, 1);    // next round in 100msec
}

void getBTC(void)
{
    digitalWrite(2,HIGH);
    disp.invertDisplay(1); 
     
    int btc = 0;
    Serial.println("\nStarting connection to server...");
    uint32_t time_out = millis();
    
while(1){
  client.setInsecure();//skip verification
  client.setHandshakeTimeout(30);
    if (!client.connect(server, 443)) {
        Serial.println("Connection failed!");
        //btc = -1;
    }
    else {
        Serial.println("Connected to server!");
        // Make a HTTP request:
        client.println("GET https://api.coinbase.com/v2/prices/BTC-USD/spot HTTP/1.0");
        client.println("Host: api.coinbase.com");
        client.println("Connection: close");
        client.println();
        //client.flush();

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") break; // end of HTTP headers
        }
        while (client.available()) {
            String line = client.readStringUntil('\n');
            int a = line.indexOf("amount");       // naive parsing of the JSON reply
            if (a > 0) {
                String amount = line.substring(a + 9);
                btc = amount.toInt();
                Serial.print("BTC = $");
                Serial.println(btc);
            }
            
        }
        delay(10);
        client.stop();
        delay(10);
        Serial.print("Connection Close");
        break;        
    }
if( ( millis() - time_out ) > 200 ) break;
delay(10);
}   
    //return btc;
    btcvalue=btc;
    tasker.setTimeout(getBTC, 120000);    // next round in 120 seconds 
    digitalWrite(2,LOW);
    disp.invertDisplay(0); 

}
