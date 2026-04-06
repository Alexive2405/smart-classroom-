#include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); 
#include <DHT.h>

//khai báo tên servo 
Servo myservo; 

//định nghĩa các chấn motor 
#define IN1 7
#define IN2 6
#define ENA 3

//khai báo biến toàn cục 
float doam;
float doC;
float doF;
int pwm;

int pos = 0; 

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
  myservo.attach(9);

} 
//hàm in ra màn hình LCD độ ẩm và nhiệt độ 
void print(){
  doam = dht.readHumidity(); //Đọc độ ẩm

  doC = dht.readTemperature(); //Đọc nhiệt độ C

  pwm = map(doC , 20 , 40 , 80 , 255);
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
  Serial.println();
  lcd.setCursor(6,1); //con trỏ cột 8 dòng 2 
  lcd.print(doC);
  lcd.setCursor(11,1); // con trỏ cột 11 dòng 2 
  lcd.print("|");
  lcd.setCursor(12,1);
  lcd.print(pwm);
  

  delay(1000);
}

//hàm motor xoay
void motor_xoay(){
  pwm = map(doC , 24 , 33 , 80 , 255);
  pwm = constrain( pwm , 80 , 255);
  digitalWrite(IN1 , HIGH);
  digitalWrite(IN2 , LOW);
  analogWrite( ENA , pwm );

}

//hàm motor dừng
void motor_dung(){
  digitalWrite( IN1 , LOW );
  digitalWrite( IN2 , LOW );
}

//xoay servo
void servo_xoay_mo(){
  for ( pos = 0 ; pos < 90 ; pos++ ){
    myservo.write(pos);
  }
  myservo.write(90);
}
void servo_xoay_dong(){
  for ( pos = 90 ; pos >= 1 ; pos--){
    myservo.write(pos);
  }
  myservo.write(0);
}

void loop() 
{
  print();
  if ( doC >= 25){
    motor_xoay();
  }

   if (doC >= 28 ){
    servo_xoay_mo();
  }

  else{
    servo_xoay_dong();
  }
}
