#pragma once

#ifndef MY_TEST

#define MY_TEST

#define ROW 10
#define COL 10

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
    Snake_Node_T* pre;
    Snake_Node_T* next;  //指向下一个
    void* data;         //指向数据域
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
    Snake_List_P* snack;
    //蛇位置改变函数
    void (*chang_pos)(pos_change_t *now_status);

    //指向当前布局的指针
    lv_obj_t* cont;
}pos_change_t;


//改变颜色
void change_page(lv_obj_t* cont, lv_style_t* style, Snake_List_T* head);

//改变颜色
void change_page(lv_obj_t* cont, lv_style_t *style, Snake_List_T * head) {
    Snake_Node_T* p = head->next;

    //lv_style_set_x(
    printf("lv_obj_get_child_cnt(cont) = %d\n", lv_obj_get_child_cnt(cont));
    for (uint32_t i = 0; i < lv_obj_get_child_cnt(cont); i++) {
        lv_obj_t* child = lv_obj_get_child(cont, i);
        /*Do something with child*/
        //先清除所有样式
        //lv_obj_remove_style_all(child);

    }
    int n = 0;
    while (p != NULL)
    {
        printf("n = %d\n", n++);
        lv_obj_t* obj = lv_obj_get_child(cont, ((Snake_Node_Info_T*)(p->data))->x \
            + ((Snake_Node_Info_T*)(p->data))->y * (COL) );

        printf("x =  %d  y = %d\n", ((Snake_Node_Info_T*)(p->data))->x, ((Snake_Node_Info_T*)(p->data))->y);
        if (!obj)
        {
            break;
        }
        lv_obj_add_style(obj, style, LV_STATE_DEFAULT);
        p = p->next;
        
    }
    printf("break!\n");
}


#endif
