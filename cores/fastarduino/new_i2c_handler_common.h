//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * Common I2C Manager API. This is automatically included by other header files,
 * you should never include it directly in your programs.
 */
#ifndef I2C_HANDLER_COMMON_HH
#define I2C_HANDLER_COMMON_HH

#include <stdint.h>
#include "boards/board_traits.h"
#include "bits.h"
#include "i2c.h"
#include "streams.h"
#include "future.h"
#include "lifecycle.h"
#include "queue.h"
#include "utilities.h"

namespace i2c
{
	/**
	 * I2CManager policy to use in case of an error during I2C transaction.
	 * @warning available only on ATmega MCU.
	 * @sa I2CManager
	 */
	enum class I2CErrorPolicy : uint8_t
	{
		/**
		 * Do nothing at all in case of an error; useful only with a synchronous
		 * I2CManager.
		 */
		DO_NOTHING,

		/**
		 * In case of an error during I2C transaction, then all I2CCommand currently
		 * in queue will be removed.
		 * @warning this means that an error with device A can trigger a removal
		 * of pending commands for device B.
		 */
		CLEAR_ALL_COMMANDS,

		/**
		 * In case of an error during I2C transaction, then all pending I2CCommand
		 * of the current transaction will be removed.
		 */
		CLEAR_TRANSACTION_COMMANDS
	};

	/**
	 * List of debug states that are reported by the I2CManager in debug mode.
	 * @sa I2CManager
	 */
	enum class DebugStatus : uint8_t
	{
		/** A start condition has just been sent. */
		START = 0,
		/** A repeat start condition has just been sent. */
		REPEAT_START,
		/** A slave address has just been sent for writing. */
		SLAW,
		/** A slave address has just been sent for reading. */
		SLAR,
		/** A byte has just be sent to the slave. */
		SEND,
		/** A byte is being received from the slave. */
		RECV,
		/** The last byte is being received from the slave. */
		RECV_LAST,
		/** A stop condition has just been sent. */
		STOP,

		/** The latest sent byte has been acknowledged by the slave. */
		SEND_OK,
		/** The latest sent byte has not been acknowledged by the slave. */
		SEND_ERROR,
		/** I2CManager has acknowledged the latest received byte from the slave. */
		RECV_OK,
		/** I2CManager has not acknowledged the latest received byte from the slave. */
		RECV_ERROR
	};

	/**
	 * The default debugging hook type.
	 * @warning Do not use this (function pointer) for your hooks! This will 
	 * increase code size and ISR delay. Rather use functors as defined in
	 * `i2c_debug.h`.
	 * @sa i2c::debug::I2CDebugRecorder
	 * @sa i2c::debug::I2CLiveDebugger
	 */
	using I2C_DEBUG_HOOK = void (*)(DebugStatus status, uint8_t data);

	/// @cond notdocumented
	// Type of commands in queue
	class I2CCommandType
	{
	public:
		constexpr I2CCommandType() = default;
		constexpr I2CCommandType(const I2CCommandType&) = default;
		constexpr I2CCommandType(uint8_t value) : value_{value} {}
		constexpr I2CCommandType(bool write, bool stop, bool finish, bool end)
			:	value_{value(write, stop, finish, end)} {}
		I2CCommandType& operator=(const I2CCommandType&) = default;

		bool is_none() const
		{
			return value_ == NONE;
		}
		bool is_write() const
		{
			return value_ & WRITE;
		}
		bool is_stop() const
		{
			return value_ & STOP;
		}
		bool is_finish() const
		{
			return value_ & FINISH;
		}
		bool is_end() const
		{
			return value_ & END;
		}
		void add_flags(uint8_t value)
		{
			value_ |= value;
		}

		static constexpr uint8_t flags(bool stop, bool finish, bool end)
		{
			return (stop ? STOP : 0) | (finish ? FINISH : 0) | (end ? END : 0);
		}

	private:
		static constexpr const uint8_t NONE = 0;
		static constexpr const uint8_t NOT_NONE = bits::BV8(0);
		static constexpr const uint8_t WRITE = bits::BV8(1);
		static constexpr const uint8_t STOP = bits::BV8(2);
		static constexpr const uint8_t FINISH = bits::BV8(3);
		static constexpr const uint8_t END = bits::BV8(4);

		static constexpr uint8_t value(bool write, bool stop, bool finish, bool end)
		{
			return NOT_NONE | (write ? WRITE : 0) | (stop ? STOP : 0) | (finish ? FINISH : 0) | (end ? END : 0);
		}

		uint8_t value_ = NONE;

		friend bool operator==(const I2CCommandType&, const I2CCommandType&);
		friend bool operator!=(const I2CCommandType&, const I2CCommandType&);
	};

	streams::ostream& operator<<(streams::ostream& out, const I2CCommandType& t);
	bool operator==(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ == b.value_);
	}
	bool operator!=(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.value_ != b.value_);
	}
	/// @endcond

	/**
	 * Light atomic I2C command as prepared by an I2C device.
	 * Each command embeds:
	 * - the command type (read, write...), 
	 * - the count of bytes to be read or  written,
	 * 
	 * @warning You should never need to use this API by yourself. This is 
	 * internally used by FastArduino I2CManager to handle I2C transactions.
	 * 
	 * @sa I2CCommand
	 */
	class I2CLightCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CLightCommand() = default;
		constexpr I2CLightCommand(const I2CLightCommand&) = default;
		constexpr I2CLightCommand(I2CCommandType type, uint8_t byte_count) : type_{type}, byte_count_{byte_count} {}

		I2CCommandType type() const
		{
			return type_;
		}
		I2CCommandType& type()
		{
			return type_;
		}
		uint8_t byte_count() const
		{
			return byte_count_;
		}
		void decrement_byte_count()
		{
			--byte_count_;
		}
		void update_byte_count(uint8_t read_count, uint8_t write_count)
		{
			if (!byte_count_)
				byte_count_ = (type_.is_write() ? write_count : read_count);
		}
		/// @endcond

	private:
		// Type of this command
		I2CCommandType type_ = I2CCommandType{};
		// The number of remaining bytes to be read or write
		uint8_t byte_count_ = 0;
	};

	/**
	 * Atomic I2C command as used internally by an asynchronous I2C Manager.
	 * You shall use it when you define a buffer of commands for an asynchronous
	 * I2C Manager constructor.
	 * 
	 * Each command embeds:
	 * - the command type (read, write...), 
	 * - the count of bytes to be read or  written,
	 * - the address of target slave device
	 * - a proxy to the future holding inputs and results of the I2C transaction 
	 * 
	 * @warning You should never need to use this API by yourself. This is 
	 * internally used by FastArduino I2CManager to handle I2C transactions.
	 * 
	 * @sa lifecycle::LightProxy
	 * @sa future::AbstractFuture
	 */
	template<typename T> class I2CCommand : public I2CLightCommand
	{
	public:
		/// @cond notdocumented
		constexpr I2CCommand() = default;
		constexpr I2CCommand(const I2CCommand&) = default;
		constexpr I2CCommand(
			const I2CLightCommand& that, uint8_t target, T future)
			:	I2CLightCommand{that}, target_{target}, future_{future} {}
		constexpr I2CCommand& operator=(const I2CCommand&) = default;

		uint8_t target() const
		{
			return target_;
		}
		T future() const
		{
			return future_;
		}

		void set_target(uint8_t target, T future)
		{
			target_ = target;
			future_ = future;
		}
		/// @endcond

	private:
		// Address of the target device (on 8 bits, already left-shifted)
		uint8_t target_ = 0;
		// A proxy to the future to be used for this command
		T future_{};
	};

	/// @cond notdocumented
	template<typename T>
	streams::ostream& operator<<(streams::ostream& out, const I2CCommand<T>& c)
	{
		out	<< '{' << c.type() << ',' 
			<< streams::hex << c.target() << '}' << streams::flush;
		return out;
	}
	/// @endcond

	/// @cond notdocumented
	// Generic support for I2C debugging
	template<bool IS_DEBUG_ = false, typename DEBUG_HOOK_ = I2C_DEBUG_HOOK>  struct I2CDebugSupport
	{
		I2CDebugSupport(UNUSED DEBUG_HOOK_ hook = nullptr) {}
		void call_hook(UNUSED DebugStatus status, UNUSED uint8_t data = 0) {}
	};
	template<typename DEBUG_HOOK_> struct I2CDebugSupport<true, DEBUG_HOOK_>
	{
		I2CDebugSupport(DEBUG_HOOK_ hook) : hook_{hook} {}
		void call_hook(DebugStatus status, uint8_t data = 0)
		{
			hook_(status, data);
		}
	private:
		DEBUG_HOOK_ hook_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for LifeCycle resolution
	template<bool HAS_LIFECYCLE_ = false> struct I2CLifeCycleSupport
	{
		template<typename T> using PROXY = lifecycle::DirectProxy<T>;
		template<typename T> static PROXY<T> make_proxy(const T& dest)
		{
			return lifecycle::make_direct_proxy(dest);
		}
		I2CLifeCycleSupport(UNUSED lifecycle::AbstractLifeCycleManager* lifecycle_manager = nullptr) {}
		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return *proxy();
		}
	};
	template<> struct I2CLifeCycleSupport<true>
	{
		template<typename T> using PROXY = lifecycle::LightProxy<T>;
		template<typename T> static PROXY<T> make_proxy(const T& dest)
		{
			return lifecycle::make_light_proxy(dest);
		}
		I2CLifeCycleSupport(lifecycle::AbstractLifeCycleManager* lifecycle_manager)
			:	lifecycle_manager_{*lifecycle_manager} {}
		template<typename T> T& resolve(PROXY<T> proxy) const
		{
			return *proxy(lifecycle_manager_);
		}
	private:
		lifecycle::AbstractLifeCycleManager& lifecycle_manager_;
	};
	/// @endcond

	/// @cond notdocumented
	// Generic support for LifeCycle resolution
	template<I2CErrorPolicy POLICY = I2CErrorPolicy::DO_NOTHING> struct I2CErrorPolicySupport
	{
		I2CErrorPolicySupport() {}
		template<typename T>
		void handle_error(UNUSED const I2CCommand<T>& current, UNUSED containers::Queue<I2CCommand<T>>& commands) {}
	};
	template<> struct I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_ALL_COMMANDS>
	{
		I2CErrorPolicySupport() {}
		template<typename T>
		void handle_error(UNUSED const I2CCommand<T>& current, containers::Queue<I2CCommand<T>>& commands)
		{
			commands.clear_();
		}
	};
	template<> struct I2CErrorPolicySupport<I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS>
	{
		I2CErrorPolicySupport() {}
		template<typename T>
		void handle_error(const I2CCommand<T>& current, containers::Queue<I2CCommand<T>>& commands)
		{
			// Clear command belonging to the same transaction (i.e. same future)
			const auto future = current.future();
			I2CCommand<T> command;
			while (commands.peek_(command))
			{
				if (command.future() != future)
					break;
				commands.pull_(command);
			}
		}
	};
	/// @endcond

	/// @cond notdocumented
	// Specific traits for I2CMode
	template<I2CMode MODE_ = I2CMode::STANDARD> struct I2CMode_trait
	{
		static constexpr I2CMode MODE = MODE_;
		static constexpr uint32_t RATE = 100'000UL;
		static constexpr uint32_t FREQUENCY = (F_CPU / RATE - 16UL) / 2;
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(4.0);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(4.7);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(4.0 + 4.7);
	};
	template<> struct I2CMode_trait<I2CMode::FAST>
	{
		static constexpr I2CMode MODE = I2CMode::FAST;
		static constexpr uint32_t RATE = 400'000UL;
		static constexpr uint32_t FREQUENCY = (F_CPU / RATE - 16UL) / 2;
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(1.3);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(0.6 + 1.3);
	};
	/// @endcond

	// Abstract generic class for synchronous I2C management
	//TODO DO WE REALLY NEED LC for SYNC MANAGER?
	template<typename ARCH_HANDLER_, I2CMode MODE_, bool HAS_LC_, bool HAS_DEBUG_, typename DEBUG_HOOK_>
	class AbstractI2CSyncManager
	{
	protected:
		using ARCH_HANDLER = ARCH_HANDLER_;
		using MODE_TRAIT = I2CMode_trait<MODE_>;
		using I2C_TRAIT = board_traits::TWI_trait;
		using REG8 = board_traits::REG8;
		using DEBUG = I2CDebugSupport<HAS_DEBUG_, DEBUG_HOOK_>;

	public:
		/**
		 * Prepare and enable the MCU for I2C transmission.
		 * Preparation includes setup of I2C pins (SDA and SCL).
		 * This method is synchronized.
		 * @sa end()
		 * @sa begin_()
		 */
		void begin()
		{
			synchronized begin_();
		}

		/**
		 * Disable MCU I2C transmission.
		 * This method is synchronized.
		 * @sa begin()
		 * @sa end_()
		 */
		void end()
		{
			synchronized end_();
		}

		/**
		 * Prepare and enable the MCU for I2C transmission.
		 * Preparation includes setup of I2C pins (SDA and SCL).
		 * This method is NOT synchronized.
		 * @sa end_()
		 * @sa begin()
		 */
		void begin_()
		{
			handler_.begin_();
		}

		/**
		 * Disable MCU I2C transmission.
		 * This method is NOT synchronized.
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			handler_.end_();
		}

		/**
		 * Return latest transmission status.
		 * Possible statuses are defined in namespace `i2c::Status`.
		 * If latest operation was OK, then `i2c::Status::OK` (`0`) is returned.
		 * Any non zero value indicates an error.
		 * @sa i2c::Status
		 */
		uint8_t status() const
		{
			return status_;
		}

	protected:
		/// @cond notdocumented
		explicit AbstractI2CSyncManager(DEBUG_HOOK_ hook = nullptr) : handler_{}, debug_{hook} {}
		/// @endcond

	private:
		// bool begin_transaction(
		// 	uint8_t target, uint8_t* output, uint8_t max_read, const uint8_t* input, uint8_t max_write)
		// {
		// 	//TODO
		// 	target_ = target;
		// 	output_ = output;
		// 	max_read_ = max_read;
		// 	input_ = input;
		// 	max_write_ = max_write;
		// }

		bool write(uint8_t target, const uint8_t* input, uint8_t size, bool is_stop)
		{
			if (!before_()) return false;

			bool ok = exec_send_slaw_(target);
			while (ok && (size-- != 0)) ok = exec_send_data_(*input++);

			// Check if we must force a STOP
			after_(is_stop, !ok);
			return ok;
		}

		bool read(uint8_t target, uint8_t* output, uint8_t size, bool is_stop)
		{
			if (!before_()) return false;

			bool ok = exec_send_slar_(target);
			while (ok && (size-- != 0)) ok = exec_receive_data_(*output++, (size == 0));

			// Check if we must force a STOP
			after_(is_stop, !ok);
			return ok;
		}

		bool end_transaction()
		{
			// Check if previously executed command already did a STOP (and needed one)
			if ((!no_stop_) && (!stopped_already_) && (!clear_commands_))
				exec_stop_();
			bool ok = !clear_commands_;
			no_stop_ = false;
			clear_commands_ = false;
			stopped_already_ = false;
			return ok;
		}

		bool before_()
		{
			if (clear_commands_) return false;
			bool ok = (no_stop_ ? exec_repeat_start_() : exec_start_());
			stopped_already_ = false;
			return ok;
		}

		void after_(bool is_stop, bool error)
		{
			if (is_stop || error)
			{
				exec_stop_();
				// Clear command belonging to the same transaction
				// ie forbid any new command until last command (add new flag for that)
				if (error)
				{
					clear_commands_ = true;
					// status_ = ~Status::OK;
				}
				// Ensure STOP is generated or not depending on latest command executed
				no_stop_ = !is_stop;
			}
		}

		// Low-level methods to handle the bus in an asynchronous way
		bool exec_start_()
		{
			debug_.call_hook(DebugStatus::START);
			return handler_.exec_start_();
		}

		bool exec_repeat_start_()
		{
			debug_.call_hook(DebugStatus::REPEAT_START);
			return handler_.exec_repeat_start_();
		}

		bool exec_send_slar_(uint8_t target)
		{
			debug_.call_hook(DebugStatus::SLAR, target);
			return handler_.exec_send_slar_(target);
		}
		
		bool exec_send_slaw_(uint8_t target)
		{
			debug_.call_hook(DebugStatus::SLAW, target);
			return handler_.exec_send_slaw_(target);
		}

		bool exec_send_data_(uint8_t data)
		{
			debug_.call_hook(DebugStatus::SEND, data);
			bool ok = handler_.exec_send_data_(data);
			debug_.call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			return ok;
		}

		bool exec_receive_data_(uint8_t& data, bool last_byte = false)
		{
			const DebugStatus debug = (last_byte ? DebugStatus::RECV_LAST : DebugStatus::RECV);
			debug_.call_hook(debug);
			bool ok = handler_.exec_receive_data_(last_byte, data);
			debug_.call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
			return ok;
		}

		void exec_stop_()
		{
			debug_.call_hook(DebugStatus::STOP);
			handler_.exec_stop_();
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(MODE_TRAIT::DELAY_AFTER_STOP);
			stopped_already_ = true;
		}

		// Flags for storing I2C transaction operation state
		bool no_stop_ = false;
		bool clear_commands_ = false;
		bool stopped_already_ = false;
		uint8_t status_;

		// Data used during an I2C transaction
		// uint8_t target_;
		// uint8_t* output_;
		// uint8_t max_read_;
		// const uint8_t* input_;
		// uint8_t max_write_;

		ARCH_HANDLER_ handler_;
		DEBUG debug_;

		template<I2CMode, typename, bool> friend class I2CDevice;
	};


	/// @cond notdocumented
	// Specific traits for I2CManager
	template<typename T> struct I2CManager_trait
	{
		static constexpr bool IS_I2CMANAGER = false;
		static constexpr bool IS_ASYNC = false;
		static constexpr bool HAS_LIFECYCLE = false;
		static constexpr bool IS_DEBUG = false;
		static constexpr I2CMode MODE = I2CMode::STANDARD;
	};

	template<bool IS_ASYNC_, bool HAS_LIFECYCLE_, bool IS_DEBUG_, I2CMode MODE_>
	struct I2CManager_trait_impl
	{
		static constexpr bool IS_I2CMANAGER = true;
		static constexpr bool IS_ASYNC = IS_ASYNC_;
		static constexpr bool HAS_LIFECYCLE = HAS_LIFECYCLE_;
		static constexpr bool IS_DEBUG = IS_DEBUG_;
		static constexpr I2CMode MODE = MODE_;
	};
	/// @endcond
}

#endif /* I2C_HANDLER_COMMON_HH */
/// @endcond
