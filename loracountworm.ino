#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <dht.h>

#define DHT21_PIN 5
// Singleton instance of the radio driver
RH_RF95 rf95;
dht DHT;
float frequency = 915.0;  //frequency settings
const float max_volts = 5.0;
const float max_analog_steps = 1023.0;
String temp;
float tem,hum,told,hold=0;
char tem_1[8]={"\0"},hum_1[8]={"\0"};
char *node_id = "<0001>";  //From LG01 via web Local Channel settings on MQTT.Please refer <> dataformat in here.
uint8_t datasend[64];
char                 databuffer[50];
volatile int counter;
int cold = 0;

void  counting(){
  counter+=1;
  //Serial.println(counter);
  
}
void Json(String temperature,String humidity,String Count){
  temp = "{";
  temp += "\"key\":";
  temp += 0001;
  temp += ",";
  temp += "\"t\":";
  temp += temperature;
  temp += ",";
  temp += "\"h\":";
  temp += humidity;
  temp += ",";
  temp += "\"C\":";
  temp += Count;
  temp += "}";
  //Serial.println(temp);
 }
void setup(){
      Serial.begin(9600);
      counter=0;
      attachInterrupt(digitalPinToInterrupt(3),counting,RISING);
      Serial.println(F("Start MQTT Example"));
      if (!rf95.init())
      Serial.println(F("init failed"));
      rf95.setFrequency(frequency);
      rf95.setTxPower(20);
      rf95.setSpreadingFactor(7);
      rf95.setSignalBandwidth(125000);
      rf95.setCodingRate4(5);
      rf95.setSyncWord(0x34);
}
void getBuffer(){
  String data = "";
  data = data + String(temp);
  
  for (int i = 0; i < data.length(); i++){
    databuffer[i] = data[i];
  }
}
void dhtWrite()
{
    char data[75] = "\0";
    for(int i = 0; i < 50; i++)
    {
       data[i] = node_id[i];
    }

     strcat(data,databuffer);
     strcpy((char *)datasend,data); 
     Serial.print((char*)datasend);    
}

void SendData()
{     
      Serial.println(F("Sending data to LG01"));
           
   
      rf95.send((char *)datasend,sizeof(datasend));  
      rf95.waitPacketSent();  // Now wait for a reply
    
      uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);

     if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
     
      Serial.print("got reply from LG01: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is LoRa server running?");
  }
}
    

void loop(){
  int chk = DHT.read21(DHT21_PIN);
  hum = DHT.humidity;
  tem= DHT.temperature;
  
  if( counter != cold || tem!= told || hum != hold){
    Json(String(tem),String(hum),String(counter));
    getBuffer();
    dhtWrite();
    SendData();
    cold = counter;
    told= tem;
    hold = hum;  
  }

}
