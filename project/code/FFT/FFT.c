#include "FFT.h"

#pragma section all "cpu1_dsram"
uint16 adc_sample_buffer[ADC_SAMPLE_SIZE] = {0};  // ADC采样缓冲区
uint16 adc_sample_index = 0;                      // 当前采样索引
uint8  send_flag = 0;                             // 采集完成标志
float  main_frequency = 10000;                        //频率
float  vpp = 1;                                   //峰峰值
uint16 wave_type = 0;

float  main_frequency_protect_max = 15000; // 保护频率最大值      
float  main_frequency_protect_min = 8000;    // 保护频率最小值

float   vpp_protect_max = 3.3; // 保护幅值最大值
float   vpp_protect_min = -0.1; // 保护幅值最小值

float fundamental_amp = 0; // 基波幅值
float second_harmonic = 0;    // 二次谐波
float third_harmonic  = 0;     // 三次谐波
float fourth_harmonic  = 0;     // 四次谐波
float fifth_harmonic  = 0;     // 四次谐波

// 查找谐波幅值 (允许±5%的频率偏差)
float harmonic2_range_min = 0;
float harmonic2_range_max = 0;
float harmonic3_range_min = 0;
float harmonic3_range_max = 0;
float harmonic5_range_max = 0;
float harmonic5_range_min = 0;

float second_ratio = 0; // 二次谐波幅值比
float third_ratio  = 0; // 三次谐波幅值比
float fourth_ratio  = 0; // 四次谐波幅值比
float fifth_ratio  = 0; // 四次谐波幅值比

#pragma section all restore

#pragma section all "cpu1_psram"
// 计算 DFT 中第 k 项的实部
double realComput(uint16 xn[], int ndft, int k) {
    double realPart = 0;
    for (int i = 0; i < ndft; ++i) {
        realPart += xn[i] * cos(2 * PI / ndft * k * i); // 余弦项
    }
    return realPart;
}

// 计算 DFT 中第 k 项的虚部
double imageComput(uint16 xn[], int ndft, int k) {
    double imagePart = 0;
    for (int i = 0; i < ndft; ++i) {
        imagePart -= xn[i] * sin(2 * PI / ndft * k * i); // 负号是因为 DFT 定义中为负指数
    }
    return imagePart;
}

// 离散傅里叶变换函数
Complex* dft(uint16 x[], int ndft) {
    // 动态分配内存保存 DFT 结果（复数数组）
    Complex* dftRes = (Complex*)malloc(ndft * sizeof(Complex));
    if (dftRes == NULL) {
        zf_assert(0);
        return 0; // 分配失败返回空
    }

    // 对每个频率分量 k，计算其对应的实部与虚部
    for (int i = 0; i < ndft; ++i) {
        dftRes[i].real = realComput(x, ndft, i);
        dftRes[i].image = imageComput(x, ndft, i);
        // 可选调试输出：
        // printf("%lf + %lfi\n", dftRes[i].real, dftRes[i].image);
    }
    return dftRes;
}

// 计算幅度谱（每个频率成分的模长）
double* ampSpectrum(Complex* dftRes, int ndft) {
    double* amp = (double*)malloc(sizeof(double) * ndft);
    if (amp == NULL) {
        zf_assert(0);
        return 0; // 分配失败
    }
    for (int i = 0; i < ndft; ++i) {
        amp[i] = sqrt(dftRes[i].real * dftRes[i].real + dftRes[i].image * dftRes[i].image);
    }
    return amp;
}

// 计算相位谱（每个频率成分的相位角）
double* phaseSpectrum(Complex* dftRes, int ndft) {
    double* phase = (double*)malloc(sizeof(double) * ndft);
    if (phase == NULL) {
        zf_assert(0);
        return 0; // 分配失败
    }
    for (int i = 0; i < ndft; ++i) {
        phase[i] = atan2(dftRes[i].image, dftRes[i].real); // 使用 atan2 保证相位角正确
    }
    return phase;
}

void Test(void)
{
    send_flag = 1;
    Complex* dftRes = dft(adc_sample_buffer, ADC_SAMPLE_SIZE);

    // 计算幅度谱与相位谱
    double* ampRes = ampSpectrum(dftRes, ADC_SAMPLE_SIZE);
    double* phaRes = phaseSpectrum(dftRes, ADC_SAMPLE_SIZE);

    // printf("复数结果\r\n");
    // // 输出 DFT 的复数结果
    // for (int i = 0; i < ADC_SAMPLE_SIZE; ++i) {
    //     printf("%lf + %lfi\r\n", dftRes[i].real, dftRes[i].image);
    // }
    // printf("\r\n");

    printf("幅度谱:\r\n");
    // 输出幅度谱
    for (uint16 i = 0; i < ADC_SAMPLE_SIZE/2; ++i) {
        printf("%lf, %d\r\n", ampRes[i], i);
    }
    printf("\n");

    // printf("相位谱:\r\n");
    // // 输出相位谱
    // for (int i = 0; i < ADC_SAMPLE_SIZE; ++i) {
    //     printf("%lf\r\n", phaRes[i]);
    // }
    // printf("\n");

    // 释放动态分配的内存
    free(dftRes);
    free(ampRes);
    free(phaRes);
    send_flag = 0;
}

void DFT_Measure(void)
{
    send_flag = 1;
//    if(send_flag == 1)
    {
        Complex* dftRes = dft(adc_sample_buffer, ADC_SAMPLE_SIZE);
        double* ampRes = ampSpectrum(dftRes, ADC_SAMPLE_SIZE);
        
        printf("幅度谱:\r\n");
        // 输出幅度谱
        for (uint16 i = 0; i < ADC_SAMPLE_SIZE/2; ++i) {
            printf("%lf, %d\r\n", ampRes[i], i);
        }
        printf("\n");

//        for (uint16 i = 0; i < ADC_SAMPLE_SIZE; ++i) {
//            printf("%d, %d\r\n", adc_sample_buffer[i], i);
//        }
//        printf("\n");


        // 寻找最大幅值（跳过直流分量）
        double max_amp = 0;
        uint16 max_idx = 0;
        
        for (uint16 i = 1; i < ADC_SAMPLE_SIZE/2; i++) {
            if (ampRes[i] > max_amp) {
                max_amp = ampRes[i];
                max_idx = i;
            }
        }
        
        // 计算主频率
        main_frequency = max_idx * 1.0 * (SAMPLING_FREQ / ADC_SAMPLE_SIZE);
        
        // ====== 波形识别逻辑 ======
        // 1. 计算基波和谐波幅值比例
        fundamental_amp = ampRes[max_idx]; // 基波幅值
        second_harmonic = 0;    // 二次谐波
        third_harmonic = 0;     // 三次谐波
        fifth_harmonic = 0;
        
        // 查找谐波幅值 (允许±5%的频率偏差)
        harmonic2_range_min = (max_idx * 2) - (max_idx / 10.0);
        harmonic2_range_max = (max_idx * 2) + (max_idx / 10.0);
        harmonic3_range_min = (max_idx * 3) - (max_idx / 10.0);
        harmonic3_range_max = (max_idx * 3) + (max_idx / 10.0);
        harmonic5_range_min = (max_idx * 5) - (max_idx / 10.0);
        harmonic5_range_max = (max_idx * 5) + (max_idx / 10.0);
        
        // 寻找二次谐波的最大幅值
        for (uint16 i = harmonic2_range_min; i <= harmonic2_range_max && i < ADC_SAMPLE_SIZE/2; i++) {
            if (ampRes[i] > second_harmonic) {
                second_harmonic = ampRes[i];
            }
        }
        
        // 寻找三次谐波的最大幅值
        for (uint16 i = harmonic3_range_min; i <= harmonic3_range_max && i < ADC_SAMPLE_SIZE/2; i++) {
            if (ampRes[i] > third_harmonic) {
                third_harmonic = ampRes[i];
            }
        }

        // 寻找四次谐波的最大幅值
        for (uint16 i = harmonic5_range_min; i <= harmonic5_range_max && i < ADC_SAMPLE_SIZE/2; i++) {
            if (ampRes[i] > fifth_harmonic) {
                fifth_harmonic = ampRes[i];
            }
        }
        
        // 计算谐波比
        second_ratio = second_harmonic / fundamental_amp;  // 二次谐波比
        third_ratio  = third_harmonic  / fundamental_amp;    // 三次谐波比
        fifth_ratio  = fifth_harmonic  / fundamental_amp;    // 五次谐波比
        
        if(third_ratio > 0.3 && fifth_ratio > 0.15)
        {
            wave_type = SQUARE_WAVE;
        }

        else if(third_ratio > 0.08 && fifth_ratio < 0.2)
        {
            wave_type = TRIANGLE_WAVE;
        }
        else if(third_ratio < 0.2 && fifth_ratio < 0.2)
        {
            wave_type = SINE_WAVE;
        }
        else
        {
            wave_type = UNKNOWN_WAVE;
            printf("三次:%f, 五次:%f\r\n", third_ratio, fifth_ratio);
        }

        printf("%f, %f, %f, %f\r\n", harmonic5_range_max, harmonic5_range_min, harmonic3_range_max, harmonic3_range_min);
        printf("%f, %f, %f, %f\r\n", third_harmonic, fifth_harmonic, third_ratio, fifth_ratio);

        // 释放内存
        free(dftRes);
        free(ampRes);
        
        send_flag = 0;
    }
}

void Sys_Protect_Func(void)
{
    if(main_frequency >= main_frequency_protect_max || main_frequency <= main_frequency_protect_min
    || vpp >= vpp_protect_max || vpp <= vpp_protect_min)
    {
        Beep_Start();
    }
}

uint16 temp_adc_sample_buffer[ADC_SAMPLE_SIZE];
void Vpp_Cal(void)
{
    memcpy(temp_adc_sample_buffer, adc_sample_buffer, sizeof(adc_sample_buffer));
    static float last_vpp = 0, a = 0.99;
    uint16 vpp_max = 0, vpp_min = 0;
    // 计算Vpp (用于保护功能)
    for(uint16 i = 0; i < ADC_SAMPLE_SIZE; i++)
    {
        if(temp_adc_sample_buffer[i] < vpp_min)
            vpp_min = adc_sample_buffer[i];
        if(temp_adc_sample_buffer[i] > vpp_max)
            vpp_max = adc_sample_buffer[i];
    }

    vpp = a * last_vpp + (1-a) * last_vpp;
    Sliding_Filter_AddData(&vpp_filter, vpp);
    vpp = Sliding_Filter_GetAverage(&vpp_filter);
    last_vpp = vpp;
    vpp = (float)(vpp_max + vpp_min) / 4096.0 *3.3;
}



#pragma section all restore
