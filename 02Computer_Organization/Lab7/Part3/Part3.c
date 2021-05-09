/* declare functions used in main function */
void disable_A9_interrupts(void); // disable interrupts in the A9 processor
void set_A9_IRQ_stack(void); // initialize the stack pointer for IRQ mode
void config_GIC(void); // configure the general interrupt controller
void config_KEYs(void); // configure pushbutton KEYs to generate interrupts
void config_HPS_timer(void); // configure HPS Timer 0
void enable_A9_interrupts (void); // enable interrupts in the A9 processor

int count = 0; // global counter for red lights
int run = 1; // global, used to increment/not the count variable

/* the main function */
int main(void)
{
    disable_A9_interrupts(); // disable interrupts in the A9 processor
    set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
    config_GIC(); // configure the general interrupt controller
    config_KEYs(); // configure pushbutton KEYs to generate interrupts
    config_HPS_timer (); // configure HPS Timer 0
    enable_A9_interrupts (); // enable interrupts in the A9 processor
    volatile int* LEDR_ptr = (int*) 0xFF200000;
    // wait for an interrupt
    while (1)
    {
        *LEDR_ptr = count;
    }
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
    /* step 1: Configure the Interrupt Set-Enable Registers (ICDISERn).
    reg_offset = (integer_div(N / 32) * 4
    value = 1 << (N mod 32) */
    reg_offset = (N >> 3) & 0xFFFFFFFC;
    index = N & 0x1F;
    value = 0x1 << index;
    address = 0xFFFED100 + reg_offset;
    *(int*)address = *(int*)address | value;
    /* step 2: Configure the Interrupt Processor Targets Register (ICDIPTRn)
    reg_offset = integer_div(N / 4) * 4
    index = N mod 4 */
    reg_offset = (N & 0xFFFFFFFC);
    index = N & 0x3;
    address = 0xFFFED800 + reg_offset + index;
    *(char *)address = (char)CPU_target;
}

/* Configure the Generic Interrupt Controller (GIC) */
void config_GIC(void)
{
    config_interrupt (73, 1); // configure the FPGA KEYs interrupt (73)
    config_interrupt (199, 1); // configure the FPGA KEYs interrupt (73)
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
    *(KEY_ptr + 2) = 0xF; // enable interrupts for KEY0 to KEY3
}

/* configure HPS Timer 0 */
void config_HPS_timer(void)
{
    volatile int* HPS_timer_ptr = (int*) 0xFFC08000;
    // stop the timer
    *(HPS_timer_ptr + 2) = 0;
    // configure the Load register
    *HPS_timer_ptr = 0x017D7840;
    // configure the control register
    *(HPS_timer_ptr + 2) = 0x2;
    //start the timer
    *(HPS_timer_ptr + 2) = 0x3;
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
void HPS_timer_ISR(void); //declare
void __attribute__((interrupt)) __cs3_isr_irq(void)
{
    // Read the ICCIAR from the CPU Interface in the GIC
    int interrupt_ID = *((int *)0xFFFEC10C);
    if (interrupt_ID == 73) // check if interrupt is from the KEYs
    {
        pushbutton_ISR();
    }
    else if (interrupt_ID == 199)
    {
        HPS_timer_ISR();
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
    volatile int * KEY_ptr = (int *) 0xFF200050;
    int press;
    press = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press; // Clear the interrupt
    if (press & 0x1) // KEY0
    {
        if(run == 1)
        run = 0;
        else
        run = 1;
    }
    else if (press & 0x2) // KEY1, increment double
    {
        volatile int* HPS_timer_ptr = (int*) 0xFFC08000;
        // stop the timer
        *(HPS_timer_ptr + 2) = 0;
        // configure the Load register
        *HPS_timer_ptr = (*HPS_timer_ptr)/2;
        //start the timer
        *(HPS_timer_ptr + 2) = 0x3;
    }
    else if (press & 0x4) // KEY2, increment half
    {
        volatile int* HPS_timer_ptr = (int*) 0xFFC08000;
        // stop the timer
        *(HPS_timer_ptr + 2) = 0;
        // configure the Load register
        *HPS_timer_ptr = (*HPS_timer_ptr)*2;
        //start the timer
        *(HPS_timer_ptr + 2) = 0x3;
    }
}
void HPS_timer_ISR(void)
{
    if (run == 1)
    {
        count = count + 1;
    }
    volatile int * HPS_timer_ptr = (int*) 0xFFC08000; // HPS timer address
    *(HPS_timer_ptr + 3); // Read timer end of interrupt register to clear the interrupt
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