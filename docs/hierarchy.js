var hierarchy =
[
    [ "spi::AbstractSPIDevice", "classspi_1_1_abstract_s_p_i_device.html", [
      [ "spi::SPIDevice< CS, CS_MODE, RATE, MODE, ORDER >", "classspi_1_1_s_p_i_device.html", null ],
      [ "spi::SPIDevice< CS, spi::ChipSelect::ACTIVE_LOW, spi::ClockRate::CLOCK_DIV_2 >", "classspi_1_1_s_p_i_device.html", [
        [ "devices::WinBond< CS >", "classdevices_1_1_win_bond.html", null ]
      ] ],
      [ "spi::SPIDevice< CSN >", "classspi_1_1_s_p_i_device.html", [
        [ "devices::rf::NRF24L01< CSN, CE >", "classdevices_1_1rf_1_1_n_r_f24_l01.html", [
          [ "devices::rf::IRQ_NRF24L01< CSN, CE, IRQ >", "classdevices_1_1rf_1_1_i_r_q___n_r_f24_l01.html", null ]
        ] ]
      ] ]
    ] ],
    [ "AbstractUART", null, [
      [ "serial::hard::UARX< USART >", "classserial_1_1hard_1_1_u_a_r_x.html", [
        [ "serial::hard::UART< USART >", "classserial_1_1hard_1_1_u_a_r_t.html", null ]
      ] ],
      [ "serial::hard::UATX< USART >", "classserial_1_1hard_1_1_u_a_t_x.html", [
        [ "serial::hard::UART< USART >", "classserial_1_1hard_1_1_u_a_r_t.html", null ]
      ] ]
    ] ],
    [ "devices::rf::NRF24L01< CSN, CE >::addr_t", "structdevices_1_1rf_1_1_n_r_f24_l01_1_1addr__t.html", null ],
    [ "devices::magneto::AllSensors", "structdevices_1_1magneto_1_1_all_sensors.html", null ],
    [ "analog::AnalogInput< APIN, AREF, SAMPLE_TYPE, MAXFREQ >", "classanalog_1_1_analog_input.html", null ],
    [ "analog::AnalogInput< BG, board::AnalogReference::AVCC, uint16_t, board::AnalogClock::MAX_FREQ_50KHz >", "classanalog_1_1_analog_input.html", [
      [ "analog::PowerVoltage< BG >", "classanalog_1_1_power_voltage.html", null ]
    ] ],
    [ "interrupt::HandlerHolder< Handler >::ArgsHodler< Args >", "structinterrupt_1_1_handler_holder_1_1_args_hodler.html", null ],
    [ "time::auto_delay", "classtime_1_1auto__delay.html", null ],
    [ "time::auto_millis", "classtime_1_1auto__millis.html", null ],
    [ "timer::Calculator< TIMER >", "structtimer_1_1_calculator.html", null ],
    [ "interrupt::HandlerHolder< Handler >::ArgsHodler< Args >::CallbackHolder< Callback >", "structinterrupt_1_1_handler_holder_1_1_args_hodler_1_1_callback_holder.html", null ],
    [ "devices::WinBond< CS >::Device", "structdevices_1_1_win_bond_1_1_device.html", null ],
    [ "eeprom::EEPROM", "classeeprom_1_1_e_e_p_r_o_m.html", [
      [ "eeprom::QueuedWriter", "classeeprom_1_1_queued_writer.html", null ]
    ] ],
    [ "streams::EmptyOutput", "classstreams_1_1_empty_output.html", null ],
    [ "events::Event", "classevents_1_1_event.html", null ],
    [ "gpio::FastMaskedPort< PORT_ >", "classgpio_1_1_fast_masked_port.html", null ],
    [ "gpio::FastPin< PORT_, BIT_ >", "classgpio_1_1_fast_pin.html", null ],
    [ "gpio::FastPinType< DPIN >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< CE >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< CLOCK >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< CS >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< DATA >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< ECHO >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< LATCH >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< RX >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< TRIGGER >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPinType< TX >", "classgpio_1_1_fast_pin_type.html", null ],
    [ "gpio::FastPort< PORT_ >", "classgpio_1_1_fast_port.html", null ],
    [ "devices::rf::NRF24L01< CSN, CE >::fifo_status_t", "uniondevices_1_1rf_1_1_n_r_f24_l01_1_1fifo__status__t.html", null ],
    [ "devices::magneto::FIFOEnable", "structdevices_1_1magneto_1_1_f_i_f_o_enable.html", null ],
    [ "streams::FormatBase", "classstreams_1_1_format_base.html", [
      [ "streams::FormattedInput< STREAM >", "classstreams_1_1_formatted_input.html", null ],
      [ "streams::FormattedOutput< STREAM >", "classstreams_1_1_formatted_output.html", null ]
    ] ],
    [ "interrupt::HandlerHolder< Handler >", "classinterrupt_1_1_handler_holder.html", null ],
    [ "devices::sonar::HCSR04< TIMER, TRIGGER, ECHO >", "classdevices_1_1sonar_1_1_h_c_s_r04.html", null ],
    [ "i2c::I2CDevice< MODE >", "classi2c_1_1_i2_c_device.html", [
      [ "devices::magneto::HMC5883L< MODE >", "classdevices_1_1magneto_1_1_h_m_c5883_l.html", null ],
      [ "devices::magneto::MPU6050< MODE, AD0 >", "classdevices_1_1magneto_1_1_m_p_u6050.html", null ]
    ] ],
    [ "i2c::I2CDevice< i2c::I2CMode::Standard >", "classi2c_1_1_i2_c_device.html", [
      [ "devices::rtc::DS1307", "classdevices_1_1rtc_1_1_d_s1307.html", null ]
    ] ],
    [ "i2c::I2CHandler< MODE >", "classi2c_1_1_i2_c_handler.html", null ],
    [ "i2c::I2CManager< MODE >", "classi2c_1_1_i2_c_manager.html", null ],
    [ "interrupt::INTSignal< PIN >", "classinterrupt_1_1_i_n_t_signal.html", null ],
    [ "devices::magneto::INTStatus", "structdevices_1_1magneto_1_1_i_n_t_status.html", null ],
    [ "containers::LinkedListImpl", "classcontainers_1_1_linked_list_impl.html", [
      [ "containers::LinkedList< T >", "classcontainers_1_1_linked_list.html", null ],
      [ "containers::LinkedList< EventHandler >", "classcontainers_1_1_linked_list.html", [
        [ "events::Dispatcher", "classevents_1_1_dispatcher.html", null ]
      ] ],
      [ "containers::LinkedList< Job >", "classcontainers_1_1_linked_list.html", [
        [ "events::Scheduler< CLOCK >", "classevents_1_1_scheduler.html", null ]
      ] ]
    ] ],
    [ "containers::LinkImpl", "classcontainers_1_1_link_impl.html", [
      [ "containers::Link< T >", "classcontainers_1_1_link.html", null ],
      [ "containers::Link< EventHandler >", "classcontainers_1_1_link.html", [
        [ "events::EventHandler", "classevents_1_1_event_handler.html", [
          [ "events::Scheduler< CLOCK >", "classevents_1_1_scheduler.html", null ]
        ] ]
      ] ],
      [ "containers::Link< Job >", "classcontainers_1_1_link.html", [
        [ "events::Job", "classevents_1_1_job.html", null ]
      ] ]
    ] ],
    [ "devices::magneto::MagneticFields", "structdevices_1_1magneto_1_1_magnetic_fields.html", null ],
    [ "devices::rf::NRF24L01< CSN, CE >::observe_tx_t", "uniondevices_1_1rf_1_1_n_r_f24_l01_1_1observe__tx__t.html", null ],
    [ "interrupt::PCISignal< PORT >", "classinterrupt_1_1_p_c_i_signal.html", null ],
    [ "interrupt::PCIType< PIN >", "structinterrupt_1_1_p_c_i_type.html", null ],
    [ "power::Power", "classpower_1_1_power.html", null ],
    [ "devices::magneto::PowerManagement", "structdevices_1_1magneto_1_1_power_management.html", null ],
    [ "analog::PWMOutput< PIN, PULSED >", "classanalog_1_1_p_w_m_output.html", null ],
    [ "analog::PWMOutput< PIN, true >", "classanalog_1_1_p_w_m_output.html", null ],
    [ "containers::Queue< T, TREF >", "classcontainers_1_1_queue.html", null ],
    [ "containers::Queue< char, char >", "classcontainers_1_1_queue.html", [
      [ "streams::InputBuffer", "classstreams_1_1_input_buffer.html", [
        [ "serial::hard::UARX< USART >", "classserial_1_1hard_1_1_u_a_r_x.html", null ],
        [ "serial::soft::AbstractUARX", "classserial_1_1soft_1_1_abstract_u_a_r_x.html", [
          [ "serial::soft::UARX< RX >", "classserial_1_1soft_1_1_u_a_r_x.html", [
            [ "serial::soft::UART< RX, TX >", "classserial_1_1soft_1_1_u_a_r_t.html", null ]
          ] ]
        ] ]
      ] ],
      [ "streams::OutputBuffer", "classstreams_1_1_output_buffer.html", [
        [ "serial::hard::UATX< USART >", "classserial_1_1hard_1_1_u_a_t_x.html", null ],
        [ "serial::soft::AbstractUATX", "classserial_1_1soft_1_1_abstract_u_a_t_x.html", [
          [ "serial::soft::UATX< TX >", "classserial_1_1soft_1_1_u_a_t_x.html", [
            [ "serial::soft::UART< RX, TX >", "classserial_1_1soft_1_1_u_a_r_t.html", null ]
          ] ]
        ] ]
      ] ]
    ] ],
    [ "containers::Queue< events::Event >", "classcontainers_1_1_queue.html", null ],
    [ "containers::Queue< uint8_t, uint8_t >", "classcontainers_1_1_queue.html", null ],
    [ "timer::RTTEventCallback< PERIOD_MS >", "classtimer_1_1_r_t_t_event_callback.html", null ],
    [ "time::RTTTime", "structtime_1_1_r_t_t_time.html", null ],
    [ "devices::magneto::Sensor3D", "structdevices_1_1magneto_1_1_sensor3_d.html", null ],
    [ "devices::servo::Servo< TIMER, PIN >", "classdevices_1_1servo_1_1_servo.html", null ],
    [ "devices::SIPO< CLOCK, LATCH, DATA >", "classdevices_1_1_s_i_p_o.html", null ],
    [ "devices::WinBond< CS >::Status", "structdevices_1_1_win_bond_1_1_status.html", null ],
    [ "devices::magneto::Status", "structdevices_1_1magneto_1_1_status.html", null ],
    [ "devices::rf::NRF24L01< CSN, CE >::status_t", "uniondevices_1_1rf_1_1_n_r_f24_l01_1_1status__t.html", null ],
    [ "timer::Timer< TIMER >", "classtimer_1_1_timer.html", [
      [ "timer::PulseTimer16< TIMER, PRESCALER >", "classtimer_1_1_pulse_timer16.html", [
        [ "timer::PulseTimer< TIMER, PRESCALER, uint16_t >", "classtimer_1_1_pulse_timer_3_01_t_i_m_e_r_00_01_p_r_e_s_c_a_l_e_r_00_01uint16__t_01_4.html", null ]
      ] ],
      [ "timer::PulseTimer8< TIMER, PRESCALER >", "classtimer_1_1_pulse_timer8.html", [
        [ "timer::PulseTimer< TIMER, PRESCALER, uint8_t >", "classtimer_1_1_pulse_timer_3_01_t_i_m_e_r_00_01_p_r_e_s_c_a_l_e_r_00_01uint8__t_01_4.html", null ]
      ] ],
      [ "timer::PulseTimer< TIMER, PRESCALER, T >", "classtimer_1_1_pulse_timer.html", null ],
      [ "timer::PulseTimer16< TIMER, PRESCALER_ >", "classtimer_1_1_pulse_timer16.html", null ],
      [ "timer::PulseTimer8< TIMER, PRESCALER_ >", "classtimer_1_1_pulse_timer8.html", null ],
      [ "timer::RTT< TIMER >", "classtimer_1_1_r_t_t.html", null ]
    ] ],
    [ "devices::rtc::tm", "structdevices_1_1rtc_1_1tm.html", null ],
    [ "serial::UARTErrors", "classserial_1_1_u_a_r_t_errors.html", [
      [ "serial::soft::AbstractUARX", "classserial_1_1soft_1_1_abstract_u_a_r_x.html", null ],
      [ "serial::soft::AbstractUATX", "classserial_1_1soft_1_1_abstract_u_a_t_x.html", null ]
    ] ],
    [ "watchdog::WatchdogSignal", "classwatchdog_1_1_watchdog_signal.html", [
      [ "watchdog::Watchdog", "classwatchdog_1_1_watchdog.html", null ]
    ] ]
];