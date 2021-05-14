/*
Function: draw eight rectangles on the screen and connect them. Move the rectangles on diagonal
author: Cai Tianhong
date: 2021/5/13
*/

volatile int pixel_buffer_start; // global variable

void clear_screen();
void wait_for_vsync (); // swap front and back buffers on VGA vertical sync
void draw_line(int x0, int y0, int x1, int y1, short int color);
int main(void)
{
    volatile int * pixel_ctrl_ptr = (int *) 0xFF203020;
    /* initialize the location of the front pixel buffer in the pixel buffer controller */
    *pixel_ctrl_ptr = 0xC8000000;
    /* Set a location for the pixel back buffer in the pixel buffer controller */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    /* clear the front pixel buffer */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    /* we draw on the back buffer */
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    /* initialize the eight boxs and lines*/
    int y = 0;
    /* Erase any boxes and lines that were drawn in the last iteration */
    while (1)
    {
        while (y <= 239)
        {
            clear_screen(); // pixel buffer start points to the pixel buffer
            y = y + 1;
            draw_line(0, y, 319, y, 0x001F); // draw the next line
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // update back buffer pointer
        }
        y = y - 1;
        while (y >= 0)
        {
            clear_screen(); // pixel buffer start points to the pixel buffer
            y = y - 1;
            draw_line(0, y, 319, y, 0x001F); // draw the next line
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // update back buffer pointer
        }
        y = y + 1;
    }
    return 0;
}

/* clear the screen*/
void plot_pixel(int x, int y, short int color);
void clear_screen()
{
    int x = 0;
    while (x < 320)
    {
        int y = 0;
        while(y < 240)
        {
            plot_pixel(x, y, 0);
            y = y + 1;
        }
        x = x + 1;
    }
}

/* draw a line on the screen
(x0,y0) is the coordinate of the first point
(x1,y1) is the coordinate of the second point*/
void plot_pixel(int x, int y, short int color);
void draw_line(int x0, int y0, int x1, int y1, short int color)
{
    //calculate the slope
    float slope = ((float)y1-(float)y0)/((float)x1-(float)x0);
    if (slope <= 1 || slope >=-1)
    {
        // move along x axis
        if(x0 <= x1)
        {
            int x = x0;
            int y;
            while (x <= x1)
            {
                y = y0 + slope * (x - x0);
                plot_pixel(x,y,color);
                x = x + 1;
            }
        }
        else
        {
            int x = x0;
            int y;
            while (x >= x1)
            {
                y = y0 + slope * (x - x0);
                plot_pixel(x,y,color);
                x = x - 1;
            }
        }
    }
    else
    {
        // move along y axis
        if(y0 <= y1)
        {
            int y = y0;
            int x;
            while (y <= y1)
            {
                x = x0 + (y - y0)/slope;
                plot_pixel(x,y,color);
                y = y + 1;
            }
        }
        else
        {
            int y = y0;
            int x;
            while (y >= y1)
            {
                x = x0 + (y - y0)/slope;
                plot_pixel(x,y,color);
                y = y - 1;
            }
        }
    }
}

/*plot a pixel on the screen*/
void plot_pixel(int x, int y, short int color)
{
    short int* pixel_address;
    pixel_address = (short int*)(pixel_buffer_start + (y << 10) + (x << 1));
    *pixel_address = color;
}

/* swap front and back buffers on VGA vertical syncronization */
void wait_for_vsync()
{
    volatile int* pixel_ctrl_ptr = (int*) 0xFF203020;
    *pixel_ctrl_ptr = 0x1;
    while (*(pixel_ctrl_ptr + 3) & 0x1)
    {
        // wait here, continue when show the back buffer picture
    }
}