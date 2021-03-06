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
#include <LTimer.h>
#include <LRTC.h>

LTimer timer0(LTIMER_0);
bool timeout = false;
int ID = 8;
char ssid[] = "CSIE-WLAN";          // your network SSID (name)
char pass[] = "wificsie";     // your network password (use for WPA, or use as key for WEP)
#define TCP_IP "140.116.82.93"
#define TCP_PORT 82
#define TIMELIMIT 595000
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

    LRTC.begin();
    LRTC.set(2019, 4, 14, 2, 0, 0);
    // turn on the timer
    timer0.begin();
    timer0.start(TIMELIMIT, LTIMER_REPEAT_MODE, _callback0, NULL);
    
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    Serial.println("delay 40 sec");
    delay(40000);
    
    connectWIFI();
    connectServer();
    
    // Send the first data
    Serial.println("First send");
    read_data();
    if(pm10 == 0 && pm25 == 0 && pm100 == 0 && temp == 0 && hum == 0){
      Serial.println("in zero if");
      while(pm10 == 0){        
        Serial.println("in while");
        delay(1000);
        read_data();
      } 
    }
    char buffer[64];
    LRTC.get();
    sprintf(buffer, "%ld/%ld/%ld %.2ld:%.2ld:%.2ld", LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());
    Serial.println(buffer);
    sprintf(send_msg, "{ 'pm10': %d, 'pm25': %d, 'pm100': %d, 'temp': %d, 'humidity': %d, 'position': %d }", pm10, pm25, pm100, temp, hum, ID);
    Serial.println(send_msg);
    send_mes(send_msg);
    
    pm10 = 0;
    pm25 = 0;
    pm100 = 0;
    temp = 0;
    hum = 0;
    
    wifiClient.stop();
    Serial.println(wifiClient.connected()); //0 dc
    WiFi.disconnect();
    Serial.println(WiFi.status());
    digitalWrite(4, LOW);
}
/* ISR for timer, set timeout flag to true */
void _callback0(void *usr_data){
  timeout = true;
}
/*u8g.firstPage();  
    do {
      draw();
      u8g.setFont(u8g_font_5x8);
      u8g.drawStr(0, 52, "(Normal)");
      } while( u8g.nextPage() );
*/      
void loop() {
  if(timeout){
    digitalWrite(4, HIGH);
  Serial.println("in loop");
  Serial.println("delay 40 sec");
    delay(40000);
  connectWIFI();
  connectServer();
  read_data();
  
  // check wifi status, dc then re-connect
  //Serial.println(WiFi.status());
//  if(WiFi.status() != WL_CONNECTED) {
//      digitalWrite(4, LOW);
//      Serial.println("WiFi has disconnected. Re-connecting...");
//      status = WL_IDLE_STATUS;
//      while (status != WL_CONNECTED) {
//          Serial.print("Attempting to connect to SSID: ");
//          Serial.println(ssid);
//          // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
//          status = WiFi.begin(ssid,pass);
//      
//          // wait 5 seconds for connection:
//          delay(2000);
//      }
//      connectServer();
//  }
  // check server status, dc then re-connect
//  Serial.println(wifiClient.status());
//  if(!wifiClient.connected()){
//      digitalWrite(4, LOW);
//      connectServer();
//  }
  
    // check the data is normal or not
    if(pm10 == 0 && pm25 == 0 && pm100 == 0 && temp == 0 && hum == 0){
      Serial.println("in zero if");
      while(pm10 == 0){        
        Serial.println("in while");
        delay(1000);
        read_data();
      } 
    }
    if(pm10 >= 1000 || temp > 45){
      Serial.println("in big if");
      while(pm10 >= 1000 || temp > 45){        
        Serial.println("in while");
        delay(1000);
        read_data();
      }
    }
    // if normal, send data
//    if(pm10 != 0 && pm25 != 0 && pm100 != 0 && temp != 0 && hum != 0){
    char buffer[64];
    LRTC.get();
    sprintf(buffer, "%ld/%ld/%ld %.2ld:%.2ld:%.2ld", LRTC.year(), LRTC.month(), LRTC.day(), LRTC.hour(), LRTC.minute(), LRTC.second());
    Serial.println(buffer);
      sprintf(send_msg, "{ 'pm10': %d, 'pm25': %d, 'pm100': %d, 'temp': %d, 'humidity': %d, 'position': %d }", pm10, pm25, pm100, temp, hum, ID);
      Serial.println(send_msg);
      send_mes(send_msg);
//    }
    // zero the variables and set timeout flag back to false
    pm10 = 0;
    pm25 = 0;
    pm100 = 0;
    temp = 0;
    hum = 0;
    wifiClient.stop();
    Serial.println(wifiClient.connected()); //0 dc
    WiFi.disconnect();  //6 dc
    Serial.println(WiFi.status());  
    digitalWrite(4, LOW);
    timeout = false;
  }
}
// pack the reading data function
void read_data(){
  Serial.println("in func");
  int count = 0;
  unsigned char c;
  unsigned char high;

  // read data
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
//      pm25PNO = 256*high + c;
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

void connectWIFI(){
    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid,pass);
    
        // wait 2 seconds for connection:
        delay(2000);
    }
    
    Serial.println("Connected to wifi");
    printCurrentNet();
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void connectServer(){
    //attempt to connect to server
    while (!wifiClient.connect(TCP_IP, TCP_PORT)){
        delay(2000);
        Serial.print("Attempting to connect to SERVER: ");
        Serial.println(TCP_IP);
    }
    
    Serial.println("connected to server");
    recv_mes();
    digitalWrite(4, HIGH);
//    delay(10000);  
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
