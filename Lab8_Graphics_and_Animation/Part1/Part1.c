/*
Function: draw lines on screen with VGA output
author: Cai Tianhong
date: 2021/5/12 
*/

volatile int pixel_buffer_start; // global variable

/*declare*/
void clear_screen();
void draw_line(int x0, int y0, int x1, int y1, short int color);
int main(void)
{
    volatile int* pixel_ctrl_ptr = (int*) 0xFF203020;
    /* Read the location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();
    draw_line(0, 0, 319, 239, 0x001F); // this line is blue
    draw_line(0, 239, 319, 0, 0x07E0); // this line is green
    draw_line(0, 0, 100, 239, 0xF800); // this line is red
    draw_line(0, 239, 100, 0, 0xF81F); // this line is a pink color
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
    float slope = (y1-y0)/(x1-x0);
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