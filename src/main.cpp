#include <Arduino.h>
#include <GyverOS.h>
#include <Servo.h> // servo library 
#include <Bounce2.h>

#define LED_PIN 13 // Дефолтный светодиод для индикации работы

#define DEBUG 1
#define ESC_PIN 9 // Пин подключения ESC
#define RECEIVER_ESC_PIN 5 // Пин подключения приёмника
#define PEDAL_PIN 6 // Пин, куда подключена педаль машины
#define MAX_SPEED_POTENTIOMETER_PIN A0 // Пин потенциометра регулировки скорости с педали

#define ESC_NEUTRAL_POSITION 1500
#define ESC_NEUTRAL_POSITION_THRESHOLD 40
#define PEDAL_MAX_THROTTLE 1800
#define ACCELERATION_SPEED 100
#define PEDAL_SHIFT_FROM_NEUTRAL 30

GyverOS<5> OS;	// указать макс. количество задач
Servo esc; // servo name
Bounce2::Button pedal_button = Bounce2::Button();
int pedal_state = 0;

// обработчики задач
void replicate_receiver_for_esc() {
  // выводит свой период в порт
  if (DEBUG){
    static uint32_t ms;
    Serial.println(millis() - ms);
    ms = millis();
  }
  int current_receiver_esc_value = pulseIn(RECEIVER_ESC_PIN, HIGH, 25000);
  if (current_receiver_esc_value == 0)
  {
    current_receiver_esc_value = ESC_NEUTRAL_POSITION; 
  }

  //work mode
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  
  if (abs(current_receiver_esc_value - ESC_NEUTRAL_POSITION) < ESC_NEUTRAL_POSITION_THRESHOLD)
  {
    if (DEBUG){
      Serial.println("Receiver ESC in neutral."); // Print the value of 
    }
    if (pedal_button.isPressed()){
      pedal_state++;
      if (pedal_state > ACCELERATION_SPEED){
        pedal_state = ACCELERATION_SPEED;
      }

      //read max speed settings
      int rotate_value = analogRead(MAX_SPEED_POTENTIOMETER_PIN);
      int max_speed_ms = map(rotate_value, 0, 1023, ESC_NEUTRAL_POSITION, PEDAL_MAX_THROTTLE);

      int pedal_esc_state = PEDAL_SHIFT_FROM_NEUTRAL +map(pedal_state, 0, ACCELERATION_SPEED, ESC_NEUTRAL_POSITION, max_speed_ms);

      if (DEBUG){
        Serial.print("Pedal is pressed; state: ");
        Serial.print(pedal_state);
        Serial.print(" max speed value: ");
        Serial.print(max_speed_ms);
        Serial.print(" esc state: ");
        Serial.println(pedal_esc_state);
      }
      esc.writeMicroseconds(pedal_esc_state);
    }else{
      pedal_state = 0;
      esc.writeMicroseconds(ESC_NEUTRAL_POSITION);
    }
  }
  else
  {
    pedal_state = 0;
    if (DEBUG){
      Serial.print("Receiver ESC channel 2:"); // Print the value of 
      Serial.println(current_receiver_esc_value);        // each channel
    }

    esc.writeMicroseconds(current_receiver_esc_value);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // подключаем задачи (порядковый номер, имя функции, период в мс)
  OS.attach(0, replicate_receiver_for_esc, 40);

  //attach esc
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_NEUTRAL_POSITION); // Turn the motor from the android app

  //attach receiver
  pinMode(RECEIVER_ESC_PIN, INPUT); // Set our input pins as such

  //attach pedal
  pedal_button.attach(PEDAL_PIN, INPUT_PULLUP); // USE EXTERNAL PULL-UP
  pedal_button.interval(5);
  pedal_button.setPressedState(LOW);

  //attach max speed potentiometer
  pinMode(MAX_SPEED_POTENTIOMETER_PIN, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  OS.tick();	// вызывать как можно чаще, задачи выполняются здесь

  //check bounce
  pedal_button.update();

  // OS.getLeft() возвращает время в мс до ближайшей задачи
  // на это время можно усыпить МК, например при помощи narcoleptic или GyverPower
  // для примера просто поставим delay на это время
  delay(OS.getLeft());
}