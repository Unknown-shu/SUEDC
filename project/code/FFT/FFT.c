#include "FFT.h"

#pragma section all "cpu1_dsram"
uint16 adc_sample_buffer[ADC_SAMPLE_SIZE] = {0};
uint16 adc_sample_index = 0;
uint8  send_flag = 0;
float  main_frequency = 10000;
float  vpp = 1;
float  vpp_plus = 0;
uint16 wave_type = 3;

float  main_frequency_protect_max = 15000;
float  main_frequency_protect_min = 8000;

float vpp_protect_max = 3.3;
float vpp_protect_min = -0.1;

float fundamental_amp = 0;
float second_harmonic = 0;
float third_harmonic  = 0;
float fourth_harmonic = 0;
float fifth_harmonic  = 0;

float harmonic2_range_min = 0;
float harmonic2_range_max = 0;
float harmonic3_range_min = 0;
float harmonic3_range_max = 0;
float harmonic5_range_max = 0;
float harmonic5_range_min = 0;

float second_ratio = 0;
float third_ratio  = 0;
float fourth_ratio = 0;
float fifth_ratio  = 0;

float SQUARE_third_ratio_min = 0.3;
float SQUARE_fifth_ratio_min = 0.15;
float TRIANGLE_third_ratio_min = 0.08;
float TRIANGLE_fifth_ratio_max = 0.2;
float SINE_third_ratio_max = 0.2;
float SINE_fifth_ratio_max = 0.2;


#pragma section all restore

#pragma section all "cpu1_psram"
static Complex dftRes[ADC_SAMPLE_SIZE];
static double ampRes[ADC_SAMPLE_SIZE];
static double phaRes[ADC_SAMPLE_SIZE];

void dft(const uint16 x[], int ndft) {
    for (int k = 0; k < ndft; ++k) {
        dftRes[k].real = 0;
        dftRes[k].image = 0;
        for (int n = 0; n < ndft; ++n) {
            double angle = 2 * PI * k * n / ndft;
            dftRes[k].real += x[n] * cos(angle);
            dftRes[k].image -= x[n] * sin(angle);
        }
    }
}

void computeAmpPhase(int ndft) {
    for (int i = 0; i < ndft; ++i) {
        ampRes[i] = sqrt(dftRes[i].real * dftRes[i].real + dftRes[i].image * dftRes[i].image);
        phaRes[i] = atan2(dftRes[i].image, dftRes[i].real);
    }
}

void Test(void) {
    send_flag = 1;
    dft(adc_sample_buffer, ADC_SAMPLE_SIZE);
    computeAmpPhase(ADC_SAMPLE_SIZE);

    for (uint16 i = 0; i < ADC_SAMPLE_SIZE / 2; ++i) {
        printf("%lf, %d\r\n", ampRes[i], i);
    }
    printf("\n");
    send_flag = 0;
}

void DFT_Measure(void) {
    send_flag = 1;
    dft(adc_sample_buffer, ADC_SAMPLE_SIZE);
    computeAmpPhase(ADC_SAMPLE_SIZE);

    for (uint16 i = 0; i < ADC_SAMPLE_SIZE / 2; ++i) {
        printf("%lf, %d\r\n", ampRes[i], i);
    }
    printf("\n");

    double max_amp = 0;
    uint16 max_idx = 0;
    for (uint16 i = 1; i < ADC_SAMPLE_SIZE / 2; i++) {
        if (ampRes[i] > max_amp) {
            max_amp = ampRes[i];
            max_idx = i;
        }
    }

    main_frequency = max_idx * 1.0 * (SAMPLING_FREQ / ADC_SAMPLE_SIZE);
    fundamental_amp = ampRes[max_idx];
    second_harmonic = 0;
    third_harmonic = 0;
    fifth_harmonic = 0;

    harmonic2_range_min = (max_idx * 2) - (max_idx / 10.0);
    harmonic2_range_max = (max_idx * 2) + (max_idx / 10.0);
    harmonic3_range_min = (max_idx * 3) - (max_idx / 10.0);
    harmonic3_range_max = (max_idx * 3) + (max_idx / 10.0);
    harmonic5_range_min = (max_idx * 5) - (max_idx / 10.0);
    harmonic5_range_max = (max_idx * 5) + (max_idx / 10.0);

    for (uint16 i = harmonic2_range_min; i <= harmonic2_range_max && i < ADC_SAMPLE_SIZE / 2; i++)
    {
        if (ampRes[i] > second_harmonic) second_harmonic = ampRes[i];
    }
    for (uint16 i = harmonic3_range_min; i <= harmonic3_range_max && i < ADC_SAMPLE_SIZE / 2; i++)
    {
        if (ampRes[i] > third_harmonic) third_harmonic = ampRes[i];
    }
    for (uint16 i = harmonic5_range_min; i <= harmonic5_range_max && i < ADC_SAMPLE_SIZE / 2; i++)
    {
        if (ampRes[i] > fifth_harmonic) fifth_harmonic = ampRes[i];
    }

    second_ratio = second_harmonic / fundamental_amp;
    third_ratio = third_harmonic / fundamental_amp;
    fifth_ratio = fifth_harmonic / fundamental_amp;

    if (third_ratio > SQUARE_third_ratio_min && fifth_ratio > SQUARE_fifth_ratio_min)
    {
        wave_type = SQUARE_WAVE;
    }
    else if (third_ratio > TRIANGLE_third_ratio_min && fifth_ratio < TRIANGLE_fifth_ratio_max)
    {
        wave_type = TRIANGLE_WAVE;
    }
    else if (third_ratio < SINE_third_ratio_max && fifth_ratio < SINE_fifth_ratio_max)
    {
        wave_type = SINE_WAVE;
    }
    else
    {
        wave_type = UNKNOWN_WAVE;
    }

    printf("%f, %f, %f, %f\r\n", harmonic5_range_max, harmonic5_range_min, harmonic3_range_max, harmonic3_range_min);
    printf("%f, %f, %f, %f\r\n", third_harmonic, fifth_harmonic, third_ratio, fifth_ratio);
    send_flag = 0;
    Beep_Timer_ShortRing();
}

void Sys_Protect_Func(void) {
    if(main_frequency >= main_frequency_protect_max || main_frequency <= main_frequency_protect_min
    || vpp >= vpp_protect_max || vpp <= vpp_protect_min) {
        Beep_Start();
    }
}

uint16 temp_adc_sample_buffer[ADC_SAMPLE_SIZE];
void Vpp_Cal(void) {
    memcpy(temp_adc_sample_buffer, adc_sample_buffer, sizeof(adc_sample_buffer));
    static float last_vpp = 0, a = 0.99;
    uint16 vpp_max = 0, vpp_min = 0;

    for(uint16 i = 0; i < ADC_SAMPLE_SIZE; i++) {
        if(temp_adc_sample_buffer[i] < vpp_min)
            vpp_min = adc_sample_buffer[i];
        if(temp_adc_sample_buffer[i] > vpp_max)
            vpp_max = adc_sample_buffer[i];
    }

    vpp = a * last_vpp + (1-a) * last_vpp;
    Sliding_Filter_AddData(&vpp_filter, vpp);
    vpp = Sliding_Filter_GetAverage(&vpp_filter);
    last_vpp = vpp;
    vpp = (float)(vpp_max + vpp_min) / 4096.0 *3.3 + vpp_plus;
}

#pragma section all restore
