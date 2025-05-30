# Контроллер ESC для детской машинки

## Описание
Проект представляет собой контроллер электронного регулятора скорости (ESC) для детской машинки. Контроллер позволяет управлять скоростью движения машинки двумя способами:
1. Через пульт дистанционного управления (приёмник)
2. Через педаль с плавным разгоном и ограничением максимальной скорости

## Основные функции
- Управление скоростью двигателя через ESC
- Двойной режим управления (пульт/педаль)
- Плавный разгон при нажатии на педаль
- Регулировка максимальной скорости через потенциометр
- Автоматическое определение нейтрального положения
- Индикация работы через светодиод

## Подключение пинов
### Основные пины
- LED_PIN (13): Светодиод для индикации работы
- ESC_PIN (9): Пин подключения ESC
- RECEIVER_ESC_PIN (5): Пин подключения приёмника
- PEDAL_PIN (6): Пин подключения педали
- MAX_SPEED_POTENTIOMETER_PIN (A0): Пин потенциометра регулировки скорости

### Константы управления
- ESC_NEUTRAL_POSITION: 1500 мкс (нейтральное положение ESC)
- ESC_NEUTRAL_POSITION_THRESHOLD: 40 мкс (порог определения нейтрали)
- PEDAL_MAX_THROTTLE: 1700 мкс (максимальная скорость с педали)
- ACCELERATION_SPEED: 100 (скорость разгона)
- PEDAL_SHIFT_FROM_NEUTRAL: 50 (смещение от нейтрали при нажатии педали)

## Установка и настройка
1. Подключите двигатель к ESC
2. Подключите приёмник к RECEIVER_ESC_PIN
3. Подключите педаль к PEDAL_PIN
4. Подключите потенциометр к MAX_SPEED_POTENTIOMETER_PIN
5. Подключите ESC к аккумулятору
6. Выполните калибровку ESC согласно инструкции

