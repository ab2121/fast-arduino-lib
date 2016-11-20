#ifndef FASTIO_HH
#define	FASTIO_HH

#include "utilities.hh"
#include "Board_traits.hh"
#include <fastarduino/iocommons.hh>

// This class maps to a PORT pin 
// SRAM size is 0
template<Board::Port PORT, uint8_t BIT>
class FastPin
{
private:
	using TRAIT = Board::Port_trait<PORT>;
	
public:
	FastPin(PinMode mode, bool value = false) INLINE
	{
		static_assert(TRAIT::DPIN_MASK & _BV(BIT), "BIT must be compatible with PORT available pins");
		set_mode(mode, value);
	}
	void set_mode(PinMode mode, bool value = false) INLINE
	{
		if (mode == PinMode::OUTPUT)
			set_ioreg_bit(TRAIT::DDR, BIT);
		else
			clear_ioreg_bit(TRAIT::DDR, BIT);
		if (value || mode == PinMode::INPUT_PULLUP)
			set_ioreg_bit(TRAIT::PORT, BIT);
		else
			clear_ioreg_bit(TRAIT::PORT, BIT);
	}
	void set() INLINE
	{
		set_ioreg_bit(TRAIT::PORT, BIT);
	}
	void clear() INLINE
	{
		clear_ioreg_bit(TRAIT::PORT, BIT);
	}
	void toggle() INLINE
	{
		set_ioreg_bit(TRAIT::PIN, BIT);
	}
	bool value() INLINE
	{
		return ioreg_bit_value(TRAIT::PIN, BIT);
	}
};

template<Board::DigitalPin DPIN>
struct FastPinType
{
	using TYPE = FastPin<Board::DigitalPin_trait<DPIN>::PORT, Board::DigitalPin_trait<DPIN>::BIT>;
};

// This class maps to a PORT and handles it all 8 bits at a time
// SRAM size is 0
template<Board::Port PORT>
class FastPort
{
private:
	using TRAIT = Board::Port_trait<PORT>;
	
public:
	FastPort() {}
	FastPort(uint8_t ddr, uint8_t port = 0) INLINE
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	
//	template<uint8_t BIT>
//	FastPin<PORT, BIT> get_pin(PinMode mode, bool value = false) INLINE;
	template<uint8_t BIT>
	FastPin<PORT, BIT> get_pin(PinMode mode, bool value = false)
	{
		return FastPin<PORT, BIT>{mode, value};
	}
	
	void set_PORT(uint8_t port) INLINE
	{
		set_ioreg_byte(TRAIT::PORT, port);
	}
	uint8_t get_PORT() INLINE
	{
		return get_ioreg_byte(TRAIT::PORT);
	}
	void set_DDR(uint8_t ddr) INLINE
	{
		set_ioreg_byte(TRAIT::DDR, ddr);
	}
	uint8_t get_DDR() INLINE
	{
		return get_ioreg_byte(TRAIT::DDR);
	}
	void set_PIN(uint8_t pin) INLINE
	{
		set_ioreg_byte(TRAIT::PIN, pin);
	}
	uint8_t get_PIN() INLINE
	{
		return get_ioreg_byte(TRAIT::PIN);
	}
};


//template<Board::DigitalPin DPIN>
//FastPin<Board::DigitalPin_trait<DPIN>::PORT, Board::DigitalPin_trait<DPIN>::BIT>
//get_pin(PinMode mode, bool value = false) INLINE;
template<Board::DigitalPin DPIN>
FastPin<Board::DigitalPin_trait<DPIN>::PORT, Board::DigitalPin_trait<DPIN>::BIT>
get_pin(PinMode mode, bool value = false)
{
	return FastPin<Board::DigitalPin_trait<DPIN>::PORT, Board::DigitalPin_trait<DPIN>::BIT>{mode, value};
}

template<>
class FastPin<Board::Port::NONE, 0>
{
public:
	FastPin(PinMode mode UNUSED, bool value UNUSED = false) INLINE {}
	void set() INLINE {}
	void clear() INLINE {}
	void toggle() INLINE {}
	bool value() INLINE
	{
		return false;
	}
};

#endif	/* FASTIO_HH */
