#include <WiFi.h>
#include <PubSubClient.h> //Lib for MQTT Pub and Sub
#include <ESP32Encoder.h>
//


//Hardware Parameter
//ECHO
#define Echo1 14
#define Trig1 27 
#define Echo2 12
#define Trig2 13
//Encoder
#define Enco_A1 23
#define Enco_A2 33
#define Enco_B1 35
#define Enco_B2 32
//Motor
#define motor1Pin1 16
#define motor1Pin2 4
#define enable1Pin 17
#define motor2Pin1 18
#define motor2Pin2 5
#define enable2Pin 19
//Line
#define line_sense_right 39
#define line_sense_left 34
//Akkuvoltage
#define Akku 36

//Wifi configuration
#include <EEPROM.h>
bool newWifi = false;
char inputt[65];
//char* inputt;
char SSID_conf[20];
char PW_conf[25];
char Broker_conf[20];
//EEPROM
int ssid_adress=0;
int pw_adress=100;
int broker_adress=200;
String SSID_conf_str;
String PW_conf_str;
String Broker_conf_str; 

// ultrasonic
int distance_v;
int tof1;
int tof2;
int starttime1;
int line_left;
int line_right;

//Akku
int u_akku = 0;
int pre_u_exp;
int u_exp;
int weight_u = 0.97;
bool u_min=false;

//encoder
ESP32Encoder encoder;
ESP32Encoder encoder2;
// timer and flag for example, not needed for encoders
unsigned long encoder2lastToggled;
unsigned long encoderlastToggled;
unsigned long lastcount_1=0;
unsigned long lastcount_2=0;
unsigned long start_micro=0;
int time_delay=0;
long count2=0;
long count1=0;

//String count2;
unsigned long signal_time=0;
unsigned long last_time=0;
unsigned long sum_time=0;
float freq=0;
float freq_2=0;
int c2;
int c1;
bool c1_neg= false;
bool c2_neg=false;
float freq_filt_1 = 0;
float freq_filt_2 = 0;
float last_freq_filt_1 = 0;
float last_freq_filt_2 = 0;
float weight = 0.9;
float f1;
float f2;

//Ringmemory
float ringsp[101];
float ringsp_2[101];
int pos=0;
int pos_2=0;
float ringsp_sum=0;
float ringsp_sum_2=0;
bool first_sum=true;
bool first_sum_2=true;

int tl=0;
int tr=0;

//PID-controll
int controlmode = 1;   //1 -> motor_right has less power ; 2 -> motor_left has less power; 3 -> both motors have the same power(motor-rotation-difference-controll is off)
float error;
unsigned long PID_loop_time;
unsigned long pre_PID_loop_time[5];
float I_sum[5];
float I_sum_max=1000;//anti_windup
float accel;
float pre_error[5];
float Kp[] = {0.55, 0.55, 0, 0.9,0};     //{motor_right, motor_left, diff_motor_right, diff_motor_left}
float Ki[] = {0.9, 0.9, 0, 0.5,0};
float Kd[] = {0.1, 0.1, 0, 0.1, 0};
int msg_mlef;
int msg_mrig;
//calibration
bool calibrate_PID = false;             // true to manipulate PID-parameter from remote
float P_calib=0.0;
float I_calib=0.0;
float D_calib=0.0;
float P_diff_c=0.0;
bool m_diff = true;                     // true to activate motor-rotation-difference-control
int mleft_d=0;
int mright_d=0;

int len=5;
char buff[10];
char buff1[21];

 
// Setting PWM properties
const int freq_pwm = 30000;
const int pwmChannel = 0;
const int pwmChannel_2 = 1;
const int resolution = 10; //10 Bit motor-geschwindigkeits-auflösung
int deathzone=450; // at 10 Bit the motor isn't running for 0-450
int maxspeed=450; // at 10 Bit 900 is equivalent to 6 Volt. NEVER RUN THE MOTOR OVER 6 VOLT!

//#ifndef STASSID
//#define STASSID "FRITZ!Box 7530"       //Enter Wfi-Name
//#define STAPSK  "12345678912345678910"  //Enter Passkey
//#endif
//const char* MQTT_BROKER = "123.123.123.12";  //Name of the mqtt broker
const char* MQTT_BROKER;
//const char* MQTT_BROKER = "test.mosquitto.org";
const char* PubTopic = "isdtfdw/channel_1";   //Topic where to publish
const char* SUBTOPIC="isdtfdw/channel_2";     //Topic where to subscribe
String clientID = "Roboter";  //Clientname for MQTT-Broker
//
// if the broker is locked with a password, uncommend the following lines
//
//const char* mqttUser = "esp32"; //User
//const char* mqttPassword = "robo"; //Password
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lLastMsg = 0;
int iTimeDelay = 2000;  //Set delay for next msg to 2 seconds
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];

void setup() 
{
  EEPROM.begin(512);
  Serial.begin(115200); //Start Serial monitor baudrate 115200
  delay(50);
  wifi_config();
  EEPROM.end();
  Serial.println("SerialMonitor enabled");
  setupWifi();
  Serial.println("Set MQTT-Server");
  mqttClient.setServer(MQTT_BROKER,1883);
  Serial.println("Set Callback-function");
  mqttClient.setCallback(callback);
  Serial.println("Finish setup()-Function");
  
  //Encoder setup
  //
  //ESP32Encoder::useInternalWeakPullResistors=DOWN;
  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors=UP;

  // use pin 19 and 18 for the first encoder
  encoder.attachHalfQuad(Enco_A1, Enco_B1);
  // use pin 17 and 16 for the second encoder
  encoder2.attachHalfQuad(Enco_A2, Enco_B2);
  //encoder2.attachFullQuad(33, 32);
    
  // set starting count value after attaching
  encoder.setCount(37);

  // clear the encoder's raw count and set the tracked count to zero
  encoder2.clearCount();
  encoder.clearCount();
  Serial.println("Encoder Start = " + String((int32_t)encoder.getCount()));
  // set the lastToggle
  encoder2lastToggled = millis();
  encoderlastToggled = millis();

  //Echo
  pinMode(Trig1, OUTPUT);
  pinMode(Echo1, INPUT);
  pinMode(Trig2, OUTPUT);
  pinMode(Echo2, INPUT);
  
  //Motor 
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);
  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq_pwm, resolution);
  ledcSetup(pwmChannel_2, freq_pwm, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel_2);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!mqttClient.connected())
    reconnectMQTT();
  akkustatus();
  rotation();
  distance();
  line();  
  if(f1>7499) f1=7499;                         //to make sure, that f1+2500 is %04d -> (f1+2500) < 10000
  if(f2>7499) f2=7499;
  if(f1<-2500) f1=-2500;                       // to make sure that f1+2500 >= 0, therefore no negative values were transceved
  if(f2<-2500) f2=-2500;
  if(tl>3000) tl=3000;
  if(tr>3000) tr=3000;
  if(tl<-3000) tl=-3000;
  if(tr<-3000) tr=-3000;
  sprintf(buff1,"%04d%04d%04d%04d%04d%04d%04d%04d",((int)f1)+2500,((int)f2)+2500,tr+3000,tl+3000,u_exp,distance_v,line_left,line_right);
  Serial.println(buff1);
  mqttClient.loop();
  mqttClient.publish(PubTopic,buff1,true);    // publish the string buff1 on publisher-topic
  Serial.print(msg_mlef);
  Serial.print(msg_mrig);
  Serial.print(controlmode);
  Serial.print(" ");
  Serial.print(calibrate_PID);
  motorspeed(msg_mlef, msg_mrig);
 
}
/*
* =================================================================
* Function:     callback   
* Returns:      /
* Description:  reads messages from mqtt subscriber-topic
* =================================================================
*/
void callback(char* topic, byte* payload, unsigned int length)
{
  String stMessage = "";
  for (int i = 0; i < length; i++)
    stMessage += String((char)payload[i]);
  msg_mlef = (((int)payload[0]-48)*100+((int)payload[1]-48)*10+payload[2]-48)-450;
  msg_mrig = (((int)payload[3]-48)*100+((int)payload[4]-48)*10+payload[5]-48)-450;
  calibrate_PID=(int)payload[7]-48;
  if(calibrate_PID){
     P_calib = (((int)payload[8]-48)*10+((int)payload[9]-48))/10.0;
     I_calib = (((int)payload[10]-48)*10+((int)payload[11]-48))/10.0;
     D_calib = (((int)payload[12]-48)*100+((int)payload[13]-48)*10+((int)payload[14]-48))/100.0;
    }
  controlmode = (int)payload[6]-48;
  Serial.println("Message is: " + stMessage);
}
/*
* =================================================================
* Function:     akkustatus   
* Returns:      /
* Description:  Check the voltage of the Akku, if the Akku is emty the Motors won't run anymore. This ensures that the battery is not damaged 
* =================================================================
*/
void akkustatus(){
  u_akku=analogRead(Akku);
  u_exp = u_akku * (1 - weight_u) + weight_u * pre_u_exp;
  pre_u_exp = u_exp;
  
  if(u_exp<1475){
    u_min=true;
  }
  
}
/*
* =================================================================
* Function:     motorspeed   
* Returns:      /
* Description:  input: (int mleft, int mright) the motorspeed of each motor, futhermore with the parameter controlmode you can turn on/off the motor-rotation-difference-control: controlmode 1 -> motor_right has less power ; 2 -> motor_left has less power; 3 -> both motors have the same power(motor-rotation-difference-controll is off)
* =================================================================
*/

void motorspeed(int mleft, int mright){
  int mlef;
  int mrig;
 
  if(m_diff){
    mleft = mleft*294/60;   // 294/60 is the factor between frequenz of the encoder and rpm(rotation per minute) of the motor/gear output
    mright = mright*294/60;
    if (controlmode==1){
      if(mright == 0){
        mleft_d=mleft;
      }
      else{
        mleft_d=mleft+PID_control(f2,f1*mleft/mright,3);
      }
      mright_d = mright;
    }
    else if (controlmode == 2){
      if(mleft == 0){
        mright_d = mright;
      }
      else{
        mright_d = mright + PID_control(f1,f2*mright/mleft,3);
      }
    }
    else if (controlmode == 3){
      mright_d = mright;
      mleft_d = mleft;
    }
    
    mlef = PID_control(f2, mleft_d, 1);
    mrig = PID_control(f1, mright_d, 0);
    tl=mlef;
    tr=mrig;
    Serial.print("mlef: ");
    Serial.print(mlef);
    Serial.print(" mrig: ");
    Serial.print(mrig);
  }
  
  else {
    mlef = mleft;
    mrig = mright;
    tl=mlef;
    tr=mrig;
  }
  
  if(mlef>maxspeed){
    mlef=maxspeed;
  }
  else if(mlef<-maxspeed){
    mlef=-maxspeed;
  }
  if(mrig>maxspeed){
    mrig=maxspeed;
  }
  if(mrig<-maxspeed){
    mrig=-maxspeed;
  }
  //to save the Akku, below 3.4V the motors must stop
  if(u_min){
    mlef=0;
    mrig=0;
    
  }
  //motorcontrol
  if(mrig>0){
    mrig=mrig+deathzone;
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    ledcWrite(pwmChannel, mrig); //controll motorspeed with mspeed 450 - 900 
  }
  else if(mrig<0){
    mrig=mrig-deathzone;
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(pwmChannel, -mrig); //controll motorspeed with mspeed 450 - 900 
  }
  else if(mrig==0){
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor1Pin1, LOW);
    ledcWrite(pwmChannel, 0); //controll motorspeed with mspeed 450 - 900 
  }
  if(mlef>0){
    mlef=mlef+deathzone;
    digitalWrite(motor2Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW);
    ledcWrite(pwmChannel_2, mlef); //controll motorspeed with mspeed 450 - 900 
  }
  else if(mlef<0){
    mlef=mlef-deathzone;
    digitalWrite(motor2Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH);
    ledcWrite(pwmChannel_2, -mlef); //controll motorspeed with mspeed 450 - 900 
  }
  else if(mlef==0){
    digitalWrite(motor2Pin1, HIGH);
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel_2, 0); //controll motorspeed with mspeed 450 - 900 
  }

}

/*
* =================================================================
* Function:     rotation   
* Returns:      /
* Description:  PID-Controller, float value_set is the setpoint value, float value_mes is the measured value (Input), int memory_num is the position in the array of stored values
* =================================================================
*/
void rotation(){
  // Loop and read the count
  count1=(int32_t)encoder.getCount();
  count2=(int32_t)encoder2.getCount();
  c2=count2-lastcount_2;
  c1=count1-lastcount_1;
  signal_time=micros();
  if(c1<0){
    c1_neg=true;
    c1=c1*-1;
  }
  else c1_neg=false;
  if(c2<0){
    c2_neg=true;
    c2=c2*-1;
  }
  else c2_neg=false;
  freq=c1*1000000/(signal_time-last_time);
  freq_2=c2*1000000/(signal_time-last_time);
  float mittelw_1=push(freq,49)/50;
  float mittelw_2=push_2(freq_2,49)/50;
  freq_filt_1=(1-weight)*mittelw_1+weight*last_freq_filt_1;
  freq_filt_2=(1-weight)*mittelw_2+weight*last_freq_filt_2;
  
  //store last values
  last_freq_filt_2=freq_filt_2;
  last_freq_filt_1=freq_filt_1;
  last_time = signal_time;
  lastcount_2 = count2;
  lastcount_1 = count1;

  // Encoder A and B are counting both so /2;
  f1=freq_filt_1/2;
  f2=freq_filt_2/2;
//  Serial.print(" f1: ");
//  Serial.print(freq_filt_1/2);
//  Serial.print(" f2: ");
//  Serial.println(freq_filt_2/2);
}
/*
* =================================================================
* Function:     PID_control   
* Returns:      int
* Description:  PID-Controller, float value_set is the setpoint value, float value_mes is the measured value (Input), int memory_num is the position in the array of stored values
* =================================================================
*/
int PID_control(float value_mes, float value_set, int memory_num){               //memory_num stays for the position in the array of stored "pre_PID_loop" and "I_sum" values
  if(calibrate_PID && memory_num==3){
    Kp[memory_num]=P_calib;
    Ki[memory_num]=I_calib;
  }
  error = value_set - value_mes;                                               //calculate error P-Glied(Geschwindigkeit)
  PID_loop_time = micros() - pre_PID_loop_time[memory_num];                       //Looptime
  accel = (error - pre_error[memory_num]) * 1000000.0 / PID_loop_time;                       //calculate derivative D-Glied(Beschleunigung)
  I_sum[memory_num] = I_sum[memory_num] + (error * (PID_loop_time / 1000000.0)); //calculate integral I-Gleid(Strecke)
  I_sum[memory_num] = max(-I_sum_max, min(I_sum_max, I_sum[memory_num]));       //anti-windup
  int PID_value = Kp[memory_num] * (error + Ki[memory_num] * I_sum[memory_num] + accel * Kd[memory_num]);            //calculate output
  pre_PID_loop_time[memory_num] = micros();                                      //store previous time
  pre_error[memory_num] = error;                                                            //store previous error
  //Serial.print(PID_value);
  //Serial.print(" ");
  return PID_value;
}
/*
* =================================================================
* Function:     push   
* Returns:      float
* Description:  ring puffer witch stores and sum the last x values of fr from type float
* =================================================================
*/
float push(float fr,int x){
  if(c1_neg){
      fr=fr*-1;
    }
  if(first_sum){
    ringsp_sum=ringsp_sum+fr;
  }
  else ringsp_sum=ringsp_sum+fr-ringsp[pos];
  
  ringsp[pos]=fr;
  pos++;
  if(pos>x){
    pos=0;
    first_sum = false;
  }
  return ringsp_sum;
}
float push_2(float fr,int x){
  if(c2_neg){
    fr=fr*-1;
  }
  if(first_sum_2){
    ringsp_sum_2=ringsp_sum_2+fr;
  }
  else ringsp_sum_2=ringsp_sum_2+fr-ringsp_2[pos_2];
  ringsp_2[pos_2]=fr;
  pos_2++;
  if(pos_2>x){
    pos_2=0;
    first_sum_2 = false;
  }
  return ringsp_sum_2;
}
/*
* =================================================================
* Function:     distance   
* Returns:      /
* Description:  get distance values between 0-400 cm
* =================================================================
*/
void distance(){
  digitalWrite(Trig1, HIGH);
  delayMicroseconds(10); 
  digitalWrite(Trig1, LOW);
  tof1 = pulseIn(Echo1, HIGH);
  digitalWrite(Trig2, HIGH);
  delayMicroseconds(10); 
  digitalWrite(Trig2, LOW);
  tof2 = pulseIn(Echo2, HIGH);
  if(tof2<tof1){
    tof1=tof2;
  }
  distance_v=340*tof1/(10000*2);
}
/*
* =================================================================
* Function:     line   
* Returns:      void
* Description:  get Linesensor values between 0-4950 0=max.withe 4950=max.black
* =================================================================
*/
void line(){
  
  line_left = analogRead(line_sense_left);
  line_right = analogRead(line_sense_right);
}
/*
* =================================================================
* Function:     wifi_config   
* Returns:      /
* Description:  reads wifi-config-data from serial Port an store them in EEPROM
* =================================================================
*/
void wifi_config(){
  Serial.println("wifi config");
  delay(1000);
  int i_pos=0;
  while(i_pos<1){
    if (Serial.available() > 0) {
      // read the incoming byte:      inputt = Serial.read();
      while(Serial.available()>0){
        char inchar=Serial.read();
        inputt[i_pos]=inchar;
        i_pos++;
      }
     newWifi= true;
     //inputt=Serial.read();
     // say what you got:
     Serial.print("I r: ");
     Serial.println(inputt);
    }
    else i_pos=1;
  }
  if(newWifi){
    i_pos=0;
    //inputt="FRITZ!Box 7530;12345678912345678910;123.123.123.12;";
    while(inputt[i_pos]!=';'){
      SSID_conf[i_pos]=inputt[i_pos];
      Serial.print(inputt[i_pos]);
      Serial.println( i_pos);
      i_pos++;
    }
    SSID_conf[i_pos]='\0';
    i_pos++;
    int pw_i=0;
    while(inputt[i_pos]!=';'){
      PW_conf[pw_i]=inputt[i_pos];
      Serial.print(inputt[i_pos]);
      Serial.println( i_pos);
      i_pos++;
      pw_i++;
    }
    PW_conf[pw_i]='\0';
    i_pos++;
    int br_i=0;
    while(inputt[i_pos]!=';'){
      Broker_conf[br_i]=inputt[i_pos];
      i_pos++;
      br_i++;
    }
    SSID_conf_str = String(SSID_conf);
    PW_conf_str = String(PW_conf);
    Broker_conf_str = String(Broker_conf); 
    EEPROM.writeString(ssid_adress,SSID_conf_str);
    EEPROM.writeString(pw_adress,PW_conf_str);
    EEPROM.writeString(broker_adress,Broker_conf_str);
    MQTT_BROKER=Broker_conf;
    #ifndef STASSID
    #define STASSID SSID_conf       //Enter Wfi-Name
    #define STAPSK  PW_conf  //Enter Passkey
    #endif
  }

   else {
    SSID_conf_str = EEPROM.readString(ssid_adress);
    PW_conf_str = EEPROM.readString(pw_adress);
    Broker_conf_str = EEPROM.readString(broker_adress);
    SSID_conf_str.toCharArray(SSID_conf,20);
    PW_conf_str.toCharArray(PW_conf,25);
    Broker_conf_str.toCharArray(Broker_conf,20);
    MQTT_BROKER=Broker_conf;
    #ifndef STASSID
    #define STASSID SSID_conf       //Enter Wfi-Name
    #define STAPSK  PW_conf  //Enter Passkey
    #endif
   }
}
void setupWifi()
{
  Serial.println("Connection to: " + String(STASSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}




void reconnectMQTT()
{
  while(!mqttClient.connected())
  {
    Serial.println("Login to MQTT-Broker");
    if(mqttClient.connect(clientID.c_str()))
    {
      Serial.println("Connected to MQTT-Broker " +String(MQTT_BROKER));
      Serial.println("Subscribe topic " + String(SUBTOPIC));
      Serial.println("connection succeeded");
      mqttClient.subscribe(SUBTOPIC,1); //Subscibe topic "SUBTOPIC"
    }
    else
    {
      Serial.println("Failed with rc=" +String(mqttClient.state()));
      Serial.println("Next MQTT-Connect in 3 sec");
      delay(3000);
    }
  }
}
