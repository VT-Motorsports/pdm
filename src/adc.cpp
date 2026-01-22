#include <zephyr-common.h>


// ADC configuration for A0 pin (PA3 - ADC12_INP15)
#define ADC_RESOLUTION 12
#define ADC_GAIN ADC_GAIN_1
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL 15  // INP15

LOG_MODULE_REGISTER(ADC);



static const struct device *adc_dev;
static struct adc_channel_cfg channel_cfg = {
    .gain = ADC_GAIN,
    .reference = ADC_REFERENCE,
    .acquisition_time = ADC_ACQUISITION_TIME,
    .channel_id = ADC_CHANNEL,
    .differential = 0
};

static int16_t sample_buffer[1];
static struct adc_sequence sequence = {
    .channels = BIT(ADC_CHANNEL),
    .buffer = sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution = ADC_RESOLUTION,
};

int init_adc(void) {
    int ret;
    
    adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc1));
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
        return -1;
    }
    
    ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret != 0) {
        LOG_ERR("Failed to setup ADC channel: %d", ret);
        return ret;
    }
    
    LOG_INF("ADC initialized successfully on channel %d", ADC_CHANNEL);
    return 0;
}

float read_adc_voltage(void) {
    int ret;
    
    ret = adc_read(adc_dev, &sequence);
    if (ret != 0) {
        LOG_ERR("ADC read failed: %d", ret);
        return -1.0f;
    }
    
    // Convert ADC value to voltage
    // STM32H7 ADC reference is typically 3.3V
    // 12-bit resolution = 4096 levels
    float voltage = (sample_buffer[0] * 3.3f) / 4096.0f;
    
    return voltage;
}

// Get voltage - simple interface that does everything internally
float get_voltage(void) {
    return read_adc_voltage();
}

// Get raw ADC value from last read
int16_t get_adc_raw_value(void) {
    return sample_buffer[0];
}


// Map voltage to current command (0-14 range)
// 0% pedal = 2.114V -> 0 amps
// 100% pedal = 1.588V -> 14 amps
int16_t get_pedal_current(void) {
    float voltage = get_voltage();
    
    if (voltage < 0) {
        return 0;  // Error reading ADC
    }
    
    const float voltage_0_percent = 2.114f;
    const float voltage_100_percent = 1.588f;

    static constexpr int max_current = 20; 
    static constexpr int min_current =  0; 
    
    // Clamp voltage to valid range
    if (voltage >= voltage_0_percent) {
        return min_current;  // Below 0% threshold
    }
    if (voltage <= voltage_100_percent) {
        return max_current;  // Above 100% threshold
    }
    
    // Linear mapping: as voltage decreases from 2.114 to 1.588, current increases from 0 to 14
    float pedal_percent = (voltage_0_percent - voltage) / (voltage_0_percent - voltage_100_percent);
    int16_t current = (int16_t)(pedal_percent * max_current);
    
    // Clamp to 0-14 range (safety)
    if (current < min_current) current = min_current;
    if (current > max_current) current = max_current;
    
    return current;
}