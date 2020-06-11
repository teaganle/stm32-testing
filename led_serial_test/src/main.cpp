#include "mbed.h"

// define LEDs
static DigitalOut power_led(LED1);
static DigitalOut rx_led(LED2);
static DigitalOut tx_led(LED3);

// define serial port
Serial serial(USBTX, USBRX);

/*
 * Serial read function that also blinks the LED
 */
char serial_read(Serial &port) {
  rx_led = true;
  char c = port.getc();
  rx_led = false;
  return c;
}

/*
 * Serial write function that also blinks the LED
 */
void serial_write(Serial &port, char write) {
  tx_led = true;
  port.putc(write);
  tx_led = false;
}

/*
 * Interrupt handler for when a serial message is received
 */
void on_rx_interrupt() {
  serial_write(serial, serial_read(serial));
}

int main() {
  // put your setup code here, to run once:

  // set serial port to 8N1 at 9600 baud
  serial.baud(9600);
  serial.format(8, SerialBase::None, 1);

  serial.attach(&on_rx_interrupt, SerialBase::RxIrq);

  // repeatedly blink the power LED at 1 Hz
  while(1) {
    power_led = !power_led;
    wait(1);
  }
}