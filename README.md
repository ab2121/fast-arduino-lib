FastArduino
-----------

FastArduino is a C++, object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU. Its objectives are:

- provide smaller and faster code for the same functionality
- ensure you pay (size and speed) only for what you use
- use real Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage of unexisting features (pins, timers...) for the target MCU
- enforce event-driven programs
- support both ATmega and ATtiny chips

It was originally inspired by Cosa library from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328 and particularly ATtiny84, I found out that the current way Cosa is built has a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks is essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; when virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers, generally limited to hierarchy of 2 levels only,one parent and direct children).

This comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times are increased as most code is inside C++ headers

Also, if you consider using FastArduino for your projects, be aware that FastArduino does not support Arduino API and does not intend to do so some day. That means you will have to first learn FastArduino API (you can use numerous examples provided for that) in order to reap its benefits. FastArduino is definitely not for newcomers to C++ programming as it makes heavy use of C++ specificities. Note that FastArduino currently uses C++11 standard.

In the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino) are built through netbeans, which was hard to initially setup, but much more friendly to use once setup is done. Also netbeans automatically provides makefiles that make it possible to build projects in command line.

Making FastArduino buildable on Arduino IDE is not on my roadmap currently (and probably won't until long, as I'm more than happy with my current setup until now).

My special setup (I work on Windows but compile everything on an Ubuntu virtual machine) is described in ArduinoDevSetup.docx. This document also describes how I setup netbeans for my projects.

Status
------

The project has started about 6 months ago only, hence it does not cover everything yet.

What the project already has:

- Utilities (queue, list, timing)
- IO support: this comes in 2 flavors, one standard implementation and one template-based, optimized for speed and size. It is possible to mix both flavors in one program.
- General Events handling
- Watchdog timer
- Timed (periodic or not) jobs scheduling
- Real Time Timer with microsecond precision
- Power sleep
- Pin Change Interrupt (PCI) handling
- External Pin Interrupt handling
- Hardware UART support (for MCU that support it, ie not for ATtiny)
- Software UART support (for all MCU)
- "C++ like" Input/Output streams (used by UART implementations)
- SPI master support
- WinBond flash memory support
- NRF24L01 device support
- Analog Digital Conversion support (in Single Conversion mode only)
- Power supply voltage measurement support

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino MEGA
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATtiny84 at 8MHz

I intend to later add support for:

- Breadboard ATtiny85 at 8MHz
- Arduino Leonardo

Important! Note that "standard" IO implementation is deemed to disappear very soon, so if you consider starting a project with FastArduino today, prefer using FastIO.

Roadmap
-------

The roadmap of supported features is the following:

1. Fast IO support (done)
2. Watchdog and events support (done)
3. Power management (done)
4. Hardware UART support (done)
5. Pin Change Interrupt support (done)
6. Software UART support (done)
7. SPI support (done)
8. External Pin Interrupt support (done)
9. Real Time Timer support (done)
10. NRF24L01P support (done)
11. SIPO shift register support
12. Analog Input support (done for single conversion mode)

13. General-purpose Timer support (will imply RTT implementation refactoring)
14. Trace support (including PROGMEM strings support)
15. EEPROM support
16. More boards variants support (Leonardo, ATtinyX5)
17. ... To be determined later, based on my own projects needs

Some refactoring is also planned, in order to revisit current way to support multiple boards with different capabilities, based on a "traits" approach. This "trait" approach refactoring has started with initial RTT implementation and is almost finished (as of December, 3rd 2016); further changes are expected in this direction, normally with only little impact (or none at all) on API, until end 2016.

The following features are not in FastArduino roadmap currently:

- PWM Output support
- I2C support
- OWI support
