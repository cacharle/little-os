typedef char           i8;
typedef short          i16;
typedef int            i32;
typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef i8             bool;

void a_out(u16 port, u8 data);
u8 a_in(u16 port);
// void a_lgdt(void *gdt);
void a_lgdt(u16 limit, u32 base);

void *memset(void *dst, u8 x, u32 size)
{
    u8 *dst_8 = dst;
    for (u32 i = 0; i < size; i++)
        dst_8[i] = x;
    return dst;
}

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

void serial_putu8_x(u16 com, u32 n)
{
    u8 b1 = (n >> 4) & 0xF;
    u8 b2 = (n >> 0) & 0xF;
    serial_putchar(com, b1 < 10 ? b1 + '0' : b1 - 10 + 'A');
    serial_putchar(com, b2 < 10 ? b2 + '0' : b2 - 10 + 'A');
}

void serial_putu32_x(u16 com, u32 n)
{
    for (int i = 0; i < 4; i++)
        serial_putu8_x(com, (n >> (8 * (3 - i))) & 0xFF);
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

struct gdt_descriptor {
    u32 address: 32;
    u16 size: 16;
};

struct gdt_entry {
    u16 limit1: 16; // maximum addressable unit
    u32 base1: 24; // address where segment begins

    u8 access_accessed: 1; // always clear
    u8 access_read_write: 1; // for code: can be read (can never write), for data: can be write (can always read)
    u8 access_direction_conforming: 1; // for data: if it grows up or down, for code: if can be executed by lower ring
    u8 access_executable: 1; // 0 = data segment, 1 = code segment
    u8 access_descriptor_type: 1; // 0 = task state segment, 1 = code or data segment
    u8 access_descriptor_privilege_level: 2; // 0-3, 0 is highest priority
    u8 access_present: 1; // always set to 1

    u8 limit2: 4;
    u8 _flags_reserved: 1;
    u8 flags_long_mode: 1; // 64-bit segment if set
    u8 flags_size: 1; // 0 = 16-bit segment, 1 = 32-bit segment
    u8 flags_granularity: 1; // 0 = limit is bytes, 1 = limit is 4K blocks
    u32 base2: 8;
} __attribute__((packed));

void gdt_entry_init(struct gdt_entry *entry, u32 base, u32 limit, u8 code_segment)
{
    memset(entry, 0, sizeof(struct gdt_entry));
    // assert limit <= 0xFFFFF
    entry->limit1 = limit & 0xFFFF;
    entry->limit2 = (limit >> 16) & 0x0F;

    entry->base1 = base & 0xFFFFFF;
    entry->base2 = (base >> 24) & 0xFF;

    entry->access_present = 1;
    entry->access_descriptor_privilege_level = 0;
    entry->access_descriptor_type = 1;
    entry->access_executable = code_segment;
    entry->access_direction_conforming = 0;
    entry->access_read_write = 1;
    entry->access_accessed = 0;

    entry->flags_granularity = 1;
    entry->flags_size = 1;
    entry->flags_long_mode = 0;
}

struct gdt_entry gdt[3] = {0};

struct GDT {
    u32 base;
    u32 limit;
    u32 flags;
    u32 access_byte;
};

// void encodeGdtEntry(u8 *target, struct GDT *source)
// {
//     // Check the limit to make sure that it can be encoded
//     // if (source.limit > 0xFFFFF) {kerror("GDT cannot encode limits larger than 0xFFFFF");}
//
//     // Encode the limit
//     target[0] = source->limit & 0xFF;
//     target[1] = (source->limit >> 8) & 0xFF;
//     target[6] = (source->limit >> 16) & 0x0F;
//
//     // Encode the base
//     target[2] = source->base & 0xFF;
//     target[3] = (source->base >> 8) & 0xFF;
//     target[4] = (source->base >> 16) & 0xFF;
//     target[7] = (source->base >> 24) & 0xFF;
//
//     // Encode the access byte
//     target[5] = source->access_byte;
//
//     // Encode the flags
//     target[6] |= (source->flags << 4);
// }

struct foo {
    u16 size;
    u32 address;
} __attribute__((packed));

int main(void)
{
    memset(&gdt[0], 0, sizeof(struct gdt_entry));
    gdt_entry_init(&gdt[1], 0x00400000, 0x003FFFFF, 1);
    gdt_entry_init(&gdt[2], 0x00800000, 0x003FFFFF, 0);
    // struct foo *lgdt_param = (struct foo*)0x800;
    // lgdt_param->address = (u32)gdt;
    // lgdt_param->size = sizeof(struct gdt_entry) * 3 - 1;
    // {
        // .address = (u32)gdt,
        // .size = sizeof(struct gdt_entry) * 3 - 1,
    // };
    // a_lgdt(lgdt_param);
    // a_lgdt(sizeof(gdt), (u32)gdt);

    // serial_configure_baud_rate(SERIAL_COM1_BASE, 2);
    // serial_configure_line(SERIAL_COM1_BASE);
    // char s[] =  "bonjour";
    // // serial_putstr(SERIAL_COM1_BASE, s);
    // // serial_putu32_x(SERIAL_COM1_BASE, 0xdead1234);
    // serial_putu32_x(SERIAL_COM1_BASE, sizeof(struct gdt_entry));
    // serial_putu32_x(SERIAL_COM1_BASE, sizeof(gdt));
    //
    // u8 *t = gdt;
    // for (int i = 0; i < sizeof(gdt); i++)
    //     serial_putchar(SERIAL_COM1_BASE, t[i]);
    // u8 flags = gdt[1].flags_size | gdt[1].flags_long_mode | gdt[1].flags_granularity;
    // serial_putchar(SERIAL_COM1_BASE, flags);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);
    // serial_putchar(SERIAL_COM1_BASE, 0x0);

    // fb_clear();
    // fb_putstr(0, 0, s);

    // u8 foo[8] = {0};
    // struct GDT g = {
    //     .base =  0x00400000,
    //     .limit = 0x003FFFFF,
    //     .flags = 0xC,
    //     .access_byte = 0x92,
    // };
    // encodeGdtEntry(foo, &g);
    // for (int i = 0; i < 8; i++)
    //     serial_putchar(SERIAL_COM1_BASE, foo[i]);

    return 0xdeadbeef;
}
