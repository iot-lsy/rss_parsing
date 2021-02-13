#include "WiFiEsp.h"
#include "LiquidCrystal_I2C.h"
#include "SoftwareSerial.h"
#include "LiquidCrystal.h"

SoftwareSerial Serial1(10, 13); // RX, TX


char ssid[] = "U+Net1D1B";            // your network SSID (name)
char pass[] = "0192023420";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

String current_Time = "";
String first_Weather = "";
String second_Weather = "";
String third_Weather = "";

const char* server = "kma.go.kr";

int count = 0;
int checkfunc_count = 0;

WiFiEspClient client;
LiquidCrystal_I2C lcd(0x27,20,4);

LiquidCrystal lcd1(12,11,7,6,5,4);

void connectServer(){

  // if you get a connection, report back via serial

  if (client.connect(server, 80)) {
    //http://www.kma.go.kr/wid/queryDFSRSS.jsp?zone=4111157300
    String url = "/wid/queryDFSRSS.jsp?zone=4111157300";
    
    client.println("GET "+url+" HTTP/1.1"); 
    client.print("HOST: www.kma.go.kr\n");
    client.println("User-Agent: arduino_uno_leesoyong");
    client.println("Connection: close");
    client.println();
 
  }
  

  
}
  

void setup_func(){

  current_Time = "";
  first_Weather = "";
  second_Weather = "";
  third_Weather = "";
  
  status = WL_IDLE_STATUS;
  
  lcd.init();
  lcd.setBacklight(HIGH);
  lcd.print("Loading...");
  
  lcd1.begin(16,2);
  lcd1.print("Loading..."); 
  Serial1.begin(9600);

  WiFi.init(&Serial1);

  
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);

  }
  connectServer();
  
}

void loop_func(){
  
  getWeatherData();
    
  if(!client.connected()){
    Serial.println();
    Serial.println("Disconnecting from server...");
    client.stop();
  }    


  current_Time = getTime(current_Time);
  first_Weather = getWeather(first_Weather);
  second_Weather = getWeather(second_Weather);
  third_Weather = getWeather(third_Weather);

  lcd1.clear();
  lcd1.setCursor(0,0);
  lcd1.print("Published_Time");
  lcd1.setCursor(0,1);
  lcd1.print(current_Time);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hr|Temp |WF_EN |Rain");
  lcd.setCursor(0,1);
  lcd.print(first_Weather);
  lcd.setCursor(0,2);
  lcd.print(second_Weather);
  lcd.setCursor(0,3);
  lcd.print(third_Weather);

  

  delay(30000);
  
  check_Front();

  WiFi.disconnect();
  lcd.clear();
  lcd1.clear();
  lcd.noBacklight();
  lcd1.noDisplay();
  
}

int check_Front(){ // no = 0, yes = 1, still = loop & no return
  int distance = 0;
  digitalWrite(9,HIGH);
  delayMicroseconds(10);
  digitalWrite(9,LOW);
  
  distance = pulseIn(2,HIGH) * 340 / 2 / 10000;
  

  if(distance<=60){
    checkfunc_count++;
    if(checkfunc_count==1){
      Serial.println("yes");
      return 1;  
    }else if(checkfunc_count>1){
      Serial.println("Still front");
      delay(5000);
      check_Front();
    }
    
  }else if(distance>60){
    Serial.println("no");
    checkfunc_count = 0;
    return 0;
  }
}


void setup()
{
  pinMode(2, INPUT);
  pinMode(9, OUTPUT);
  Serial.begin(9600);
}



void getWeatherData(){

  int count = 0;
  
  while(client.available()) { 
    
    char c = client.read();

    if(c=='<') count++;

    if(count==29){
      current_Time += c;
    }else if(count==40||count==44||count==56||count==58){
      first_Weather += c;
    }else if(count==120||count==124||count==136||count==138){
      second_Weather += c;
    }else if(count==200||count==204||count==216||count==218){
      third_Weather += c;
    }
  }
  
}

String getTime(String line){
  //<tm>202102072000

  String date = "";
  String temp = "";
  
  temp = line.substring(8,10);
  date += temp;
  date += "-";

  temp = line.substring(10,12);
  date += temp;
  date += "  ";

  temp = line.substring(12,14);
  date += temp;
  date += ":";

  temp = line.substring(14,16);
  date += temp;

  return date;
 
}

String getWeather(String line){

  String temp = "";
  String temp1 = "";

  int s = 0;
  int e = 0;


  s = line.indexOf("<hour>")+6;
  e = line.indexOf("<temp>");


  temp += line.substring(s,e);
  if(e-s==2){
    temp+="|";
  }else{
    temp+=" |";
  }


  s = line.indexOf("<temp>")+6;
  e = line.indexOf("<wfEn>");

  temp += line.substring(s,e);
  if(e-s==5){
    temp+="|";
  }else if(e-s==4){
    temp+=" |";
  }else if(e-s==3){
    temp+="  |";
  }
  
  s = line.indexOf("<wfEn>")+6;
  e = line.indexOf("<pop>");


  temp1 = line.substring(s,e);

  if(temp1 == "Mostly Cloudy"){
    temp+="Cloudy|";
  }else if(temp1 == "Rain/Snow"){
    temp+="Rn/Snw|";
  }else if(e-s==6){
    temp+=temp1;
    temp+="|";
  }else if(e-s==5){
    temp+=line.substring(s,e);
    temp+=" |";
  }else if(e-s==4){
    temp+=line.substring(s,e);
    temp+="  |";
  }
   
  s = line.indexOf("<pop>")+5;
  e = line.length();

  temp += line.substring(s,e);
  

  return temp;
}


void loop()
{ 
  int check = check_Front();
  
  if(check==1){
    count = 0;
    setup_func();
    loop_func();
  }else{
    if(count==0){
      lcd.init();
      lcd1.begin(16,2);
      lcd.clear();
      lcd1.clear();
      lcd.noBacklight();
      lcd1.noDisplay();
      count++;
    }
  }

  delay(10000);
  
}
