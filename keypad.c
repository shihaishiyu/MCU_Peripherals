#include "delay.h"
#include <REGX52.H>

// 按键状态定义
enum KeyState { 
    KEY_IDLE,        // 空闲状态
    KEY_DETECTED,    // 检测到按键按下（等待消抖）
    KEY_CONFIRMED,   // 按键已确认（准备返回键值）
    KEY_WAIT_RELEASE // 等待按键释放
};

// 非阻塞式按键扫描函数
unsigned char keypad_scan_nonblocking(void) {
    // 静态变量保持状态（在多次调用间保持值）
    static enum KeyState state = KEY_IDLE;
    static unsigned char key = 0;
    static unsigned char debounce_count = 0;
    static unsigned char current_row = 0;
    static unsigned char current_col = 0;
    
    // 行列引脚映射
    unsigned char rows[] = {7, 6, 5, 4}; // P1.7-P1.4
    unsigned char cols[] = {3, 2, 1, 0}; // P1.3-P1.0
    
    switch (state) {
        case KEY_IDLE:
            // 扫描下一行
            P1 = 0xFF; // 重置所有行
            P1 &= ~(1 << rows[current_row]); // 设置当前行为低电平
            
            // 扫描所有列
            for (current_col = 0; current_col < 4; current_col++) {
                // 检测列引脚是否低电平（按键按下）
                if (!(P1 & (1 << cols[current_col]))) {
                    key = current_row * 4 + current_col + 1; // 计算键值
                    debounce_count = 0; // 重置消抖计数器
                    state = KEY_DETECTED; // 进入消抖状态
                    return 0; // 本次无按键返回
                }
            }
            
            // 移动到下一行
            current_row = (current_row + 1) % 4;
            return 0; // 本次无按键返回
            
        case KEY_DETECTED:
            // 消抖计数器增加
            debounce_count++;
            
            // 检查是否达到消抖时间（假设每10ms调用一次）
            if (debounce_count >= 2) { // 20ms消抖时间
                // 重新检测按键状态
                P1 = 0xFF;
                P1 &= ~(1 << rows[current_row]);
                
                // 确认按键仍然按下
                if (!(P1 & (1 << cols[current_col]))) {
                    state = KEY_CONFIRMED; // 进入确认状态
                } else {
                    state = KEY_IDLE; // 抖动，返回空闲状态
                }
            }
            return 0; // 本次无按键返回
            
        case KEY_CONFIRMED:
            state = KEY_WAIT_RELEASE; // 进入等待释放状态
            return key; // 返回检测到的键值
            
        case KEY_WAIT_RELEASE:
            // 持续检测按键是否释放
            P1 = 0xFF;
            P1 &= ~(1 << rows[current_row]);
            
            // 检查列引脚是否恢复高电平
            if (P1 & (1 << cols[current_col])) {
                state = KEY_IDLE; // 按键已释放
            }
            return 0; // 本次无按键返回
    }
    
    return 0; // 默认返回无按键
}