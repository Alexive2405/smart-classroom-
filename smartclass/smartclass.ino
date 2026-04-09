#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); 
#include <DHT.h>

//khai báo tên servo 
Servo myservo1; 
Servo myservo2;

//định nghĩa các chấn motor 
#define IN1 7
#define IN2 6
#define ENA 3

//khai báo biến toàn cục 
float doam;
float doC;
float doF;
float pwm;
float hi;

//Khai bao pid
float pre_pid=0;
float previous_error = 0;
float kp=5;
float kd=1;
float ki=0;
float setpoint=25;
float v_goc = 138;

//Servo 
int pos1 = 0;
int pos2 = 180;
bool servo1state = false; 
bool servo2state = false;
bool servo_dang_chay = false;

const int DHTPIN = 5;//Chân Out của cảm biến nối chân số 5 Arduino
const int DHTTYPE = DHT11;   // Khai báo kiểu cảm biến là DHT11 
DHT dht(DHTPIN, DHTTYPE); //Khai báo thư viện chân cảm biến và kiểu cảm biến

//set up linh kiện
void setup() 
{

  Serial.begin(9600);
  //setup DHT11 và LCD1602
  dht.begin(); //Khởi động cảm biến

  lcd.init(); //Khởi động LCD                    
  lcd.backlight(); //Mở đèn
  lcd.setCursor(0,0);
  lcd.print("DO AM:");
  lcd.setCursor(0,1);
  lcd.print("C|PWM: ");

  //set up motor quạt
  pinMode(ENA , OUTPUT );
  pinMode( IN1 , OUTPUT );
  pinMode( IN2 , OUTPUT );

  //set up servo
  myservo1.attach(9);
  myservo2.attach(10);

}


float tinh_pid(float setpoint, float doC) {
  float error = doC - setpoint;

  // nếu thấp hơn setpoint → giảm quạt nhanh
  if (error < 0) {
    pre_pid *= 0.8;
    previous_error = error;
    return pre_pid;
  }

  // deadband (chỉ áp dụng khi phía trên)
  if (error < 0.7) {
    pre_pid *= 0.9;
    previous_error = error;
    return pre_pid;
  }

  float derivative = error - previous_error;
  float pid = kp * error + kd * derivative;

  // làm mượt
  pid = 0.7 * pre_pid + 0.3 * pid;

  previous_error = error;
  pre_pid = pid;

  return pid;
}

//hàm in ra màn hình LCD độ ẩm và nhiệt độ 
void print(){
  doam = dht.readHumidity(); //Đọc độ ẩm

  doC = dht.readTemperature(); //Đọc nhiệt độ C

  pwm = constrain ( tinh_pid(setpoint,doC), -128, 127 );
  //float finalpwm = constrain( pwm , 0 , 255 );

  // Kiểm tra cảm biến có hoạt động hay không
  if (isnan(doam) || isnan(doC) ) 
  {
    Serial.println("Không có giá trị trả về từ cảm biến DHT");
    return;
  }

  Serial.print("Độ ẩm: ");
  Serial.print(doam);
  lcd.setCursor(8,0); //con trỏ vị trí số 8, hiện ô số 9
  lcd.print(doam);
  lcd.setCursor(12,0); //Con trở ở vị trí 12, hiện ô 13
  lcd.print("%");
  
  Serial.print("%  Nhiệt độ: ");
  Serial.print(doC);
  Serial.print("°C | ");
  Serial.print("pwm:");
  Serial.print( pwm );
  Serial.print(" v moi: ");
  Serial.print (v_goc + pwm);
  Serial.println();
  lcd.setCursor(6,1); //con trỏ cột 8 dòng 2 
  lcd.print(doC);
  lcd.setCursor(10,1); // con trỏ cột 11 dòng 2 
  lcd.print("|");
  lcd.setCursor(11,1);
  lcd.print(pwm + v_goc);
  // In trạng thái servo
  Serial.print("Servo: ");
  if (servo1state) {
    Serial.print("Servo1 MO ");
  }
  else{
    Serial.print("Servo1 DONG ");
  }
  if (servo2state) {
    Serial.print("Servo2 MO ");
  }
  else{
    Serial.print("Servo2 DONG ");
  }
  Serial.println();

  delay(500);
}

//xoay servo
void servo_dieu_khien(){
  if ( doam >= 80 && !servo1state && !servo2state && doC >= 31 ){
    while ( pos1 <= 90 && pos2 >= 90 ){
      myservo1.write(pos1);
      myservo2.write(pos2);
      pos1++;
      pos2--;
      delay(15);
    }
    servo1state = true;
    servo2state = true;
    Serial.println("servo1 mo");
    Serial.println("servo2 mo");
  }

  else if ( doam <= 40 && servo1state && servo2state && doC <= 25 ){
    while( pos1 <= 90 && pos2 >= 90){
      myservo1.write(pos1);
      myservo2.write(pos2);
      pos1--;
      pos2++;
      delay(15);
    }
    servo1state = false;
    servo2state = false;
    Serial.println("servo1 dong");
    Serial.println("servo2 dong");
  }

  if (!servo_dang_chay) {
    if (servo2state && servo1state){
      myservo1.write(90);  // Giữ ở 90° nếu đang mở
      myservo2.write(90);
    }
    else{
      myservo1.write(0);   // Giữ ở 0° nếu đang đóng
      myservo2.write(180);
    }
  }
}


//hàm motor xoay theo PID
void motor_xoay (int pwm ) {
  digitalWrite(IN1 , HIGH);
  digitalWrite(IN2 , LOW);
  analogWrite(ENA, v_goc + pwm);
}

//hàm motor dừng
void motor_dung(){
  digitalWrite( IN1 , LOW );
  digitalWrite( IN2 , LOW );
}

void loop() 
{
  print();
  if ( doC < 24 ){
    motor_dung();
  }
  else {
    motor_xoay(pwm);
  }
  servo_dieu_khien();
  delay(50);
}


