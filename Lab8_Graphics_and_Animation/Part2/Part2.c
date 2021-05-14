/*
Function: draw lines on screen with VGA output. Move the horizontal line up and down.
author: Cai Tianhong
date: 2021/5/13
*/

volatile int pixel_buffer_start; // global variable

/*declare*/
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int color);
void wait_for_vsync(); //swap front and back buffers on VGA vertical syncronization
int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*) 0xFF203020;
    /* Read the location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
    *(pixel_ctrl_ptr + 1) = pixel_buffer_start;
    clear_screen();
    draw_line(0, 0, 319, 0, 0x001F); // draw the top line in blue
    wait_for_vsync();
    int y = 0;
    while (1)
    {
        while (y <= 239)
        {
            draw_line(0, y, 319, y, 0); // earse the line
            y = y + 1;
            draw_line(0, y, 319, y, 0x001F); // draw the next line
            wait_for_vsync();
        }
        y = y - 1;
        while (y >= 0)
        {
            draw_line(0, y, 319, y, 0); // earse the line
            y = y - 1;
            draw_line(0, y, 319, y, 0x001F); // draw the next line
            wait_for_vsync();
        }
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
    while(*(pixel_ctrl_ptr + 3) & 0x1)
    {
        //wait here until swap
    }
}