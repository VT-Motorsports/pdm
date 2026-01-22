# VCU Board Migration - Execution Checklist

## Pre-Migration Verification

- [ ] Zephyr workspace installed and working
- [ ] Can build and flash Nucleo H753ZI successfully
- [ ] west command line tools working
- [ ] Custom STM32H753VIT6 hardware available
- [ ] SWD debugger/programmer available (ST-Link, J-Link, etc.)
- [ ] Have your hardware schematic ready

## Step 1: Copy Board Definition Files

```bash
# Navigate to your VCU repository root
cd /path/to/your/vcu_repo

# Create boards directory structure if it doesn't exist
mkdir -p boards/st

# Copy the vcu_stm32 board definition
cp -r /path/to/vcu_board_template/boards/st/vcu_stm32 boards/st/

# Verify files copied correctly
ls -la boards/st/vcu_stm32/
```

Expected output:
```
board.cmake
board.yml
Kconfig.defconfig
Kconfig.vcu_stm32
README.md
vcu_stm32.dts
vcu_stm32_defconfig
```

- [ ] Board files copied successfully
- [ ] All 7 files present

## Step 2: Update Project Configuration

### Update CMakeLists.txt

```bash
# Backup existing CMakeLists.txt
cp CMakeLists.txt CMakeLists.txt.backup

# Edit CMakeLists.txt to add BOARD_ROOT
```

Add this BEFORE `find_package(Zephyr ...)`:
```cmake
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
```

- [ ] CMakeLists.txt updated
- [ ] BOARD_ROOT set before find_package(Zephyr ...)

### Copy Example Files (if starting fresh)

```bash
# If you don't have these files yet:
cp /path/to/vcu_board_template/prj.conf .
cp /path/to/vcu_board_template/src/main.cpp src/
```

- [ ] prj.conf configured
- [ ] Test main.cpp in place (or your existing code ready)

## Step 3: First Build Attempt

```bash
# Clean build with new board
west build -b vcu_stm32 --pristine

# Or without west:
rm -rf build
cmake -B build -DBOARD=vcu_stm32 -GNinja
ninja -C build
```

### Common Build Errors and Fixes:

#### Error: "Board vcu_stm32 not found"
**Fix:** Verify BOARD_ROOT is set in CMakeLists.txt before find_package

#### Error: "SOC_STM32H753XX not defined"
**Fix:** Check Kconfig.vcu_stm32 and vcu_stm32_defconfig

#### Error: "Devicetree errors"
**Fix:** Check vcu_stm32.dts syntax, verify includes

- [ ] Build completes successfully
- [ ] No devicetree errors
- [ ] Firmware binary generated (build/zephyr/zephyr.bin)

## Step 4: Hardware Preparation

### Connect Hardware

1. Connect SWD debugger to your custom board:
   - SWDIO
   - SWCLK
   - GND
   - VCC (or power board separately)

2. Connect UART console (USART3 by default):
   - TX: PD8
   - RX: PD9
   - GND

3. Open serial terminal:
```bash
# Linux/Mac
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200

# Windows
# Use PuTTY or TeraTerm
```

- [ ] SWD connected
- [ ] UART console connected
- [ ] Serial terminal open at 115200 baud
- [ ] Board powered

## Step 5: Flash Firmware

```bash
# Flash with west
west flash

# Or with OpenOCD directly:
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/zephyr/zephyr.elf verify reset exit"

# Or with J-Link:
JLinkExe -device STM32H753VI -if SWD -speed 4000 -autoconnect 1
# In J-Link prompt:
# loadfile build/zephyr/zephyr.bin 0x08000000
# r
# g
```

- [ ] Firmware flashed successfully
- [ ] No flash errors

## Step 6: First Boot Verification

### Check Serial Console

Expected output:
```
*** Booting Zephyr OS vX.X.X ***
[00:00:00.000,000] <inf> vcu_bringup: ===========================================
[00:00:00.000,000] <inf> vcu_bringup: VCU STM32H753VIT6 Board Bring-Up Test
[00:00:00.000,000] <inf> vcu_bringup: ===========================================
[00:00:00.000,000] <inf> vcu_bringup: Board: vcu_stm32
[00:00:00.000,000] <inf> vcu_bringup: SoC: STM32H753VIT6 (LQFP100)
[00:00:00.000,000] <inf> vcu_bringup: System Clock: 480000000 Hz
```

### If No Console Output:

1. **Check UART pins** - verify PD8/PD9 or your configured pins
2. **Check baud rate** - try 9600, 38400, 115200
3. **Check HSE clock** - if garbage output, HSE frequency wrong in DTS
4. **Check power** - verify 3.3V on VDD pins

- [ ] Serial console shows output
- [ ] Boot message appears
- [ ] No garbage characters
- [ ] System clock shows 480 MHz (or expected value)

## Step 7: GPIO/LED Test

- [ ] LED blinks at 1Hz
- [ ] Serial log shows "Heartbeat #N - LED toggled"
- [ ] Correct GPIO pin confirmed

### If LED Doesn't Blink:

1. Check LED pin in schematic vs vcu_stm32.dts
2. Verify GPIO port is enabled
3. Use oscilloscope/logic analyzer on LED pin
4. Check if pin exists on LQFP100 package

## Step 8: CAN Peripheral Test

Check serial output for:
```
[00:00:00.XXX,XXX] <inf> vcu_bringup: Testing CAN peripheral...
[00:00:00.XXX,XXX] <inf> vcu_bringup: CAN test PASSED - peripheral ready
```

- [ ] CAN peripheral initializes
- [ ] No CAN errors in log

### If CAN Fails:

1. Verify FDCAN1 pins (PD0/PD1 by default)
2. Check CAN transceiver power
3. Verify pin assignments match hardware

## Step 9: Clock Validation

### Verify System Clock

From serial output, check:
```
System Clock: 480000000 Hz
```

Should be **exactly 480 MHz** for standard config.

### Clock Notes for HSI Configuration

- **No external crystal required** - board uses internal 64 MHz HSI oscillator
- HSI has approximately 1% accuracy across temperature/voltage
- This is sufficient for CAN (up to 1 Mbps), UART, and most peripherals
- UART baud rate should be accurate at 115200 (no garbage characters)
- If you see timing issues, verify PLL settings in vcu_stm32.dts

**HSI is inherently stable** - you should NOT see garbage UART output or clock drift issues that would occur with a missing/failed external crystal.

- [ ] System clock correct (480 MHz)
- [ ] UART baud rate correct (no garbage)
- [ ] Timestamps in logs incrementing properly

## Step 10: Pin Mapping Customization

Now customize the board for your actual hardware:

1. Open your hardware schematic
2. Edit `boards/st/vcu_stm32/vcu_stm32.dts`
3. Update pin assignments for:
   - [ ] UART console (if not PD8/PD9)
   - [ ] CAN bus pins
   - [ ] LED(s)
   - [ ] Button(s) (if any)
   - [ ] Other peripherals

### Pin Change Template:

```dts
// Example: Change UART from PD8/PD9 to PA2/PA3
&usart2 {
    pinctrl-0 = <&usart2_tx_pa2 &usart2_rx_pa3>;
    pinctrl-names = "default";
    current-speed = <115200>;
    status = "okay";
};

// Update chosen to use new UART
chosen {
    zephyr,console = &usart2;
    zephyr,shell-uart = &usart2;
};
```

Rebuild and test after each change.

## Step 11: Enable Additional Peripherals

Add peripherals as needed:

### SPI Example:
```dts
&spi1 {
    pinctrl-0 = <&spi1_sck_pa5 &spi1_miso_pa6 &spi1_mosi_pa7>;
    pinctrl-names = "default";
    cs-gpios = <&gpioa 4 GPIO_ACTIVE_LOW>;
    status = "okay";
};
```

Add to prj.conf:
```
CONFIG_SPI=y
```

### I2C Example:
```dts
&i2c1 {
    pinctrl-0 = <&i2c1_scl_pb6 &i2c1_sda_pb7>;
    pinctrl-names = "default";
    clock-frequency = <I2C_BITRATE_FAST>;
    status = "okay";
};
```

Add to prj.conf:
```
CONFIG_I2C=y
```

### ADC Example:
```dts
&adc1 {
    pinctrl-0 = <&adc1_inp0_pa0>;
    pinctrl-names = "default";
    st,adc-clock-source = <SYNC>;
    st,adc-prescaler = <4>;
    status = "okay";
};
```

Add to prj.conf:
```
CONFIG_ADC=y
```

- [ ] SPI configured and tested
- [ ] I2C configured and tested
- [ ] ADC configured and tested
- [ ] PWM/Timers configured
- [ ] Additional CANs configured

## Step 12: Migrate Application Code

Now that the board boots reliably:

1. Start with simple functions first
2. Port one module at a time
3. Test each module before moving to next
4. Use the devicetree API for all GPIO/peripheral access

Example migration pattern:
```cpp
// Old direct register access:
// GPIO_PORT->BSRR = GPIO_PIN_5;

// New Zephyr devicetree approach:
const struct gpio_dt_spec my_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(my_gpio), gpios);
gpio_pin_set_dt(&my_pin, 1);
```

- [ ] State machine code ported
- [ ] CAN message handlers ported
- [ ] Sensor reading code ported
- [ ] Safety/watchdog code ported
- [ ] Telemetry code ported

## Success Criteria

✅ Board boots reliably every time
✅ Serial console output clean and correct
✅ LED blinks as expected
✅ CAN peripheral initializes
✅ All required peripherals working
✅ Application code running
✅ No hard faults or crashes
✅ Ready for vehicle integration testing

## Troubleshooting Resources

### Zephyr Documentation
- https://docs.zephyrproject.org/latest/boards/index.html
- https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html

### STM32H7 Reference
- STM32H753 Reference Manual (RM0433)
- STM32H753VI Datasheet (check LQFP100 pinout)
- AN5361: Getting started with STM32H7

### Common Issues
1. **Garbage UART**: HSE clock frequency wrong
2. **No UART at all**: Wrong pins or UART not enabled
3. **Hard fault on boot**: Stack overflow, check CONFIG_MAIN_STACK_SIZE
4. **CAN not working**: Transceiver not powered or wrong pins
5. **GPIO not responding**: Pin doesn't exist on LQFP100 package

## Next Steps After Successful Bring-Up

1. Create hardware-in-loop (HIL) test bench
2. Integrate with vehicle CAN network
3. Add sensor calibration procedures
4. Implement telemetry/logging
5. Add OTA update capability
6. Vehicle integration testing

---

**Note:** Keep the Nucleo board as a known-good reference platform during migration. If something doesn't work on the custom board, test the same code on Nucleo first to isolate hardware vs. software issues.
