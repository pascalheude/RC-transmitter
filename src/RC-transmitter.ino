/*****************************************************************/
/*                                                               */
/* RC Transmitter : Arduino Pro Mini 5V 8MHz                     */
/*                                                               */
/*****************************************************************/
#include <SPI.h>
//#include <NRFLite.h>
#include <RF24.h>
#include "printf.h"
#include "standard.h"
#include "mapping.h"

//#define SPY
#define SPY_PLOTTER
#define RADIO_ID 1
#define DESTINATION_RADIO_ID 0
#define TX_PERIOD 100 // ms

typedef struct
{
    UNS32 tx_time;
    UNS8 from_radio_id;
    UNS8 failed_tx_count;
    UNS8 l_joystick_button;
    UNS8 l_x_joystick;
    UNS8 l_y_joystick;
    UNS8 r_joystick_button;
    UNS8 r_x_joystick;
    UNS8 r_y_joystick;
    UNS8 l_potentiometer;
    UNS8 r_potentiometer;
    UNS8 l_toggle_switch;
    UNS8 r_toggle_switch;
    UNS8 lo_push_button;
    UNS8 li_push_button;
    UNS8 ri_push_button;
    UNS8 ro_push_button;
} T_radio_packet;

static BOOLEAN F_calibration_done;
static BOOLEAN F_led_high;
static UNS8 CPT_PIT;
static REAL32 TAB_joystick[4];
static REAL32 TAB_joystick_calib[4];
static REAL32 l_x_joystick_calib;
static REAL32 l_y_joystick_calib;
static REAL32 r_x_joystick_calib;
static REAL32 r_y_joystick_calib;
#ifdef __RF24_H__
RF24 radio(CE, CSN, 1000000);
#endif
#ifdef _NRFLite_h_
NRFLite radio;
#endif
T_radio_packet radio_data;

const unsigned char address[6] = "PIPE1";

#define L_X 0
#define L_Y 1
#define R_X 2
#define R_Y 3
#define ANALOG_MIN (REAL32)0.0f
#define ANALOG_MID (REAL32)511.5f
#define ANALOG_MAX (REAL32)1023.0f
#define RADIO_MID (REAL32)127.5f
#define RADIO_MAX (REAL32)255.0f

void setup()
{
#ifdef _NRFLite_h_
    BOOLEAN F_init_radio_ok;
#endif
    UNS8 i;

    F_calibration_done = false;
    F_led_high = false;
    CPT_PIT = 1;
    for(i=L_X;i <= R_Y;i++)
    {
        TAB_joystick_calib[i] = ANALOG_MID;
    }
#if defined(SPY) || defined(SPY_PLOTTER)
    Serial.begin(115200);
    printf_begin(); // To allow the use of printDetails of RF24 library
#endif // SPY || SPY_PLOTTER
    pinMode(L_JOYSTICK_BUTTON, INPUT_PULLUP);
    pinMode(R_JOYSTICK_BUTTON, INPUT_PULLUP);
    pinMode(L_TOGGLE_SWITCH, INPUT_PULLUP);
    pinMode(R_TOGGLE_SWITCH, INPUT_PULLUP);
    pinMode(LO_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(LI_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(RI_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(RO_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    // By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
    // Both the RX and TX radios must have the same bitrate and channel to communicate with each other.
    // You can run the 'ChannelScanner' example to help select the best channel for your environment.
    // You can assign a different bitrate and channel as shown below.
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE2MBPS, 100) // THE DEFAULT
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE1MBPS, 75)
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE250KBPS, 0)
#ifdef _NRFLite_h_
    F_init_radio_ok = radio.init(RADIO_ID, CE, CSN);
    if (F_init_radio_ok == false)
#endif
#ifdef __RF24_H__
    radio.begin();
    radio.setChannel(125);
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_250KBPS);
    radio.setAddressWidth(5);
    radio.setAutoAck(false);
    radio.openWritingPipe(address);
    radio.stopListening();
    if (radio.isChipConnected() == false)
#endif
    {
#if defined(SPY) || defined(SPY_PLOTTER)
        Serial.println("Cannot communicate with radio");
        radio.printDetails();
#endif // SPY || SPY_PLOTTER
        digitalWrite(LED, HIGH);
        while(true == true); // Wait here forever.
    }
    else
    {
#if defined(SPY) || defined(SPY_PLOTTER)
#endif // SPY || SPY_PLOTTER
    }
    radio_data.from_radio_id = RADIO_ID;
}

void loop()
{
    BOOLEAN F_send_radio_ok;
    UNS8 i;
    UNS32 loop_timer;

    loop_timer = millis() + TX_PERIOD;
    radio_data.tx_time = millis();
    // Read the analog inputs [0 - 1023]
    TAB_joystick[L_X] = (REAL32)analogRead(A1);
    TAB_joystick[L_Y] = (REAL32)analogRead(A0);
    TAB_joystick[R_X] = (REAL32)analogRead(A2);
    TAB_joystick[R_Y] = (REAL32)analogRead(A3);
    // Convert the analog read value from 0 to 1023 into a UNS8 value from 0 to 255
    radio_data.l_potentiometer = map(analogRead(A7), 0, 1023, 0, 255);
    radio_data.r_potentiometer = map(analogRead(A6), 0, 1023, 0, 255);
    // Read all digital inputs
    radio_data.l_joystick_button = digitalRead(L_JOYSTICK_BUTTON);
    radio_data.r_joystick_button = digitalRead(R_JOYSTICK_BUTTON);
    radio_data.l_toggle_switch = digitalRead(L_TOGGLE_SWITCH);
    radio_data.r_toggle_switch = digitalRead(R_TOGGLE_SWITCH);
    radio_data.lo_push_button = digitalRead(LO_PUSH_BUTTON);
    radio_data.li_push_button = digitalRead(LI_PUSH_BUTTON);
    radio_data.ri_push_button = digitalRead(RI_PUSH_BUTTON);
    radio_data.ro_push_button = digitalRead(RO_PUSH_BUTTON);
    // Check for calibration request
    if (radio_data.l_toggle_switch == 0)
    {
        if (F_calibration_done == false)
        {
            for(i=L_X;i <= R_Y;i++)
            {
                TAB_joystick_calib[i] = TAB_joystick[i];
            }
            F_calibration_done = true;
        }
        else
        {
        }
    }
    else
    {
        F_calibration_done = false;
    }
    // Correct the analog values
    for(i=L_X;i <= R_Y;i++)
    {
        if (TAB_joystick[i] <= TAB_joystick_calib[i])
        {
            TAB_joystick[i] = RADIO_MID * TAB_joystick[i] / TAB_joystick_calib[i];
        }
        else
        {
            TAB_joystick[i] = (RADIO_MID * TAB_joystick[i] / (ANALOG_MAX - TAB_joystick_calib[i])) +
                            ANALOG_MAX - ((RADIO_MID * ANALOG_MAX) / (ANALOG_MAX - TAB_joystick_calib[i]));
        }
    }
    radio_data.l_x_joystick = (UNS8)TAB_joystick[L_X];
    radio_data.l_y_joystick = (UNS8)TAB_joystick[L_Y];
    radio_data.r_x_joystick = (UNS8)TAB_joystick[R_X];
    radio_data.r_y_joystick = (UNS8)TAB_joystick[R_Y];
    // By default, 'send' transmits data and waits for an acknowledgement.  If no acknowledgement is received,
    // it will try again up to 16 times.  This retry logic is built into the radio hardware itself, so it is very fast.
    // You can also perform a NO_ACK send that does not request an acknowledgement.  In this situation, the data packet
    // will only be transmitted a single time and there is no verification of delivery.  So NO_ACK sends are suited for
    // situations where performance is more important than reliability.
    //   radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::REQUIRE_ACK) // THE DEFAULT
    //   radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::NO_ACK)
#ifdef __RF24_H__
    // When ACK is disabled, the function radio.write returns true
    F_send_radio_ok = radio.write(&radio_data, sizeof(radio_data));
#endif
#ifdef _NRFLite_h_
    // When using NO_ACK parameter, the function radio.send always returns false
    F_send_radio_ok = radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::NO_ACK);
#endif
    if (F_send_radio_ok == true)
    {
#ifdef SPY
        Serial.print(radio_data.tx_time);
        Serial.print(",");
        Serial.print(radio_data.l_x_joystick);
        Serial.print(",");
        Serial.print(radio_data.l_y_joystick);
        Serial.print(",");
        Serial.print(radio_data.r_x_joystick);
        Serial.print(",");
        Serial.print(radio_data.r_y_joystick);
        Serial.print(",");
        Serial.print(radio_data.l_potentiometer);
        Serial.print(",");
        Serial.print(radio_data.r_potentiometer);
        Serial.print(",");
        Serial.print(radio_data.l_joystick_button);
        Serial.print(",");
        Serial.print(radio_data.r_joystick_button);
        Serial.print(",");
        Serial.print(radio_data.l_toggle_switch);
        Serial.print(",");
        Serial.print(radio_data.r_toggle_switch);
        Serial.print(",");
        Serial.print(radio_data.lo_push_button);
        Serial.print(",");
        Serial.print(radio_data.li_push_button);
        Serial.print(",");
        Serial.print(radio_data.ri_push_button);
        Serial.print(",");
        Serial.println(radio_data.ro_push_button);
#endif // SPY
#ifdef SPY_PLOTTER
        Serial.print(">Tx time:");
        Serial.println(radio_data.tx_time);
        Serial.print(">Yaw:");
        Serial.println(radio_data.l_x_joystick);
        Serial.print(">Throttle:");
        Serial.println(radio_data.l_y_joystick);
        Serial.print(">Roll:");
        Serial.println(radio_data.r_x_joystick);
        Serial.print(">Pitch:");
        Serial.println(radio_data.r_y_joystick);
#endif // SPY_PLOTTER
    }
    else
    {
#if defined(SPY) || defined(SPY_PLOTTER)
        Serial.println("No ACK from radio");
#endif // SPY || SPY_PLOTTER
        radio_data.failed_tx_count++;
    }
    while(millis() < loop_timer)
    {
    }
    if (CPT_PIT >= 5)
    {
        CPT_PIT = 1;
        if (F_led_high)
        {
            F_led_high = false;
            digitalWrite(LED, LOW);
        }
        else
        {
            F_led_high = true;
            digitalWrite(LED, HIGH);
        }
    }
    else
    {
        CPT_PIT++;
    }
}