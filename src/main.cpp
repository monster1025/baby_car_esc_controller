#include <Arduino.h>
#include <GyverOS.h>
#include <Servo.h> // библиотека для управления ESC
#include <Bounce2.h> // библиотека для обработки дребезга кнопки педали

// Определение пинов
#define LED_PIN 13 // Светодиод для индикации работы
#define DEBUG 0 // Флаг отладки (0 - выключен, 1 - включен)
#define ESC_PIN 9 // Пин подключения ESC
#define RECEIVER_ESC_PIN 5 // Пин подключения приёмника
#define PEDAL_PIN 5 // Пин подключения педали
#define MAX_SPEED_POTENTIOMETER_PIN A0 // Пин потенциометра регулировки максимальной скорости

// Аналоговая педаль
#define PEDAL_ANALOG_PIN A5
#define PEDAL_MIN 175
#define PEDAL_MAX 880
#define PEDAL_THRESHOLD 30

// Константы управления ESC
#define ESC_NEUTRAL_POSITION 1500 // Нейтральное положение ESC (микросекунды)
#define ESC_NEUTRAL_POSITION_THRESHOLD 40 // Порог определения нейтрального положения
#define PEDAL_MAX_THROTTLE 1700 // Максимальная скорость с педали
#define ACCELERATION_SPEED 100 // Скорость разгона (количество шагов)
#define PEDAL_SHIFT_FROM_NEUTRAL 50 // Смещение от нейтрали при нажатии педали

// Создание объектов
GyverOS<5> OS; // Создание планировщика задач
Servo esc; // Объект для управления ESC

// Функция репликации сигнала с приёмника на ESC
void replicate_receiver_for_esc() {
  // Чтение сигнала с приёмника
  int current_receiver_esc_value = pulseIn(RECEIVER_ESC_PIN, HIGH, 25000);
  if (current_receiver_esc_value == 0) {
    current_receiver_esc_value = ESC_NEUTRAL_POSITION; // Если сигнал потерян, устанавливаем нейтраль
  }

  // Индикация работы
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  
  // Проверка на нейтральное положение приёмника
  if (abs(current_receiver_esc_value - ESC_NEUTRAL_POSITION) < ESC_NEUTRAL_POSITION_THRESHOLD) {
    if (DEBUG) {
      Serial.println("Receiver ESC in neutral.");
    }
    
    // Обработка нажатия педали
    int pedal_state = analogRead(PEDAL_ANALOG_PIN);
    if (abs(pedal_state - PEDAL_MIN) > PEDAL_THRESHOLD) {
      // Чтение настройки максимальной скорости с потенциометра
      int rotate_value = analogRead(MAX_SPEED_POTENTIOMETER_PIN);
      int max_speed_ms = map(rotate_value, 0, 1023, ESC_NEUTRAL_POSITION, PEDAL_MAX_THROTTLE);

      // Расчет текущей скорости с учетом разгона
      int pedal_esc_state = PEDAL_SHIFT_FROM_NEUTRAL + map(pedal_state, PEDAL_MIN, PEDAL_MAX, ESC_NEUTRAL_POSITION, max_speed_ms);

      if (DEBUG) {
        Serial.print("Pedal is pressed; state: ");
        Serial.print(pedal_state);
        Serial.print(" max speed value: ");
        Serial.print(max_speed_ms);
        Serial.print(" esc state: ");
        Serial.println(pedal_esc_state);
      }
      esc.writeMicroseconds(pedal_esc_state);
    } else {
      // Сброс состояния педали и установка нейтрали
      esc.writeMicroseconds(ESC_NEUTRAL_POSITION);
    }
  } else {
    if (DEBUG) {
      Serial.print("Receiver ESC channel 2:");
      Serial.println(current_receiver_esc_value);
    }
    esc.writeMicroseconds(current_receiver_esc_value);
  }
}

void pedal_test() {
  int pedal_state = analogRead(PEDAL_ANALOG_PIN);
  Serial.print("pedal_state: ");
  Serial.println(pedal_state);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // Настройка приёмника
  pinMode(RECEIVER_ESC_PIN, INPUT);

  // Настройка аналоговой педали
  pinMode(PEDAL_ANALOG_PIN, INPUT);

  // Настройка потенциометра
  pinMode(MAX_SPEED_POTENTIOMETER_PIN, INPUT);

  // Настройка ESC
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_NEUTRAL_POSITION);

  // Подключение задачи репликации сигнала (выполняется каждые 40 мс)
  OS.attach(0, replicate_receiver_for_esc, 40);
  //OS.attach(0, pedal_test, 240);
}

void loop() {
  OS.tick(); // Выполнение задач планировщика
  delay(OS.getLeft()); // Ожидание до следующей задачи
}