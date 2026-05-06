#include "my_gui.h"
#include "lvgl.h"
#include "usart.h"
#include "ds18b20.h"  
#include "spi.h"
#include "24l01.h" 

extern uint16_t adc_value;

// 第一个屏幕上的控件
lv_obj_t* btn1;      // START按钮
lv_obj_t* btn2;      // STOP按钮
lv_obj_t* label1;
lv_obj_t* label2;
lv_obj_t* meter1;
lv_meter_scale_t* scale1;
lv_meter_indicator_t* indic1;
lv_obj_t* meter2;
lv_meter_scale_t* scale2;
lv_meter_indicator_t* indic2;

lv_timer_t* timer1;  // ADC采集任务

// 屏幕切换相关控件
lv_obj_t* btn_next;      // 第一个屏幕上的Next按钮
lv_obj_t* scr1;          // 保存第一个屏幕的指针
lv_obj_t* scr2;          // 第二个屏幕
lv_obj_t* btn_back;      // 第二个屏幕上的Back按钮
lv_obj_t* chart2;        // 第二个屏幕上的图表
lv_chart_series_t* ser_adc;   // ADC电压数据系列（蓝色）
lv_chart_series_t* ser_temp;  // 温度数据系列（红色）

static void my_timer1_cb(lv_timer_t* timer)
{
    static char tmp_buf[33] = {'\0'};
    static short temperature = 0; 
    static uint16_t temp = 0;	
    static int32_t adc_display = 0;  // 仪表盘1显示值（电压*100）
    
    temp = adc_value;
    temperature = DS18B20_Get_Temp();
    
    // 计算仪表盘显示值（0~400对应0~4V）
    adc_display = (int32_t)(((float)temp / 4095 * 3.3) * 100);
    
    // 更新仪表盘
    lv_meter_set_indicator_value(meter1, indic1, adc_display);
    lv_meter_set_indicator_value(meter2, indic2, (int32_t)temperature);	
    
    printf("adc_value=%d\tvlot=%.2f  ds18b20=%d\r\n", temp, (float)temp / 4095 * 3.3, temperature);
    
    // 准备发送数据（ADC原始值+温度）
    sprintf(tmp_buf, "%d%d", temp, temperature);
    if (NRF24L01_TxPacket((uint8_t *)tmp_buf) == TX_OK)
    {
        printf("send ok!!!\r\n");
    }
    
    // 实时更新第二个屏幕的图表（两条曲线）
    if (ser_adc != NULL && ser_temp != NULL)
    {
        lv_chart_set_next_value(chart2, ser_adc, adc_display);   // 蓝色：ADC电压（0~400）
        lv_chart_set_next_value(chart2, ser_temp, temperature);  // 红色：温度（0~400）
    }
}

void btn_event_cb(lv_event_t* e)
{
    lv_obj_t* target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (target == btn1)
    {
        if (code == LV_EVENT_RELEASED)
        {
            lv_timer_resume(timer1);
        }
    }
    else if (target == btn2)
    {
        if (code == LV_EVENT_RELEASED)
        {
            lv_timer_pause(timer1);            
            lv_meter_set_indicator_value(meter1, indic1, 0);
            lv_meter_set_indicator_value(meter2, indic2, 0);
        }
    }
    else if (target == btn_next)   // Next按钮：切换到第二个屏幕
    {
        if (code == LV_EVENT_RELEASED)
        {
            lv_scr_load(scr2);
        }
    }
}

void back_btn_event_cb(lv_event_t* e)
{
    lv_obj_t* target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (target == btn_back && code == LV_EVENT_RELEASED)
    {
        lv_scr_load(scr1);
    }
}

void my_gui(void)
{
    // ========== 保存第一个屏幕的指针 ==========
    scr1 = lv_scr_act();

    // ========== 第一个屏幕上的所有控件 ==========
    btn1 = lv_btn_create(scr1);
    lv_obj_set_size(btn1, 100, 50);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_MID, -100, 0);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_RELEASED, NULL);

    btn2 = lv_btn_create(scr1);
    lv_obj_set_size(btn2, 100, 50);
    lv_obj_align(btn2, LV_ALIGN_BOTTOM_MID, 100, 0);
    lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_RELEASED, NULL);

    label1 = lv_label_create(btn1);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label1, "START");
    lv_obj_set_style_text_color(label1, lv_color_hex(0xFF318C), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    label2 = lv_label_create(btn2);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label2, "STOP");
    lv_obj_set_style_text_color(label2, lv_color_hex(0xFF318C), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_14, LV_STATE_DEFAULT);

    // 仪表盘1（ADC电压，显示范围0~400对应0~4V）
    meter1 = lv_meter_create(scr1);
    lv_obj_align(meter1, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(meter1, 120, 120);
    lv_obj_set_style_bg_color(meter1, lv_color_hex(0xFFFF00), LV_STATE_DEFAULT);
    scale1 = lv_meter_add_scale(meter1);
    lv_meter_set_scale_ticks(meter1, scale1, 41, 1, 8, lv_color_hex(0x3291EA));
    scale1->r_mod = 11;
    lv_meter_set_scale_major_ticks(meter1, scale1, 8, 2, 12, lv_color_hex(0xCC3637), 11);
    indic1 = lv_meter_add_needle_line(meter1, scale1, 4, lv_color_hex(0x5EDB50), -15);
    lv_meter_set_scale_range(meter1, scale1, 0, 400, 270, 135);
    lv_meter_set_indicator_value(meter1, indic1, 0);
		
    // 仪表盘2（温度）
    meter2 = lv_meter_create(scr1);
    lv_obj_align(meter2, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_size(meter2, 120, 120);
    lv_obj_set_style_bg_color(meter2, lv_color_hex(0xFFFF00), LV_STATE_DEFAULT);
    scale2 = lv_meter_add_scale(meter2);
    lv_meter_set_scale_ticks(meter2, scale2, 41, 1, 8, lv_color_hex(0x3291EA));
    scale2->r_mod = 11;
    lv_meter_set_scale_major_ticks(meter2, scale2, 8, 2, 12, lv_color_hex(0xCC3637), 11);
    indic2 = lv_meter_add_needle_line(meter2, scale2, 4, lv_color_hex(0x5EDB50), -15);
    lv_meter_set_scale_range(meter2, scale2, 0, 400, 270, 135);
    lv_meter_set_indicator_value(meter2, indic2, 0);
		
    // Next按钮（切换屏幕）
    btn_next = lv_btn_create(scr1);
    lv_obj_set_size(btn_next, 100, 50);
    lv_obj_align(btn_next, LV_ALIGN_BOTTOM_MID, 0, -80);
    lv_obj_add_event_cb(btn_next, btn_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_t* label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, "Next");
    lv_obj_align(label_next, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_next, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);

    // 定时器（默认暂停）
    timer1 = lv_timer_create(my_timer1_cb, 50, NULL);
    lv_timer_pause(timer1);

    // ========== 创建第二个屏幕（图表 + Back按钮） ==========
    scr2 = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr2, lv_color_hex(0xCCCCCC), LV_STATE_DEFAULT);

    // 创建图表（折线图）
    chart2 = lv_chart_create(scr2);
    lv_obj_set_size(chart2, 240, 180);
    lv_obj_align(chart2, LV_ALIGN_TOP_MID, 0, 20);
    lv_chart_set_type(chart2, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart2, LV_CHART_AXIS_PRIMARY_Y, 0, 400);  // Y轴范围0~400
    lv_chart_set_point_count(chart2, 20);                         // 显示20个历史点
    lv_chart_set_div_line_count(chart2, 5, 5);

    // 添加ADC电压数据系列（蓝色）
    ser_adc = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    // 添加温度数据系列（红色）
    ser_temp = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    
    // 预填充0值，避免图表初始显示空白
    for (int i = 0; i < 20; i++) {
        lv_chart_set_next_value(chart2, ser_adc, 0);
        lv_chart_set_next_value(chart2, ser_temp, 0);
    }

    // Back按钮
    btn_back = lv_btn_create(scr2);
    lv_obj_set_size(btn_back, 100, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_obj_add_event_cb(btn_back, back_btn_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
}