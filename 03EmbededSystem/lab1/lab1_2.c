#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../address_map_arm.h"
/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void * map_physical (int, unsigned int, unsigned int);
void close_physical (int);
int unmap_physical (void *, unsigned int);
void delay(int);

/* This program increments the contents of the red LED parallel port */
int main(void)
{
    volatile int * LEDR_ptr; // virtual address pointer to red LEDs
    int fd = -1; // used to open /dev/mem
    void *LW_virtual; // virtual addresses for light-weight bridge
    // Create virtual memory access to the FPGA light-weight bridge
    if ((fd = open_physical (fd)) == -1)
        return (-1);
    if (!(LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)))
        return (-1);
    
    // Set virtual address pointer to I/O port
    printf("LW_virtual address: %p\n",LW_virtual);
    LEDR_ptr = LW_virtual + (unsigned long)LEDR_BASE;
    printf("LEDR address: %p\n",LEDR_ptr);

    // the process of control
    *LEDR_ptr=1;
    while(1)
    {
        int i=0;
        while(i<=8)
        {
            delay(2);
            *LEDR_ptr=*LEDR_ptr+pow(2,i);
            i=i+1;
        }
        i=8;
        while (i>=0)
        {
            delay(2);
            *LEDR_ptr=*LEDR_ptr-pow(2,i);
            i=i-1;
        }
    }

    unmap_physical (LW_virtual, LW_BRIDGE_SPAN);
    close_physical (fd);
    return 0;
}

/* Open /dev/mem to give access to physical addresses */
int open_physical (int fd)
{
    if (fd == -1) // check if already open
    if ((fd = open( "/dev/mem", (O_RDWR | O_SYNC))) == -1)
    {
        printf ("ERROR: could not open \"/dev/mem\"...\n");
        return (-1);
    }
    printf("open mem sucess\n");
    return fd;
}

/* Close /dev/mem to give access to physical addresses */
void close_physical (int fd)
{
    close (fd);
}
/* Establish a virtual address mapping for the physical addresses starting
* at base and extending by span bytes */
void* map_physical(int fd, unsigned int base, unsigned int span)
{
    void *virtual_base;
    // Get a mapping from physical addresses to virtual addresses
    virtual_base = mmap (NULL, span, (PROT_READ | PROT_WRITE), MAP_SHARED,fd, base);
    if (virtual_base == MAP_FAILED)
    {
        printf ("ERROR: mmap() failed...\n");
        close (fd);
        return (NULL);
    }
    printf("get virtual_base sucess\n");
    printf("virtual base address: %p\n",virtual_base);
    return virtual_base;
}
int unmap_physical(void * virtual_base, unsigned int span)
{
    if (munmap (virtual_base, span) != 0)
    {
        printf ("ERROR: munmap() failed...\n");
        return (-1);
    }
    printf("unmap sucess\n");
    return 0;
}

/*delay function*/
void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds; 

    // Storing start time 
    clock_t start_time = clock(); 

    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
} 