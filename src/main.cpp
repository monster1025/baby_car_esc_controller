#include <Arduino.h>
#include <GyverOS.h>
#include <Servo.h> // библиотека для управления ESC
#include <Bounce2.h> // библиотека для обработки дребезга кнопки педали

// Определение пинов
#define LED_PIN 13 // Светодиод для индикации работы
#define DEBUG 0 // Флаг отладки (0 - выключен, 1 - включен)
#define ESC_PIN 9 // Пин подключения ESC
#define RECEIVER_ESC_PIN 5 // Пин подключения приёмника
#define PEDAL_PIN 6 // Пин подключения педали
#define MAX_SPEED_POTENTIOMETER_PIN A0 // Пин потенциометра регулировки максимальной скорости

// Константы управления ESC
#define ESC_NEUTRAL_POSITION 1500 // Нейтральное положение ESC (микросекунды)
#define ESC_NEUTRAL_POSITION_THRESHOLD 40 // Порог определения нейтрального положения
#define PEDAL_MAX_THROTTLE 1700 // Максимальная скорость с педали
#define ACCELERATION_SPEED 100 // Скорость разгона (количество шагов)
#define PEDAL_SHIFT_FROM_NEUTRAL 50 // Смещение от нейтрали при нажатии педали

// Создание объектов
GyverOS<5> OS; // Создание планировщика задач
Servo esc; // Объект для управления ESC
Bounce2::Button pedal_button = Bounce2::Button(); // Объект для обработки педали
int pedal_state = 0; // Состояние педали (для плавного разгона)

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
    if (pedal_button.isPressed()) {
      // Увеличение состояния педали для плавного разгона
      pedal_state++;
      if (pedal_state > ACCELERATION_SPEED) {
        pedal_state = ACCELERATION_SPEED;
      }

      // Чтение настройки максимальной скорости с потенциометра
      int rotate_value = analogRead(MAX_SPEED_POTENTIOMETER_PIN);
      int max_speed_ms = map(rotate_value, 0, 1023, ESC_NEUTRAL_POSITION, PEDAL_MAX_THROTTLE);

      // Расчет текущей скорости с учетом разгона
      int pedal_esc_state = PEDAL_SHIFT_FROM_NEUTRAL + map(pedal_state, 0, ACCELERATION_SPEED, ESC_NEUTRAL_POSITION, max_speed_ms);

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
      pedal_state = 0;
      esc.writeMicroseconds(ESC_NEUTRAL_POSITION);
    }
  } else {
    // Если приёмник не в нейтрали, используем его сигнал напрямую
    pedal_state = 0;
    if (DEBUG) {
      Serial.print("Receiver ESC channel 2:");
      Serial.println(current_receiver_esc_value);
    }
    esc.writeMicroseconds(current_receiver_esc_value);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  // Настройка приёмника
  pinMode(RECEIVER_ESC_PIN, INPUT);

  // Настройка педали
  pedal_button.attach(PEDAL_PIN, INPUT_PULLUP);
  pedal_button.interval(5);
  pedal_button.setPressedState(LOW);

  // Настройка потенциометра
  pinMode(MAX_SPEED_POTENTIOMETER_PIN, INPUT);

  // Настройка ESC
  esc.attach(ESC_PIN);
  esc.writeMicroseconds(ESC_NEUTRAL_POSITION);

  // Подключение задачи репликации сигнала (выполняется каждые 40 мс)
  OS.attach(0, replicate_receiver_for_esc, 40);
}

void loop() {
  OS.tick(); // Выполнение задач планировщика
  pedal_button.update(); // Обновление состояния педали
  delay(OS.getLeft()); // Ожидание до следующей задачи
}