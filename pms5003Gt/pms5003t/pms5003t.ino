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

char ssid[] = "Sodagreen";          // your network SSID (name)
char pass[] = "w431231853211";     // your network password (use for WPA, or use as key for WEP)
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

    connectWIFI();
    connectServer();
    
    recv_mes();
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
  // put your main code here, to run repeatedly:
  int count = 0;
  unsigned char c;
  unsigned char high;

  // check wifi status, dc then re-connect
  if(status != WL_CONNECTED) {
    connectWIFI();
    connectServer();
  }

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
      Serial.print("CF=1, PM1.0=");
      Serial.print(pmcf10);
      Serial.println(" ug/m3");
      pm10 = pmcf10;
    }
    else if(count == 7){
      pmcf25 = 256*high + c;
      Serial.print("CF=1, PM2.5=");
      Serial.print(pmcf25);
      Serial.println(" ug/m3");
      pm25 = pmcf25;
    }
    else if(count == 9){
      pmcf100 = 256*high + c;
      Serial.print("CF=1, PM10=");
      Serial.print(pmcf100);
      Serial.println(" ug/m3");
      pm100 = pmcf100;
    }
    else if(count == 11){
      pmat10 = 256*high + c;
      Serial.print("atmosphere, PM1.0=");
      Serial.print(pmat10);
      Serial.println(" ug/m3");
    }
    else if(count == 13){
      pmat25 = 256*high + c;
      Serial.print("atmosphere, PM2.5=");
      Serial.print(pmat25);
      Serial.println(" ug/m3");
    }
    else if(count == 15){
      pmat100 = 256*high + c;
      Serial.print("atmosphere, PM10=");
      Serial.print(pmat100);
      Serial.println(" ug/m3");
    }
    else if(count == 17){
      pm03PNO= 256*high + c;
      Serial.print("above0.3um, no=");
      Serial.print(pm03PNO);
      Serial.println(" number");
    }
    else if(count == 19){
      pm05PNO = 256*high + c;
      Serial.print("above0.5um, no=");
      Serial.print(pm05PNO);
      Serial.println(" number");
    }
    else if(count == 21){
      pm10PNO = 256*high + c;
      Serial.print("above 1.0um, no=");
      Serial.print(pm10PNO);
      Serial.println(" number");
    }
    else if(count == 23){
      pm25PNO = 256*high + c;
      Serial.print("above 2.5um, no=");
      Serial.print(pm25PNO);
      Serial.println(" number");
    }
    else if(count == 25){
      Temperature = 256*high + c;
      Serial.print("Temperature = ");
      Serial.print(Temperature/10);
      Serial.println(" ^C ");
      temp = Temperature/10;
    }
    else if(count == 27){
      Humidity = 256*high + c;
      Serial.print("Humidity = ");
      Serial.print(Humidity/10) ;
      Serial.println(" % ");
      hum = Humidity/10;
    }
    count++;
  }

  // sending data message
  sprintf(send_msg, "{ 'pm10': %d, 'pm25': %d, 'pm100': %d, 'temp': %d, 'humidity': %d }", pm10, pm25, pm100, temp, hum);
  Serial.println(send_msg);
  send_mes(send_msg);
  
  while(mySerial.available()) mySerial.read();
  Serial.println();
  // send every 5 seconds
  delay(10000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectWIFI(){
    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid,pass);
    
        // wait 5 seconds for connection:
        delay(5000);
    }
    
    Serial.println("Connected to wifi");
    //printWifiStatus();
}

void connectServer(){
    //attempt to connect to server
    while (!wifiClient.connect(TCP_IP, TCP_PORT)){
        delay(300);
        Serial.print("Attempting to connect to SERVER: ");
        Serial.println(TCP_IP);
    }
    
    Serial.println("connected to server");
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
          Serial.println(recv_buf);
          break;
    }
    else Serial.println("no received message");
  }
  sprintf(recv_buf, "");
}