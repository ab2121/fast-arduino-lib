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

/// @cond api

/**
 * @file 
 * Real-time Timer API.
 */
#ifndef RTT_HH
#define RTT_HH

#include "boards/io.h"
#include <avr/interrupt.h>
#include "interrupts.h"
#include "timer.h"
#include "time.h"
#include "events.h"

/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::RTT to work
 * properly.
 * This will not register any user callback though; if you need to register a
 * method or function to be called back every time one millsiecond has elapsed,
 * you need to use `REGISTER_RTT_ISR_METHOD` or `REGISTER_RTT_ISR_FUNCTION`
 * instead.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * 
 * @sa REGISTER_RTT_ISR_METHOD
 * @sa REGISTER_RTT_ISR_FUNCTION
 */
#define REGISTER_RTT_ISR(TIMER_NUM)               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))      \
	{                                             \
		timer::isr_handler_rtt::rtt<TIMER_NUM>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::RTT to work
 * properly, along with a callback method that will be notified every millisecond.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function) that
 * takes an `uint32_t` argument that will receive the total number of milliseconds
 * elapsed since the RTT has started.
 * 
 * @sa REGISTER_RTT_ISR_FUNCTION
 * @sa REGISTER_RTT_ISR
 */
#define REGISTER_RTT_ISR_METHOD(TIMER_NUM, HANDLER, CALLBACK)               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                                \
	{                                                                       \
		timer::isr_handler_rtt::rtt_method<TIMER_NUM, HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::RTT to work
 * properly, along with a callback function that will be notified every millisecond.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered; this function must accept an `uint32_t` argument that will receive 
 * the total number of milliseconds elapsed since the RTT has started.
 * 
 * @sa REGISTER_RTT_ISR_METHOD
 * @sa REGISTER_RTT_ISR
 */
#define REGISTER_RTT_ISR_FUNCTION(TIMER_NUM, CALLBACK)               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                         \
	{                                                                \
		timer::isr_handler_rtt::rtt_function<TIMER_NUM, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a timer::RTT to work
 * properly, along with a callback to timer::RTTEventCallback.
 * 
 * @param TIMER_NUM the number of the TIMER feature for the target MCU
 * @param EVENT the `events::Event<T>` type to be generated by RTTEventCallback
 * @param PERIOD the period, in ms, at which RTTEventCallback will generate events;
 * this must be a power of 2.
 * 
 * NOTE: it is important that @p EVENT and @p PERIOD match an `RTTEventCallback<EVENT, PERIOD>`
 * instance in your code.
 * 
 * @sa RTTEventCallback
 */
#define REGISTER_RTT_EVENT_ISR(TIMER_NUM, EVENT, PERIOD)               \
	ISR(CAT3(TIMER, TIMER_NUM, _COMPA_vect))                           \
	{                                                                  \
		timer::isr_handler_rtt::rtt_event<TIMER_NUM, EVENT, PERIOD>(); \
	}

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_RTT_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_RTT_ISR_HANDLERS_FRIEND      \
	friend struct timer::isr_handler_rtt; \
	DECL_TIMER_COMP_FRIENDS

namespace timer
{
	/**
	 * Utility class to avoid costly instantiation of `time::RTTTime` from an
	 * interrupt routine. It can be later (outside an ISR) converted into a
	 * real `time::RTTTime`.
	 * 
	 * It is usually created from raw Timer registers values.
	 * This type shall usually not be directly used as RTTRawTime<T> but actually
	 * obtained from an RTT<NTIMER> instance, as `RTT::RAW_TIME`.
	 * 
	 * @tparam T the type of registers for the Timer providing raw values,
	 * either `uint8_t` or `uint16_t`
	 * 
	 * @sa time::RTTTime
	 * @sa RTT::RAW_TIME
	 */
	template<typename T> class RTTRawTime
	{
	public:
		/**
		 * Create a new `RTTRawTime` from @p millis milliseconds, and values
		 * from Timer @p counter value that will allow microseconds calculation
		 * later on.
		 * 
		 * @param millis the milliseconds elapsed as accounted by an `RTT` instance
		 * @param counter the counter of the Timer used by the `RTT` instance
		 * @param max_counter the maximum counter value for the Timer
		 */
		RTTRawTime(uint32_t millis, T counter, T max_counter)
			: millis_{millis}, counter_{counter}, max_counter_{max_counter}
		{}

		/**
		 * A constant to signify "no time". This constant takes no code to build,
		 * compared with `RTTRawTime{0, 0, 0}`.
		 */
		static constexpr RTTRawTime<T> EMPTY_TIME{};

		/**
		 * Convert this `RTTRawTime` instance to a fully usable `time::RTTTime`.
		 * This method performs potentially costly computation and hence should
		 * not be used inside an ISR.
		 */
		time::RTTTime as_real_time() const
		{
			return time::RTTTime{millis_, uint16_t(ONE_MILLI * counter_ / (1UL + max_counter_))};
		}

	private:
		constexpr RTTRawTime() : buffer_{} {}

		static constexpr const uint32_t ONE_MILLI = 1000UL;

		// This union is an ugly hack to trick the compiler so that it understands
		// the default constructor just needs 0 initialization for EMPTY_TIME constant,
		// and thus can accept the fact EMPTY_TIME is a pure constexpr
		union
		{
			struct
			{
				uint32_t millis_;
				T counter_;
				T max_counter_;
			};
			uint8_t buffer_[sizeof(uint32_t) + 2 * sizeof(T)];
		};
	};

	/**
	 * API to handle a real-time timer.
	 * A real-time timer keeps track of time with micro-second precision.
	 * In order to perform properly, an appropriate ISR must be registered for it.
	 * 
	 * A real-time timer can be used to:
	 * - capture the duration of some event with good accuracy
	 * - implement timeouts in programs waiting for an event to occur
	 * - delay program execution for some us or ms
	 * - generate periodic events
	 * 
	 * @tparam NTIMER_ the AVR timer used by this RTT
	 * @sa board::Timer
	 * @sa REGISTER_RTT_ISR()
	 */
	template<board::Timer NTIMER_> class RTT : private Timer<NTIMER_>
	{
	public:
		/** The AVR timer used by this RTT. */
		static constexpr const board::Timer NTIMER = NTIMER_;

	private:
		using TRAIT = typename Timer<NTIMER>::TRAIT;
		using TYPE = typename Timer<NTIMER>::TYPE;
		using PRESCALER = typename Timer<NTIMER>::PRESCALER;

	public:
		/**
		 * The adequate `RTTRawTime` type for this `RTT`.
		 * @sa RTTRawTime
		 */
		using RAW_TIME = RTTRawTime<TYPE>;

		/**
		 * Construct a new real-time timer handler and initializes its current
		 * time to 0ms.
		 * Note that this constructor does **not** start the timer.
		 * @sa begin()
		 * @sa millis()
		 */
		RTT() : Timer<NTIMER>{TimerMode::CTC, MILLI_PRESCALER, TimerInterrupt::OUTPUT_COMPARE_A}, milliseconds_{} {}

		/**
		 * Register this RTT with the matching ISR that should have been
		 * registered with REGISTER_RTT_ISR().
		 * Calling this method is mandatory for this timer to work.
		 * @sa REGISTER_RTT_ISR()
		 */
		void register_rtt_handler()
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Elapsed time, in milliseconds, since this timer has started.
		 * If `millis(uint32_t)` is called, this sets a new time reference point
		 * to count elapsed time from.
		 * If you want to get more precision about the elpased time, you can get
		 * the number of microsecond elapsed, **in addition to `millis()`**, by
		 * calling `micros()`.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `millis_()` instead.
		 * @return elapsed time in ms
		 * @sa millis(uint32_t)
		 * @sa micros()
		 * @sa millis_()
		 */
		inline uint32_t millis() const
		{
			synchronized return milliseconds_;
		}

		/**
		 * Elapsed time, in milliseconds, since this timer has started.
		 * If `millis(uint32_t)` is called, this sets a new time reference point
		 * to count elapsed time from.
		 * If you want to get more precision about the elpased time, you can get
		 * the number of microsecond elapsed, **in addition to `millis()`**, by
		 * calling `micros()`.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `millis()` instead.
		 * @return elapsed time in ms
		 * @sa millis(uint32_t)
		 * @sa micros()
		 * @sa millis()
		 */
		inline uint32_t millis_() const
		{
			return milliseconds_;
		}

		/**
		 * Delay program execution for the given amount of milliseconds.
		 * Contrarily to `time::delay_ms()`, this method does not perform a busy
		 * loop, but it calls `time::yield()` which will put the MCU in sleep
		 * mode but will be awakened every ms (by a timer interrupt) and check if
		 * required delay has elapsed already.
		 * 
		 * @param ms the number of milliseconds to hold program execution
		 * @sa time::yield()
		 */
		void delay(uint32_t ms) const
		{
			uint32_t end = millis() + ms + 1;
			while (millis() < end) time::yield();
		}

		/**
		 * Compute the microseconds part (from `0` to `999`) of the time that has
		 * elapsed, since this timer has started. The milliseconds part is
		 * provided by `millis()`.
		 * In general, you will not call this method unless you are sure the
		 * elapsed time is strictly less than 1ms.
		 * If you need the elapsed time with microsecond precision, then you
		 * should call `time()` which returns an `time::RTTTime` structure that
		 * contains both milliseconds and microseconds.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `micros_()` instead.
		 * @return the us part of elapsed time
		 * @sa millis()
		 * @sa time()
		 * @sa micros_()
		 */
		inline uint16_t micros() const
		{
			synchronized return compute_micros();
		}

		/**
		 * Compute the microseconds part (from `0` to `999`) of the time that has
		 * elapsed, since this timer has started. The milliseconds part is
		 * provided by `millis()`.
		 * In general, you will not call this method unless you are sure the
		 * elapsed time is strictly less than 1ms.
		 * If you need the elapsed time with microsecond precision, then you
		 * should call `time()` which returns an `time::RTTTime` structure that
		 * contains both milliseconds and microseconds.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `micros()` instead.
		 * @return the us part of elapsed time
		 * @sa millis()
		 * @sa time()
		 * @sa micros()
		 */
		inline uint16_t micros_() const
		{
			return compute_micros();
		}

		/**
		 * Elapsed time, in milliseconds and microseconds, since this timer has
		 * started.
		 * If you do not need microsecond precision, you should instead use 
		 * `millis()`.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `time_()` instead.
		 * @return elapsed time in ms and us
		 * @sa millis()
		 * @sa micros()
		 * @sa time_()
		 */
		time::RTTTime time() const
		{
			synchronized return time_();
		}

		/**
		 * Elapsed time, in milliseconds and microseconds, since this timer has
		 * started.
		 * If you do not need microsecond precision, you should instead use 
		 * `millis()`.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `time()` instead.
		 * @return elapsed time in ms and us
		 * @sa millis()
		 * @sa micros()
		 * @sa time()
		 */
		time::RTTTime time_() const
		{
			return time::RTTTime(milliseconds_, compute_micros());
		}

		/**
		 * Elapsed time, in raw representation, since this timer has
		 * started.
		 * This method is a fast substitute for `time()`; instead of returning
		 * a `time::RTTTime` which takes longer to instantiate, it returns a
		 * simpler, faster `RTTRawTime` which contains the same information
		 * but unprocessed yet.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `raw_time_()` instead.
		 * @return elapsed time in ms and us
		 * @sa time()
		 * @sa raw_time_()
		 * @sa RTTRawTime
		 */
		RAW_TIME raw_time() const
		{
			synchronized return raw_time_();
		}

		/**
		 * Elapsed time, in raw representation, since this timer has
		 * started.
		 * This method is a fast substitute for `time_()`; instead of returning
		 * a `time::RTTTime` which takes longer to instantiate, it returns a
		 * simpler, faster `RTTRawTime` which contains the same information
		 * but unprocessed yet.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `raw_time()` instead.
		 * @return elapsed time in ms and us
		 * @sa time_()
		 * @sa raw_time()
		 * @sa RTTRawTime
		 */
		RAW_TIME raw_time_() const
		{
			return RAW_TIME{milliseconds_, (volatile TYPE&) TRAIT::TCNT, (volatile TYPE&) TRAIT::OCRA};
		}

		/**
		 * Reset the current milliseconds count of this RTT to the given value.
		 * Evey elapsed millisecond will then be added to this new value.
		 * @param ms the new millisecond start
		 */
		inline void millis(uint32_t ms)
		{
			synchronized
			{
				milliseconds_ = ms;
				// Reset timer counter
				TRAIT::TCNT = 0;
			}
		}

		/**
		 * Start this real-time timer, hence elapsed time starts getting counted
		 * from then.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `begin_()` instead.
		 * @sa end()
		 * @sa begin_()
		 */
		inline void begin()
		{
			synchronized begin_();
		}

		/**
		 * Start this real-time timer, hence elapsed time starts getting counted
		 * from then.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `begin()` instead.
		 * @sa end_()
		 * @sa begin()
		 */
		inline void begin_()
		{
			milliseconds_ = 0;
			Timer<NTIMER>::begin_(MILLI_COUNTER);
		}

		/**
		 * Stop this real-time timer, hence time gets not counted anymore.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `end_()` instead.
		 * @sa begin()
		 * @sa end_()
		 */
		inline void end()
		{
			Timer<NTIMER>::end();
		}

		/**
		 * Stop this real-time timer, hence time gets not counted anymore.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `end()` instead.
		 * @sa begin_()
		 * @sa end()
		 */
		inline void end_()
		{
			Timer<NTIMER>::end_();
		}

		/**
		 * Get a reference to the underlying `Timer` of this `RTT`.
		 */
		inline Timer<NTIMER>& timer()
		{
			return *this;
		}

	private:
		volatile uint32_t milliseconds_;

		void on_timer()
		{
			++milliseconds_;
		}

		using CALC = Calculator<NTIMER>;
		static constexpr const uint32_t ONE_MILLI = 1000UL;
		static constexpr const PRESCALER MILLI_PRESCALER = CALC::CTC_prescaler(ONE_MILLI);
		static constexpr const TYPE MILLI_COUNTER = CALC::CTC_counter(MILLI_PRESCALER, ONE_MILLI);

		inline uint16_t compute_micros() const
		{
			return uint16_t(ONE_MILLI * ((volatile TYPE&) TRAIT::TCNT) / (1UL + (volatile TYPE&) TRAIT::OCRA));
		}

		friend struct isr_handler_rtt;
	};

	/**
	 * Utility to generate events from an `RTT` instance at a given period.
	 * The generated events have type `events::Type::RTT_TIMER`.
	 * 
	 * @tparam EVENT the `events::Event<T>` type to be generated
	 * @tparam PERIOD_MS the period, in ms, at which events will be generated;
	 * this must be a power of 2.
	 * 
	 * @sa RTT
	 * @sa REGISTER_RTT_EVENT_ISR
	 */
	template<typename EVENT, uint32_t PERIOD_MS = 1024> class RTTEventCallback
	{
		static_assert(events::Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");
		static_assert((PERIOD_MS & (PERIOD_MS - 1)) == 0, "PERIOD_MS must be a power of 2");

	public:
		/**
		 * Create a `RTTEventCallback` that will push periodic events to @p event_queue.
		 * 
		 * @param event_queue the event queue which new events will be pushed to
		 * 
		 * NOTE: for this to work, you need to register the proper ISR with REGISTER_RTT_EVENT_ISR().
		 * 
		 * @sa REGISTER_RTT_EVENT_ISR
		 */
		RTTEventCallback(containers::Queue<EVENT>& event_queue) : event_queue_{event_queue} {}

	private:
		void on_rtt_change(uint32_t millis)
		{
			if ((millis & (PERIOD_MS - 1)) == 0) event_queue_.push_(EVENT{events::Type::RTT_TIMER});
		}

		containers::Queue<EVENT>& event_queue_;

		friend struct isr_handler_rtt;
	};

	/// @cond notdocumented

	// All RTT-related methods called by pre-defined ISR are defined here
	//====================================================================

	struct isr_handler_rtt
	{
		template<uint8_t TIMER_NUM_> static void rtt()
		{
			static constexpr board::Timer NTIMER = isr_handler::check_timer<TIMER_NUM_>();
			interrupt::HandlerHolder<RTT<NTIMER>>::handler()->on_timer();
		}

		template<uint8_t TIMER_NUM_, typename HANDLER_, void (HANDLER_::*CALLBACK_)(uint32_t)> static void rtt_method()
		{
			static constexpr board::Timer NTIMER = isr_handler::check_timer<TIMER_NUM_>();
			auto handler = interrupt::HandlerHolder<RTT<NTIMER>>::handler();
			handler->on_timer();
			interrupt::CallbackHandler<void (HANDLER_::*)(uint32_t), CALLBACK_>::call(handler->millis());
		}

		template<uint8_t TIMER_NUM_, void (*CALLBACK_)(uint32_t)> static void rtt_function()
		{
			static constexpr board::Timer NTIMER = isr_handler::check_timer<TIMER_NUM_>();
			auto handler = interrupt::HandlerHolder<RTT<NTIMER>>::handler();
			handler->on_timer();
			CALLBACK_(handler->millis());
		}

		template<uint8_t TIMER_NUM_, typename EVENT_, uint32_t PERIOD_> static void rtt_event()
		{
			static constexpr board::Timer NTIMER = isr_handler::check_timer<TIMER_NUM_>();
			auto handler = interrupt::HandlerHolder<RTT<NTIMER>>::handler();
			handler->on_timer();
			interrupt::HandlerHolder<RTTEventCallback<EVENT_, PERIOD_>>::handler()->on_rtt_change(handler->millis());
		}
	};
	/// @endcond
}

#endif /* RTT_HH */
/// @endcond
