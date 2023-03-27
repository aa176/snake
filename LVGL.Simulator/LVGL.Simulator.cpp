/*
 * PROJECT:   LVGL PC Simulator using Visual Studio
 * FILE:      LVGL.Simulator.cpp
 * PURPOSE:   Implementation for LVGL ported to Windows Desktop
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include "my_test.h"
#include <Windows.h>

#include "resource.h"

#if _MSC_VER >= 1200
 // Disable compilation warnings.
#pragma warning(push)
// nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)
// 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4244)
#endif

#include "lvgl/lvgl.h"
#include "lvgl/examples/lv_examples.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/win32drv/win32drv.h"


//头插法
void Snake_Node_End_Add(Snake_List_T* head, Snake_Node_Info_T tmp);

//创建节点
Snake_Node_T* Snake_Node_Create(Snake_Node_Info_T new_node);

Snake_Node_T* Snake_Node_Create(Snake_Node_Info_T new_node) {
    Snake_Node_Info_T* new_node_info = (Snake_Node_Info_T*)malloc(sizeof(Snake_Node_Info_T));
    new_node_info->x = new_node.x;
    new_node_info->y = new_node.y;

    //lv_malloc

    Snake_Node_T* terurn_node = (Snake_Node_T*)malloc(sizeof(Snake_Node_T));
    terurn_node->next = NULL;
    terurn_node->pre = NULL;
    terurn_node->data = new_node_info;
    return terurn_node;
}

void Snake_Node_End_Add(Snake_List_T* head, Snake_Node_Info_T new_node) {
    //创建新节点
    Snake_Node_T* node = Snake_Node_Create(new_node);

    //printf(" node->x %d  node->y %d\n",  ((Snake_Node_Info_T *)(node->data))->x, ((Snake_Node_Info_T*)(node->data))->y);
    Snake_Node_T* tmp = head->next;
    if (head == NULL)
    {
        return;
    }
    //尾插
    if (head->count == 0)
    {
        head->next = node;
        head->pre = node;
        head->count++;
    }
    else {
        head->next->next = node;
        node->pre = head->next;
        head->next = node;
        head->count++;
    }
}

#if _MSC_VER >= 1200
// Restore compilation warnings.
#pragma warning(pop)
#endif

#include <stdio.h>

bool single_display_mode_initialization()
{
    if (!lv_win32_init(
        GetModuleHandleW(NULL),
        SW_SHOW,
        800,
        480,
        LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDI_LVGL))))
    {
        return false;
    }

    lv_win32_add_all_input_devices_to_group(NULL);

    return true;
}

#include <process.h>
#include <lv_drivers/indev/keyboard.h>


HANDLE g_window_mutex = NULL;
bool g_initialization_status = false;

#define LVGL_SIMULATOR_MAXIMUM_DISPLAYS 16
HWND g_display_window_handles[LVGL_SIMULATOR_MAXIMUM_DISPLAYS];

unsigned int __stdcall lv_win32_window_thread_entrypoint(
    void* raw_parameter)
{
    size_t display_id = *(size_t*)(raw_parameter);

    HINSTANCE instance_handle = GetModuleHandleW(NULL);
    int show_window_mode = SW_SHOW;
    HICON icon_handle = LoadIconW(instance_handle, MAKEINTRESOURCE(IDI_LVGL));
    lv_coord_t hor_res = 800;
    lv_coord_t ver_res = 450;

    wchar_t window_title[256];
    memset(window_title, 0, sizeof(window_title));
    _snwprintf(
        window_title,
        256,
        L"LVGL Simulator for Windows Desktop (Display %d)",
        display_id);

    g_display_window_handles[display_id] = lv_win32_create_display_window(
        window_title,
        hor_res,
        ver_res,
        instance_handle,
        icon_handle,
        show_window_mode);
    if (!g_display_window_handles[display_id])
    {
        return 0;
    }

    g_initialization_status = true;

    SetEvent(g_window_mutex);

    MSG message;
    while (GetMessageW(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    lv_win32_quit_signal = true;

    return 0;
}

bool multiple_display_mode_initialization()
{
    if (!lv_win32_init_window_class())
    {
        return false;
    }

    for (size_t i = 0; i < LVGL_SIMULATOR_MAXIMUM_DISPLAYS; ++i)
    {
        g_initialization_status = false;

        g_window_mutex = CreateEventExW(NULL, NULL, 0, EVENT_ALL_ACCESS);

        _beginthreadex(
            NULL,
            0,
            lv_win32_window_thread_entrypoint,
            &i,
            0,
            NULL);

        WaitForSingleObjectEx(g_window_mutex, INFINITE, FALSE);

        CloseHandle(g_window_mutex);

        if (!g_initialization_status)
        {
            return false;
        }
    }

    lv_win32_window_context_t* context = (lv_win32_window_context_t*)(
        lv_win32_get_window_context(g_display_window_handles[0]));
    if (context)
    {
        lv_win32_pointer_device_object = context->mouse_device_object;
        lv_win32_keypad_device_object = context->keyboard_device_object;
        lv_win32_encoder_device_object = context->mousewheel_device_object;
    }

    lv_win32_add_all_input_devices_to_group(NULL);

    return true;
}

//static lv_obj_t* label;
//
//static void slider_event_cb(lv_event_t* e)
//{
//    lv_obj_t* slider = lv_event_get_target(e);
//
//    /*Refresh the text*/
//    lv_label_set_text_fmt(label, "%d", lv_slider_get_value(slider));
//    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
//}
//

//网格页面指针
lv_obj_t* cont;

//网格样式
static lv_style_t style_cont_child;
//蛇头
Snake_List_T* head = NULL;

//蛇吃的食物
Snake_Node_Info_T* food = NULL;

//记录当前的状态等
pos_change_t pos_c = { 0 };

//void snaker_status_to_n(pos_change_t* pos_all) {
//    pos_t* head = pos_all->snack;
//    pos_change_t* p = pos_all;
//
//
//    while (p->snack != NULL)
//    {
//        p->snack->change_flag = 'n';
//        
//        p->snack = p->snack->next;
//    }
//    pos_all->snack = head;
//}

//建立一个9*9网格
void init_page(lv_obj_t** cont) {
    //row 
    static lv_coord_t col_dsc[ROW] = { 0 };
    static lv_coord_t row_dsc[COL] = { 0 };
    //
    for (uint32_t i = 0; i < sizeof(col_dsc) / sizeof(short) - 1; i++)
    {
        col_dsc[i] = 25;
    } 
    col_dsc[ROW - 1] = LV_GRID_TEMPLATE_LAST;

    for (uint32_t i = 0; i < sizeof(row_dsc) / sizeof(short) - 1; i++)
    {
        row_dsc[i] = 25;
    } 
    row_dsc[COL - 1] = LV_GRID_TEMPLATE_LAST;

    *cont = lv_obj_create(lv_scr_act());
    //进行赋值cont指针赋值
    pos_c.cont = *cont;

    lv_obj_set_style_grid_column_dsc_array(*cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(*cont, row_dsc, 0);
    lv_group_add_obj(lv_group_get_default(), *cont);
    lv_group_focus_obj(*cont);

         

    //lv_obj_set_size(cont, 800, 750);
    lv_obj_center(*cont);
    lv_obj_set_layout(*cont, LV_LAYOUT_GRID);

    lv_obj_t* obj;
    lv_obj_t* label;


    //设置背景颜色
    static lv_style_t style_obj;
    lv_style_init(&style_obj);
    //lv_style_set_bg_color(&style_obj, lv_color_hex(0xFFFFFF));

    lv_obj_set_style_pad_row(*cont, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(*cont, 0, LV_PART_MAIN);

    for (uint32_t i = 0; i < ROW; i++)
    {
        for (uint32_t j = 0; j < COL; j++)
        {
            obj = lv_obj_create(*cont);
            //lv_obj_add_style(obj, &style_obj, LV_STATE_DEFAULT);

            lv_obj_set_size(obj, 40, 40);
            //obj = lv_btn_create(cont);

            lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, j, 1, LV_GRID_ALIGN_STRETCH, i, 1);

            //lv_label_set_text_fmt(label, "%d-%d", col + 1, row + 1);
            //lv_obj_center(label);
            //printf("i = %d\n", i);
        }
    }

}

void show_page(lv_obj_t* cont, Snake_List_T* snacker);
void my_timer1(lv_timer_t* timer)
{
    static uint32_t count = 0;
    pos_change_t* user_data = (pos_change_t*)(timer->user_data);  
    //printf("my_timer called with user data: %d\n", *user_data);
    //printf("user_data = %d\n ", (*user_data).dirction);
    /*Do something with LVGL*/

    //printf(" %d s\n", count);
    //回调前，状态置为n
    //snaker_status_to_n(user_data);
    //改变位置
    (*user_data).chang_pos(user_data);
    //printf(" my_timer1 : x = %d, y = %d \n", (*user_data).snack->x, (*user_data).snack->y);
    /*if (count % 5 == 0)
    {
        
    }*/
    change_page(cont, &style_cont_child, head);
    count++; 
}

void my_timer2(lv_timer_t* timer) {
    //lv_obj_t * user_data = (lv_obj_t *)(timer->user_data);
    
}
static void clicked(lv_event_t* e) {
    
    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xFFFFFF));

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    //lv_obj_remove_style_all(label);
    //printf("test!\n");
    lv_obj_add_style(label, &style_btn, LV_STATE_DEFAULT);
    
    lv_obj_get_style_bg_color(label, LV_PART_MAIN);
}
//键盘事件
static void keypad_event(lv_event_t* e) {
    int key = 0;
    lv_event_code_t code = lv_event_get_code(e);
    pos_change_t* now_pos = (pos_change_t *)lv_event_get_user_data(e);
    if (code == LV_EVENT_KEY)
    {
        key = lv_indev_get_key(lv_indev_get_act());
        /*key = 119  = 115 key = 97 key = 100*/
        //printf("key2 = %d\n", key);
        switch (key)
        {
        case 119:
            (*now_pos).dirction = top;
            break;
        case 115:
            (*now_pos).dirction = bottom;
            break;
        case 97:
            (*now_pos).dirction = left;
            break;
        case 100:
            (*now_pos).dirction = right;
            break;

        default:
            break;
        }
        printf("(*now_pos).dirction = %d\n", (*now_pos).dirction);
        printf("%d   %d    %d   %d \n", top, bottom, left, right);
        printf("key2 = %d\n", key);
        
    }
    //lv_obj_remove_style_all(label);
}

static void event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    int key = 0;
    switch (code) {
    case LV_EVENT_PRESSED:
        lv_label_set_text(label, "The last button event:\nLV_EVENT_PRESSED");
        break;
    case LV_EVENT_CLICKED:
        lv_label_set_text(label, "The last button event:\nLV_EVENT_CLICKED");
        break;
    case LV_EVENT_LONG_PRESSED:
        lv_label_set_text(label, "The last button event:\nLV_EVENT_LONG_PRESSED");
        break;
    case LV_EVENT_LONG_PRESSED_REPEAT:
        lv_label_set_text(label, "The last button event:\nLV_EVENT_LONG_PRESSED_REPEAT");
        break;

    case LV_EVENT_KEY:
        key = lv_indev_get_key(lv_indev_get_act());
        //printf("key = %d\n", key);
        break;
    default:
        //printf("%d\n" ,lv_event_get_code(e));
        break;
    }
}

#define BTN_NUM 10

//蛇初始化
void init_snaker(Snake_List_T **head) {
    //初始化头节点
    *head = (Snake_List_T*)malloc(sizeof(Snake_List_T));
    (*head)->next = NULL;
    (*head)->pre = NULL;
    (*head)->count = 0; 
}

//食物初始化
void init_food() {
    /*food = (pos_t *)malloc(sizeof(struct postion));
    food->x = ROW / 2;
    food->y = COL / 2;*/
}
void chang_pos(pos_change_t* pos_all);

void init_now_status() {
    pos_c.dirction = right;
    pos_c.snack = head;
    pos_c.chang_pos = chang_pos;
}

//位置的改变
void Snake_Postion_Change(pos_change_t* pos_all);

void Snake_Postion_Change(pos_change_t* pos_all) {

    switch (pos_all->dirction)
    {
    case right:
        if (((Snake_Node_Info_T*)(pos_all->snack->next->data))->x >= 0 && ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x < (ROW -2) )
        {
            ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x += 1;
            //printf("x = %d  y = %d\n", ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x, ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y);
        }
        break;
    case left:
        if (((Snake_Node_Info_T*)(pos_all->snack->next->data))->x > 0 && ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x <= (ROW -2) )
        {
            ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x -= 1;
            //printf("x = %d  y = %d\n", ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x, ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y);
        }
        break;
    case top:
        if (((Snake_Node_Info_T*)(pos_all->snack->next->data))->y > 0 && ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y <= (COL - 2))
        {
            ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y -= 1;
            //printf("x = %d  y = %d\n", ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x, ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y);
        }
        break;

    case bottom:
        if (((Snake_Node_Info_T*)(pos_all->snack->next->data))->y >= 0 && ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y < (COL - 2))
        {
            ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y += 1;
            //printf("x = %d  y = %d\n", ((Snake_Node_Info_T*)(pos_all->snack->next->data))->x, ((Snake_Node_Info_T*)(pos_all->snack->next->data))->y);
        }
        break;
    }
}
//蛇移动
void Snake_Move(pos_change_t* pos_all);
void Snake_Move(pos_change_t* pos_all) {

    Snake_List_T* snake = pos_all->snack;

    if (snake->count == 0 )
    {
        return;
    }
    //等于1的时候，需要不断改变位置
    if (snake->count == 1)
    {
        Snake_Postion_Change(pos_all);
        return;
    }
    //等于2的时候，需要头增加
    if (snake->count == 2)
    {
        Snake_Postion_Change(pos_all);
        return;
    }
    //大于2的时候，需要头增加，尾删除
    if (snake->count > 2)
    {
        Snake_Postion_Change(pos_all);
        return;
    }

}
void chang_pos(pos_change_t* pos_all) {
    //Snake_List_T* head = pos_all->snack;
    //pos_change_t* p = pos_all;

    Snake_Move(pos_all);
    //pos_all->snack = head;

}
void show_page(lv_obj_t *cont, Snake_List_T* snacker) {
    Snake_List_T* p = snacker;

    //样式设置
    //static lv_style_t style_child;
    //lv_style_init(&style_child);
    //lv_style_set_bg_color(&style_child, lv_color_hex(0xFF0000));
    

    /*while (p != NULL)
    {
        lv_obj_add_style(lv_obj_get_child(cont, (p->x )  + p->y * (COL - 1) + 1 ), &style_child, LV_STATE_DEFAULT);
        lv_obj_set_size(lv_obj_get_child(cont, (p->x) + p->y * (COL - 1) + 1), 40, 40);
        p = p->next;
    }*/

}

static int n;
int main()
{
    lv_init();

    
    if (!single_display_mode_initialization())
    {
        return -1;
    }

    //初始化
    init_food();

    lv_style_init(&style_cont_child);
    //lv_style_set_bg_color(&style_cont_child, lv_color_hex(0xff0000));
    //lv_style_set_bg_opa(&style_cont_child, LV_OPA_50);
    //lv_style_set_border_width(&style_cont_child, 2);
    //lv_style_set_border_color(&style_cont_child, lv_color_black());

    //lv_style_set_x(&style_cont_child, 25);
    //lv_style_set_y(&style_cont_child, 25);
    //lv_style_set
    init_snaker(&head);
    //添加两个节点
    Snake_Node_Info_T a1 ;
    a1.x = 4;
    a1.y = 4;

    Snake_Node_End_Add(head, a1);

    /*Snake_Node_Info_T a2 ;
    a2.x = 3;
    a2.y = 3;
    Snake_Node_End_Add(head, a2);*/

    init_now_status();

    init_page(&cont);
    lv_obj_set_size(cont, 500, 500);

    //添加事件
    lv_obj_add_event_cb(cont, keypad_event, LV_EVENT_ALL, &pos_c);

    //改变颜色
    change_page(cont, &style_cont_child, head);

    ////样式设置
    //static lv_style_t style_child2;
    //lv_style_init(&style_child2);
    //lv_style_set_bg_color(&style_child2, lv_color_hex(0xFF0000));
    //for (uint32_t i = 0; i < ROW; i++)
    //{
    //    for (uint32_t j = 0; j < COL; j++)
    //    {
    //        lv_obj_add_style(lv_obj_get_child(cont, i + j * (COL - 1) + 1), &style_child2, LV_STATE_DEFAULT);
    //        //Sleep(2);
    //    }
    //}

    /*static lv_style_t style_obj2;
    lv_style_init(&style_obj2);
    lv_style_set_bg_color(&style_obj2, lv_color_hex(0xFF0000));
    if (lv_obj_get_child(cont, 2) != NULL)
    {
        printf("lv_obj_get_child()\n");
    }*/
    //lv_obj_add_style(lv_obj_get_child(cont, 2), &style_obj2, LV_STATE_DEFAULT);

    //lv_obj_invalidate(lv_obj_get_child(cont, 2));
    //lv_obj_t* btn1 = lv_btn_create(lv_scr_act());

    //lv_obj_set_size(screen, 400, 500);

    //lv_obj_set_size(btn1, 100, 50);

    //lv_obj_t* cont2 = lv_obj_create(lv_scr_act());
    //lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_ROW);
    //lv_obj_t *btn1 = lv_btn_create(cont2);
    //
    //lv_obj_t* label1 = lv_label_create(cont2);
    ////lv_obj_add_event_cb(btn1, clicked, LV_EVENT_CLICKED, lv_obj_get_child(cont, 2));

    //lv_label_set_text(label1, "label1\n");

    //printf("lv_obj_get_parent: %s\n", lv_obj_get_parent(btn1)->h_layout);
    //btn样式
    //static lv_style_t style_btn;
    //lv_style_init(&style_btn);
    //lv_style_set_bg_color(&style_btn, lv_color_hex(0xff0000));
    //lv_style_set_bg_opa(&style_btn, LV_OPA_50);
    //lv_style_set_border_width(&style_btn, 2);
    //lv_style_set_border_color(&style_btn, lv_color_black());

    //lv_obj_add_style(btn1, &style_btn, LV_STATE_PRESSED);  /*overwrite only some colors to red when pressed*/

    //lv_obj_t* info_label = lv_label_create(lv_scr_act());
    //lv_label_set_text(info_label, "The last button event:\nNone");
    //lv_obj_add_event_cb(btn1, event_cb, LV_EVENT_ALL, info_label);


    //定时器
    static uint32_t user_data = 10;
    lv_timer_t* timer = lv_timer_create(my_timer1, 500, &pos_c);

    //lv_timer_t* timer = lv_timer_create(my_timer2, 500, cont);






    /*if (!multiple_display_mode_initialization())
    {
        return -1;
    }
    else
    {
        for (size_t i = 0; i < LVGL_SIMULATOR_MAXIMUM_DISPLAYS; ++i)
        {
            lv_win32_window_context_t* context = (lv_win32_window_context_t*)(
                lv_win32_get_window_context(g_display_window_handles[i]));
            if (context)
            {
                lv_disp_set_default(context->display_device_object);
                switch (i)
                {
                case 0:
                    lv_demo_widgets();
                    break;
                case 1:
                    lv_demo_benchmark();
                    break;
                case 2:
                    lv_example_style_1();
                    break;
                case 3:
                    lv_example_get_started_1();
                    break;
                case 4:
                    lv_example_anim_1();
                    break;
                case 5:
                    lv_example_style_2();
                    break;
                case 6:
                    lv_example_get_started_2();
                    break;
                case 7:
                    lv_example_anim_2();
                    break;
                case 8:
                    lv_example_style_3();
                    break;
                case 9:
                    lv_example_get_started_3();
                    break;
                case 10:
                    lv_example_anim_3();
                    break;
                case 11:
                    lv_example_style_4();
                    break;
                case 12:
                    lv_example_style_5();
                    break;
                case 13:
                    lv_example_style_6();
                    break;
                case 14:
                    lv_example_imgfont_1();
                    break;
                case 15:
                    lv_example_style_7();
                    break;
                default:
                    break;
                }
            }
        }
    }*/

    //lv_win32_window_context_t* context = (lv_win32_window_context_t*)(
    //    lv_win32_get_window_context(g_display_window_handles[1]));
    //if (context)
    //{
    //    lv_obj_t* scr = lv_disp_get_scr_act(context->display_device_object);

    //    /*Create a slider in the center of the display*/
    //    lv_obj_t* slider = lv_slider_create(scr);
    //    lv_obj_set_width(slider, 200);                          /*Set the width*/
    //    lv_obj_center(slider);                                  /*Align to the center of the parent (screen)*/
    //    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/

    //    /*Create a label above the slider*/
    //    label = lv_label_create(scr);
    //    lv_label_set_text(label, "0");
    //    lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15);    /*Align top of the slider*/
    //}

    /*
     * Demos, benchmarks, and tests.
     *
     * Uncomment any one (and only one) of the functions below to run that
     * item.
     */

    // ----------------------------------
    // my freetype application
    // ----------------------------------

    ///*Init freetype library
    // *Cache max 64 faces and 1 size*/
    //lv_freetype_init(64, 1, 0);

    ///*Create a font*/
    //static lv_ft_info_t info;
    //info.name = "./lvgl/src/extra/libs/freetype/arial.ttf";
    //info.weight = 36;
    //info.style = FT_FONT_STYLE_NORMAL;
    //lv_ft_font_init(&info);

    ///*Create style with the new font*/
    //static lv_style_t style;
    //lv_style_init(&style);
    //lv_style_set_text_font(&style, info.font);

    ///*Create a label with the new style*/
    //lv_obj_t* label = lv_label_create(lv_scr_act());
    //lv_obj_add_style(label, &style, 0);
    //lv_label_set_text(label, "FreeType Arial Test");

    // ----------------------------------
    // my Win32 filesystem driver application
    // ----------------------------------

    /*::lv_fs_win32_init();

    lv_fs_dir_t d;
    if (lv_fs_dir_open(&d, "/") == LV_FS_RES_OK)
    {
        char b[MAX_PATH];
        memset(b, 0, MAX_PATH);
        while (lv_fs_dir_read(&d, b) == LV_FS_RES_OK)
        {
            printf("%s\n", b);
        }

        lv_fs_dir_close(&d);
    }*/

    // ----------------------------------
    // Demos from lv_examples
    // ----------------------------------

    //lv_demo_widgets();           // ok
    //lv_demo_benchmark();
    //lv_demo_keypad_encoder();    // ok
    //lv_demo_music();             // removed from repository
    // lv_demo_printer();           // removed from repository
    //lv_demo_stress();            // ok

    // ----------------------------------
    // LVGL examples
    // ----------------------------------

    /*
     * There are many examples of individual widgets found under the
     * lvgl\exampless directory.  Here are a few sample test functions.
     * Look in that directory to find all the rest.
     */

    // lv_ex_get_started_1();
    // lv_ex_get_started_2();
    // lv_ex_get_started_3();

    // lv_example_flex_1();
    // lv_example_flex_2();
    // lv_example_flex_3();
    // lv_example_flex_4();
    // lv_example_flex_5();
    // lv_example_flex_6();        // ok

    // lv_example_grid_1();
    // lv_example_grid_2();
    // lv_example_grid_3();
    // lv_example_grid_4();
    // lv_example_grid_5();
    // lv_example_grid_6();

    // lv_port_disp_template();
    // lv_port_fs_template();
    // lv_port_indev_template();

    // lv_example_scroll_1();
    // lv_example_scroll_2();
    // lv_example_scroll_3();

    // lv_example_style_1();
    // lv_example_style_2();
    // lv_example_style_3();
    // lv_example_style_4();        // ok
    // lv_example_style_6();        // file has no source code
    // lv_example_style_7();
    // lv_example_style_8();
    // lv_example_style_9();
    // lv_example_style_10();
    // lv_example_style_11();       // ok

    // ----------------------------------
    // LVGL widgets examples
    // ----------------------------------

    // lv_example_arc_1();
    // lv_example_arc_2();

    // lv_example_bar_1();          // ok
    // lv_example_bar_2();
    // lv_example_bar_3();
    // lv_example_bar_4();
    // lv_example_bar_5();
    // lv_example_bar_6();          // issues

    // lv_example_btn_1();
    // lv_example_btn_2();
    // lv_example_btn_3();

    // lv_example_btnmatrix_1();
    // lv_example_btnmatrix_2();
    // lv_example_btnmatrix_3();

    // lv_example_calendar_1();

    // lv_example_canvas_1();
    // lv_example_canvas_2();

    // lv_example_chart_1();        // ok
    // lv_example_chart_2();        // ok
    // lv_example_chart_3();        // ok
    // lv_example_chart_4();        // ok
    // lv_example_chart_5();        // ok
    // lv_example_chart_6();        // ok

    // lv_example_checkbox_1();

    // lv_example_colorwheel_1();   // ok

    // lv_example_dropdown_1();
    // lv_example_dropdown_2();
    // lv_example_dropdown_3();

    // lv_example_img_1();
    // lv_example_img_2();
    // lv_example_img_3();
    // lv_example_img_4();         // ok

    // lv_example_imgbtn_1();

    // lv_example_keyboard_1();    // ok

    // lv_example_label_1();
    // lv_example_label_2();       // ok

    // lv_example_led_1();

    // lv_example_line_1();

    // lv_example_list_1();

    // lv_example_meter_1();
    // lv_example_meter_2();
    // lv_example_meter_3();
    // lv_example_meter_4();       // ok

    // lv_example_msgbox_1();

    // lv_example_obj_1();         // ok

    // lv_example_roller_1();
    // lv_example_roller_2();      // ok

    // lv_example_slider_1();      // ok
    // lv_example_slider_2();      // issues
    // lv_example_slider_3();      // issues

    // lv_example_spinbox_1();

    // lv_example_spinner_1();     // ok

    // lv_example_switch_1();      // ok

    // lv_example_table_1();
    // lv_example_table_2();       // ok

    // lv_example_tabview_1();

    // lv_example_textarea_1();    // ok
    // lv_example_textarea_2();
    // lv_example_textarea_3();    // ok, but not all button have functions

    // lv_example_tileview_1();    // ok

    // lv_example_win_1();         // ok

    // ----------------------------------
    // Task handler loop
    // ----------------------------------

    while (!lv_win32_quit_signal)
    {
        lv_task_handler();
        Sleep(1);
    }

    return 0;
}
