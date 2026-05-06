
#include "my_gui.h"
#include "lvgl.h"
#include "bsp_usart1.h"
#include "ds18b20.h"  
#include "spi.h"
#include "24l01.h"
#include "bsp_esp8266.h"
#include "stdlib.h"
#include "stdio.h" 

// 外部全局变量（假设在主程序中定义）
extern uint16_t adc_value;

/* 屏幕对象 */
lv_obj_t* screen1;      // 第一个屏幕（仪表盘界面）
lv_obj_t* screen2;      // 第二个屏幕（图表界面）

/* 第一个屏幕上的控件 */
lv_obj_t* btn1;         // START按钮
lv_obj_t* btn2;         // STOP按钮
lv_obj_t* label1;       // START按钮标签
lv_obj_t* label2;       // STOP按钮标签
lv_obj_t* btn_go;       // 切换到屏幕2的按钮

lv_obj_t* meter1;       // 电压仪表盘
lv_meter_scale_t* scale1;
lv_meter_indicator_t* indic1;

lv_obj_t* meter2;       // 温度仪表盘
lv_meter_scale_t* scale2;
lv_meter_indicator_t* indic2;

/* 第二个屏幕上的控件 */
lv_obj_t* btn_back;     // 返回屏幕1的按钮
lv_obj_t* label_back;   // Back按钮标签
lv_obj_t* chart;        // 图表控件
lv_chart_series_t* temp_ser;   // 温度序列（红色）
lv_chart_series_t* hum_ser;    // 电压序列（蓝色）
lv_obj_t* chart_title;          // 图表标题

lv_timer_t* timer1;     // 定时器，用于周期性读取数据并更新界面
static uint8_t sample_cnt = 0;  // 采样计数（本例未使用，可扩展）

/* 函数声明 */
static void my_timer_cb(lv_timer_t* timer);
static void btn_event_cb(lv_event_t* e);
static void reset_chart_data(void);

/* 清空图表中的所有数据点 */
static void reset_chart_data(void)
{
    if (chart == NULL) return;
    uint16_t point_cnt = lv_chart_get_point_count(chart);
    for (uint16_t i = 0; i < point_cnt; i++) {
        lv_chart_set_next_value(chart, temp_ser, 0);  // 温度清0
        lv_chart_set_next_value(chart, hum_ser, 0);   // 电压清0
    }
    sample_cnt = 0;
}

/* 定时器回调函数：读取无线数据，更新仪表盘和图表 */
static void my_timer_cb(lv_timer_t* timer)
{
    static int receive_data = 0;
    static char receive_buf[33] = { '\0' };
		static char wifi_buf[33] = { '\0' };
    static short temperature = 0;
    static uint16_t adc_raw = 0;      // ADC原始值 (0-4095)
    static int16_t voltage_val = 0;   // 电压值放大10倍后的整数 (0~33)
    static int16_t temp_val = 0;      // 温度值 (摄氏度)

    // 1. 通过24L01接收数据
    if (NRF24L01_RxPacket(receive_buf) == 0)
    {
        printf("receive ok!!!\r\n");
        printf("raw data: %s\r\n", receive_buf);
    }

    // 假设收到的字符串格式为 "ADC温度"，例如 "1234025"
    // 其中前四位或三位是ADC值（除以1000），后三位是温度
    receive_data = atoi(receive_buf);
    adc_raw = receive_data / 1000;          // ADC原始值 (0-4095)
    temperature = receive_data % 1000;      // DS18B20温度 (整数摄氏度)

    // 2. 更新仪表盘
    // 仪表盘1：电压，范围0~400，实际电压值 = adc_raw/4095*3.3，再乘以100得到0-330之间的值
    lv_meter_set_indicator_value(meter1, indic1, (int32_t)(((float)adc_raw / 4095 * 3.3) * 100));
    // 仪表盘2：温度，范围0~100
    lv_meter_set_indicator_value(meter2, indic2, (int32_t)(temperature));
    
    printf("adc_value=%d\tvoltage=%.2fV  temperature=%d°C\r\n", adc_raw, (float)adc_raw / 4095 * 3.3, temperature);

    // 3. 计算图表用的数值
    voltage_val = (int16_t)(((float)adc_raw / 4095 * 3.3) * 10);   // 电压值放大10倍，范围0~33
    temp_val = temperature;                                        // 温度值，范围-55~125

    printf("chart values: voltage_val=%d, temp_val=%d\r\n", voltage_val, temp_val);
		
				sprintf(wifi_buf, "ADC=%d    T=%d\r\n", adc_raw, temperature);
        ESP8266_SendString(ENABLE, wifi_buf, 0, Single_ID_0);
        printf("WiFi forwarded: %s", wifi_buf);

    // 4. 更新图表（如果图表已创建且序列有效）
    if (chart != NULL && temp_ser != NULL && hum_ser != NULL)
    {
        lv_chart_set_next_value(chart, hum_ser, voltage_val);   // 电压序列（蓝色）
        lv_chart_set_next_value(chart, temp_ser, temp_val);     // 温度序列（红色）
        lv_chart_refresh(chart);  // 强制刷新图表
    }
}

/* 所有按钮的事件回调函数 */
static void btn_event_cb(lv_event_t* e)
{
    lv_obj_t* target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code != LV_EVENT_RELEASED) return;  // 只处理释放事件

    if (target == btn1) {
        /* START按钮：清空仪表盘和图表，然后恢复定时器 */
        lv_meter_set_indicator_value(meter1, indic1, 0);
        lv_meter_set_indicator_value(meter2, indic2, 0);
        reset_chart_data();          // 清空图表历史数据
        lv_timer_resume(timer1);     // 启动定时器，开始接收数据
    }
    else if (target == btn2) {
        /* STOP按钮：暂停定时器，并将仪表盘归零（图表数据保留） */
        lv_timer_pause(timer1);
        lv_meter_set_indicator_value(meter1, indic1, 0);
        lv_meter_set_indicator_value(meter2, indic2, 0);
        // 注意：不清空图表，以便观察停止前的最后数据
    }
    else if (target == btn_go) {
        /* Go按钮：切换到第二个屏幕（图表界面） */
        lv_scr_load(screen2);
    }
    else if (target == btn_back) {
        /* Back按钮：返回第一个屏幕（仪表盘界面） */
        lv_scr_load(screen1);
    }
}

/* 创建第一个屏幕：包含电压/温度仪表盘、START/STOP/Go按钮 */
static void create_screen1(void)
{
    /* 获取当前活动屏幕作为 screen1 */
    screen1 = lv_scr_act();

    /* ----- START按钮 ----- */
    btn1 = lv_btn_create(screen1);
    lv_obj_set_size(btn1, 100, 50);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, -100, 0);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_RELEASED, NULL);

    /* ----- STOP按钮 ----- */
    btn2 = lv_btn_create(screen1);
    lv_obj_set_size(btn2, 100, 50);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 100, 0);
    lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_RELEASED, NULL);

    /* START按钮标签 */
    label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "START");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label1, lv_color_hex(0xFF318C), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    /* STOP按钮标签 */
    label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "STOP");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label2, lv_color_hex(0xFF318C), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    /* ----- 切换到屏幕2的Go按钮 ----- */
    btn_go = lv_btn_create(screen1);
    lv_obj_set_size(btn_go, 80, 40);
    lv_obj_align(btn_go, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_add_event_cb(btn_go, btn_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_t* label_go = lv_label_create(btn_go);
    lv_label_set_text(label_go, "Go");
    lv_obj_align(label_go, LV_ALIGN_CENTER, 0, 0);

    /* ----- 电压仪表盘（左侧）----- */
    meter1 = lv_meter_create(screen1);
    lv_obj_align(meter1, LV_ALIGN_TOP_LEFT, 0, 60);
    lv_obj_set_size(meter1, 140, 140);
    lv_obj_set_style_bg_color(meter1, lv_color_hex(0xFFFF00), LV_STATE_DEFAULT);  // 黄色背景

    scale1 = lv_meter_add_scale(meter1);
    lv_meter_set_scale_ticks(meter1, scale1, 41, 1, 8, lv_color_hex(0x3291EA));   // 小刻度
    scale1->r_mod = 9;
    lv_meter_set_scale_major_ticks(meter1, scale1, 5, 2, 12, lv_color_hex(0xCC3637), 5); // 大刻度

    indic1 = lv_meter_add_needle_line(meter1, scale1, 4, lv_color_hex(0x5EDB50), -15); // 指针

    // 电压范围0~400（对应0~330mV? 实际显示电压*100后的值，最大3.3V对应330）
    lv_meter_set_scale_range(meter1, scale1, 0, 400, 270, 135);
    lv_meter_set_indicator_value(meter1, indic1, 0);

    /* ----- 温度仪表盘（右侧）----- */
    meter2 = lv_meter_create(screen1);
    lv_obj_align(meter2, LV_ALIGN_TOP_RIGHT, 0, 60);
    lv_obj_set_size(meter2, 140, 140);
    lv_obj_set_style_bg_color(meter2, lv_color_hex(0xFFFF00), LV_STATE_DEFAULT);

    scale2 = lv_meter_add_scale(meter2);
    lv_meter_set_scale_ticks(meter2, scale2, 41, 1, 8, lv_color_hex(0x3291EA));
    scale2->r_mod = 9;
    lv_meter_set_scale_major_ticks(meter2, scale2, 5, 2, 12, lv_color_hex(0xCC3637), 5);

    indic2 = lv_meter_add_needle_line(meter2, scale2, 4, lv_color_hex(0x5EDB50), -15);

    // 温度范围0~100摄氏度
    lv_meter_set_scale_range(meter2, scale2, 0, 400, 270, 135);
    lv_meter_set_indicator_value(meter2, indic2, 0);
}

/* 创建第二个屏幕：包含折线图（温度和电压）和Back按钮 */
static void create_screen2(void)
{
    screen2 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen2, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);  // 白色背景

    // 图表标题
    chart_title = lv_label_create(screen2);
    lv_label_set_text(chart_title, "Temperature (Red)   Voltage (Blue)");
    lv_obj_set_style_text_color(chart_title, lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_align(chart_title, LV_ALIGN_TOP_MID, 0, 10);

    // 创建图表控件
    chart = lv_chart_create(screen2);
    lv_obj_set_size(chart, 280, 180);
    lv_obj_align(chart, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(chart, lv_color_hex(0xE0E0E0), LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(chart, 2, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(chart, lv_color_hex(0x000000), LV_STATE_DEFAULT);

    // 设置图表类型、范围、数据点数
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);   // Y轴范围0~100（电压0~33和温度0~100都在内）
    lv_chart_set_point_count(chart, 60);                          // 显示最近60个点
    lv_chart_set_div_line_count(chart, 5, 6);                     // 辅助线

    // 添加两个数据序列：温度（红色），电压（蓝色）
    temp_ser = lv_chart_add_series(chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
    hum_ser = lv_chart_add_series(chart, lv_color_hex(0x0000FF), LV_CHART_AXIS_PRIMARY_Y);

    // 将图表所有历史数据初始化为0
    uint16_t point_cnt = lv_chart_get_point_count(chart);
    for (uint16_t i = 0; i < point_cnt; i++) {
        lv_chart_set_next_value(chart, temp_ser, 0);
        lv_chart_set_next_value(chart, hum_ser, 0);
    }

    // 返回按钮（Back）
    btn_back = lv_btn_create(screen2);
    lv_obj_set_size(btn_back, 100, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(btn_back, btn_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0xDDDDDD), LV_STATE_DEFAULT);

    label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0xFF318C), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_back, &lv_font_montserrat_14, LV_STATE_DEFAULT);
}

/* GUI主初始化函数（由外部调用） */
void my_gui(void)
{
    /* 创建第一个屏幕（仪表盘界面） */
    create_screen1();

    /* 创建第二个屏幕（图表界面），此时还未加载 */
    create_screen2();

    /* 创建定时器，周期50ms，启动时处于暂停状态，由START按钮恢复 */
    timer1 = lv_timer_create(my_timer_cb, 50, NULL);
    lv_timer_pause(timer1);   // 初始暂停，等待用户按下START
}
