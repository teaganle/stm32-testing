#include <mbed.h>
#include <string>
#include <list>

// the constructors for the CAN interfaces (rd, td)
CAN can_1(PD_0, PD_1);
CAN can_2(PB_5, PB_6);

Serial usb(USBTX, USBRX);

DigitalOut can_led(LED1);
DigitalOut rx_led(LED2);
DigitalOut tx_led(LED3);

bool usb_rx_flag;
bool can_rx_flag;

/*
 * Serial write function that also blinks the LED
 */
void serial_write(Serial &port, string data) {
	tx_led = true;
	port.printf("%s", data.c_str());
	tx_led = false;
}

/*
 * Function to send messages over CAN. It will only send strings of length < 7.
 *
 * @param data The data to send over CAN.
 * @return Whether or not the message was sent successfully.
 */
bool can_send_msg(string data) {
	if (data.length() > 7) {
		return false;
	} else {
		return true;
	}

	CANMessage message(1, data.c_str(), data.length() + 1);
	bool success = false;

	can_led = true;

	if (can_1.write(message)) {
		usb.printf("Sent CAN message: %s\n\r", data.c_str());
		success = true;
	} else {
		usb.printf("Failed to send CAN message: %s\n\r", data.c_str());
		success = false;
	}

	can_led = false;
}

/*
 * Interrupt service routine for USB rx.
 */
void usb_ISR() {
	usb_rx_flag = true;
	usb.attach(NULL);
}

/*
 * Interrupt service routine for CAN rx
 */
void can_ISR() {
	can_rx_flag = true;
	can_led = true;
	can_2.attach(NULL);
}

/*
 * Flushes the serial buffer for a given serial port
 *
 * @param &port Reference to a serial device
 */
void flush_serial_buffer(Serial &port) {
	char c;
	while (port.readable()) {
		c = port.getc();
	}
}

/*
 * Interrupt handler for USB rx. Takes input from the USB port and sends over
 * the CAN bus
 */
void on_usb_rx_interrupt() {
	string read_in = "";
	list<string> messages;
	char c;

	usb.printf("Input: ");

	while(true) {
		c = usb.getc();

		if (c == '\r') {
			messages.push_back(read_in);
			break;
		} else if (c == 0x7f && !read_in.empty()) {
			read_in.pop_back();
			usb.printf("\b \b");
		} else {
			read_in += c;
			usb.printf("%c", c);
		}

		if (read_in.length() == 7) {
			messages.push_back(read_in);
			read_in = "";
		}
	}

	usb.printf("\n\r");
	
	while(!messages.empty()) {
		can_send_msg(messages.front());
		messages.pop_front();
	}

	flush_serial_buffer(usb);
	usb.attach(&usb_ISR, Serial::RxIrq);

}

/*
 * Interrupt handler for CAN rx. Echoes the message from the CAN bus to the
 * serial terminal.
 */
void on_can_rx_interrupt() {
	CANMessage message;

	while (can_2.read(message, 0)) {
		usb.printf("Received CAN message: %s\n\r", message.data);
	}

	can_led = false;
	can_2.attach(&can_ISR, CAN::RxIrq);
}

int main() {
	usb.attach(&usb_ISR, Serial::RxIrq);
	can_2.attach(&can_ISR, CAN::RxIrq);

	int counter = 1;
	
	while(1) {
		if (usb_rx_flag) {
			on_usb_rx_interrupt();
			usb_rx_flag = false;
		}

		if (can_rx_flag) {
			on_can_rx_interrupt();
			can_rx_flag = false;
		}

		can_send_msg(to_string(counter));
		counter++;

		wait_us(1E6);
	}
}
