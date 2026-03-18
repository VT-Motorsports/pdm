uses zephyr, follow zephyr installation guidelines, install in parent directory with all 

please comment 
dont force push, make sure code compiles before PR

>west build -b nucleo_h753zi -p always < for building pristine for dev board>
west flash

open terminal on STLINK-V3 Virtual COM port at 115200 baud for debug output. 


use clangd for linting and compiler warning
C++11 with certain extensions to C++20 no STD:: thread control blocks use zephyr kernel functions 
zephyr latest release at 4.2.0. 

 <dbg> adc_stm32: adc_stm32_init: Initializing adc@58026000
[00:00:00.000,000] <err> adc_stm32: clock control device not ready
[00:00:00.000,000] <dbg> adc_stm32: adc_stm32_init: Initializing adc@40022000
[00:00:00.000,000] <err> adc_stm32: clock control device not ready
[00:00:00.000,000] <err> can_stm32h7: CAN pinctrl setup failed (-19)
*** Booting Zephyr OS build v4.2.0 ***
[00:00:00.000,000] <dbg> os: z_impl_k_mutex_lock: 0x24003f80 took mutex 0x24001248, count: 1, orig prio: 0
[00:00:00.001,000] <dbg> os: z_impl_k_mutex_unlock: mutex 0x24001248 lock_count: 1
[00:00:00.001,000] <dbg> os: z_impl_k_mutex_unlock: new owner of mutex 0x24001248: 0 (prio: -1000)
[00:00:00.001,000] <dbg> os: k_sched_unlock: scheduler unlocked (0x24003f80:0)
[00:00:00.001,000] <inf> main: REACHED MAIN
[00:00:00.101,000] <inf> main: AFTER WAIT
[00:00:00.101,000] <inf> main: ***VCU ENTERED MAIN***

[00:00:00.102,000] <inf> main: === VCU Starting ===
[00:00:00.102,000] <inf> system: Initializing system resources...
[00:00:00.102,000] <inf> system: Heap allocation test succeeded
[00:00:00.102,000] <inf> system: System initialized successfully
[00:00:00.102,000] <inf> hardware: Initializing hardware...
[00:00:00.102,000] <inf> hardware: Initializing ADCs...
[00:00:00.102,000] <err> hardware: ADC device adc1 not ready
[00:00:00.102,000] <err> hardware: Failed to initialize ADCs
[00:00:00.103,000] <err> main: Hardware init failed!
                                                              