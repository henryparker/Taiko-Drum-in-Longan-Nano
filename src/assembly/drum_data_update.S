.section .text
.globl drum_data_update_assemble
.type drum_data_update_assemble, @function
drum_data_update:
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
    ###if (drum_list->drum_mum == drum_list->display_num) return;
    beq t2, t3, exit
      ###if (drum_list->first_drum == NULL)
      bne t0, x0, else1
        #drum_list->first_drum = drum;
        sw a0, 0(a1)
        #drum_list->last_displayd_drum = drum;
        sw a0, 4(a1)      
      else1:
        #drum_list->last_displayd_drum->next_drum
        addi t4, t1, 8
        #drum_list->last_displayd_drum = drum_list->last_displayd_drum->next_drum;
        sw t4, 4(a1)
    #drum_list->display_num++;
    addi t3, t3, 1
    sw t3, 8(a1)
    ###endif
exit:
    lw ra, 0(sp)
    addi sp, sp, 4
    ret

    