typedef char           i8;
typedef short          i16;
typedef int            i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef i8             bool;

void a_out(u16 port, u8 data);
u8 a_in(u16 port);

u16 *fb = (u16*)0x000B8000;

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

#define SERIAL_COM1_BASE 0x3F8

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

#define SERIAL_LINE_ENABLE_DLAB 0x80

void fb_move_cursor(u16 pos)
{
    a_out(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    a_out(FB_DATA_PORT, ((pos >> 8) & 0x00FF));
    a_out(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    a_out(FB_DATA_PORT, (pos & 0x00FF));
}

// internal clock is 115200 Hz
// give a divisor for the clock rate (i.e, higher divisor, lower baud rate)
void serial_configure_baud_rate(u16 com, u16 divisor)
{
    a_out(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    a_out(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
    a_out(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}


// d    = dlab enabled
// b    = break control
// prty = number of parity bits
// s    = number of stop bits (0 => 1bit, 1 => 2bits)
// dl   = data length
// | d | b | prty  | s | dl  |
// | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
//
//
void serial_configure_line(u16 com)
{
    a_out(SERIAL_LINE_COMMAND_PORT(com), 0x03);
    a_out(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
    a_out(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

bool serial_is_transmit_fifo_empty(u16 com)
{
    return (a_in(SERIAL_LINE_STATUS_PORT(com)) & 0x20) != 0;
}

void serial_putchar(u16 com, char c)
{
    while (!serial_is_transmit_fifo_empty(com))
        ;
    a_out(SERIAL_DATA_PORT(com), c);
}

void serial_putstr(u16 com, const char *s)
{
    for (; *s != '\0'; s++)
        serial_putchar(com, *s);

}

#define FB_WIDTH 80  // can't have global?
#define FB_HEIGHT 25

void fb_putchar_c(u32 y, u32 x, char c, u8 fg, u8 bg)
{
    const u32 pos = y * FB_WIDTH + x;
    fg &= 0xFF;
    bg &= 0xFF;
    fb[pos] = c | (((bg << 4) | fg) << 8);
}

void fb_putchar(u32 y, u32 x, char c)
{
    fb_putchar_c(y, x, c, FB_WHITE, FB_BLACK);
}

void fb_putstr_c(u32 y, u32 x, const char *s, u8 fg, u8 bg)
{
    for (; *s != '\0'; s++, x++)
        fb_putchar_c(y, x, *s, fg, bg);
}

void fb_putstr(u32 y, u32 x, const char *s)
{
    fb_putstr_c(y, x, s, FB_WHITE, FB_BLACK);
}

void fb_clear(void)
{
    for (u32 i = 0; i < FB_HEIGHT; i++)
        for (u32 j = 0; j < FB_WIDTH; j++)
        {
            fb_putchar(i, j, ' ');
        }
}


int main(void)
{
    serial_configure_baud_rate(SERIAL_COM1_BASE, 2);
    serial_configure_line(SERIAL_COM1_BASE);
    char s[] =  "bonjour";
    serial_putstr(SERIAL_COM1_BASE, s);

    fb_clear();
    fb_putstr(0, 0, s);
    // fb_putchar(0, 0, 'A');
    // fb_putchar(1, 1, 'B');
    // fb_putchar(2, 2, 'C');
    // fb_putchar(3, 3, 'D');
    // fb_putchar(4, 4, 'E');

    // fb_move_cursor(100);

    // fb[0] = 'A';
    // fb[1] = 0x28;

    return 0xdeadbeef;
}
