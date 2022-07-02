#include "lcd/lcd.h"
#include <string.h>
#include "utils.h"
#include "img.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>


#define LEFT_BUTTON 0
#define RIGHT_BUTTON 1
#define TAIKO_START_X_BEGIN 131
#define TAIKO_START_X_END 150
#define TAIKO_START_Y_BEGIN 37
#define TAIKO_START_Y_END 56
#define MAX_DRUM_NUM 6 // number of drum in the drum list
#define DRUM_START_SPEED 1 // object moves pixel per loop
#define JUDGE_CENTER 38 // center for drum beat judgement
#define HEALTH_BOUND 40
#define INIT_SPEED 3

typedef struct Drum {
    int mode; // 0 if the drum is red; 1 if the drum is blue
    int speed; // object moves pixel per loop, initialize 1
    struct Drum* next_drum; // record the next drum of the current drum in the Drum List, if it is the end, point to the head in the list
    // bool is_displayd; // 1 if the Drum is displayed in the screen; 0 otherwise
    int left_most_position; // record the Drum current left most x coordinate
} Drum;

typedef struct Drum_list {
    Drum* first_drum; // the first drum which is displayd in the screen, NULL if no drum has been displayd
    Drum* last_displayd_drum; // the last drum of list which displayd in the screen; NULL if no drum has been displayd
    u16 drum_mum; // record total drum number in the list
    u16 display_num; // record drum number that is in the screen
} Drum_list;

/******************************************************************************
	   Function description: initialize all Drum and Drum list: 
       Entry data: init_speed    initialize all drum speed to DRUM_START_SPEED
                   init_mode     initialize all drum colour to 0 red; 1 blue
                   drum          drum to be initialized
                   drum_list     drum list to be initialized
       Return value: None
******************************************************************************/
void drum_and_drum_list_init(int init_speed, Drum* drum, Drum_list* drum_list) {
    for (int i = 0; i < MAX_DRUM_NUM - 1; i++ ) {
        drum[i].mode = Get_Random() % 2;
        drum[i].speed = init_speed;
        drum[i].next_drum = drum + i + 1;
        drum[i].left_most_position = TAIKO_START_X_BEGIN;
    }
    drum[MAX_DRUM_NUM - 1].mode = Get_Random() % 2;
    drum[MAX_DRUM_NUM - 1].speed = init_speed;
    drum[MAX_DRUM_NUM - 1].next_drum = drum;
    drum[MAX_DRUM_NUM - 1].left_most_position = TAIKO_START_X_BEGIN;

    drum_list->first_drum = NULL;
    drum_list->last_displayd_drum = NULL;
    drum_list->drum_mum = MAX_DRUM_NUM;
    drum_list->display_num = 0;
}
/******************************************************************************
	   Function description: return 0/1 for whether add a new drum to display
       Entry data: loop_num     every 20 loop add a new drum
       Return value: 1 add a drum; 0 otherwise
******************************************************************************/
bool add_condition(u16 loop_num) {
    if ((loop_num % 14 == 0) && (Get_Random() % 10 < 7)) return TRUE;
    return FALSE;
}

/******************************************************************************
	   Function description: update drum list, if not all drum is in the screen,
                             add the drum to the list (change last_displayd_drum)
******************************************************************************/
void add_drum(Drum* drum, Drum_list* drum_list) {
    if (drum_list->drum_mum == drum_list->display_num) return;
    if (drum_list->first_drum == NULL) {
        drum_list->first_drum = drum;
        drum_list->last_displayd_drum = drum;
    }
    else {
        drum_list->last_displayd_drum = drum_list->last_displayd_drum->next_drum;
    }
    drum_list->display_num++;
}

void loop_delay(u16 score) {
    uint32_t delay_time;
    if (score > 1000) return;
    else delay_1ms(20 - score / 50);
}

/******************************************************************************
	   Function description: update drum data:
                             left_most_postion: drum in the screen positon -= speed,
                                                place edge to x = 1
******************************************************************************/
void drum_data_update(Drum* drum, Drum_list* drum_list) {
    Drum* temp = drum_list->first_drum;
    for (int i = 0; i < drum_list->display_num; i++) {
        temp->left_most_position -= temp->speed;
        if (temp->left_most_position < 1) temp->left_most_position = 1;
        temp = temp->next_drum;
    }
}
/******************************************************************************
	   Function description: display all drum in the drum list from
                             first_drum to last_displace to the screen
******************************************************************************/
void drum_display(Drum* drum, Drum_list* drum_list) {
    Drum* temp = drum_list->first_drum;
    for (int i = 0; i < drum_list->display_num; i++) {
        LCD_Fill(temp->left_most_position + 19, TAIKO_START_Y_BEGIN, temp->left_most_position + 19 + temp->speed, TAIKO_START_Y_END, BLACK);
        if (temp->mode == 0) LCD_Show_tgdr(temp->left_most_position, TAIKO_START_Y_BEGIN, tgdr_red);
        else LCD_Show_tgdr(temp->left_most_position, TAIKO_START_Y_BEGIN, tgdr_blue);
        temp = temp->next_drum;
    }
}
/******************************************************************************
	   Function description: print full black to those node reach to end of edge
                             remove them from drum list (change first_drum) and 
                             decrease display_num in list
******************************************************************************/
void edge_drum_delete(Drum* drum, Drum_list* drum_list, int *health) {
    Drum* temp = drum_list->first_drum;
    u16 loop = drum_list->display_num;
    for(int i = 0; i < loop; i++) {
        if (temp->left_most_position == 1) {
            LCD_Fill(1, 37, 20, 56, BLACK);
            drum_list->display_num--;
            if (i != loop - 1) drum_list->first_drum = drum_list->first_drum->next_drum;
            else {
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
            }
            temp->left_most_position = TAIKO_START_X_BEGIN;
            *health -= 2;
        }
    }
}

/******************************************************************************
	   Function description: print color for different drum hit
                             mode 0: prefect, display gold
                             mode 1: great, display blue green
                             mode 2: miss, display red
******************************************************************************/
void hit_color(int mode) {
    u8 radius_lim = 10;
    u8 *text1 = (u8*)"Perfect";
    u8 *text2 = (u8*)"Great";
    u8 *text3 = (u8*)"Miss";
    if (mode == 0)
        for (u8 i = 0; i < radius_lim; i += 3) {
            LCD_DrawCircle(38, 46, i, 0xFFF1);
            LCD_ShowString(10, 63, text1, 0xFFF1);
        }
    else if (mode == 1)
        for (u8 i = 0; i < radius_lim; i += 3)
        {
            LCD_DrawCircle(38, 46, i, 0x2F5C);
            LCD_ShowString(20, 63, text2, 0x2F5C);
        }
    else if (mode == 2)
        for (u8 i = 0; i < radius_lim; i += 3)
        {
            LCD_DrawCircle(38, 46, i, 0xF8EF);
            LCD_ShowString(20, 63, text3, 0xF8EF);
        }
}

void Inp_init(void)
{
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
}

void Adc_init(void) 
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0|GPIO_PIN_1);
    RCU_CFG0|=(0b10<<14)|(1<<28);
    rcu_periph_clock_enable(RCU_ADC0);
    ADC_CTL1(ADC0)|=ADC_CTL1_ADCON;
}

void IO_init(void)
{
    Inp_init(); // inport init
    Adc_init(); // A/D init
    Lcd_Init(); // LCD init
}

void display_main_menu(void) {
    LCD_Clear(BLACK);
    bool temp = 0;
    for(int i = 1; i < 160; i += 20) {
        temp = !temp;
        for (int j = 1; j < 80; j += 20) {
            if (temp) LCD_ShowPic(i, j, i + 19, j + 19, tgdr_red);
            else LCD_ShowPic(i, j, i + 19, j + 19, tgdr_blue);
            delay_1ms(100);
            temp = !temp;
        }
    }
    delay_1ms(500);
    LCD_Clear(BLACK);
    u8 *title = (u8*)"TAIKO DRUM";
    u8 *select1 = (u8*)"play";
    u8 *select2 = (u8*)"help";
    LCD_ShowString(40, 20, title, WHITE);
    LCD_ShowString(20, 50, select1, WHITE);
    LCD_ShowString(110, 50, select2, WHITE);
    delay_1ms(500);
    LCD_ShowPic(10, 10, 29, 29, tgdr_red);
    delay_1ms(500);
    LCD_ShowPic(131, 10, 150, 29, tgdr_blue);
    delay_1ms(500);
}

void display_help(void) {
    u8 *p1 = (u8*)"Welcome to TAIKO   RUM MASTER TAIKO!  In this game, you  should press the   button when the";
    u8 *p2 = (u8*)"TAIKO moves to the center of the drum. Press left button when TAIKO is red. Press right button";
    u8 *p3 = (u8*)"when TAIKO is blue. Get the rhythm and have fun!(Though  there is no music";
    LCD_Clear(BLACK);
    LCD_ShowString(0, 0, p1, WHITE);
    delay_1ms(1000);
    while(1) {
        if (Get_Button(RIGHT_BUTTON)) break;
    }
    LCD_Clear(BLACK);
    LCD_ShowString(0, 0, p2, WHITE);
    delay_1ms(1000);
    while(1) {
        if (Get_Button(RIGHT_BUTTON)) break;
    }
    LCD_Clear(BLACK);
    LCD_ShowString(0, 0, p3, WHITE);
    delay_1ms(1000);
    while(1) {
        if (Get_Button(RIGHT_BUTTON)) break;
    }
}

/******************************************************************************
	   Function description: draw health line and score number
       Entry data: a    health line length, ranges from 1 to 40
                   b    score number
       Return value: None
******************************************************************************/
void display_health_and_score(int a, u16 b) {
    u8 *p1 = (u8*)"HP";
    // u8 *p2 = (u8*)"Score";
    u16 count = 0;
    u32 temp = b;
    if (b == 0) count = 1;
    while(temp != 0) {
        temp /= 10;
        count++;
    }

    LCD_ShowString(0, 0, p1, WHITE);
    LCD_Fill(20 + a, 7, 60, 10, BLACK);
    LCD_DrawLine(20, 10, 20 + a, 10, WHITE);
    LCD_DrawLine(20, 9, 20 + a, 9, WHITE);
    LCD_DrawLine(20, 8, 20 + a, 8, WHITE);
    LCD_DrawLine(20, 7, 20 + a, 7, WHITE);
    LCD_ShowNum(90, 1, b, count, WHITE);
}

/******************************************************************************
	   Function description: deal with the left most drum in the screen with left button pressed
                             if there is no drum in the screen, health -= 2
                             
                             if there are drums in the screen, if drum is red, 3 cases:
                             1. radius within center <= 5, health += 2, score += 5, first drum disappears
                             2. radius within center >5 <16, health += 1, score += 3, first drum disappears
                             3. radius within center >= 16, health -= 2, first drum not disappears
                             
                             if there are drums in the screen, if drum is blue, 2 cases:
                             1. radius within center <16, health -2, first drum disappears
                             2. radius within center >= 16, health -= 2, first drum not disappears
******************************************************************************/
void press_left_button(int *health, u16 *score, Drum_list *drum_list, int *hit_count) {
    if (drum_list->display_num == 0) {
        *health -= 2;
        return;
    }
    int center_distance = abs(drum_list->first_drum->left_most_position + 9 - JUDGE_CENTER);
    *hit_count = 0;
    if (drum_list->first_drum->mode == 0) {
        if (drum_list->display_num == 1) {
            if (center_distance <= 5) {
                *health += 2;
                *score += 5;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(0);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else if (center_distance < 16) {
                *health += 1;
                *score += 3;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(1);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
        else {
            if (center_distance <= 5) {
                *health += 2;
                *score += 5;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(0);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
                return;
            }
            else if (center_distance < 16) {
                *health += 1;
                *score += 3;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(1);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
    }
    else {
        if (drum_list->display_num == 1) {
            if (center_distance < 16) {
                *health -= 2;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(2);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
        else {
            if (center_distance < 16) {
                *health -= 2;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(2);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
                return;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
    }
}

/******************************************************************************
	   Function description: deal with the left most drum in the screen with right button pressed
                             if there is no drum in the screen, health -= 2
                             
                             if there are drums in the screen, if drum is blue, 3 cases:
                             1. radius within center <= 5, health += 2, score += 5, first drum disappears
                             2. radius within center >5 <16, health += 1, score += 3, first drum disappears
                             3. radius within center >= 16, health -= 2, first drum not disappears
                             
                             if there are drums in the screen, if drum is red, 2 cases:
                             1. radius within center <16, health -2, first drum disappears
                             2. radius within center >= 16, health -= 2, first drum not disappears
******************************************************************************/
void press_right_button(int *health, u16 *score, Drum_list *drum_list, int* hit_count) {
    if (drum_list->display_num == 0) {
        *health -= 2;
        return;
    }
    int center_distance = abs(drum_list->first_drum->left_most_position + 9 - JUDGE_CENTER);
    *hit_count = 0;
    if (drum_list->first_drum->mode == 1) {
        if (drum_list->display_num == 1) {
            if (center_distance <= 5) {
                *health += 2;
                *score += 5;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(0);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else if (center_distance < 16) {
                *health += 1;
                *score += 3;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(1);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
        else {
            if (center_distance <= 5) {
                *health += 2;
                *score += 5;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(0);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
                return;
            }
            else if (center_distance < 16) {
                *health += 1;
                *score += 3;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(1);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
    }
    else {
        if (drum_list->display_num == 1) {
            if (center_distance < 16) {
                *health -= 2;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(2);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = NULL;
                drum_list->last_displayd_drum = NULL;
                return;
            }
            else {
                *health -= 2;
                hit_color(2);
                return;
            }
        }
        else {
            if (center_distance < 16) {
                *health -= 2;
                LCD_Fill(drum_list->first_drum->left_most_position, 37, drum_list->first_drum->left_most_position + 19, 56, BLACK);
                hit_color(2);
                drum_list->first_drum->left_most_position = TAIKO_START_X_BEGIN;
                drum_list->display_num--;
                drum_list->first_drum = drum_list->first_drum->next_drum;
                return;
            }
            else {
                *health -= 2;\
                hit_color(2);
                return;
            }
        }
    }
}

int main(void)
{
    // Global Variable Initialize
    int mode = 2; // 0 if select help in the main menu; 1 if select play in the main menu; 2 if go back to main menu
    int health = HEALTH_BOUND; // health initialize 40
    u16 score = 0; // score initialize 1
    int hit_count = 0; // hit count initialize 0

    // Drum Initialize
    Drum drum[MAX_DRUM_NUM];
    Drum_list drum_list;
    drum_and_drum_list_init(INIT_SPEED, drum, &drum_list);
    
    // screen initialize
    IO_init();
    // Make screen full black
    LCD_Clear(BLACK);
    BACK_COLOR=BLACK;

    mode = 2;
    while (1) {
        // Scope Variable Initialize
        u32 loop_num = 0; // total loop time in mode 1(play), useful for speed change and add drum
        u16 lock = 1; // 0 means can load get_button; otherwise, get_button is locked
        /* --------------
         Description:  display main menu, press left button for 
                       selection of game, press right button for
                       selection of help document
        ---------------- */
        if (mode == 2) {
            display_main_menu();
            while(1) {
                rand();
                if (Get_Button(LEFT_BUTTON)) {
                    mode = 1;
                    break;
                }
                if (Get_Button(RIGHT_BUTTON)) {
                    mode = 0;
                    break;
                }
            }
        }
        /* --------------
         Description:  display help text, press right button 
                       to continue the next page until final
                       page, press boot to go back to the
                       main menu
        ---------------- */
        if (mode == 0) {
            display_help();
            mode = 2;
        }
        /* --------------
         Description: enter the game. Two colors TAIKO comes 
                      one after another in line, and a circle 
                      is at the end of the line. Red TAIKO 
                      corresponds to left button while Blue
                      TAIKO corresponds to right button.
                      Receive button signal. If corrsponding 
                      signal up when TAIKO in the circle, add 
                      score and health. Otherwise, lose health.
                      When health down to 0 or less, game over.
                      TAIKO comes faster with time.
        ---------------- */
        if (mode == 1) {
            LCD_Clear(BLACK);
            // load unchanging scene
            LCD_DrawLine(10, 30, 150, 30, WHITE);
            LCD_DrawLine(10, 31, 150, 31, WHITE);
            LCD_DrawLine(10, 61, 150, 62, WHITE);
            LCD_DrawLine(10, 62, 150, 63, WHITE);
            while (1) {
                display_health_and_score(health, score);
                // Drum Addition
                if (add_condition(loop_num)) {
                    add_drum(drum, &drum_list);
                }
                // Drum speed up
                loop_delay(score);
                // Drum data update
                drum_data_update(drum, &drum_list);
                // Drum display
                drum_display(drum, &drum_list);
                // Button Reaction
                if (Get_Button(LEFT_BUTTON) && !lock) {
                    press_left_button(&health, &score, &drum_list, &hit_count);
                    lock = 1;
                }
                if (Get_Button(RIGHT_BUTTON) && !lock) {
                    press_right_button(&health, &score, &drum_list, &hit_count);
                    lock = 1;
                }
                if (health >= 40) health = 40;
                // Edge Drum deletion
                edge_drum_delete(drum, &drum_list, &health);

                LCD_DrawCircle(38, 46, 14, WHITE);
                loop_num++;
                hit_count++;
                if (hit_count % 5 == 0) LCD_Fill(10, 63, 70, 80, BLACK);
                lock = ((lock + 1) % 13)*lock;
                // Game Over
                if (health <= 0) {
                    health = 0;
                    display_health_and_score(health, score);
                    while (1) {
                        u8 *p1 = (u8*)"Game Over";
                        LCD_ShowString(70, 60, p1, WHITE);
                        delay_1ms(2000);
                    }
                }
                /* test */
                // LCD_ShowNum(10, 60, loop_num, 5, WHITE);
                // health = 40;
            }
        }

    }
}
