#include "delay.h"
#include <REGX52.H>

// ����״̬����
enum KeyState { 
    KEY_IDLE,        // ����״̬
    KEY_DETECTED,    // ��⵽�������£��ȴ�������
    KEY_CONFIRMED,   // ������ȷ�ϣ�׼�����ؼ�ֵ��
    KEY_WAIT_RELEASE // �ȴ������ͷ�
};

// ������ʽ����ɨ�躯��
unsigned char keypad_scan_nonblocking(void) {
    // ��̬��������״̬���ڶ�ε��ü䱣��ֵ��
    static enum KeyState state = KEY_IDLE;
    static unsigned char key = 0;
    static unsigned char debounce_count = 0;
    static unsigned char current_row = 0;
    static unsigned char current_col = 0;
    
    // ��������ӳ��
    unsigned char rows[] = {7, 6, 5, 4}; // P1.7-P1.4
    unsigned char cols[] = {3, 2, 1, 0}; // P1.3-P1.0
    
    switch (state) {
        case KEY_IDLE:
            // ɨ����һ��
            P1 = 0xFF; // ����������
            P1 &= ~(1 << rows[current_row]); // ���õ�ǰ��Ϊ�͵�ƽ
            
            // ɨ��������
            for (current_col = 0; current_col < 4; current_col++) {
                // ����������Ƿ�͵�ƽ���������£�
                if (!(P1 & (1 << cols[current_col]))) {
                    key = current_row * 4 + current_col + 1; // �����ֵ
                    debounce_count = 0; // ��������������
                    state = KEY_DETECTED; // ��������״̬
                    return 0; // �����ް�������
                }
            }
            
            // �ƶ�����һ��
            current_row = (current_row + 1) % 4;
            return 0; // �����ް�������
            
        case KEY_DETECTED:
            // ��������������
            debounce_count++;
            
            // ����Ƿ�ﵽ����ʱ�䣨����ÿ10ms����һ�Σ�
            if (debounce_count >= 2) { // 20ms����ʱ��
                // ���¼�ⰴ��״̬
                P1 = 0xFF;
                P1 &= ~(1 << rows[current_row]);
                
                // ȷ�ϰ�����Ȼ����
                if (!(P1 & (1 << cols[current_col]))) {
                    state = KEY_CONFIRMED; // ����ȷ��״̬
                } else {
                    state = KEY_IDLE; // ���������ؿ���״̬
                }
            }
            return 0; // �����ް�������
            
        case KEY_CONFIRMED:
            state = KEY_WAIT_RELEASE; // ����ȴ��ͷ�״̬
            return key; // ���ؼ�⵽�ļ�ֵ
            
        case KEY_WAIT_RELEASE:
            // ������ⰴ���Ƿ��ͷ�
            P1 = 0xFF;
            P1 &= ~(1 << rows[current_row]);
            
            // ����������Ƿ�ָ��ߵ�ƽ
            if (P1 & (1 << cols[current_col])) {
                state = KEY_IDLE; // �������ͷ�
            }
            return 0; // �����ް�������
    }
    
    return 0; // Ĭ�Ϸ����ް���
}