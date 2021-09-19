//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * API to handle Real-Time Clock DS1307 I2C chip.
 */

#ifndef DS1307_H
#define DS1307_H

#include "../array.h"
#include "../i2c.h"
#include "../future.h"
#include "../utilities.h"
#include "../i2c_handler.h"
#include "../i2c_device.h"

// Device driver guidelines:
// - Template on MANAGER, subclass of I2CDevice
// - define types aliases PARENT, PROXY and FUTURE
// - constructor shall pass one of i2c::Mode i2c::I2C_STANDARD or i2c::I2C_FAST
//   to inherited constructor, in order tos pecify the BEST mode supported by this driver
// - Define FUTURE subclass (inside device class) for every future requiring input (constant, or user-provided)
//   naming convention: MethodNameFuture
// - FUTURE subclass shall have explicit constructor with mandatory input arguments (no default)
// - each async API method returns int (error code) and takes a PROXY to specific future as unique argument
// - each async API shall have a matching sync API with same name but different prototype:
//   direct input arguments, reference to output arguments, bool return (true if OK)
//   each sync API calls the matching async API after creating the proper Future on the stack
//   then it calls await() or get() on the future before returning.

namespace devices
{
	/**
	 * Defines API for Real-Time Clock chips usage.
	 */
	namespace rtc
	{
	}
}

namespace devices::rtc
{
	/** Days of the week. */
	enum class WeekDay : uint8_t
	{
		SUNDAY = 1,
		MONDAY,
		TUESDAY,
		WEDNESDAY,
		THURSDAY,
		FRIDAY,
		SATURDAY
	};

	/**
	 * The datetime structure used by the RTC API.
	 * This is inspired by C standard time.h header but was slightly adapted to
	 * fit DS1307 RTC chip.
	 * @sa DS1307::get_datetime()
	 * @sa DS1307::set_datetime()
	 */
	struct tm
	{
		/** seconds after the minute - [ 0 to 59 ] */
		uint8_t tm_sec = 0;
		/** minutes after the hour - [ 0 to 59 ] */
		uint8_t tm_min = 0;
		/** hours since midnight - [ 0 to 23 ] */
		uint8_t tm_hour = 0;
		/** days since Sunday - [ 1 to 7 ] */
		WeekDay tm_wday = WeekDay(0);
		/** day of the month - [ 1 to 31 ] */
		uint8_t tm_mday = 0;
		/** months since January - [ 1 to 12 ] */
		uint8_t tm_mon = 0;
		/** years since 2000 */
		uint8_t tm_year = 0;
	};

	/**
	 * The possible frequencies that can be generated by DS1307 RTC SQW/OUT pin.
	 * @sa DS1307::enable_output()
	 */
	enum class SquareWaveFrequency : uint8_t
	{
		FREQ_1HZ = 0x00,
		FREQ_4096HZ = 0x01,
		FREQ_8192HZ = 0x02,
		FREQ_32768HZ = 0x03
	};

	/**
	 * I2C device driver for the DS1307 RTC chip.
	 * Note that this chip only supports standard I2C mode (100 KHz).
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class DS1307 : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		struct SetTMHolder
		{
			explicit SetTMHolder(const tm& datetime)
			{
				tm_.tm_sec = utils::binary_to_bcd(datetime.tm_sec);
				tm_.tm_min = utils::binary_to_bcd(datetime.tm_min);
				tm_.tm_hour = utils::binary_to_bcd(datetime.tm_hour);
				tm_.tm_mday = utils::binary_to_bcd(datetime.tm_mday);
				tm_.tm_mon = utils::binary_to_bcd(datetime.tm_mon);
				tm_.tm_year = utils::binary_to_bcd(datetime.tm_year);
			}

			uint8_t address_ = TIME_ADDRESS;
			tm tm_;
		};

	public:
		/**
		 * Create a new device driver for a DS1307 chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit DS1307(MANAGER& manager) : PARENT{manager, DEVICE_ADDRESS, i2c::I2C_STANDARD, true} {}

		/**
		 * Get the size of the additional RAM size of the chip.
		 */
		static constexpr uint8_t ram_size()
		{
			return RAM_SIZE;
		}

		// Asynchronous API
		//==================
		/**
		 * Create a future to be used by asynchronous method set_datetime(SetDatetimeFuture&).
		 * This is used by `set_datetime()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * 
		 * @param datetime the new date/time to set on the RTC chip; all values 
		 * must be properly set and be correct, in particular, note that
		 * `tm::tm_wday` must be set correctly by yourself (the RTC chip does not
		 * calculate it itself).
		 * 
		 * @sa set_datetime(SetDatetimeFuture&)
		 */
		class SetDatetimeFuture : public FUTURE<void, SetTMHolder>
		{
			using PARENT = FUTURE<void, SetTMHolder>;

		public:
			/// @cond notdocumented
			explicit SetDatetimeFuture(const tm& datetime) : PARENT{SetTMHolder{datetime}} {}
			SetDatetimeFuture(SetDatetimeFuture&&) = default;
			SetDatetimeFuture& operator=(SetDatetimeFuture&&) = default;
			/// @endcond
		};

		/**
		 * Change date and time of the RTC chip connected to this driver.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `SetDatetimeFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa SetDatetimeFuture
		 * @sa set_datetime(const tm&)
		 * @sa errors
		 */
		int set_datetime(PROXY<SetDatetimeFuture> future)
		{
			// send register address to write to (0)
			// send datetime at address 0
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method get_datetime(GetDatetimeFuture&).
		 * This is used by `get_datetime()` to get device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * 
		 * @sa get_datetime(GetDatetimeFuture&)
		 */
		class GetDatetimeFuture : public FUTURE<tm, uint8_t>
		{
		public:
			/// @cond notdocumented
			GetDatetimeFuture() : FUTURE<tm, uint8_t>{TIME_ADDRESS} {}
			GetDatetimeFuture(GetDatetimeFuture&&) = default;
			GetDatetimeFuture& operator=(GetDatetimeFuture&&) = default;

			bool get(tm& datetime)
			{
				if (!FUTURE<tm, uint8_t>::get(datetime)) return false;
				// convert DS1307 output (BCD) to integer type
				datetime.tm_sec = utils::bcd_to_binary(datetime.tm_sec);
				datetime.tm_min = utils::bcd_to_binary(datetime.tm_min);
				datetime.tm_hour = utils::bcd_to_binary(datetime.tm_hour);
				datetime.tm_mday = utils::bcd_to_binary(datetime.tm_mday);
				datetime.tm_mon = utils::bcd_to_binary(datetime.tm_mon);
				datetime.tm_year = utils::bcd_to_binary(datetime.tm_year);
				return true;
			}
			/// @endcond
		};

		/**
		 * Get the current date and time from the RTC chip.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `GetDatetimeFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa GetDatetimeFuture
		 * @sa get_datetime(tm&)
		 * @sa errors
		 */
		int get_datetime(PROXY<GetDatetimeFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method set_ram(SetRamFuture<SIZE>&).
		 * This is used by `set_ram()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * @tparam SIZE_ the number of bytes to set to the chip RAM
		 * @param address the address of the first cell to set, must be between
		 * `0` and `ram_size()`.
		 * @param data a point to a buffer of size @p SIZE_ which content will be 
		 * written to the chip RAM starting at @p address
		 * 
		 * @sa set_ram(SetRamFuture<SIZE>&)
		 */
		template<uint8_t SIZE_>
		class SetRamFuture : public FUTURE<void, containers::array<uint8_t, SIZE_ + 1>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, SIZE_ + 1>>;
			using IN = typename PARENT::IN;

			static IN build_array(uint8_t address, const uint8_t (&data)[SIZE_])
			{
				IN input;
				input[0] = static_cast<uint8_t>(address + RAM_START);
				input.set(1, data);
				return input;
			}

		public:
			/// @cond notdocumented
			explicit SetRamFuture(uint8_t address, const uint8_t (&data)[SIZE_])
			: PARENT{build_array(address, data)}
			{
				static_assert(SIZE_ <= RAM_SIZE, "SIZE_ template paraneter must be less than RAM_SIZE!");
			}
			SetRamFuture(SetRamFuture<SIZE_>&&) = default;
			SetRamFuture& operator=(SetRamFuture<SIZE_>&&) = default;

			bool is_input_valid() const
			{
				return ((this->get_input()[0] + SIZE_) <= RAM_END);
			}
			/// @endcond
		};

		/**
		 * Set several cells of the chip internal RAM to specified values.
		 * @warning Asynchronous API!
		 * 
		 * @tparam SIZE the number of bytes to be written; `address + SIZE` must be
		 * less than `ram_size()`.
		 * @param future a `SetRamFuture<SIZE>` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa SetRamFuture<SIZE>
		 * @sa set_ram(uint8_t, const uint8_t(&)[])
		 * @sa get_ram(GetRamFuture<SIZE>&)
		 * @sa errors
		 */
		template<uint8_t SIZE> int set_ram(PROXY<SetRamFuture<SIZE>> future)
		{
			if (!this->resolve(future).is_input_valid())
				return errors::EINVAL;
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method set_ram(SetRam1Future&).
		 * This is used by `set_ram()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * @param address the address of the cell to set, must be between
		 * `0` and `ram_size()`.
		 * @param data the value to put at @p address
		 * 
		 * @sa set_ram(SetRam1Future&)
		 */
		class SetRam1Future : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		public:
			/// @cond notdocumented
			explicit SetRam1Future(uint8_t address, uint8_t data)
				:	PARENT{{static_cast<uint8_t>(address + RAM_START), data}} {}
			SetRam1Future(SetRam1Future&&) = default;
			SetRam1Future& operator=(SetRam1Future&&) = default;

			bool is_input_valid() const
			{
				return (this->get_input()[0] < RAM_END);
			}
			/// @endcond
		};

		/**
		 * Set one cell of the chip internal RAM to the specified value.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `SetRam1Future` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa SetRam1Future
		 * @sa get_ram(GetRam1Future&)
		 * @sa set_ram(uint8_t, uint8_t)
		 * @sa errors
		 */
		int set_ram(PROXY<SetRam1Future> future)
		{
			if (!this->resolve(future).is_input_valid())
				return errors::EINVAL;
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method get_ram(GetRamFuture<SIZE>&).
		 * This is used by `get_ram()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * @tparam SIZE the number of bytes to read from DS1307 RAM
		 * @param address the address of the cell to read, must be between
		 * `0` and `ram_size()`.
		 * 
		 * @sa get_ram(GetRamFuture<SIZE>&)
		 */
		template<uint8_t SIZE_>
		class GetRamFuture : public FUTURE<containers::array<uint8_t, SIZE_>, uint8_t>
		{
			using PARENT = FUTURE<containers::array<uint8_t, SIZE_>, uint8_t>;
		public:
			/// @cond notdocumented
			explicit GetRamFuture(uint8_t address) : PARENT{static_cast<uint8_t>(address + RAM_START)}
			{
				static_assert(SIZE_ <= RAM_SIZE, "SIZE_ template paraneter must be less than RAM_SIZE!");
			}
			GetRamFuture(GetRamFuture<SIZE_>&&) = default;
			GetRamFuture& operator=(GetRamFuture<SIZE_>&&) = default;

			bool is_input_valid() const
			{
				return ((this->get_input() + SIZE_) <= RAM_END);
			}
			/// @endcond
		};

		/**
		 * Get values of several cells from the chip internal RAM.
		 * @warning Asynchronous API!
		 * 
		 * @tparam SIZE the number of bytes to read; `address + SIZE` must be
		 * less than `ram_size()`.
		 * @param future an `GetRamFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa GetRamFuture<SIZE>
		 * @sa set_ram(SetRamFuture<SIZE>&)
		 * @sa get_ram(uint8_t, uint8_t*, uint8_t)
		 * @sa errors
		 */
		template<uint8_t SIZE> int get_ram(PROXY<GetRamFuture<SIZE>> future)
		{
			if (!this->resolve(future).is_input_valid())
				return errors::EINVAL;
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method get_ram(GetRam1Future&).
		 * This is used by `get_ram()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * @param address the address of the cell to read, must be between
		 * `0` and `ram_size()`.
		 * 
		 * @sa get_ram(GetRam1Future&)
		 */
		class GetRam1Future : public FUTURE<uint8_t, uint8_t>
		{
			using PARENT = FUTURE<uint8_t, uint8_t>;
		public:
			/// @cond notdocumented
			explicit GetRam1Future(uint8_t address) : PARENT{static_cast<uint8_t>(address + RAM_START)} {}
			GetRam1Future(GetRam1Future&&) = default;
			GetRam1Future& operator=(GetRam1Future&&) = default;

			bool is_input_valid() const
			{
				return this->get_input() < RAM_END;
			}
			/// @endcond
		};

		/**
		 * Get the value of one cell of the chip internal RAM.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `GetRam1Future` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa GetRam1Future
		 * @sa get_ram(uint8_t, uint8_t&)
		 * @sa set_ram(SetRam1Future&)
		 * @sa errors
		 */
		int get_ram(PROXY<GetRam1Future> future)
		{
			if (!this->resolve(future).is_input_valid())
				return errors::EINVAL;
			return this->launch_commands(future, {this->write(), this->read()});
		}

		/**
		 * Create a future to be used by asynchronous method halt_clock(HaltClockFuture&).
		 * This is used by `halt_clock()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * 
		 * @sa halt_clock(HaltClockFuture&)
		 */
		class HaltClockFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
		public:
			/// @cond notdocumented
			// just write 0x80 at address 0
			HaltClockFuture() : FUTURE<void, containers::array<uint8_t, 2>>{{TIME_ADDRESS, CLOCK_HALT}} {}
			HaltClockFuture(HaltClockFuture&&) = default;
			HaltClockFuture& operator=(HaltClockFuture&&) = default;
			/// @endcond
		};

		/**
		 * Disable the RTC oscillator, hence the time will not be updated anymore.
		 * If you want to re-enable the clock, you need to set a new date/time
		 * with `set_datetime()`.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `HaltClockFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa HaltClockFuture
		 * @sa set_datetime()
		 * @sa halt_clock()
		 * @sa errors
		 */
		int halt_clock(PROXY<HaltClockFuture> future)
		{
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method enable_output(EnableOutputFuture&).
		 * This is used by `enable_output()` to set device data, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished.
		 * @param frequency one of the supported RTC frequencies, defined in
		 * `SquareWaveFrequency` enum
		 * 
		 * @sa enable_output(EnableOutputFuture&)
		 */
		class EnableOutputFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		public:
			/// @cond notdocumented
			explicit EnableOutputFuture(SquareWaveFrequency frequency)
			{
				ControlRegister control;
				control.sqwe_ = true;
				control.rs_ = uint8_t(frequency);
				typename PARENT::IN input;
				input[0] = CONTROL_ADDRESS;
				input[1] = control.data_;
				this->reset_input_(input);
			}
			EnableOutputFuture(EnableOutputFuture&&) = default;
			EnableOutputFuture& operator=(EnableOutputFuture&&) = default;
			/// @endcond
		};

		/**
		 * Enable square wave output to the SQW/OUT pin of the RTC chip.
		 * @warning Asynchronous API!
		 * 
		 * @param future an `EnableOutputFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa EnableOutputFuture
		 * @sa enable_output(SquareWaveFrequency)
		 * @sa disable_output(DisableOutputFuture&)
		 * @sa errors
		 */
		int enable_output(PROXY<EnableOutputFuture> future)
		{
			return this->launch_commands(future, {this->write()});
		}

		/**
		 * Create a future to be used by asynchronous method disable_output(DisableOutputFuture&).
		 * This is used by `disable_output()` to perform the I2C transaction, 
		 * and it shall be used by the caller to determine when the I2C transaction 
		 * is finished.
		 * @param output_value value to force on SQW/OUT pin
		 * 
		 * @sa disable_output(DisableOutputFuture&)
		 */
		class DisableOutputFuture : public FUTURE<void, containers::array<uint8_t, 2>>
		{
			using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
		public:
			/// @cond notdocumented
			explicit DisableOutputFuture(bool output_value)
			{
				ControlRegister control;
				control.out_ = output_value;
				typename PARENT::IN input;
				input[0] = CONTROL_ADDRESS;
				input[1] = control.data_;
				this->reset_input_(input);
			}
			DisableOutputFuture(DisableOutputFuture&&) = default;
			DisableOutputFuture& operator=(DisableOutputFuture&&) = default;
			/// @endcond
		};

		/**
		 * Enable square wave output to the SQW/OUT pin of the RTC chip.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `DisableOutputFuture` passed by the caller, that will be
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa DisableOutputFuture
		 * @sa disable_output(bool)
		 * @sa enable_output()
		 * @sa errors
		 */
		int disable_output(PROXY<DisableOutputFuture> future)
		{
			return this->launch_commands(future, {this->write()});
		}

		// Synchronous API
		//=================
		/**
		 * Change date and time of the RTC chip connected to this driver.
		 * @warning Blocking API!
		 * 
		 * @param datetime the new date/time to set on the RTC chip; all values 
		 * must be properly set and be correct, in particular, note that
		 * `tm::tm_wday` must be set correctly by yourself (the RTC chip does not
		 * calculate it itself).
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_datetime(SetDatetimeFuture&)
		 */
		bool set_datetime(const tm& datetime)
		{
			SetDatetimeFuture future{datetime};
			if (set_datetime(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get the current date and time from the RTC chip.
		 * @warning Blocking API!
		 * 
		 * @param datetime a reference to a `tm` variable that will be filled with
		 * current date and time from the RTC chip
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_datetime(GetDatetimeFuture&)
		 */
		bool get_datetime(tm& datetime)
		{
			GetDatetimeFuture future;
			if (get_datetime(PARENT::make_proxy(future)) != 0) return false;
			return future.get(datetime);
		}

		/**
		 * Disable the RTC oscillator, hence the time will not be updated anymore.
		 * If you want to re-enable the clock, you need to set a new date/time
		 * with `set_datetime()`.
		 * @warning Blocking API!
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_datetime()
		 * @sa halt_clock(HaltClockFuture&)
		 */
		bool halt_clock()
		{
			HaltClockFuture future;
			if (halt_clock(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Enable square wave output to the SQW/OUT pin of the RTC chip.
		 * @warning Blocking API!
		 * 
		 * @param frequency one of the supported RTC frequencies, defined in
		 * `SquareWaveFrequency` enum
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa disable_output()
		 * @sa enable_output(EnableOutputFuture&)
		 */
		bool enable_output(SquareWaveFrequency frequency = SquareWaveFrequency::FREQ_1HZ)
		{
			EnableOutputFuture future{frequency};
			if (enable_output(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Disable square wave output to the SQW/OUT pin of the RTC chip.
		 * @warning Blocking API!
		 * 
		 * @param output_value value to force on SQW/OUT pin
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa enable_output()
		 * @sa disable_output(DisableOutputFuture&)
		 */
		bool disable_output(bool output_value = false)
		{
			DisableOutputFuture future{output_value};
			if (disable_output(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Set one cell of the chip internal RAM to the specified value.
		 * @warning Blocking API!
		 * 
		 * @param address the address of the cell to write, must be between
		 * `0` and `ram_size()`.
		 * @param data the value to put at @p address
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_ram(uint8_t)
		 * @sa set_ram(SetRam1Future&)
		 */
		bool set_ram(uint8_t address, uint8_t data)
		{
			SetRam1Future future{address, data};
			if (set_ram(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get the value of one cell of the chip internal RAM.
		 * @warning Blocking API!
		 * 
		 * @param address the address of the cell to read, must be between
		 * `0` and `ram_size()`.
		 * @return the value read at @p address
		 * 
		 * @sa set_ram(uint8_t, uint8_t)
		 * @sa get_ram(GetRam1Future&)
		 */
		uint8_t get_ram(uint8_t address)
		{
			GetRam1Future future{address};
			if (get_ram(PARENT::make_proxy(future)) != 0) return false;
			uint8_t data = 0;
			future.get(data);
			return data;
		}

		/**
		 * Set several cells of the chip internal RAM to specified values.
		 * @warning Blocking API!
		 * 
		 * @tparam SIZE the number of bytes to be written; `address + SIZE` must be
		 * less than `ram_size()`.
		 * @param address the address of the first cell to write, must be between
		 * `0` and `ram_size()`.
		 * @param data pointer to a buffer containing the values to write
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_ram(uint8_t, uint8_t(&)[])
		 * @sa set_ram(SetRamFuture<SIZE>&)
		 */
		template<uint8_t SIZE> bool set_ram(uint8_t address, const uint8_t (&data)[SIZE])
		{
			SetRamFuture<SIZE> future{address, data};
			if (set_ram(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Get values of several cells from the chip internal RAM.
		 * @warning Blocking API!
		 * 
		 * @tparam SIZE the number of bytes to read; `address + SIZE` must be
		 * less than `ram_size()`.
		 * @param address the address of the first cell to read, must be between
		 * `0` and `ram_size()`.
		 * @param data pointer to a buffer where read values will be copied
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_ram(uint8_t, const uint8_t(&)[])
		 * @sa get_ram(GetRamFuture<SIZE>&)
		 */
		template<uint8_t SIZE> bool get_ram(uint8_t address, uint8_t (&data)[SIZE])
		{
			GetRamFuture<SIZE> future{address};
			if (get_ram(PARENT::make_proxy(future)) != 0) return false;
			typename GetRamFuture<SIZE>::OUT temp;
			if (!future.get(temp)) return false;
			memcpy(data, temp.data(), SIZE);
			return true;
		}

		/**
		 * Write any type of data to the chip internal RAM.
		 * @warning Blocking API!
		 * 
		 * @tparam T the type of data to be written
		 * @param address the address where to write data, must be between
		 * `0` and `ram_size() - sizeof(T)`.
		 * @param data the actual data of type @p T, to be written
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_ram(uint8_t, T&)
		 * @sa set_ram(SetRamFuture<SIZE>&)
		 */
		template<typename T> bool set_ram(uint8_t address, const T& data)
		{
			uint8_t temp[sizeof(T)];
			utils::as_array<T>(data, temp);
			SetRamFuture<sizeof(T)> future{address, temp};
			if (set_ram(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Read any type of data from the chip internal RAM.
		 * Note that this method only copies bytes to a variable of some type, no
		 * constructor will be called; hence, it is preferrable to use only simple
		 * struct types.
		 * @warning Blocking API!
		 * 
		 * @tparam T the type of data to be read
		 * @param address the address where to find data to read, must be between
		 * `0` and `ram_size() - sizeof(T)`.
		 * @param data a reference to a variable that will be copied the read content
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_ram(uint8_t, const T&)
		 * @sa get_ram(GetRamFuture<SIZE>&)
		 */
		template<typename T> bool get_ram(uint8_t address, T& data)
		{
			GetRamFuture<sizeof(T)> future{address};
			if (get_ram(PARENT::make_proxy(future)) != 0) return false;
			return future.get(reinterpret_cast<uint8_t&>(data));
		}

	private:
		static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
		static constexpr const uint8_t RAM_START = 0x08;
		static constexpr const uint8_t RAM_END = 0x40;
		static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;
		static constexpr const uint8_t TIME_ADDRESS = 0x00;
		static constexpr const uint8_t CLOCK_HALT = 0x80;
		static constexpr const uint8_t CONTROL_ADDRESS = 0x07;

		union ControlRegister
		{
			explicit ControlRegister(uint8_t data = 0) : data_{data} {}

			uint8_t data_;
			struct
			{
				uint8_t rs_ : 2;
				uint8_t res1_ : 2;
				bool sqwe_ : 1;
				uint8_t res2_ : 2;
				bool out_ : 1;
			};
		};
	};
}

#endif /* DS1307_H */
/// @endcond
