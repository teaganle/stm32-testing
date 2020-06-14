#include <mbed.h>
#include <mbed_events.h>

#include <list>
#include <map>
#include <string>
#include <vector>

class terminal {
 public:
  /*
   * Constructor that takes a serial device as argument
   * @param port Reference to port that the terminal will handle
   */
  terminal(Serial &port);

  /*
   * Constructor that will default to use the USB serial terminal
   */
  terminal();

 private:
  recv_ISR();

  send_ISR();

  recv_handler();

  send_handler();

  Thread *_terminal_thread;

  /*
   * All instances of the terminal will use the shared event queue
   */
  EventQueue *_event_queue;

  /*
   * The serial device which will host the port
   */
  Serial *_port;

  /*
   * List of all the valid commands for the table and the callback
   * function and number of arguments for that command
   */
  map<string, map<int, Callback>> *command_table;

  /*
   */
  List<string> commands;

  bool recv_flag;
  bool send_flag;
};

terminal::terminal(Serial *port, EventQueue *event_queue) {
  _terminal_thread = new Thread(term_thread);
  _port = port;
  _event_queue = event_queue;
}

terminal::terminal(Serial *port) { terminal(port, mbed_event_queue()); }

void terminal::thread() {
  // fuck shit up boys
}

void terminal::recv_ISR() {
  recv_flag = true;
  _port->attach(NULL);
}

void terminal::send_ISR() {
  // not defined
}

void terminal::recv_handler() {
  string read_in = "";
  vector<string> command;
  char c;

  while (true) {
    c = _port->getc();

    if (c == '\r') {
      break;
    } else if (c == 0x7f && !read_in.empty()) {
      read_in.pop_back();
      _port->printf("\b \b");
    } else {
      read_in += c;
      _port->printf("%c", c);
    }
  }

  _port->printf("\n\r");

  for (int pos = read_in.find(" "); pos < read_in.length();
       pos = read_in.find(" ")) {
    command.push_back(read_in.substr(0, pos));
    read_in.erase(0, pos + 1);
  }
  command.push_back(read_in);

  command_lookup(command[0], command.size());
  _port->attach(&recv_ISR, Serial::RxIrq);
}

/*
 * Looks up the correct callback for a function with the specified command name
 * and number of arguments
 *
 * @param &command Reference to command
 * @param args Number of arguments excluding the command name itself
 * @return Callback to the function
 */
Callback terminal::command_lookup(string &command, int args) {
  return command_table->at(command).at(args);
}