// sensor
/*  pm5003T 線 腳位 (TX綠 接 Arduino pin D2(rx) )
 *                   (SET白 接 Arduino pin D4也可不接)
 vcc GND  SET  RX  TX  RESET
 紫  橘   白   藍  綠  黃
*/
#include <SoftwareSerial.h>
#include <SPI.h>
#include <WiFi.h>
#include <LWiFi.h>
#include <WiFiClient.h>
#include <string.h>
#define TIMELIMIT 867000

int ID = 6;
char ssid[] = "arch-5107";          // your network SSID (name)
//char ssid[] = "CSIE-WLAN";          // your network SSID (name)
//char pass[] = "ncku46545";
//char pass[] = "wificsie";     // your network password (use for WPA, or use as key for WEP)
#define TCP_IP "140.116.82.93"
#define TCP_PORT 82
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;
static int messageLen;

SoftwareSerial mySerial(6, 7);//rx,tx
long pmcf10=0;
long pmcf25=0;
long pmcf100=0;
long pmat10=0;
long pmat25=0;
long pmat100=0;
long pm03PNO =0;
long pm05PNO =0;
long pm10PNO =0;
long pm25PNO =0;
long Temperature =0;
long Humidity =0;

long pm10;
long pm25;
long pm100;
long temp;
long hum;

char recv_buf[1024];
char send_buf[1024];
char send_msg[1024];

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    mySerial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    turn_on_sensor();

    connectWIFI();
    connectServer();
    
    Serial.println("message received");
}
/*u8g.firstPage();  
    do {
      draw();
      u8g.setFont(u8g_font_5x8);
      u8g.drawStr(0, 52, "(Normal)");
      } while( u8g.nextPage() );
*/

void loop() {
  // read data
  read_data();
  
  // check wifi status, dc then re-connect
  //Serial.println(WiFi.status());
  if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi has disconnected. Re-connecting...");
      status = WL_IDLE_STATUS;
      connectWIFI();
      connectServer();
  }
  // check server status, dc then re-connect
  //Serial.println(wifiClient.status());
  if(!wifiClient.connected()){
      connectServer();
  }
  digitalWrite(LED_BUILTIN, HIGH);
  // sending data message
  sprintf(send_msg, "{ 'pm10': %d, 'pm25': %d, 'pm100': %d, 'temp': %d, 'humidity': %d, 'position': %d }", pm10, pm25, pm100, temp, hum, ID);
  Serial.println(send_msg);
  if(pm10 != 0 && pm25 != 0 && pm100 != 0 && temp != 0 && hum != 0){
    send_mes(send_msg);
    pm10 = 0;
    pm25 = 0;
    pm100 = 0;
    temp = 0;
    hum = 0;
    digitalWrite(4, LOW);
    wifiClient.stop();
    // send every 9.5 mins
    delay(TIMELIMIT);
    turn_on_sensor();
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void read_data(){
    int count = 0;
    unsigned char c;
    unsigned char high;
    
    while (mySerial.available()) {
    c = mySerial.read();
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
      Serial.println("check failed");
      break;
    }
    if(count > 40){
      Serial.println("complete");
      break;
    }
    else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14|| count == 16|| count == 18|| count == 20|| count == 22|| count == 24|| count == 26|| count == 28 ) high = c;
    else if(count == 5){
      pmcf10 = 256*high + c;
    }
    else if(count == 7){
      pmcf25 = 256*high + c;
    }
    else if(count == 9){
      pmcf100 = 256*high + c;
    }
    else if(count == 11){
      pmat10 = 256*high + c;
      pm10 = pmat10;
    }
    else if(count == 13){
      pmat25 = 256*high + c;
      pm25 = pmat25;
    }
    else if(count == 15){
      pmat100 = 256*high + c;
      pm100 = pmat100;
    }
    else if(count == 17){
      pm03PNO= 256*high + c;
    }
    else if(count == 19){
      pm05PNO = 256*high + c;
    }
    else if(count == 21){
      pm10PNO = 256*high + c;
    }
    else if(count == 23){
      pm25PNO = 256*high + c;
    }
    else if(count == 25){
      Temperature = 256*high + c;
      temp = Temperature/10;
    }
    else if(count == 27){
      Humidity = 256*high + c;
      hum = Humidity/10;
    }
    count++;
  }
  while(mySerial.available()) mySerial.read();
}

void turn_on_sensor(){
  digitalWrite(4, HIGH);
  Serial.println("delay 30 sec");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(30000);
  digitalWrite(LED_BUILTIN, LOW);
}

void connectWIFI(){
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        // status = WiFi.begin(ssid, pass);
        status = WiFi.begin(ssid);
    
        // wait 2 seconds for connection:
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
        
    }
    
    Serial.println("Connected to wifi");
    //printWifiStatus();
    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
}

void connectServer(){
    //attempt to connect to server
    while (!wifiClient.connect(TCP_IP, TCP_PORT)){
        delay(5000);
        Serial.print("Attempting to connect to SERVER: ");
        Serial.println(TCP_IP);
        
        digitalWrite(LED_BUILTIN, HIGH);
        delay(300);
        digitalWrite(LED_BUILTIN, LOW);
        delay(300);
    }
    
    Serial.println("connected to server");
    recv_mes();
}


void send_mes(char mes[]){
    sprintf(send_buf,"%s", mes);
    wifiClient.write(send_buf, strlen(send_buf));
    wifiClient.flush();
    sprintf(send_buf, "");
}

void recv_mes(){
  while(1){
    if ((messageLen = wifiClient.available()) > 0) {
          int i = 0;
          do{
              recv_buf[i++] = wifiClient.read();
          } while(i<200 && recv_buf[i-1]!='\r' && recv_buf[i-1]!='\0');
          recv_buf[i-1] = '\0';
          Serial.println("Received message:");
          Serial.println(recv_buf);
          break;
    }
    else {
      Serial.println("no received message");
      delay(1000);
    }
  }
}
