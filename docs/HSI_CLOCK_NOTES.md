# ⚠️ IMPORTANT: HSI Clock Configuration

## Your Board Uses Internal HSI Oscillator

Your VCU board is configured to use the **internal 64 MHz HSI oscillator** instead of an external crystal. This is a key design decision with important implications.

## What This Means

### ✅ Advantages
- **No external crystal needed** - lower BOM cost
- **Fewer components** - no crystal, load caps, or PCB traces to oscillator pins
- **More robust** - no crystal startup issues or mechanical vibration concerns
- **Simpler layout** - no critical analog routing for oscillator

### ⚠️ Limitations
- **~1% accuracy** - HSI has approximately ±1% frequency tolerance across temperature and voltage
- **Temperature drift** - frequency varies slightly with chip temperature
- **Not suitable for USB** - USB requires precise clock (use external crystal for USB)
- **Less precise than crystal** - external HSE typically has ±50 ppm accuracy

## What Works Fine with HSI

✅ **CAN Bus** (up to 1 Mbps) - 1% tolerance is well within CAN spec  
✅ **UART** (up to 115200+) - no garbage characters expected  
✅ **SPI/I2C** - master mode works perfectly  
✅ **Timers/PWM** - 1% accuracy sufficient for most applications  
✅ **ADC sampling** - timing not critical  
✅ **General GPIO** - no timing constraints  

## What Doesn't Work with HSI

❌ **USB** - requires <0.25% clock accuracy (need external crystal)  
⚠️ **High-speed CAN-FD** (>2 Mbps) - may need crystal for best reliability  
⚠️ **Precision timing** - if you need <1% timing accuracy, use crystal  

## Your Configuration Details

```c
Clock Source: HSI (64 MHz internal RC oscillator)
PLL Input: 64 MHz / 8 = 8 MHz
VCO Freq: 8 MHz * 240 = 1920 MHz
System Clock: 1920 MHz / 4 = 480 MHz
Peripheral Clocks: Derived from 480 MHz system clock
```

### Actual Clock Tree
```
HSI (64 MHz) 
  └─> /8 → 8 MHz 
      └─> PLL (×240) → 1920 MHz VCO
          ├─> /4 → 480 MHz (System Clock)
          ├─> /8 → 240 MHz (PLL_Q for peripherals)
          └─> /4 → 480 MHz (PLL_R)
```

## Verification on First Boot

When you flash and boot, check the serial console:

```
System Clock: 480000000 Hz   ← Should be exactly 480 MHz
```

If you see this, your HSI is configured correctly!

## Common Questions

### Q: Will my UART work reliably?
**A:** Yes! 1% accuracy is more than enough for standard baud rates. You won't see garbage characters.

### Q: What about CAN bus reliability?
**A:** CAN spec allows for ±1.58% sampling point tolerance. HSI's ~1% is well within spec for standard CAN (125k-1M bps).

### Q: Can I add USB later?
**A:** You would need to:
1. Add external crystal (8 or 25 MHz typical)
2. Modify `vcu_stm32.dts` to use HSE instead of HSI
3. Reconfigure PLL to use HSE as source
4. Populate crystal load capacitors on PCB

### Q: Should I be worried about temperature drift?
**A:** For automotive/racing environments (0-85°C), HSI typically drifts less than 1%. Your UART, CAN, and peripherals will work fine.

## If You Need to Switch to External Crystal Later

Edit `/boards/st/vcu_stm32/vcu_stm32.dts`:

```dts
// Remove or comment out HSI:
// &clk_hsi {
//     status = "okay";
//     hsi-div = <1>;
// };

// Add HSE crystal:
&clk_hse {
    clock-frequency = <DT_FREQ_M(25)>;  // or 8 MHz
    status = "okay";
};

// Update PLL to use HSE:
&pll {
    div-m = <5>;     // For 25 MHz HSE: 25/5 = 5 MHz
    mul-n = <192>;   // 5 * 192 = 960 MHz
    div-p = <2>;     // 960/2 = 480 MHz
    div-q = <4>;
    div-r = <2>;
    clocks = <&clk_hse>;  // Change from clk_hsi to clk_hse
    status = "okay";
};
```

Then rebuild:
```bash
west build -b vcu_stm32 --pristine
west flash
```

## Bottom Line

**Your current HSI configuration is perfectly fine for a VCU!** All your peripherals (CAN, UART, ADC, GPIO, timers) will work reliably. The 1% accuracy is well within automotive specifications.

Don't worry about clock precision unless you specifically need:
- USB device/host functionality
- Sub-millisecond timing precision
- External synchronization requirements

For Formula SAE EV VCU applications, **HSI is an excellent choice** that simplifies your design and reduces cost.

---

**TL;DR**: Your board uses internal HSI. This is good! Everything will work. No external crystal needed. 🎉
