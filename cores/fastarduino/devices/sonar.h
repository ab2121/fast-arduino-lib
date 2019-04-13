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

#ifndef SONAR_H
#define SONAR_H

#include <fastarduino/boards/board.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/utilities.h>

#include <fastarduino/int.h>
#include <fastarduino/pci.h>

//TODO document!
// Utilities to handle ISR callbacks
#define REGISTER_HCSR04_INT_ISR(TIMER, INT_NUM, TRIGGER, ECHO)						\
	ISR(CAT3(INT, INT_NUM, _vect))													\
	{																				\
		devices::sonar::isr_handler::sonar_int<INT_NUM, TIMER, TRIGGER, ECHO>();	\
	}

#define REGISTER_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)								\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                							\
	{                                                               							\
		devices::sonar::isr_handler::sonar_pci<PCI_NUM, TIMER, TRIGGER, ECHO, ##__VA_ARGS__>();	\
	}

#define REGISTER_DISTINCT_HCSR04_PCI_ISR(TIMER, PCI_NUM, TRIGGER, ECHO, ...)								\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                 										\
	{                                                                										\
		devices::sonar::isr_handler::sonar_distinct_pci<PCI_NUM, TIMER, TRIGGER, ECHO,  ##__VA_ARGS__>();	\
	}

#define REGISTER_HCSR04_INT_ISR_METHOD(TIMER, INT_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)            		\
	ISR(CAT3(INT, INT_NUM, _vect))                                                                  		\
	{                                                                                               		\
		devices::sonar::isr_handler::sonar_int_method<INT_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>();	\
	}

#define REGISTER_HCSR04_INT_ISR_FUNCTION(TIMER, INT_NUM, TRIGGER, ECHO, CALLBACK)                	\
	ISR(CAT3(INT, INT_NUM, _vect))                                                               	\
	{                                                                                            	\
		devices::sonar::isr_handler::sonar_int_function<INT_NUM, TIMER, TRIGGER, ECHO, CALLBACK>();	\
	}

#define REGISTER_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO, HANDLER, CALLBACK)            		\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                		\
	{                                                                                               		\
		devices::sonar::isr_handler::sonar_pci_method<PCI_NUM, TIMER, TRIGGER, ECHO, HANDLER, CALLBACK>();	\
	}

#define REGISTER_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO, CALLBACK)                  	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                               	\
	{                                                                                              	\
		devices::sonar::isr_handler::sonar_pci_function<PCI_NUM, TIMER, TRIGGER, ECHO, CALLBACK>();	\
	}

#define REGISTER_HCSR04_RTT_TIMEOUT(TIMER, SONAR, ...)										\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))												\
	{																						\
		devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>();	\
	}

#define REGISTER_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, HANDLER, CALLBACK, SONAR, ...)			\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))													\
	{																							\
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>())	\
			interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();					\
	}

#define REGISTER_HCSR04_RTT_TIMEOUT_FUNCTION(TIMER_NUM, CALLBACK, SONAR, ...)					\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))													\
	{																							\
		if (devices::sonar::isr_handler::sonar_rtt_change<TIMER_NUM, SONAR, ##__VA_ARGS__>())	\
			interrupt::CallbackHandler<void (*)(), CALLBACK>::call();							\
	}

#define REGISTER_MULTI_HCSR04_PCI_ISR_METHOD(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK)	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                                                    		\
	{                                                                                                   		\
		devices::sonar::isr_handler::multi_sonar_pci_method<PCI_NUM, TIMER, TRIGGER, 							\
			ECHO_PORT, ECHO_MASK, HANDLER, CALLBACK>();															\
	}

#define REGISTER_MULTI_HCSR04_PCI_ISR_FUNCTION(TIMER, PCI_NUM, TRIGGER, ECHO_PORT, ECHO_MASK, CALLBACK)	\
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                              						\
	{                                                                             						\
		devices::sonar::isr_handler::multi_sonar_pci_function<PCI_NUM, TIMER, TRIGGER, 					\
			ECHO_PORT, ECHO_MASK, CALLBACK>();															\
	}

#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT(TIMER, SONAR)								\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))										\
	{																				\
		devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR>();	\
	}

#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT_METHOD(TIMER_NUM, SONAR, HANDLER, CALLBACK)			\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))													\
	{																							\
		using EVENT = devices::sonar::SonarEvent;												\
		EVENT event = 																			\
			devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR>();			\
		if (event.timeout())																	\
			interrupt::CallbackHandler<void (HANDLER::*)(const EVENT&), CALLBACK>::call(event);	\
	}

#define REGISTER_MULTI_HCSR04_RTT_TIMEOUT_FUNCTION(TIMER_NUM, SONAR,  CALLBACK)			\
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))											\
	{																					\
		using EVENT = devices::sonar::SonarEvent;										\
		EVENT event = 																	\
			devices::sonar::isr_handler::multi_sonar_rtt_change<TIMER_NUM, SONAR>();	\
		if (event.timeout())															\
			interrupt::CallbackHandler<void (*)(const EVENT&), CALLBACK>::call(event);	\
	}

#define DECL_SONAR_ISR_HANDLERS_FRIEND			\
	friend struct devices::sonar::isr_handler;	\
	DECL_INT_ISR_FRIENDS						\
	DECL_PCINT_ISR_FRIENDS						\
	DECL_TIMER_COMP_FRIENDS

namespace devices::sonar
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

	enum class SonarType : uint8_t
	{
		BLOCKING,
		ASYNC_INT,
		ASYNC_PCINT
	};

	template<board::Timer NTIMER_> class AbstractSonar
	{
	public:
		using RTT = timer::RTT<NTIMER_>;

		inline bool ready() const
		{
			return status_ == READY;
		}

		inline uint16_t latest_echo_us() const
		{
			synchronized
			{
				if (status_ == READY)
					return echo_time_();
				else
					return 0;
			}
		}

	private:
		using RAW_TIME = typename RTT::RAW_TIME;

	protected:
		AbstractSonar(const RTT& rtt)
			:	rtt_{rtt}, status_{UNKNOWN}, timeout_time_ms_{}, 
				echo_start_{RAW_TIME::EMPTY_TIME}, echo_end_{RAW_TIME::EMPTY_TIME}
		{
		}

		uint16_t async_echo_us(uint16_t timeout_ms)
		{
			uint32_t now = rtt_.millis();
			now += timeout_ms;
			// Wait for echo signal start
			while (status_ != READY)
				if (rtt_.millis() >= now)
				{
					synchronized
					{
						status_ = READY;
						echo_start_ = echo_end_ = RAW_TIME::EMPTY_TIME;
					}
					return 0;
				}
			return echo_time_();
		}

		template<board::DigitalPin ECHO>
		uint16_t blocking_echo_us(typename gpio::FastPinType<ECHO>::TYPE& echo, uint16_t timeout_ms)
		{
			uint32_t now = rtt_.millis();
			now += timeout_ms;
			while (!echo.value())
				if (rtt_.millis() >= now) return 0;
			synchronized
			{
				status_ = ECHO_STARTED;
				echo_start_ = rtt_.raw_time();
			}
			// Wait for echo signal end
			while (echo.value())
				if (rtt_.millis() >= now) return 0;
			synchronized
			{
				status_ = READY;
				echo_end_ = rtt_.raw_time();
				return echo_time_();
			}
		}

		inline void trigger_sent(uint16_t timeout_ms)
		{
			synchronized
			{
				status_ = TRIGGERED;
				timeout_time_ms_ = rtt_.millis_() + timeout_ms;
			}
		}

		inline bool pulse_edge(bool rising)
		{
			if (rising && status_ == TRIGGERED)
			{
				status_ = ECHO_STARTED;
				echo_start_ = rtt_.raw_time_();
			}
			else if ((!rising) && status_ == ECHO_STARTED)
			{
				status_ = READY;
				echo_end_ = rtt_.raw_time_();
				return true;
			}
			return false;
		}

		inline bool rtt_time_changed()
		{
			if (status_ != READY && rtt_.millis_() >= timeout_time_ms_)
			{
				status_ = READY;
				echo_start_ = echo_end_ = RAW_TIME::EMPTY_TIME;
				return true;
			}
			return false;
		}

	private:
		uint16_t echo_time_() const
		{
			return uint16_t((echo_end_.as_real_time() - echo_start_.as_real_time()).total_micros());
		}

		const RTT& rtt_;

		static constexpr const uint8_t UNKNOWN = 0x00;
		static constexpr const uint8_t TRIGGERED = 0x10;
		static constexpr const uint8_t ECHO_STARTED = 0x11;
		static constexpr const uint8_t READY = 0x20;

		volatile uint8_t status_;
		uint32_t timeout_time_ms_;

		RAW_TIME echo_start_;
		RAW_TIME echo_end_;
	};

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_,
			 SonarType SONAR_TYPE_ = SonarType::BLOCKING>
	class HCSR04 : public AbstractSonar<NTIMER_>
	{
	public:
		using RTT = timer::RTT<NTIMER_>;
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		static constexpr const board::DigitalPin ECHO = ECHO_;
		static constexpr const SonarType SONAR_TYPE = SONAR_TYPE_;

	private:
		using PARENT = AbstractSonar<NTIMER_>;
		using ECHO_PIN_TRAIT = board_traits::DigitalPin_trait<ECHO>;
		using ECHO_PORT_TRAIT = board_traits::Port_trait<ECHO_PIN_TRAIT::PORT>;
		static_assert(SONAR_TYPE != SonarType::ASYNC_INT || ECHO_PIN_TRAIT::IS_INT,
					  "SONAR_TYPE == ASYNC_INT but ECHO is not an INT pin");
		static_assert(SONAR_TYPE != SonarType::ASYNC_PCINT || ECHO_PORT_TRAIT::PCINT != 0xFF,
					  "SONAR_TYPE == ASYNC_PCINT but ECHO is not an PCI pin");

	public:
		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		HCSR04(const RTT& rtt) : PARENT{rtt}, trigger_{gpio::PinMode::OUTPUT}, echo_{gpio::PinMode::INPUT}
		{
		}

		inline void register_handler()
		{
			static_assert(SONAR_TYPE != SonarType::BLOCKING,
						  "register_handler() must not be called with SonarType::BLOCKING");
			interrupt::register_handler(*this);
		}

		// Blocking API
		// Do note that timeout here is for the whole method not just for the sound echo, hence it
		// must be bigger than just the time to echo the maximum roundtrip distance (typically x2)
		uint16_t echo_us(uint16_t timeout_ms)
		{
			async_echo(timeout_ms);
			return await_echo_us(timeout_ms);
		}

		uint16_t await_echo_us(uint16_t timeout_ms)
		{
			if (SONAR_TYPE != SonarType::BLOCKING)
				return this->async_echo_us(timeout_ms);
			else
				return this->template blocking_echo_us<ECHO>(echo_, timeout_ms);
		}

		// We want to avoid using await_echo_us() to handle state & timeout!
		void async_echo(uint16_t timeout_ms, bool trigger = true)
		{
			this->trigger_sent(timeout_ms);
			if (trigger) this->trigger();
		}

	private:
		bool on_pin_change()
		{
			static_assert(SONAR_TYPE == SonarType::ASYNC_INT || SONAR_TYPE == SonarType::ASYNC_PCINT,
						  "on_pin_change() must be called only with SonarType::ASYNC_INT or ASYNC_PCINT");
			return this->pulse_edge(echo_.value());
		}

		bool on_rtt_change()
		{
			return this->rtt_time_changed();
		}

		inline void trigger()
		{
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		typename gpio::FastPinType<ECHO>::TYPE echo_;

		// Make friends with all ISR handlers
		friend struct isr_handler;
	};

	//FIXME replace RTTTime with RAW_TIME (need to change SonarEvent to a template then)
	struct SonarEvent
	{
	public:
		SonarEvent(bool timeout = false) : timeout_{timeout}, started_{}, ready_{}, time_{}
		{
		}
		SonarEvent(uint8_t started, uint8_t ready, time::RTTTime time) : started_{started}, ready_{ready}, time_{time}
		{
		}

		bool timeout() const
		{
			return timeout_;
		}
		uint8_t started() const
		{
			return started_;
		}
		uint8_t ready() const
		{
			return ready_;
		}
		time::RTTTime time() const
		{
			return time_;
		}

	private:
		bool timeout_;
		uint8_t started_;
		uint8_t ready_;
		time::RTTTime time_;
	};

	template<board::Timer NTIMER_, board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_>
	class MultiHCSR04
	{
	public:
		static constexpr const board::DigitalPin TRIGGER = TRIGGER_;
		static constexpr const board::Port ECHO_PORT = ECHO_PORT_;
		static constexpr const uint8_t ECHO_MASK = ECHO_MASK_;

	private:
		using PTRAIT = board_traits::Port_trait<ECHO_PORT>;
		static_assert(PTRAIT::PCINT != 0xFF, "ECHO_PORT_ must support PCINT");
		static_assert((PTRAIT::DPIN_MASK & ECHO_MASK) == ECHO_MASK, "ECHO_MASK_ must contain available PORT pins");

	public:
		using RTT = timer::RTT<NTIMER_>;

		static constexpr const uint16_t MAX_RANGE_M = 4;
		static constexpr const uint16_t DEFAULT_TIMEOUT_MS = MAX_RANGE_M * 2 * 1000UL / SPEED_OF_SOUND + 1;

		MultiHCSR04(RTT& rtt)
			:	rtt_{rtt}, started_{}, ready_{}, active_{false}, timeout_time_ms_{}, 
				trigger_{gpio::PinMode::OUTPUT}, echo_{0}
		{
			interrupt::register_handler(*this);
		}

		uint8_t ready() const
		{
			return ready_;
		}

		bool all_ready() const
		{
			return ready_ == ECHO_MASK;
		}

		void set_ready()
		{
			if (active_)
			{
				ready_ = ECHO_MASK;
				active_ = false;
			}
		}

		void trigger(uint16_t timeout_ms)
		{
			started_ = 0;
			ready_ = 0;
			timeout_time_ms_ = rtt_.millis() + timeout_ms;
			active_ = true;
			// Pulse TRIGGER for 10us
			trigger_.set();
			time::delay_us(TRIGGER_PULSE_US);
			trigger_.clear();
		}

	private:
		SonarEvent on_pin_change()
		{
			if (!active_) return SonarEvent{};
			// Compute the newly started echoes
			uint8_t pins = echo_.get_PIN();
			uint8_t started = pins & ~started_;
			// Compute the newly finished echoes
			uint8_t ready = ~pins & started_ & ~ready_;
			// Update status of all echo pins
			started_ |= started;
			ready_ |= ready;
			if (ready_ == ECHO_MASK) active_ = false;
			return SonarEvent{started, ready, rtt_.time_()};
		}

		SonarEvent on_rtt_change()
		{
			if (active_ && rtt_.millis_() >= timeout_time_ms_)
			{
				active_ = false;
				return SonarEvent{true};
			}
			return SonarEvent{};
		}

		static constexpr const uint16_t TRIGGER_PULSE_US = 10;

		RTT& rtt_;
		volatile uint8_t started_;
		volatile uint8_t ready_;
		volatile bool active_;
		uint32_t timeout_time_ms_;
		typename gpio::FastPinType<TRIGGER>::TYPE trigger_;
		gpio::FastMaskedPort<ECHO_PORT, ECHO_MASK> echo_;

		// Make friends with all ISR handlers
		friend struct isr_handler;
	};

	/// @cond notdocumented
	// All sonar-related methods called by pre-defined ISR are defined here
	struct isr_handler
	{
		template<uint8_t INT_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_, board::DigitalPin ECHO_>
		static bool sonar_int()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::DigitalPin_trait<ECHO_>::IS_INT, "ECHO must be an INT pin.");
			static_assert(board_traits::ExternalInterruptPin_trait<ECHO_>::INT == INT_NUM_,
						"ECHO INT number must match INT_NUM");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO_, SonarType::ASYNC_INT>;
			return interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_,
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_int_method()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t INT_NUM_, board::Timer TIMER_,
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, void (*CALLBACK_)()>
		static void sonar_int_function()
		{
			if (sonar_int<INT_NUM_, TIMER_, TRIGGER_, ECHO_>())
				CALLBACK_();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, board::DigitalPin TRIGGER_>
		static bool sonar_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO1_, board::DigitalPin... ECHOS_>
		static bool sonar_pci()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			static_assert(board_traits::DigitalPin_trait<ECHO1_>::PORT == board_traits::PCI_trait<PCI_NUM_>::PORT,
						"ECHO port must match PCI_NUM port");
			static_assert(_BV(board_traits::DigitalPin_trait<ECHO1_>::BIT) & board_traits::PCI_trait<PCI_NUM_>::PCI_MASK,
						"ECHO must be a PCINT pin");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO1_, SonarType::ASYNC_PCINT>;
			bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			// handle other echo pins
			return result || sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHOS_...>();
		}

		template<bool DUMMY_>
		static bool sonar_rtt_change_helper()
		{
			return false;
		}

		template<bool DUMMY_, typename SONAR1_, typename... SONARS_>
		static bool sonar_rtt_change_helper()
		{
			bool result = interrupt::HandlerHolder<SONAR1_>::handler()->on_rtt_change();
			// handle other sonars
			return result || sonar_rtt_change_helper<DUMMY_, SONARS_...>();
		}

		template<uint8_t TIMER_NUM_, typename... SONARS_>
		static bool sonar_rtt_change()
		{
			// Update RTT time
			timer::isr_handler_rtt::rtt<TIMER_NUM_>();
			// Ask each sonar to check if timeout is elapsed
			return sonar_rtt_change_helper<false, SONARS_...>();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void sonar_pci_method()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>())
				interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, void (*CALLBACK_)()>
		static void sonar_pci_function()
		{
			if (sonar_pci<PCI_NUM_, TIMER_, TRIGGER_, ECHO_>())
				CALLBACK_();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_>
		static bool sonar_distinct_pci()
		{
			return false;
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::DigitalPin ECHO_, board::DigitalPin... TRIGGER_ECHOS_>
		static bool sonar_distinct_pci()
		{
			timer::isr_handler::check_timer<TIMER_>();
			// handle first echo pin
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT != board::Port::NONE, "PORT must support PCI");
			static_assert(board_traits::DigitalPin_trait<ECHO_>::PORT == board_traits::PCI_trait<PCI_NUM_>::PORT,
						"ECHO port must match PCI_NUM port");
			static_assert(_BV(board_traits::DigitalPin_trait<ECHO_>::BIT) & board_traits::PCI_trait<PCI_NUM_>::PCI_MASK,
						"ECHO must be a PCINT pin");
			using SONAR = HCSR04<TIMER_, TRIGGER_, ECHO_, SonarType::ASYNC_PCINT>;
			bool result = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			// handle other echo pins
			return result || sonar_pci<PCI_NUM_, TIMER_, TRIGGER_ECHOS_...>();
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_,
			typename HANDLER_, void (HANDLER_::*CALLBACK_)(const SonarEvent&)>
		static void multi_sonar_pci_method()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT == ECHO_PORT_, "ECHO_PORT must match PCI_NUM");
			using PTRAIT = board_traits::Port_trait<ECHO_PORT_>;
			static_assert((PTRAIT::DPIN_MASK & ECHO_MASK_) == ECHO_MASK_, "ECHO_MASK must contain available PORT pins");
			using SONAR = MultiHCSR04<TIMER_, TRIGGER_, ECHO_PORT_, ECHO_MASK_>;
			SonarEvent event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			if (event.ready() || event.started())
				interrupt::CallbackHandler<void (HANDLER_::*)(const SonarEvent&), CALLBACK_>::call(event);
		}

		template<uint8_t PCI_NUM_, board::Timer TIMER_, 
			board::DigitalPin TRIGGER_, board::Port ECHO_PORT_, uint8_t ECHO_MASK_, 
			void (*CALLBACK_)(const SonarEvent&)>
		static void multi_sonar_pci_function()
		{
			timer::isr_handler::check_timer<TIMER_>();
			static_assert(board_traits::PCI_trait<PCI_NUM_>::PORT == ECHO_PORT_, "ECHO_PORT must match PCI_NUM");
			using PTRAIT = board_traits::Port_trait<ECHO_PORT_>;
			static_assert((PTRAIT::DPIN_MASK & ECHO_MASK_) == ECHO_MASK_, "ECHO_MASK must contain available PORT pins");
			using SONAR = MultiHCSR04<TIMER_, TRIGGER_, ECHO_PORT_, ECHO_MASK_>;
			SonarEvent event = interrupt::HandlerHolder<SONAR>::handler()->on_pin_change();
			if (event.ready() || event.started())
				CALLBACK_(event);
		}

		template<uint8_t TIMER_NUM_, typename SONAR_>
		static SonarEvent multi_sonar_rtt_change()
		{
			// Update RTT time
			timer::isr_handler_rtt::rtt<TIMER_NUM_>();
			return interrupt::HandlerHolder<SONAR_>::handler()->on_rtt_change();
		}
	};
	/// @endcond
}

#endif /* SONAR_H */
