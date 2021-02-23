/*****************************************************************/
/*                                                               */
/* RC Transmitter : Arduino Pro Mini 3.3V 8 MHz                  */
/*                                                               */
/*****************************************************************/
#include <SPI.h>
#include <NRFLite.h>
#include "standard.h"
#include "mapping.h"

#define SPY
#define RADIO_ID 1
#define DESTINATION_RADIO_ID 0
#define TX_PERIOD 200 // ms

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

NRFLite radio(Serial);
T_radio_packet radio_data;

#ifdef SPY
void printSerial(UNS8 data, char *format, BOOLEAN last) {
  char formated_data[4];

  sprintf(formated_data, format, data);
  Serial.print(formated_data);
  if (last) {
    Serial.println();
  }
  else {
    Serial.print(","); 
  }
}
#endif // SPY

void setup()
{
    BOOLEAN F_init_radio_ok;

#ifdef SPY
    Serial.begin(115200);
#endif // SPY
    pinMode(L_JOYSTICK_BUTTON, INPUT_PULLUP);
    pinMode(R_JOYSTICK_BUTTON, INPUT_PULLUP);
    pinMode(L_TOGGLE_SWITCH, INPUT_PULLUP);
    pinMode(R_TOGGLE_SWITCH, INPUT_PULLUP);
    pinMode(LO_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(LI_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(RI_PUSH_BUTTON, INPUT_PULLUP);
    pinMode(RO_PUSH_BUTTON, INPUT_PULLUP);
    // By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
    // Both the RX and TX radios must have the same bitrate and channel to communicate with each other.
    // You can run the 'ChannelScanner' example to help select the best channel for your environment.
    // You can assign a different bitrate and channel as shown below.
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE2MBPS, 100) // THE DEFAULT
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE1MBPS, 75)
    //   radio.init(RADIO_ID, PINradio_CE, PINradio_CSN, NRFLite::BITRATE250KBPS, 0)
    F_init_radio_ok = radio.init(RADIO_ID, CE, CSN);
    if (F_init_radio_ok = false)
    {
#ifdef SPY
        Serial.println("Cannot communicate with radio");
#endif // SPY
        while(true == true); // Wait here forever.
    }
    else
    {
    }
#ifdef SPY
    radio.printDetails();
#endif // SPY
    radio_data.from_radio_id = RADIO_ID;
}

void loop()
{
    BOOLEAN F_send_radio_ok;

    radio_data.tx_time = millis();
    // Convert the analog read value from 0 to 1023 into a UNS8 value from 0 to 255
    radio_data.l_x_joystick = map(analogRead(A1), 0, 1023, 0, 255);
    radio_data.l_y_joystick = map(analogRead(A0), 0, 1023, 0, 255);
    radio_data.r_x_joystick = map(analogRead(A2), 0, 1023, 0, 255);
    radio_data.r_y_joystick = map(analogRead(A3), 0, 1023, 0, 255);
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
    // By default, 'send' transmits data and waits for an acknowledgement.  If no acknowledgement is received,
    // it will try again up to 16 times.  This retry logic is built into the radio hardware itself, so it is very fast.
    // You can also perform a NO_ACK send that does not request an acknowledgement.  In this situation, the data packet
    // will only be transmitted a single time and there is no verification of delivery.  So NO_ACK sends are suited for
    // situations where performance is more important than reliability.
    //   radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::REQUIRE_ACK) // THE DEFAULT
    //   radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::NO_ACK)
    F_send_radio_ok = radio.send(DESTINATION_RADIO_ID, &radio_data, sizeof(radio_data), NRFLite::NO_ACK);
    if (true)
//    if (F_send_radio_ok == true) the return from radio.send is not set to true because NO_ACK parameter
    {
#ifdef SPY
        Serial.print(radio_data.tx_time);
        Serial.print(",");
        printSerial(radio_data.l_x_joystick, "%03d", false);
        printSerial(radio_data.l_y_joystick, "%03d", false);
        printSerial(radio_data.r_x_joystick, "%03d", false);
        printSerial(radio_data.r_y_joystick, "%03d", false);
        printSerial(radio_data.l_potentiometer, "%03d", false);
        printSerial(radio_data.r_potentiometer, "%03d", false);
        printSerial(radio_data.l_joystick_button, "%1d", false);
        printSerial(radio_data.r_joystick_button, "%1d", false);
        printSerial(radio_data.l_toggle_switch, "%1d", false);
        printSerial(radio_data.r_toggle_switch, "%1d", false);
        printSerial(radio_data.lo_push_button, "%1d", false);
        printSerial(radio_data.li_push_button, "%1d", false);
        printSerial(radio_data.ri_push_button, "%1d", false);
        printSerial(radio_data.ro_push_button, "%1d", true);
#endif // SPY
    }
    else
    {
#ifdef SPY
        Serial.println("...Failed");
#endif // SPY
        radio_data.failed_tx_count++;
    }
    delay(TX_PERIOD);
}