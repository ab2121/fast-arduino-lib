//   Copyright 2016-2019 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * Hardware UART sample to test begin/end methods.
 * 
 * Wiring:
 * - on Arduino UNO, LEONARDO and MEGA:
 *   - Use standard TX/RX
 */

#include <fastarduino/time.h>
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UART_ISR(0)

static const board::USART USART = board::USART::USART0;

// Buffers for UART
static const uint8_t INPUT_BUFFER_SIZE = 64;
static const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];
static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	// Start UART
	serial::hard::UART<USART> uart{input_buffer, output_buffer};
	streams::ostream out = uart.out();
	streams::istream in = uart.in();

	// Check buffer handling at end()
	// The following should not appear has output buffer is locked until begin() is called
	out << F("BEFORE: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");

	uart.begin(9600);
	// The following should partly appear until UATX is ended and buffer cleared
	out << F("CLEAR: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");
	uart.end(serial::BufferHandling::CLEAR);
	time::delay_ms(2000);

	uart.begin(9600);
	// The following shall fully appear until output buffer is locked in end()
	out << F("FLUSH: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");
	uart.end(serial::BufferHandling::FLUSH);
	time::delay_ms(2000);

	uart.begin(9600);
	// The following shall partly appear  then be complete in full after 2 seconds
	out << F("KEEP: ABCDEFGHIKLMNOPQRSTUVWXYZ\n");
	uart.end(serial::BufferHandling::KEEP);
	time::delay_ms(2000);
	uart.begin(9600);
	time::delay_ms(2000);

	// Start UART
	int value;

	// NOTE: if you type 123 456 (+NL) in console, then 456 will be forgotten
	in >> value;
	out << F("value=") << value << streams::endl;
	time::delay_ms(2000);
	uart.end(serial::BufferHandling::CLEAR);

	uart.begin(9600);
	// NOTE: if you type 456 789 (+NL) in console, then 789 will be available for next step
	in >> value;
	out << F("value=") << value << streams::endl;
	time::delay_ms(2000);
	uart.end(serial::BufferHandling::KEEP);

	uart.begin(9600);
	// NOTE: if you typed 456 789 (+NL) in console beofre then 789 should immediately appear
	in >> value;
	out << F("value=") << value << streams::endl;
	time::delay_ms(2000);
	uart.end(serial::BufferHandling::CLEAR);
}