#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "ti_msp_dl_config.h"

/*==================================================================
 *  行驶状态控制函数
 *==================================================================*/

/* 初始化所有控制用的 PID 实例（循迹/直行/转弯） */
void Motor_Control_Init(void);

/* 循线行驶：循迹环(PID steering) + 速度环(base PWM) */
void Control_Line_Track(void);

/* 直行：速度环 + 角度环（CY-Z 陀螺仪航向保持） */
void Control_Straight(void);
void Control_Straight_Reset(void);  // 复位首次进入标志（重新归零角度）

/* 直角转弯 */
void Control_Corner(void);

#endif
