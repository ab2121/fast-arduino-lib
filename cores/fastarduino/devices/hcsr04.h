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

#ifndef HCSR04_H
#define HCSR04_H

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

// Utilities to handle ISR callbacks
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)										\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "ECHO must be an INT pin.");			\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,					\
	"ECHO INT number must match INT_NUM");															\
ISR(CAT3(INT, INT_NUM, _vect))																		\
{																									\
	using TRAIT = board_traits::Timer_trait<TIMER>;													\
	TRAIT::TYPE counter = TRAIT::TCNT;																\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_INT;		\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;					\
	CALL_HANDLER_RETURN_(SONAR_HANDLER, &SONAR_HANDLER::on_pin_change, bool, TRAIT::TYPE)(counter);	\
}

#define CALL_HCSR4_(ECHO, DUMMY)																	\
{																									\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_PCINT;		\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;					\
	CALL_HANDLER_RETURN_(SONAR_HANDLER, &SONAR_HANDLER::on_pin_change, bool, TRAIT::TYPE)(counter);	\
}

#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)								\
FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, ECHO, ##__VA_ARGS__)											\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	FOR_EACH(CALL_HCSR4_, EMPTY, ECHO, ##__VA_ARGS__)											\
}

#define CALL_DISTINCT_HCSR4_(SONAR, DUMMY)	\
CALL_HANDLER_RETURN_(SONAR, &SONAR::on_pin_change, bool, TRAIT::TYPE)(counter);

#define REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, SONAR, ...)							\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	FOR_EACH(CALL_DISTINCT_HCSR4_, EMPTY, SONAR, ##__VA_ARGS__)									\
}

#define REGISTER_HCSR04_ICP_ISR(TIMER_NUM, TRIGGER, ECHO)										\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_ICP;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	CALL_HANDLER_RETURN_(SONAR_HANDLER, &SONAR_HANDLER::on_capture, bool, TRAIT::TYPE)(capture);\
}

#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)		\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");		\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,				\
	"PIN INT number must match INT_NUM");														\
ISR(CAT3(INT, INT_NUM, _vect))																	\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_INT;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(counter);									\
}

#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)				\
static_assert(board_traits::DigitalPin_trait< ECHO >::IS_INT, "PIN must be an INT pin.");		\
static_assert(board_traits::ExternalInterruptPin_trait< ECHO >::INT == INT_NUM ,				\
	"PIN INT number must match INT_NUM");														\
ISR(CAT3(INT, INT_NUM, _vect))																	\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_INT;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALLBACK (counter);																		\
}

#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)		\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																	\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_PCINT;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(counter);									\
}

#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)				\
CHECK_PCI_PIN_(ECHO, PCI_NUM)																	\
ISR(CAT3(PCINT, PCI_NUM, _vect))																\
{																								\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	TRAIT::TYPE counter = TRAIT::TCNT;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_PCINT;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_pin_change(counter))														\
		CALLBACK (counter);																		\
}

#define REGISTER_HCSR04_ICP_ISR_METHOD(TIMER_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)				\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_ICP;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_capture(capture))															\
		CALL_HANDLER_(HANDLER, CALLBACK, TRAIT::TYPE)(capture);									\
}

#define REGISTER_HCSR04_ICP_ISR_FUNCTION(TIMER_NUM, TRIGGER, ECHO, CALLBACK)					\
ISR(CAT3(TIMER, TIMER_NUM, _CAPT_vect))															\
{																								\
	constexpr board::Timer TIMER = CAT(board::Timer::TIMER, TIMER_NUM);							\
	using TRAIT = board_traits::Timer_trait<TIMER>;												\
	static_assert(TRAIT::ICP_PIN == ECHO, "ECHO must be an ICP pin.");							\
	TRAIT::TYPE capture = TRAIT::ICR;															\
	static const devices::sonar::SonarType SONARTYPE = devices::sonar::SonarType::ASYNC_ICP;	\
	using SONAR_HANDLER = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, SONARTYPE>;				\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR_HANDLER);										\
	auto handler = SONAR_HOLDER::handler();														\
	if (handler->on_capture(capture))															\
		CALLBACK (capture);																		\
}

#define REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(PCI_NUM, SONAR, HANDLER, CALLBACK)	\
ISR(CAT3(PCINT, PCI_NUM, _vect))												\
{																				\
	static_assert(SONAR::ECHO_PORT == board_traits::PCI_trait< PCI_NUM >::PORT,	\
		"SONAR::ECHO_PORT port must match PCI_NUM port");						\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR);								\
	auto handler = SONAR_HOLDER::handler();										\
	auto event = handler->on_pin_change();										\
	if (event.ready() || event.started())										\
		CALL_HANDLER_(HANDLER, CALLBACK, decltype(event))(event);				\
}

#define REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(PCI_NUM, SONAR, CALLBACK)		\
ISR(CAT3(PCINT, PCI_NUM, _vect))												\
{																				\
	static_assert(SONAR::ECHO_PORT == board_traits::PCI_trait< PCI_NUM >::PORT,	\
		"SONAR::ECHO_PORT port must match PCI_NUM port");						\
	using SONAR_HOLDER = HANDLER_HOLDER_(SONAR);								\
	auto handler = SONAR_HOLDER::handler();										\
	auto event = handler->on_pin_change();										\
	if (event.ready || event.started)											\
		CALLBACK (event);														\
}

namespace devices
{
namespace sonar
{
	static constexpr const uint32_t SPEED_OF_SOUND = 340UL;
	
	// Conversion methods
	static constexpr uint16_t echo_us_to_distance_mm(uint16_t echo_us)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Divide by 2 as echo time includes full sound round-trip
		return uint16_t(echo_us * SPEED_OF_SOUND / 1000UL / 2UL);
	}

	static constexpr uint16_t distance_mm_to_echo_us(uint16_t distance_mm)
	{
		// 340 m/s => 340000mm in 1000000us => 340/1000 mm/us
		// Multiply by 2 as echo time must include full sound round-trip
		return uint16_t(distance_mm * 1000UL * 2UL / SPEED_OF_SOUND);
	}

	enum class SonarType
	{
		BLOCKING,
		ASYNC_INT,
		ASYNC_PCINT,
		ASYNC_ICP
	};

	template<board::Timer TIMER_>
	class AbstractSonar
	{
	public:
		using TIMER = timer::Timer<TIMER_>;
		using TYPE = typename TIMER::TIMER_TYPE;

		inline bool ready() const
		{
			return status_ & READY;
		}
		
		inline TYPE latest_echo_ticks() const
		{
			if (sizeof(TYPE) > 1)
				synchronized return echo_pulse_;
			else
				return echo_pulse_;
		}

	protected:
		AbstractSonar(TIMER& timer):timer_{timer}, echo_pulse_{0}, status_{0} {}

		TYPE async_echo_ticks(TYPE timeout_ticks)
		{
			// Wait for echo signal start
			while (!(status_ & READY))
				if (timer_.ticks() >= timeout_ticks)
				{
					status_ = READY;
					return 0;
				}
			return echo_pulse_;
		}

		template<board::DigitalPin ECHO>
		TYPE blocking_echo_ticks(typename gpio::FastPinType<ECHO>::TYPE& echo, TYPE timeout_ticks)
		{
			timer_.reset();
			while (!echo.value())
				if (timer_.ticks() >= timeout_ticks)
					return 0;
			echo_pulse_ = timer_.ticks();
			status_ = STARTED;
			// Wait for echo signal end
			while (echo.value())
				if (timer_.ticks() >= timeout_ticks)
					return 0;
			echo_pulse_ = timer_.ticks() - echo_pulse_;
			status_ = READY;
			return echo_pulse_;
		}

		inline void trigger_sent(bool capture, bool reset)
		{
			if (capture) timer_.set_input_capture(timer::TimerInputCapture::RISING_EDGE);
			if (reset) timer_.reset();
			status_ = 0;
		}

		inline bool pulse_edge(bool rising, TYPE ticks)
		{
			if (rising && status_ == 0)
			{
				echo_pulse_ = ticks;
				status_ = STARTED;
			}
			else if ((!rising) && status_ == STARTED)
			{
				echo_pulse_ = ticks - echo_pulse_;
				status_ = READY;
				return true;
			}
			return false;
		}

		inline bool pulse_captured(TYPE capture)
		{
			bool rising = (timer_.input_capture() == timer::TimerInputCapture::RISING_EDGE);
			if (rising)
				timer_.set_input_capture(timer::TimerInputCapture::FALLING_EDGE);
			return pulse_edge(rising, capture);
		}

	private:
		TIMER& timer_;
		volatile TYPE echo_pulse_;

		static constexpr const uint8_t READY = 0x01;
		static constexpr const uint8_t STARTED = 0x02;
		volatile uint8_t status_;
	};

	template<
		board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, 
		SonarType SONAR_TYPE_ = SonarType::BLOCKING>
	class HCSR04: public AbstractSonar<TIMER_>
	{
	private:
		using PARENT = AbstractSonar<TIMER_>;
		using TIMER_TRAIT = board_traits::Timer_trait<TIMER_>;
		using ECHO_PIN_TRAIT = board_traits::DigitalPin_trait<ECHO_>;
		using ECHO_PORT_TRAIT = board_traits::Port_trait<ECHO_PIN_TRAIT::PORT>;
		static_assert(SONAR_TYPE_ != SonarType::ASYNC_ICP || TIMER_TRAIT::ICP_PIN == ECHO_, 
			"SONAR_TYPE == ASYNC_ICP but ECHO is not an ICP pin");
		static_assert(SONAR_TYPE_ != SonarType::ASYNC_INT || ECHO_PIN_TRAIT::IS_INT, 
			"SONAR_TYPE == ASYNC_INT but ECHO is not an INT pin");
		static_assert(SONAR_TYPE_ != SonarType::ASYNC_PCINT || ECHO_PORT_TRAIT::PCINT != 0xFF, 
			"SONAR_TYPE == ASYNC_PCINT but ECHO is not an PCI pin");
				
	public:
		using TIMER = timer::Timer<TIMER_>;
		using TYPE = typename TIMER::TIMER_TYPE;
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		HCSR04(TIMER& timer)
			:	PARENT{timer}, 
				trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT} {}

		inline void register_handler()
		{
			static_assert(SONAR_TYPE_ != SonarType::BLOCKING, 
				"register_handler() must not be called with SonarType::BLOCKING");
			interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		TYPE echo_ticks(TYPE timeout_ticks)
		{
			async_echo();
			return await_echo_ticks(timeout_ticks);
		}

		TYPE await_echo_ticks(TYPE timeout_ticks)
		{
			if (SONAR_TYPE_ != SonarType::BLOCKING)
				return this->async_echo_ticks(timeout_ticks);
			else
				return this->template blocking_echo_ticks<ECHO_>(echo_, timeout_ticks);
		}
		
		// We want to avoid using await_echo_us() to handle state & timeout!
		void async_echo(bool trigger = true)
		{
			this->trigger_sent(SONAR_TYPE_ == SonarType::ASYNC_ICP, trigger);
			if (trigger) this->trigger();
		}

		bool on_pin_change(TYPE ticks)
		{
			static_assert(SONAR_TYPE_ == SonarType::ASYNC_INT || SONAR_TYPE_ == SonarType::ASYNC_PCINT, 
				"on_pin_change() must be called only with SonarType::ASYNC_INT or ASYNC_PCINT");
			return this->pulse_edge(echo_.value(), ticks);
		}

		bool on_capture(TYPE capture)
		{
			static_assert(SONAR_TYPE_ == SonarType::ASYNC_ICP, 
				"on_capture() must be called only with SonarType::ASYNC_ICP");
			return this->pulse_captured(capture);
		}

	private:
		inline void trigger()
		{
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}
		
		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		typename gpio::FastPinType<TRIGGER_>::TYPE trigger_;
		typename gpio::FastPinType<ECHO_>::TYPE echo_;
	};

	template<board::Timer TIMER_>
	struct SonarEvent
	{
	public:
		using TYPE = typename board_traits::Timer_trait<TIMER_>::TYPE;

		SonarEvent():started_{}, ready_{}, ticks_{} {}
		SonarEvent(uint8_t started, uint8_t ready, TYPE ticks)
			:started_{started}, ready_{ready}, ticks_{ticks} {}

		uint8_t started() const
		{
			return started_;
		}
		uint8_t ready() const
		{
			return ready_;
		}
		TYPE ticks() const
		{
			return ticks_;
		}

	private:
		uint8_t started_;
		uint8_t ready_;
		TYPE ticks_;
	};

	template<board::Timer TIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_>
	class MultiHCSR04
	{
		using PTRAIT = board_traits::Port_trait<ECHO_PORT_>;
		static_assert(PTRAIT::PCINT != 0xFF, "ECHO_PORT_ must support PCINT");
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK_) == ECHO_MASK_, "ECHO_MASK_ must contain available PORT pins");

	public:
		using TIMER = timer::Timer<TIMER_>;
		using TYPE = typename TIMER::TIMER_TYPE;
		using EVENT = SonarEvent<TIMER_>;
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		static constexpr const board::Port ECHO_PORT = ECHO_PORT_;
		static constexpr const uint8_t ECHO_MASK = ECHO_MASK_;
		
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		MultiHCSR04(TIMER& timer)
			:timer_{timer}, started_{}, ready_{}, active_{false}, trigger_{gpio::PinMode::OUTPUT}, echo_{ECHO_MASK_, 0}
		{
			interrupt::register_handler(*this);
		}

		uint8_t ready() const
		{
			return ready_;
		}

		bool all_ready() const
		{
			return ready_ == ECHO_MASK_;
		}

		void set_ready()
		{
			if (active_)
			{
				ready_ = ECHO_MASK_;
				active_ = false;
			}
		}

		void trigger()
		{
			started_ = 0;
			ready_ = 0;
			active_ = true;
			timer_.reset();
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

		EVENT on_pin_change()
		{
			TYPE ticks = timer_._ticks();
			// Compute the newly started echoes
			uint8_t pins = echo_.get_PIN();
			uint8_t started = pins & ~started_;
			// Compute the newly finished echoes
			uint8_t ready = ~pins & started_ & ~ready_;
			// Update status of all echo pins
			started_ |= started;
			ready_ |= ready;
			if (ready_ == ECHO_MASK_)
				active_ = false;
			return EVENT{started, ready, ticks};
		}

	private:
		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		TIMER& timer_;
		volatile uint8_t started_;
		volatile uint8_t ready_;
		volatile bool active_;
		typename gpio::FastPinType<TRIGGER_>::TYPE trigger_;
		gpio::FastMaskedPort<ECHO_PORT_> echo_;
	};
}
}

#endif /* HCSR04_H */
