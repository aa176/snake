#pragma once

#ifndef MY_TEST

#define MY_TEST

#define ROW 13
#define COL 15

//static inline lv_obj_t * lv_scr_act(void); 

#include <stdio.h>
#include <src/misc/lv_timer.h>
#include <src/core/lv_obj.h>
#include <src/layouts/grid/lv_grid.h>
//game
void page_init();

void my_timer1(lv_timer_t *timer);

//
typedef struct snake_node_info Snake_Node_Info_T;

//蛇 节点信息
typedef struct snake_node_info
{
    int x;
    int y;
    char change_flag;
}Snake_Node_Info_T, *Snake_Node_Info_P;

//
typedef struct snake_node Snake_Node_T;
//蛇 节点
typedef struct snake_node {
    void* data;         //指向数据域
    Snake_Node_T* next;  //指向下一个
    Snake_Node_T* pre;  
    
}Snake_Node_T, *Snake_Node_P;

//
typedef struct snake_list Snake_List_T;
//蛇头---> 句柄
typedef struct snake_list
{
    Snake_Node_T* next;
    Snake_Node_T* pre;
    uint32_t count;
}Snake_List_T, *Snake_List_P;

//方向
enum MyEnum
{
    right = 0,
    left,
    top,
    bottom
};

typedef struct postion_chang pos_change_t;
//typedef (*chang_pos)(pos_t* pos_all);
typedef struct postion_chang
{
    //方向
    int dirction;
    //蛇头
    Snake_List_T* snack;
    //蛇位置改变函数
    void (*chang_pos)(pos_change_t *now_status);
    //指向当前布局的指针
    lv_obj_t* cont;

}pos_change_t;


//改变颜色
void change_page(lv_obj_t* cont, lv_style_t* style, Snake_List_T* head);

//改变颜色
void change_page(lv_obj_t* cont, lv_style_t *style, Snake_List_T * head) {
    //Snake_Node_T* p = head->next;

    //lv_style_set_x(
    //printf("lv_obj_get_child_cnt(cont) = %d\n", lv_obj_get_child_cnt(cont));
    lv_style_set_bg_color(style, lv_color_hex(0xff0000));

    //for (uint32_t i = 0; i < lv_obj_get_child_cnt(cont); i++) {
    //    lv_obj_t* child = lv_obj_get_child(cont, i);
    //    lv_obj_remove_style_all(child);
    //}

    //for (uint32_t i = 0; i < lv_obj_get_child_cnt(cont); i++) {
    //    lv_obj_t* child = lv_obj_get_child(cont, i);     
    //    lv_obj_add_style(child, style, LV_STATE_DEFAULT);
    //    lv_obj_invalidate(child);
    //    lv_obj_remove_style_all(child);
    //    lv_obj_set_size(child, 50, 50);
    //    lv_obj_remove_style(child, style, LV_STATE_DEFAULT);
    //}

    //定义一个移动的指针指向第一个节点
    Snake_Node_T* pmove = head->pre;
    //不为空
    lv_style_set_bg_color(style, lv_color_hex(0x000000));
    while (pmove)
    {
        //printf("pmove\n");
        lv_obj_t* obj = lv_obj_get_child(cont, ((Snake_Node_Info_T*)(pmove->data))->x \
            + ((Snake_Node_Info_T*)(pmove->data))->y * (COL));
        //打印数据
        printf("1 x =  %d  y = %d\n", ((Snake_Node_Info_T*)(pmove->data))->x, ((Snake_Node_Info_T*)(pmove->data))->y);

        if (!obj)
        {
            break;
        }
        //lv_obj_set_size(obj, 25, 25);
        lv_obj_remove_style(obj, style, LV_STATE_DEFAULT);
        
        lv_obj_add_style(obj, style, LV_STATE_DEFAULT);
        //lv_obj_invalidate(obj);
        //用后项指针去做遍历
        pmove = pmove->next;
    }

    //printf("break!\n");
}


#endif
