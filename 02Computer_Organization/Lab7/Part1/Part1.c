/* declare functions used in main function */
void disable_A9_interrupts(void); // disable interrupts in the A9 processor
void set_A9_IRQ_stack(void); // initialize the stack pointer for IRQ mode
void config_GIC(void); // configure the general interrupt controller
void config_KEYs(void); // configure pushbutton KEYs to generate interrupts
void enable_A9_interrupts (void); // enable interrupts in the A9 processor

/* the main function */
int main(void)
{
    disable_A9_interrupts(); // disable interrupts in the A9 processor
    set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
    config_GIC(); // configure the general interrupt controller
    config_KEYs(); // configure pushbutton KEYs to generate interrupts
    enable_A9_interrupts (); // enable interrupts in the A9 processor
    // wait for an interrupt
    while (1)
    {}
}

/* disable interrupts in the A9 processor */
void disable_A9_interrupts(void)
{
    int status = 0b11010011; // 11: disable interrupte, 0: arm mode, 10011: supervisor mode
    asm(
        "msr cpsr, %[ps]"
        :
        :[ps] "r"(status)
        );
}

/* Initialize the banked stack pointer register for IRQ mode */
void set_A9_IRQ_stack(void)
{
    int stack, mode;
    stack = 0xFFFFFFFF - 7; // top of A9 onchip memory, aligned to 8 bytes
    /* change processor to IRQ mode with interrupts disabled */
    mode = 0b11010010; // 11: disable interrupte, 0: arm mode, 10010: IRQ mode
    asm(
        "msr cpsr, %[ps]"
        :
        : [ps] "r" (mode)
        );
    /* set banked stack pointer */
    asm(
        "mov sp, %[ps]"
        :
        : [ps] "r" (stack)
        );
    /* go back to SVC mode before executing subroutine return! */
    mode = 0b11010011;
    asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
}

/* Config Distributer part of GIC */
void config_interrupt(int N, int CPU_target)
{
    int reg_offset, index, value, address;
    /* Configure the Interrupt Set-Enable Registers (ICDISERn).
    reg_offset = (integer_div(N / 32) * 4
    value = 1 << (N mod 32) */
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    /* Now that we know the register address and value, set the appropriate bit */
    *(int *)address |= value;
    /* Configure the Interrupt Processor Targets Register (ICDIPTRn)
    reg_offset = integer_div(N / 4) * 4
    index = N mod 4 */
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    /* Now that we know the register address and value, write to (only) the
    appropriate byte */
    *(char *)address = (char)CPU_target;
}

/* Configure the Generic Interrupt Controller (GIC) */
void config_GIC(void)
{
    config_interrupt (73, 1); // configure the FPGA KEYs interrupt (73)
    // Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all priorities
    *((int *) 0xFFFEC104) = 0xFFFF;
    // Set CPU Interface Control Register (ICCICR). Enable signaling of interrupts
    *((int *) 0xFFFEC100) = 1;
    // Configure the Distributor Control Register (ICDDCR) to send pending interrupts to CPUs
    *((int *) 0xFFFED000) = 1;
}

/* Set up the pushbutton KEYs port in the FPGA */
void config_KEYs(void)
{
    volatile int * KEY_ptr = (int *) 0xFF200050; // pushbutton KEY base address
    *(KEY_ptr + 2) = 0xF; // enable interrupts for the four KEYs
}

/* Turn on interrupts in the ARM processor */
void enable_A9_interrupts(void)
{
    int status = 0b01010011; // 01: enable interrupte, 0: arm mode, 10011: supervisor mode
    asm(
        "msr cpsr, %[ps]"
        :
        :[ps] "r"(status)
        );
}

/* Define the IRQ exception handler */
void pushbutton_ISR(void); //declare
void __attribute__((interrupt)) __cs3_isr_irq(void)
{
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *)0xFFFEC10C);
    if (interrupt_ID == 73) // check if interrupt is from the KEYs
    {
        pushbutton_ISR();
    }
    else
    {
        while (1); // if unexpected, then stay here
    }
    // Write to the End of Interrupt Register (ICCEOIR)
    *((int *)0xFFFEC110) = interrupt_ID;
}
void pushbutton_ISR(void)
{
    /* display of the HEX */
    char number[4] = {0b00111111, 0b00000110, 0b01011011, 0b01001111};
    char blank = 0b00000000;
    /* KEY base address */
    volatile int* KEY_ptr = (int*) 0xFF200050;
    /* HEX display base address */
    volatile char* HEX0_ptr = (char*) 0xFF200020;
    volatile char* HEX1_ptr = HEX0_ptr + 1;
    volatile char* HEX2_ptr = HEX0_ptr + 2;
    volatile char* HEX3_ptr = HEX0_ptr + 3;
    int press;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    if (press & 0x1) // KEY0
    {
        if (*HEX0_ptr == blank)
        *HEX0_ptr = number[0];
        else
        *HEX0_ptr = blank;
    }
    else if (press & 0x2) // KEY1
    {
        if (*HEX1_ptr == blank)
        *HEX1_ptr = number[1];
        else
        *HEX1_ptr = blank;
    }
    else if (press & 0x4) // KEY2
    {
        if (*HEX2_ptr == blank)
        *HEX2_ptr = number[2];
        else
        *HEX2_ptr = blank;
    }
    else // press & 0x8, which is KEY3
    {
        if (*HEX3_ptr == blank)
        *HEX3_ptr = number[3];
        else
        *HEX3_ptr = blank;
    }
}

// Define the remaining exception handlers
void __attribute__((interrupt)) __cs3_reset(void) {
while (1);
}
void __attribute__((interrupt)) __cs3_isr_undef(void) {
while (1);
}
void __attribute__((interrupt)) __cs3_isr_swi(void) {
while (1);
}
void __attribute__((interrupt)) __cs3_isr_pabort(void) {
while (1);
}
void __attribute__((interrupt)) __cs3_isr_dabort(void) {
while (1);
}
void __attribute__((interrupt)) __cs3_isr_fiq(void) {
while (1);
}