//   Copyright 2016-2017 Jean-Francois Poilpret
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

/// @cond api

/**
 * @file 
 * General Purpose (digital) Input Output API.
 */
#ifndef FASTIO_HH
#define	FASTIO_HH

#include "utilities.h"
#include "boards/board_traits.h"

/**
 * Defines all API to manipulate general-purpose digital input/output pins.
 */
namespace gpio
{
	/**
	 * Defines the configurable mode of a digital IO pin.
	 */
	enum class PinMode: uint8_t
	{
		/** Digital pin is configured as high-impedance (open drain) input. */
		INPUT,
		/** Digital pin is configured as input with an internal pullup resistor. */
		INPUT_PULLUP,
		/** Digital pin is configured as output. */
		OUTPUT,
	};

	/// @cond experimental
	// Experimental API, maybe removed if judged bad
	template<board::Port PORT, uint8_t BIT> class FastPin;
	class SlowPin
	{
	public:
		void set()
		{
			PORT_ |= BIT_;
		}
		void clear()
		{
			PORT_ &= ~BIT_;
		}
		void toggle()
		{
			PIN_ |= BIT_;
		}
		bool value()
		{
			return PIN_ & BIT_;
		}
		void set_mode(PinMode mode, bool value)
		{
			if (mode == PinMode::OUTPUT)
				DDR_ |= BIT_;
			else
				DDR_ &= ~BIT_;
			if (value || mode == PinMode::INPUT_PULLUP)
				PORT_ |= BIT_;
			else
				PORT_ &= ~BIT_;
		}
		
	private:
		SlowPin(volatile uint8_t& DDR, volatile uint8_t& PIN, volatile uint8_t& PORT, uint8_t BIT)
			:DDR_{DDR}, PIN_{PIN}, PORT_{PORT}, BIT_{BIT} {}
		
		volatile uint8_t& DDR_;
		volatile uint8_t& PIN_;
		volatile uint8_t& PORT_;
		const uint8_t BIT_;
		
		template<board::Port PORT, uint8_t BIT>
		friend class FastPin;
	};
	/// @endcond

	/**
	 * API that manipulates one digital IO pin of a given port.
	 * Implementation is highly optimized for size and speed: instances use
	 * no SRAM at all, most common methods use only 1 AVR instruction.
	 * 
	 * Note that, although it has public constructors, you generally do not 
	 * construct `FastPin` instances directly. You better use `FastPinType`,
	 * which is directly provided a `board::DigitalPin` constant. You may also
	 * obtain a `FastPin` instance through `FastPort::get_pin()`.
	 * 
	 * @tparam PORT_ the target port to which this pin belongs
	 * @tparam BIT_ the bit position (from `0` to `7`), in port, of this pin;
	 * note that if this position is not mapped to a physical IO of the MCU target,
	 * then a compilation error will occur.
	 * @sa board::Port
	 * @sa gpio::FastPinType
	 * @sa gpio::FastPort::get_pin()
	 */
	template<board::Port PORT_, uint8_t BIT_>
	class FastPin
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		/** The port to which this pin belongs. */
		static constexpr const board::Port PORT = PORT_;
		/** The bit position (from `0` to `7`), in port, of this pin. */
		static constexpr const uint8_t BIT = BIT_;

		/**
		 * Construct a `FastPin` without any physical setup on target MCU.
		 * This is useful if default pin direction and value are OK for you and 
		 * you want to avoid calling mode setup on target MCU.
		 */
		FastPin() INLINE
		{
			static_assert(TRAIT::DPIN_MASK & _BV(BIT), "BIT must be compatible with PORT available pins");
		}
		
		/**
		 * Construct a `FastPin` with the given mode and initial value.
		 * The pin mode is forced on the target MCU.
		 * 
		 * @param mode the mode of this pin
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 * @sa set_mode()
		 */
		FastPin(PinMode mode, bool value = false) INLINE
		{
			static_assert(TRAIT::DPIN_MASK & _BV(BIT), "BIT must be compatible with PORT available pins");
			set_mode(mode, value);
		}
		
		/**
		 * Set mode (direction) and value (if output) of this pin.
		 * 
		 * @param mode the mode of this pin
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 */
		void set_mode(PinMode mode, bool value = false) INLINE
		{
			if (mode == PinMode::OUTPUT)
				TRAIT::DDR |= _BV(BIT);
			else
				TRAIT::DDR &= ~_BV(BIT);
			if (value || mode == PinMode::INPUT_PULLUP)
				TRAIT::PORT |= _BV(BIT);
			else
				TRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Set pin level to `HIGH` (i\.e\. Vcc).
		 * This method will generally use 1 single instruction in most cases.
		 */
		void set() INLINE
		{
			TRAIT::PORT |= _BV(BIT);
		}
		
		/**
		 * Set pin level to `LOW` (i\.e\. GND).
		 * This method will generally use 1 single instruction in most cases.
		 */
		void clear() INLINE
		{
			TRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Toggle pin level, i\.e\. set it to `LOW` if it was `HIGH`, and `HIGH` 
		 * if it was `LOW`.
		 * This method will generally use 1 single instruction in most cases.
		 */
		void toggle() INLINE
		{
			TRAIT::PIN |= _BV(BIT);
		}
		
		/**
		 * Return the current level of this pin.
		 * @retval true if current pin  level is `HIGH` (i\.e\. Vcc)
		 * @retval false if current pin  level is `LOW` (i\.e\. GND)
		 */
		bool value() INLINE
		{
			return TRAIT::PIN & _BV(BIT);
		}

		/// @cond experimental
		SlowPin as_slow_pin()
		{
			return SlowPin{TRAIT::DDR, TRAIT::PORT, TRAIT::PIN, BIT};
		}
		/// @endcond
	};

	/**
	 * API that manipulates a whole digital IO port.
	 * Implementation is highly optimized for size and speed: instances use
	 * no SRAM at all, most common methods use only 2 AVR instructions.
	 * 
	 * Note that using this API means that every method manipulates ALL pins of 
	 * a port at a time. If you need to handle several, but not all, pins of a 
	 * port, consider using gpio::FastMaskedPort instead.
	 * 
	 * @tparam PORT_ the target port
	 * @sa board::Port
	 * @sa gpio::FastMaskedPort
	 */
	template<board::Port PORT_>
	class FastPort
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		/** The actual port in target MCU. */
		static constexpr const board::Port PORT = PORT_;

		/**
		 * Construct a `FastPort` without any physical setup on target MCU.
		 * This is useful if default pins directions and values are OK for you and 
		 * you want to avoid calling mode setup on target MCU.
		 */
		FastPort() {}
		
		/**
		 * Construct a `FastPort` with the given direction byte and initial values
		 * byte.
		 * The pins mode is forced on the target MCU.
		 * 
		 * @param ddr the direction to set (in `DDR` register of this port) for 
		 * each pin (1 bit is one pin, when `1` the pin is set as output, 
		 * when `0` as input).
		 * @param port the initial values for `PORT` register of this port; each  
		 * bit is for one pin of the port, its meaning depends on the pin direction: 
		 * if input, then it fixes if pullup resistor should be used, if output,
		 * then it fixes the output level of the pin.
		 * @sa set_DDR()
		 * @sa set_PORT()
		 */
		FastPort(uint8_t ddr, uint8_t port = 0) INLINE
		{
			set_DDR(ddr);
			set_PORT(port);
		}

		/**
		 * Create a `FastPin` instance for a given pin of this port, and sets its
		 * direction mode and level value (if output).
		 * Note that you would normally not need this method, as each pin should
		 * normally already set properly first through this port.
		 * You should rather use `get_pin()` instead.
		 * 
		 * @tparam BIT the bit position for which to return a `FastPin` instance;
		 * if there is no pin at this bit position, a compilation error will occur.
		 * @return a `FastPin` instance allowing direct manipulation of the given 
		 * pin
		 * @sa get_pin()
		 */
		template<uint8_t BIT>
		FastPin<PORT, BIT> get_pin(PinMode mode, bool value = false)
		{
			return FastPin<PORT, BIT>{mode, value};
		}

		/**
		 * Create a `FastPin` instance for a given pin of this port.
		 * No additional setup is performed on that pin: it just keeps its current
		 * direction and value.
		 * 
		 * @tparam BIT the bit position for which to return a `FastPin` instance;
		 * if there is no pin at this bit position, a compilation error will occur.
		 * @return a `FastPin` instance allowing direct manipulation of the given 
		 * pin
		 */
		template<uint8_t BIT>
		FastPin<PORT, BIT> get_pin()
		{
			return FastPin<PORT, BIT>{};
		}

		/**
		 * Set the 8-bits value for port PORT register.
		 * If a pin is set currently set as output, then the matching bit in `port`
		 * will set the output level of this pin.
		 * If a pin is set currently as input, then the matching bit in `port`
		 * defines if a pullup resistor is used or not.
		 * 
		 * @param port the initial values for `PORT` register of this port; each  
		 * bit is for one pin of the port, its meaning depends on the pin direction: 
		 * if input, then it fixes if pullup resistor should be used, if output,
		 * then it fixes the output level of the pin.
		 * @sa set_DDR()
		 */
		void set_PORT(uint8_t port) INLINE
		{
			TRAIT::PORT = port;
		}
		
		/**
		 * Get the current 8-bit value of port PORT register.
		 * Each bit maps to a pin configuration in this port.
		 * Depending on DDR configuration for a pin, the PORT value is interpreted
		 * differently: for an input pin, the matching PORT bit indicates if this
		 * pin has a pullup register on it.
		 * 
		 * @return the value of PORT register
		 */
		uint8_t get_PORT() INLINE
		{
			return TRAIT::PORT;
		}
		
		/**
		 * Set the 8-bits value for port DDR (direction) register.
		 * Each pin direction is decided by the matching bit.
		 * 
		 * @param ddr the direction to set (in `DDR` register of this port) for 
		 * each pin (1 bit is one pin, when `1` the pin is set as output, 
		 * when `0` as input).
		 * @sa set_PORT()
		 */
		void set_DDR(uint8_t ddr) INLINE
		{
			TRAIT::DDR = ddr;
		}
		
		/**
		 * Get the current 8-bit value of port DDR (direction) register.
		 * Each pin direction is decided by the matching bit.
		 * 
		 * @return the value of DDR register
		 */
		uint8_t get_DDR() INLINE
		{
			return TRAIT::DDR;
		}
		
		/**
		 * Set the 8-bits value for port PIN register. Writing a `1` bit in this
		 * register will toggle the matching PORT bit; writing `0` has no effect.
		 * 
		 * @param pin the value to write to PIN register for this port
		 */
		void set_PIN(uint8_t pin) INLINE
		{
			TRAIT::PIN = pin;
		}

		/**
		 * Get the 8-bits value of PIN register for this port, i\.e\. the current
		 * level of every pin of the port, be it an output or an input pin.
		 * 
		 * @return the value of PIN register
		 */
		uint8_t get_PIN() INLINE
		{
			return TRAIT::PIN;
		}
	};

	//TODO Infer whether mask should be a template parameter instead?
	//TODO or at least a const?
	/**
	 * API that manipulates a part of a digital IO port.
	 * Implementation is highly optimized for size and speed: instances use
	 * 1 byte SRAM only.
	 * 
	 * Using this API allows you to manipulate several pins of a port at once,
	 * but without having to care for other pins of that port: the API always 
	 * ensure that only those selected pins get modified.
	 * This is useful when, for instance, you handle a 4x4 keypad with one port,
	 * 4 pins out and 4 pins in, then you can define 2 `FastMaskedPort` instances,
	 * one for output pins, the other for input pins.
	 * 
	 * Note that, although more efficient than using individual `FastPin`s, it is
	 * not as efficient as using only one single `FastPort` to handle all its pins.
	 * 
	 * @tparam PORT_ the target port
	 * @sa board::Port
	 */
	template<board::Port PORT_>
	class FastMaskedPort
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		/** The actual port in target MCU. */
		static constexpr const board::Port PORT = PORT_;

		/**
		 * Construct a `FastMaskedPort` without any physical setup on target MCU.
		 * This is useful if default pins directions and values are OK for you and 
		 * you want to avoid calling mode setup on target MCU.
		 * 
		 * @param mask the bit mask determining which pins of the port are handled
		 * by this instance; only these pins will be impacted by `FastMaskedPort` 
		 * methods.
		 */
		FastMaskedPort(uint8_t mask = 0):_mask{mask} {}

		/**
		 * Construct a `FastMaskedPort` for the pins selected by the provide 
		 * bits mask, with the given direction byte and initial values
		 * byte.
		 * The pins mode are forced on the target MCU.
		 * 
		 * @param mask the bit mask determining which pins of the port are handled
		 * by this instance; only these pins will be impacted by `FastMaskedPort` 
		 * methods.
		 * @param ddr the direction to set (in `DDR` register of this port) for 
		 * each pin (1 bit is one pin, when `1` the pin is set as output, 
		 * when `0` as input).
		 * @param port the initial values for `PORT` register of this port; each  
		 * bit is for one pin of the port, its meaning depends on the pin direction: 
		 * if input, then it fixes if pullup resistor should be used, if output,
		 * then it fixes the output level of the pin.
		 * @sa set_DDR()
		 * @sa set_PORT()
		 */
		FastMaskedPort(uint8_t mask, uint8_t ddr, uint8_t port = 0)
		:_mask{mask}
		{
			set_DDR(ddr);
			set_PORT(port);
		}

		/**
		 * Set the 8-bits value for port PORT register, this value will be masked
		 * according to the provided bit mask provided in constructor.
		 * If a pin is set currently set as output, then the matching bit in `port`
		 * will set the output level of this pin.
		 * If a pin is set currently as input, then the matching bit in `port`
		 * defines if a pullup resistor is used or not.
		 * 
		 * @param port the initial values for `PORT` register of this port; each  
		 * bit is for one pin of the port, its meaning depends on the pin direction: 
		 * if input, then it fixes if pullup resistor should be used, if output,
		 * then it fixes the output level of the pin.
		 * @sa set_DDR()
		 */
		void set_PORT(uint8_t port) INLINE
		{
			TRAIT::PORT = (TRAIT::PORT & ~_mask) | (port & _mask);
		}
		
		/**
		 * Get the current 8-bit value of port PORT register, masked according to
		 * the bit mask provided at construction time.
		 * Each bit maps to a pin configuration in this port.
		 * For all pins not part of the mask, returned value is `0`.
		 * Depending on DDR configuration for a pin, the PORT value is interpreted
		 * differently: for an input pin, the matching PORT bit indicates if this
		 * pin has a pullup register on it.
		 * 
		 * @return the value of PORT register masked with constructor-provided bit mask
		 */
		uint8_t get_PORT() INLINE
		{
			return TRAIT::PORT & _mask;
		}
		
		/**
		 * Set the 8-bits value for port DDR (direction) register, this value 
		 * will be masked according to the provided bit mask provided in constructor.
		 * Each pin direction is decided by the matching bit.
		 * 
		 * @param ddr the direction to set (in `DDR` register of this port) for 
		 * each pin (1 bit is one pin, when `1` the pin is set as output, 
		 * when `0` as input).
		 * @sa set_PORT()
		 */
		void set_DDR(uint8_t ddr) INLINE
		{
			TRAIT::DDR = (TRAIT::DDR & ~_mask) | (ddr & _mask);
		}
		
		/**
		 * Get the current 8-bit value of port DDR (direction) register, masked 
		 * according to the bit mask provided at construction time.
		 * Each pin direction is decided by the matching bit.
		 * For all pins not part of the mask, returned value is `0`.
		 * 
		 * @return the value of DDR register masked with constructor-provided bit mask
		 */
		uint8_t get_DDR() INLINE
		{
			return TRAIT::DDR & _mask;
		}

		/**
		 * Set the 8-bits value for port PIN register, this value will be masked 
		 * according to the provided bit mask provided in constructor.
		 * Writing a `1` bit in this register will toggle the matching PORT bit; 
		 * writing `0` has no effect.
		 * 
		 * @param pin the value to write to PIN register for this port
		 */
		void set_PIN(uint8_t pin) INLINE
		{
			TRAIT::PIN = pin & _mask;
		}
		
		/**
		 * Get the current 8-bit value of PIN register for this port, masked 
		 * according to the bit mask provided at construction time.
		 * 
		 * @return the value of PIN register masked with constructor-provided bit mask
		 */
		uint8_t get_PIN() INLINE
		{
			return TRAIT::PIN & _mask;
		}

	private:
		uint8_t _mask;
	};

	/**
	 * API that manipulates a given digital IO pin of a the target MCU.
	 * It provides static methods to directly manipulate an IO pin (mode, level...)
	 * but it also helps you find out the exact `FastPin` type for a given IO pin.
	 * 
	 * Implementation is highly optimized for size and speed: it uses
	 * no SRAM at all, most common methods use only 1 AVR instruction.
	 * 
	 * The following snippet demonstrates usage of `FastPinType` to declare a 
	 * `FastPin` instance for later use in a function:
	 * 
	 * @code
	 * void f()
	 * {
	 *     gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
	 *     led.clear();
	 *     ...
	 *     led.set();
	 * }
	 * @endcode
	 * 
	 * The next snippet demonstrate direct use of `FastPinType` static methods, 
	 * when you don't need an instance of `FastPin`:
	 * 
	 * @code
	 * void f()
	 * {
	 *     gpio::FastPinType<board::DigitalPin::LED>::set_mode(gpio::PinMode::INPUT);
	 *     ...
	 * }
	 * @endcode
	 * 
	 * @tparam DPIN a unique digital pin for the MCU target
	 * @sa gpio::DigitalPin
	 * @sa gpio::FastPin
	 */
	template<board::DigitalPin DPIN>
	class FastPinType
	{
	private:
		using TRAIT = board_traits::DigitalPin_trait<DPIN>;
		using PTRAIT = board_traits::Port_trait<TRAIT::PORT>;
		
	public:
		/** The port to which `DPIN` belongs. */
		static constexpr const board::Port PORT = TRAIT::PORT;
		/** The bit position of `DPIN` within its port. */
		static constexpr const uint8_t BIT = TRAIT::BIT;
		/** The bit-mask to use when accessing `DPIN` through `PORT`. */
		static constexpr const uint8_t MASK = _BV(BIT);
		
		/** The exact `FastPin` parameterized type for `DPIN` IO pin. */
		using TYPE = FastPin<PORT, BIT>;
		/** The exact `FastPort` parameterized type that `DPIN` IO pin belongs to. */
		using PORT_TYPE = FastPort<PORT>;
		
		/**
		 * Set mode (direction) and value (if output) of `DPIN`.
		 * 
		 * @param mode the mode of `PIN`
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 */
		static void set_mode(PinMode mode, bool value = false)
		{
			if (mode == PinMode::OUTPUT)
				PTRAIT::DDR |= _BV(BIT);
			else
				PTRAIT::DDR &= ~_BV(BIT);
			if (value || mode == PinMode::INPUT_PULLUP)
				PTRAIT::PORT |= _BV(BIT);
			else
				PTRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Set pin level to `HIGH` (i\.e\. Vcc).
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void set()
		{
			PTRAIT::PORT |= _BV(BIT);
		}
		
		/**
		 * Set pin level to `LOW` (i\.e\. GND).
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void clear()
		{
			PTRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Toggle pin level, i\.e\. set it to `LOW` if it was `HIGH`, and `HIGH` 
		 * if it was `LOW`.
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void toggle()
		{
			PTRAIT::PIN |= _BV(BIT);
		}
		
		/**
		 * Return the current level pin `DPIN`.
		 * @retval true if current pin  level is `HIGH` (i\.e\. Vcc)
		 * @retval false if current pin  level is `LOW` (i\.e\. GND)
		 */
		static bool value()
		{
			return PTRAIT::PIN & _BV(BIT);
		}
	};

	/// @cond notdocumented
	template<>
	class FastPinType<board::DigitalPin::NONE>
	{
	public:
		static constexpr const board::Port PORT = board::Port::NONE;
		static constexpr const uint8_t BIT = 0;
		static constexpr const uint8_t MASK = 0;
		using TYPE = FastPin<PORT, BIT>;
		using PORT_TYPE = FastPort<PORT>;
		
		static void set_mode(UNUSED PinMode mode, UNUSED bool value = false) {}
		static void set() {}
		static void clear() {}
		static void toggle() {}
		static bool value()
		{
			return false;
		}
	};
	/// @endcond

	/// @cond notdocumented
	template<>
	class FastPin<board::Port::NONE, 0>
	{
	public:
		FastPin() INLINE {}
		FastPin(PinMode mode UNUSED, bool value UNUSED = false) INLINE {}
		void set() INLINE {}
		void clear() INLINE {}
		void toggle() INLINE {}
		bool value() INLINE
		{
			return false;
		}
	};
	/// @endcond
}

#endif	/* FASTIO_HH */
/// @endcond
