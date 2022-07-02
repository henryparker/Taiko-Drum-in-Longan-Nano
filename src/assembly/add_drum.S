.section .text
.globl add_drum_assemble
.type add_drum_assemble, @function

#typedef struct Drum_list {
### the first drum which is displayd in the screen, NULL if no drum has been displayd
#    Drum* first_drum; 
### the last drum of list which displayd in the screen; NULL if no drum has been displayd
#    Drum* last_displayd_drum; 
### record total drum number in the list
#    u16 display_num; 
### record drum number that is in the screen
#    u16 drum_mum; 
#} Drum_list;

add_drum:
    addi sp, sp, -4
    sw ra, 0(sp)

    # drum_list->first_drum
    lw t0, 0(a1) 
    # drum_list->last_displayd_drum
    lw t1, 4(a1) 
    # drum_mum
    lh t2, 6(a1) 
    # display_num
    lh t3, 8(a1) 
   
    #Drum* temp = drum_list->first_drum;
    mv t4, t0
    #typedef struct Drum {
    #int mode; // 0 if the drum is red; 1 if the drum is blue
    #int speed; // object moves pixel per loop, initialize 1
    #struct Drum* next_drum; // record the next drum of the current drum in the Drum List, if it is the end, point to the head in the list
    #// bool is_displayd; // 1 if the Drum is displayed in the screen; 0 otherwise
    #int left_most_position; // record the Drum current left most x coordinate
    #} Drum;
    #int i  = 0
    li t5, 0 
    bge t5, t3, exit 
      #temp->left_most_position -= temp->speed;
      lw t6, 4(t4)
      lw s0, 12(t4)
      sub s0, s0, t6
      sw s0, 12(t4)
      ###if (temp->left_most_position < 1)
      li t6, 1
      bge s0, t6, exit2
      #temp->left_most_position = 1;
        li s0, 1
        sw s0, 12(t4)
      exit2:
      #temp = temp->next_drum;
      lw t4, 8(t4)
exit:
    lw ra, 0(sp)
    addi sp, sp, 4
    ret

