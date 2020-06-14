#include <mbed.h>

#include "terminal.h"

Thread can_recv_thread;
Thread usb_terminal_thread;

int main() {
	// put your setup code here, to run once:

	while(1) {
		usb_terminal_thread.start(&terminal
	}
}