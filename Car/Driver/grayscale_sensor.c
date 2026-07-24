#include "grayscale_sensor.h"
#include "ti_msp_dl_config.h"

uint8_t g_sensor_raw_data = 0xFF;   // 原始数据: 0=黑, 1=白
int16_t cx = 35;
uint8_t Flag        = 0;            // 丢线标志: 0=正常, 1=丢线
uint8_t Status      = 0xFF;         // 当前八路循迹数值 (实时)
uint8_t Last_Status = 0xFF;         // 丢线前最后一帧有效值 (仅在正常巡线时更新)

void Grayscale_Sensor_Init(void)
{
    PL_HIGH();
    SCK_HIGH();

    // PB13 (SDA) → 输入+上拉
    DL_GPIO_initDigitalInputFeatures(GPIO_Sensor_PIN_SDA_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
}

// 74HC165 读取 8 通道
static uint8_t Grayscale_Read_All(void)
{
    uint8_t data = 0;

    PL_LOW();
    __asm volatile ("nop");
    PL_HIGH();
    __asm volatile ("nop");

    if (SDA_READ()) data |= (1 << 7);

    for (uint8_t i = 0; i < 7; i++) {
        SCK_HIGH();
        __asm volatile ("nop");
        SCK_LOW();
        __asm volatile ("nop");
        if (SDA_READ()) data |= (1 << (6 - i));
    }

    return data;
}

// data → cx
// data 中 bit=0 表示黑线, bit=1 表示白
// 物理: 通道0~3=右, 通道4~7=左
// 权重: 右→大(右转), 左→小(左转)
// 通道 i: 权重 = (7-i)*10
static uint16_t Grayscale_Data_To_CX(uint8_t data)
{
    uint16_t sum = 0;
    uint8_t count = 0;

    for (uint8_t i = 0; i < 8; i++) {
        if (!(data & (1 << i))) {       // bit=0 → 黑线
            sum += (uint16_t)((7 - i) * 10);
            count++;
        }
    }

    if (count == 0) return 255;  // 全白(0xFF) → 丢线

    return (uint16_t)((sum + (count / 2)) / count);
}

uint8_t Grayscale_Update(void)
{
    g_sensor_raw_data = Grayscale_Read_All();
    Status = g_sensor_raw_data;          // 实时更新当前值

    if (g_sensor_raw_data == 0xFF) {
        cx   = 35;  // 全白 → 丢线
        Flag = 0;
        // 丢线时不更新 Last_Status，保留丢线前最后一帧有效值
    } else if (g_sensor_raw_data == 0x00) {
        cx   = 35;  // 全黑 → 静止
        Flag = 1;
        Last_Status = Status;
    } else {
        cx   = Grayscale_Data_To_CX(g_sensor_raw_data);
        Flag = 2;
        Last_Status = Status;
    }
    return Flag;
}
