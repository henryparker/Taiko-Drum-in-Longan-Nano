.data 
    str1: .string "Perfect"
    str2: .string "Great"
    str3: .string "Miss"
.section .text
.globl hit_color_assemble_RGB
.type hit_color_assemble_RGB, @function
# Function description: print color for different drum hit
#                              mode 0: prefect, display gold
#                              mode 1: great, display blue green
#                              mode 2: miss, display red
.text
hit_color_assemble:
    addi sp, sp, -20
    sw ra, 0(sp)
    sw a0, 4(sp)
    sw a1, 8(sp)
    sw a2, 12(sp)
    sw a3, 16(sp)

    
    # u8 radius_lim = 10;
    li t0, 10
    # mode == 0 go mode 0
    beq a0, x0, mode0;
    li t1, 1
    # mode == 1 go mode 1
    beq a0, t1, mode1;
    li t1, 2
    # mode == 2 go mode 2
    beq a0, t1, mode2;

mode0:
    # iterator
    li t1, 0
    # store variable
    addi sp, sp, -8
    sw t0, 0(sp)
    sw t1, 4(sp)

mode0_loop:
    # variable for LCD_DrawCircle
    li a0, 38
    li a1, 46
    mv a2, t1
    li a3, 0xFFF1
    # LCD_DrawCircle(38, 46, i, 0xFFF1);
    jal LCD_DrawCircle
    # variable for LCD_ShowString
    li a0, 10
    li a1, 63
    la a2, str1
    li a3, 0xFFF1
    # LCD_ShowString(10, 63, text1, 0xFFF1);
    jal LCD_ShowString

    # load variable
    lw t0, 0(sp)
    lw t1, 4(sp)
    addi sp, sp, 8

    # iterator adding
    addi t1, t1, 3
    # i < radius_lim go back mode0_loop
    blt t1, t0, mode0_loop # if a < b then target
    jal exit

mode1:
    # iterator
    li t1, 0
    # store variable
    addi sp, sp, -8
    sw t0, 0(sp)
    sw t1, 4(sp)

mode1_loop:
    # variable for LCD_DrawCircle
    li a0, 38
    li a1, 46
    mv a2, t1
    li a3, 0x0FEA
    # LCD_DrawCircle(38, 46, i, 0x0FEA);
    jal LCD_DrawCircle
    # variable for LCD_ShowString
    li a0, 20
    li a1, 63
    la a2, str2
    li a3, 0x0FEA
    # LCD_ShowString(20, 63, text2, 0x0FEA);
    jal LCD_ShowString

    # load variable
    lw t0, 0(sp)
    lw t1, 4(sp)
    addi sp, sp, 8

    # iterator adding
    addi t1, t1, 3
    # i < radius_lim go back mode1_loop
    blt t1, t0, mode1_loop # if a < b then target
    jal exit

mode2:
    # iterator
    li t1, 0
    # store variable
    addi sp, sp, -8
    sw t0, 0(sp)
    sw t1, 4(sp)

mode2_loop:
    # variable for LCD_DrawCircle
    li a0, 38
    li a1, 46
    mv a2, t1
    li a3, 0x47B8
    # LCD_DrawCircle(38, 46, i, 0x47B8);
    jal LCD_DrawCircle
    # variable for LCD_ShowString
    li a0, 20
    li a1, 63
    la a2, str3
    li a3, 0x47B8
    # LCD_ShowString(20, 63, text2, 0x47B8);
    jal LCD_ShowString

    # load variable
    lw t0, 0(sp)
    lw t1, 4(sp)
    addi sp, sp, 8

    # iterator adding
    addi t1, t1, 3
    # i < radius_lim go back mode2_loop
    blt t1, t0, mode2_loop # if a < b then target
    jal exit

exit:
    lw ra, 0(sp)
    lw a1, 8(sp)
    lw a2, 12(sp)
    lw a3, 16(sp)
    addi sp, sp, 20
    ret