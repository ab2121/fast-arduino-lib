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
 * Support for jobs scheduling.
 */
#ifndef SCHEDULER_HH
#define SCHEDULER_HH

#include "events.h"
#include "linked_list.h"

namespace events
{
	class Job;

	/**
	 * Schedule jobs at predefined periods of time.
	 * The timebase is provided by @p CLOCK @p clock instance.
	 * A scheduler is an `EventHandler` that must thus be attached to a `Dispatcher`
	 * as in this snippet:
	 * @code
	 * using namespace events;
	 * using EVENT = Event<void>;
	 * 
	 * REGISTER_WATCHDOG_CLOCK_ISR(EVENT)
	 * 
	 * class MyJob: public Job
	 * {
	 *     ...
	 * }
	 * 
	 * int main()
	 * {
	 *     // Create event queue
	 *     const uint8_t EVENT_QUEUE_SIZE = 32;
	 *     EVENT buffer[EVENT_QUEUE_SIZE];
	 *     containers::Queue<EVENT> event_queue{buffer};
	 * 
	 *     // Prepare event dispatcher, clock and scheduler
	 *     Dispatcher<EVENT> dispatcher;
	 *     watchdog::Watchdog<EVENT> watchdog{event_queue};
	 *     Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	 *     dispatcher.insert(scheduler);
	 * 
	 *     // Create and register a job
	 *     MyJob job;
	 *     scheduler.schedule(job);
	 * 
	 *     // Start clock (watchdog)
	 *     watchdog.begin(watchdog::TimeOut::TO_64ms);
	 * 
	 *     // Main event loop
	 *     while (true)
	 *     {
	 *         EVENT event = pull(event_queue);
	 *         dispatcher.dispatch(event);
	 *     }
	 * }
	 * @endcode
	 * In that snippet, we use `Watchdog` as the clock source, but other sources are 
	 * available.
	 * 
	 * @tparam CLOCK_ the type of @p clock that will be used as time base
	 * @tparam EVENT_ the `events::Event<T>` dispatched by the system
	 * 
	 * @sa Job
	 * @sa Event
	 * @sa Dispatcher
	 * @sa watchdog::Watchdog
	 * @sa time::RTT
	 */
	template<typename CLOCK_, typename EVENT_>
	class Scheduler : public EventHandler<EVENT_>, public containers::LinkedList<Job>
	{
	public:
		Scheduler(const Scheduler<CLOCK_, EVENT_>&) = delete;
		Scheduler<CLOCK_, EVENT_>& operator=(const Scheduler<CLOCK_, EVENT_>&) = delete;

		/** The type of @p clock source used by this Scheduler. */
		using CLOCK = CLOCK_;
		/** The `events::Event<T>` dispatched by the system and expected by this Scheduler. */
		using EVENT = EVENT_;

		/**
		 * Create a new Scheduler based on the given @p clock.
		 * @param clock the clock providing the timebase for this scheduler
		 * @param type the type of event generated by @p clock
		 */
		Scheduler(const CLOCK& clock, uint8_t type) INLINE : EventHandler<EVENT>{type}, clock_{clock} {}

		/// @cond notdocumented
		void on_event(UNUSED const EVENT& event) override INLINE
		{
			traverse(JobCaller{clock_});
		}
		/// @endcond

		/**
		 * Add @p job to this scheduler.
		 * @param job the job to be added to this scheduler
		 * @sa Job
		 */
		void schedule(Job& job) INLINE
		{
			insert(job);
		}

		/**
		 * Remove @p job from this scheduler.
		 * Note that when a job is not periodic (i.e. it is a one-shot job) then 
		 * it is automatically unscheduled after first execution. 
		 * @param job the job to be removed from this scheduler; does nothing if
		 * job was not previously added to this scheduler.
		 * @sa Job
		 */
		void unschedule(Job& job) INLINE
		{
			remove(job);
		}

	private:
		class JobCaller
		{
		public:
			explicit JobCaller(const CLOCK& clock) : clock_{clock} {}
			bool operator()(Job& job);

		private:
			const CLOCK& clock_;
		};

		const CLOCK& clock_;
	};

	/**
	 * Abstract class holding some action to be executed at given periods of time.
	 * You should subclass `Job` and implement virtual method `on_schedule()`.
	 * A `Job` may be "one-shot" (executed exactly once) or "periodic" (executed 
	 * at regular time intervals).
	 * Every job must be added to a `Scheduler` in order to be executed at 
	 * specified time.
	 * 
	 * @sa Scheduler::schedule()
	 */
	class Job : public containers::Link<Job>
	{
	public:
		/**
		 * Tell if this job is periodic or not.
		 */
		bool is_periodic() const INLINE
		{
			return period_ != 0;
		}

		/**
		 * Tell next time (in ms) when this job shall be executed.
		 * Time reference for this value is provided by the `Scheduler` clock.
		 */
		uint32_t next_time() const INLINE
		{
			return next_time_;
		}

		/**
		 * Return the period of this job, or `0` if this is a one-shot job.
		 */
		uint32_t period() const INLINE
		{
			return period_;
		}

		/**
		 * Reschedule this job for a later time (in ms).
		 * Time reference for @p when is provided by the `Scheduler` clock.
		 * This gets automatically called by `Scheduler` after every execution of 
		 * a periodic job.
		 * @param when next time (in ms) at which this job shall be executed;
		 * note that actual execution time accuracy depends a lot on how busy 
		 * your main event loop is.
		 */
		void reschedule(uint32_t when) INLINE
		{
			next_time_ = when;
		}

	protected:
		Job(const Job&) =  default;
		Job& operator=(const Job&) = default;
		
		/**
		 * This method is called by `Scheduler` whenever  current clock time is
		 * greater or equal to `next_time()`.
		 * You must override this method to make it perform what you need.
		 * @param millis the current time at which this method is called, as
		 * provided by `Scheduler` clock
		 */
		virtual void on_schedule(uint32_t millis) = 0;

		/**
		 * Construct a new `Job`.
		 * @param next next time (in ms) at which this job shall be executed the 
		 * first time
		 * @param period the period (in ms) at which this job shall be periodically
		 * executed, or `0` if this must be a one-shot job.
		 */
		Job(uint32_t next = 0, uint32_t period = 0) INLINE : next_time_{next}, period_{period} {}

	private:
		uint32_t next_time_;
		uint32_t period_;

		template<typename CLOCK, typename T> friend class Scheduler;
	};

	/// @cond notdocumented
	template<typename CLOCK, typename T> bool Scheduler<CLOCK, T>::JobCaller::operator()(Job& job)
	{
		uint32_t now = clock_.millis();
		if (job.next_time() <= now)
		{
			job.on_schedule(now);
			if (!job.is_periodic()) return true;
			job.reschedule(now + job.period());
		}
		return false;
	}
	/// @endcond
}

#endif /* SCHEDULER_HH */
/// @endcond
