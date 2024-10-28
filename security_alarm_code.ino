/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-motion-sensor
 */
#include <IRremote.h>
#include <LiquidCrystal_I2C.h>

#define LED 13
#define IR_RECEIVE_PIN 8

#define TASTA_1 69
#define TASTA_2 70
#define TASTA_3 71
#define TASTA_4 68
#define TASTA_5 64
#define TASTA_6 67
#define TASTA_7 7
#define TASTA_8 21
#define TASTA_9 9
#define TASTA_0 25
#define TASTA_S 22
#define TASTA_D 13
#define TASTA_UP 24
#define TASTA_DOWN 82
#define TASTA_LEFT 8
#define TASTA_RIGHT 90
#define TASTA_OK 28

#define LED_ON 10
#define LED_OFF 9
#define LED_IR 6

const int PIN_TO_SENSOR = 2; // the pin that OUTPUT pin of sensor is connected to
int pinStateCurrent = LOW;   // current state of pin
int pinStatePrevious = LOW;  // previous state of pin
const int buzzer = 3;
int alarma = 0;
volatile long int count = 0;
volatile long int count_comm = 0;
volatile int lock_timer = 0;
int input = 0;
volatile int command = 0;
int STATE = 0;
const int STATE_0 = 0;
const int STATE_1 = 1;
const int STATE_2 = 2;
const int STATE_3 = 3;
const int STATE_4 = 4;
const int STATE_5 = 5;
const int STATE_6 = 6;
const int STATE_7 = 7;
const int STATE_10 = 10;
const int STATE_20 = 20;
const int STATE_30 = 30;
int last_comm = 0;
int alarm_timer = 5000;
int prev_state = 0;

int flag_alarma = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

ISR(TIMER1_COMPA_vect)
{
    // cod întrerupere
    count++;
    count_comm++;
}

void configure_timer1()
{
    // exemplu de configurare pentru Timer 1 în mod CTC
    // care va genera întreruperi cu frecvența de 2Hz
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 16;             // compare match register 16MHz/256/2Hz-1
    TCCR1B |= (1 << WGM12); // CTC mode
    // TCCR1B |= (1 << CS12);    // 256 prescaler
    TCCR1B |= (1 << CS12) | (1 << CS10); // 1024
}

void init_timer1()
{
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
}

void setup()
{
    Serial.begin(9600); // initialize serial
    IrReceiver.begin(IR_RECEIVE_PIN);
    pinMode(PIN_TO_SENSOR, INPUT); // set arduino pin to input mode to read value from OUTPUT pin of sensor
    pinMode(LED, OUTPUT);
    pinMode(buzzer, OUTPUT);  // Set buzzer - pin 9 as an output
    pinMode(LED_OFF, OUTPUT); // OFF
    digitalWrite(LED_OFF, HIGH);
    pinMode(LED_ON, OUTPUT); // ON
    pinMode(LED_IR, OUTPUT); // IR
    cli();
    configure_timer1();
    init_timer1();
    // activăm întreruperile globale
    sei();
    lcd.init(); // initialize the lcd
    lcd.backlight();
}

void sound()
{

    digitalWrite(buzzer, LOW);
    delay(1000);
    digitalWrite(buzzer, HIGH);
    delay(1000);
    digitalWrite(buzzer, LOW);
}

void loop()
{
    pinStatePrevious = pinStateCurrent;           // store old state
    pinStateCurrent = digitalRead(PIN_TO_SENSOR); // read new state
    if (alarma == 1)
    {
        sound();
    }

    if (flag_alarma == 1 && count >= alarm_timer)
    {
        digitalWrite(LED_ON, HIGH);
        if (pinStatePrevious == LOW && pinStateCurrent == HIGH)
        { // pin state change: LOW -> HIGH
            Serial.println("Motion detected!");
            digitalWrite(LED, HIGH);
            alarma = 1;
        }
        else if (pinStatePrevious == HIGH && pinStateCurrent == LOW)
        { // pin state change: HIGH -> LOW
            Serial.println("Motion stopped!");
            digitalWrite(LED, LOW);
        }
    }

    if (count_comm >= 1000)
    {
        digitalWrite(LED_IR, HIGH);
    }
    else
    {
        digitalWrite(LED_IR, LOW);
    }

    if (IrReceiver.decode())
    {
        // Serial.println("Ai murit???");
        IrReceiver.resume();
        if (count_comm >= 1000)
        {
            command = IrReceiver.decodedIRData.command;
        }
        if (command != 0)
        {
            Serial.println(command);
            count_comm = 0;
        }
    }

    if (prev_state == STATE_4 && count >= alarm_timer)
    {
        STATE = STATE_4;
        prev_state = 0;
        Serial.println("DA PETRADS");
    }

    switch (STATE)
    {
    case STATE_0:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Alarma");
        lcd.setCursor(2, 1);
        lcd.print("Securitate");
        if (command == TASTA_OK)
        {
            STATE = STATE_1;
        }

        break;
    case STATE_1:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("> On/Off");
        lcd.setCursor(0, 1);
        lcd.print("Set timer");
        if (command == TASTA_DOWN)
            STATE = STATE_10;
        if (command == TASTA_LEFT)
            STATE = STATE_0;
        if (command == TASTA_OK)
        {
            STATE = STATE_2;
        }
        break;
    case STATE_10:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("On/Off");
        lcd.setCursor(0, 1);
        lcd.print("> Set timer");
        if (command == TASTA_UP)
            STATE = STATE_1;
        if (command == TASTA_LEFT)
            STATE = STATE_0;
        if (command == TASTA_OK)
            STATE = STATE_3;
        break;
    case STATE_2:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("> ON");
        lcd.setCursor(0, 1);
        lcd.print("OFF");
        if (command == TASTA_DOWN)
            STATE = STATE_20;
        if (command == TASTA_LEFT && prev_state == 0)
            STATE = STATE_1;
        if (command == TASTA_OK)
        {
            STATE = STATE_4;
            flag_alarma = 1;
            digitalWrite(LED_OFF, LOW);
            count = 0;
        }
        break;
    case STATE_20:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ON");
        lcd.setCursor(0, 1);
        lcd.print("> OFF");
        if (command == TASTA_UP)
            STATE = STATE_2;
        if (command == TASTA_LEFT && prev_state == 0)
            STATE = STATE_1;
        if (command == TASTA_OK)
        {
            STATE = STATE_0;
            alarma = 0;
            flag_alarma = 0;
            digitalWrite(LED_OFF, HIGH);
            digitalWrite(LED_ON, LOW);
            prev_state = 0;
        }
        break;
    case STATE_3:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("> 5 sec");
        lcd.setCursor(0, 1);
        lcd.print("10 sec");
        if (command == TASTA_DOWN)
            STATE = STATE_30;
        if (command == TASTA_LEFT)
            STATE = STATE_1;
        if (command == TASTA_OK)
        {
            STATE = STATE_1;
            alarm_timer = 5000;
        }
        break;
    case STATE_30:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("5 sec");
        lcd.setCursor(0, 1);
        lcd.print("> 10 sec");
        if (command == TASTA_UP)
            STATE = STATE_3;
        if (command == TASTA_LEFT)
            STATE = STATE_1;
        if (command == TASTA_OK)
        {
            STATE = STATE_1;
            alarm_timer = 10000;
        }
        break;
    case STATE_4:
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Alarma");
        lcd.setCursor(2, 1);
        lcd.print("activata!!!");
        if (command == TASTA_D)
        {
            alarma = 0;
            flag_alarma = 0;
            STATE = STATE_2;
            digitalWrite(LED_OFF, HIGH);
            digitalWrite(LED_ON, LOW);
        }
        if (command == TASTA_LEFT && count < alarm_timer)
        {
            STATE = STATE_2;
            prev_state = STATE_4;
        }
        break;
    default:
        // code block
        break;
    }

    command = 0;
}
