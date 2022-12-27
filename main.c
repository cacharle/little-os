typedef char           i8;
typedef short          i16;
typedef int            i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

void a_out(u16 port, u8 data);

u16 *fb = (u16*)0x000B8010;

#define FB_BLACK         0
#define FB_BLUE          1
#define FB_GREEN         2
#define FB_CYAN          3
#define FB_RED           4
#define FB_MAGENTA       5
#define FB_BROWN         6
#define FB_LIGHT_GREY    7
#define FB_DARK_GREY     8
#define FB_LIGHT_BLUE    9
#define FB_LIGHT_GREEN   10
#define FB_LIGHT_CYAN    11
#define FB_LIGHT_RED     12
#define FB_LIGHT_MAGENTA 13
#define FB_LIGHT_BROWN   14
#define FB_WHITE         15

#define FB_COMMAND_PORT      0x3D4
#define FB_DATA_PORT         0x3D5
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

void fb_move_cursor(u16 pos)
{
    a_out(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    a_out(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    a_out(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    a_out(FB_DATA_PORT, (pos & 0x00FF));
}

void fb_putchar(u32 y, u32 x, i8 c, u8 fg, u8 bg)
{
    const u32 fb_width = 80;  // can't have global?
    // const u32 fb_height = 25;
    const u32 pos = y * fb_width + x;
    fg &= 0xFF;
    bg &= 0xFF;
    fb[pos] = c | (((bg << 4) | fg) << 8);
}


int main(void)
{
    fb_putchar(0, 0, 'A', FB_GREEN, FB_BLACK);
    fb_putchar(1, 1, 'B', FB_GREEN, FB_BLACK);
    fb_putchar(2, 2, 'C', FB_GREEN, FB_BLACK);
    fb_putchar(3, 3, 'D', FB_GREEN, FB_BLACK);
    fb_putchar(4, 4, 'E', FB_GREEN, FB_BLACK);

    // fb_move_cursor(100);

    // fb[0] = 'A';
    // fb[1] = 0x28;

    return 0xdeadbeef;
}
