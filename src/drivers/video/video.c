#include "video.h"
#include "mailbox/mailbox.h"
#include "log.h"
#include <sys/timer.h>
#include "mm.h"
#include "../dma/dmachannel.h"
#include "../../log/printf.h"

#define PROPTAG_END			0x00000000

#define PROPTAG_GET_FIRMWARE_REVISION	0x00000001
#define PROPTAG_SET_CURSOR_INFO		0x00008010
#define PROPTAG_SET_CURSOR_STATE	0x00008011
#define PROPTAG_GET_BOARD_MODEL		0x00010001
#define PROPTAG_GET_BOARD_REVISION	0x00010002
#define PROPTAG_GET_MAC_ADDRESS		0x00010003
#define PROPTAG_GET_BOARD_SERIAL	0x00010004
#define PROPTAG_GET_ARM_MEMORY		0x00010005
#define PROPTAG_GET_VC_MEMORY		0x00010006
#define PROPTAG_SET_POWER_STATE		0x00028001
#define PROPTAG_GET_CLOCK_RATE		0x00030002
#define PROPTAG_GET_MAX_CLOCK_RATE	0x00030004
#define PROPTAG_GET_TEMPERATURE		0x00030006
#define PROPTAG_GET_MIN_CLOCK_RATE	0x00030007
#define PROPTAG_GET_TURBO		0x00030009
#define PROPTAG_GET_MAX_TEMPERATURE	0x0003000A
#define PROPTAG_GET_EDID_BLOCK		0x00030020
#define PROPTAG_SET_CLOCK_RATE		0x00038002
#define PROPTAG_SET_TURBO		0x00038009
#define PROPTAG_SET_SET_GPIO_STATE	0x00038041
#define PROPTAG_ALLOCATE_BUFFER		0x00040001
#define PROPTAG_GET_DISPLAY_DIMENSIONS	0x00040003
#define PROPTAG_GET_PITCH		0x00040008
#define PROPTAG_GET_DEPTH   0x00044005
#define PROPTAG_GET_TOUCHBUF		0x0004000F
#define PROPTAG_GET_GPIO_VIRTBUF	0x00040010
#define PROPTAG_SET_PHYS_WIDTH_HEIGHT	0x00048003
#define PROPTAG_SET_VIRT_WIDTH_HEIGHT	0x00048004
#define PROPTAG_SET_DEPTH		0x00048005
#define PROPTAG_SET_VIRTUAL_OFFSET	0x00048009
#define PROPTAG_SET_PALETTE		0x0004800B
#define PROPTAG_WAIT_FOR_VSYNC		0x0004800E
#define PROPTAG_SET_BACKLIGHT		0x0004800F
#define PROPTAG_SET_TOUCHBUF		0x0004801F
#define PROPTAG_SET_GPIO_VIRTBUF	0x00048020
#define PROPTAG_GET_COMMAND_LINE	0x00050001
#define PROPTAG_GET_DMA_CHANNELS	0x00060001

typedef struct {
    dword brushColor;
    dword backColor;
    dword textColor;
    dword width;
    dword height;
    dword bpp;
    dword pitch;
    dword size;
    dword *base;
} ScreenInfo;

static ScreenInfo screenInfo = {
    .brushColor = 0x00000000,
    .backColor = 0x00000000,
    .textColor = 0xFFFFFFFF,
    .width = 1824,
    .height = 984,
    .bpp = 32,
    .pitch = 984 * 4
};

#define SCREEN_WIDTH screenInfo.width
#define SCREEN_HEIGHT screenInfo.height

typedef struct {
	MailboxTag tag1;
	dword xres, yres;
	MailboxTag tag2;
	dword xres_virtual, yres_virtual;
	MailboxTag tag3;
	dword bpp;
	MailboxTag tag5;
	dword base, screen_size;
	MailboxTag tag6;
	dword pitch;
} FrameBufferRequest;

static FrameBufferRequest fbinfo __attribute__((aligned(16))) = {
    .tag1 = { PROPTAG_SET_PHYS_WIDTH_HEIGHT,
            8, 0, },
        .xres = 1824,
        .yres = 984,
    .tag2 = { PROPTAG_SET_VIRT_WIDTH_HEIGHT,
            8, 0, },
        .xres_virtual = 1824,
        .yres_virtual = 984,
    .tag3 = { PROPTAG_SET_DEPTH, 4, 4 },
        .bpp = 32,
    .tag5 = { PROPTAG_ALLOCATE_BUFFER, 8, 4 },
        .base = 0,
        .screen_size = 0,
    .tag6 = { PROPTAG_GET_PITCH, 4, 4 },
        .pitch = 0,
};

static FrameBufferRequest fbinfoDEFAULT __attribute__((aligned(16))) = {
    .tag1 = { PROPTAG_SET_PHYS_WIDTH_HEIGHT,
            8, 8 },
        .xres = 1824,
        .yres = 984,
    .tag2 = { PROPTAG_SET_VIRT_WIDTH_HEIGHT,
            8, 8 },
        .xres_virtual = 1824,
        .yres_virtual = 984,
    .tag3 = { PROPTAG_SET_DEPTH, 4, 4 },
        .bpp = 32,
    .tag5 = { PROPTAG_ALLOCATE_BUFFER, 8, 4 },
        .base = 16,
        .screen_size = 0,
    .tag6 = { PROPTAG_GET_PITCH, 4, 4 },
        .pitch = 0,
};

//static FrameBufferRequest fbinfo;

byte *screenRed;
byte *sreenBlack;
byte *screenBlue;
byte *screenWhite;
byte *screenGreen;
DmaChannel *ch;
byte *preBuffer;

void do_delay() {
    volatile int n = 0;
    volatile int i = 0;
    for (; n < 3000000; n++) {
        i--;
    }

}

dword video_get_color(VColor color) {
    if (screenInfo.bpp == 16) {
        switch(color) {
            case Black: return 0x0000;
            case Blue: return 0x0010;
            case Red: return 0x8000;
            case Magenta: return 0x8010;
            case Green: return 0x0400;
            case Cyan: return 0x0410;
            case Yellow: return 0x8400;
            case White: return 0x8410;
            case BrightBlue: return 0x001F;
            case BrightRed: return 0xF800;
            case BrightMagenta: return 0xF81F;
            case BrightGreen: return 0x07E0;
            case BrightCyan: return 0x07FF;
            case BrightYellow: return 0xFFE0;
            case BrightWhite: return 0xFFFF;
        }
    } else {
        switch(color) {
            case Black: return 0x00000000;
            case Blue: return 0x0000AA00;
            case Red: return 0xAA000000;
            case Magenta: return 0xCC000000;
            case Green: return 0x00AA0000;
            case Cyan: return 0x00AA0000;
            case Yellow: return 0xCCCC0000;
            case White: return 0xFFFFFFFF;
            case BrightBlue: return 0x0000FF00;
            case BrightRed: return 0xFF000000;
            case BrightMagenta: return 0xFF0F0F00;
            case BrightGreen: return 0x00FF0000;
            case BrightCyan: return 0x00FFF000;
            case BrightYellow: return 0xFFFF0000;
            case BrightWhite: return 0xFFFFFFFF;
        }
    }
}

void video_init() {
    ch = dma_open_channel(DCNormal);
    preBuffer = (allocPage() + 0x00F00000);
/*
    video_set_text_color(Yellow);

    video_set_resolution(480, 320, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 1", 100, 100);
    do_delay();

    video_set_resolution(640, 480, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 2", 100, 100);
    do_delay();

    video_set_resolution(1024, 768, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 3", 100, 100);
    do_delay();

    video_set_resolution(1600, 900, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 4", 100, 100);
    do_delay();

    video_set_resolution(1824, 984, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 5", 100, 100);
    do_delay();

    video_set_resolution(3840, 2160, 16);
    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(Black);
    video_draw_string("VIDEO TEST 6", 100, 100);
    do_delay();

    video_clear_screen(Black);
    video_clear_screen(White);
    video_clear_screen(Red);
    video_clear_screen(Green);
    video_clear_screen(Blue);
    video_clear_screen(BrightBlue);
    video_clear_screen(BrightRed);
    video_clear_screen(Cyan);
    video_clear_screen(Magenta);
    video_clear_screen(Black);

    video_clear_screen(0);
    video_draw_string("READY TO GO", 100, 100);
    do_delay();
    do_delay();
    do_delay();
*/

}

void video_set_text_color(VColor vcolor) {
    screenInfo.textColor = video_get_color(vcolor);
}

void video_scroll(dword y) {
    byte *p = (byte *)fbinfo.base;
    dma_setup_mem_copy(ch, p, p + (y * SCREEN_WIDTH * screenInfo.bpp), fbinfo.screen_size - (y * SCREEN_WIDTH * screenInfo.bpp), 2, false);
    dma_start(ch);
    dma_wait(ch);
}

void video_clear_screen(VColor vcolor) {
    dword color = video_get_color(vcolor);
    int pixel_offset = 0;
    int pitch = screenInfo.pitch; //4 * SCREEN_HEIGHT; //5120;
    screenInfo.backColor = color;

    int r = (color & 0xFF000000) >> 24;
    int g = (color & 0x00FF0000) >> 16;
    int b = (color & 0x0000FF00) >> 8;
    int a = 0xFF;
    
    for(int y = 0; y < SCREEN_HEIGHT; y++ )
    {
        for(int x = 0; x < SCREEN_WIDTH; x++ ) {
            pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );

            if (screenInfo.bpp == 32) {
                preBuffer[ pixel_offset++ ] = r;
                preBuffer[ pixel_offset++ ] = g;
                preBuffer[ pixel_offset++ ] = b;
                preBuffer[ pixel_offset++ ] = a;
            } else {
                preBuffer[ pixel_offset++ ] = (color >> 8) & 0xFF;
                preBuffer[ pixel_offset++ ] = color & 0xFF;
            }
        }
    }

    byte *p = (byte *)fbinfo.base;
    int total = fbinfo.screen_size;
    dword start = 0;

    while(total > 0) {
        int numBytes = total;

        if (numBytes > 0xFFFFFF) {
            numBytes = 0xFFFFFF;
        }

        dma_setup_mem_copy(ch, p + start, preBuffer + start, numBytes, 2, false);
        dma_start(ch);
        dma_wait(ch);

        start += 0xFFFFFF;
        total -= 0xFFFFFF;
    }
}

void draw_pixel(int x, int y, dword color) {
    int pitch = screenInfo.pitch; //4 * SCREEN_HEIGHT; //5120;
    int pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );
    volatile unsigned char* fb = (unsigned char*)((fbinfo.base | 0x40000000) & ~0xc0000000);

    int r = (color & 0xFF000000) >> 24;
    int g = (color & 0x00FF0000) >> 16;
    int b = (color & 0x0000FF00) >> 8;
    int a = 0xFF;
    
    if (screenInfo.bpp == 32) {
        fb[ pixel_offset++ ] = r;
        fb[ pixel_offset++ ] = g;
        fb[ pixel_offset++ ] = b;
        fb[ pixel_offset++ ] = a;
    } else {
        fb[ pixel_offset++ ] = (color >> 8) & 0xFF;
        fb[ pixel_offset++ ] = color & 0xFF;
    }
}

void video_update_info() {

    MailboxMessage mbx;
    MailboxTransfer mt __attribute__((aligned(16)));
    mt.tagData.tagId = PROPTAG_GET_DISPLAY_DIMENSIONS;
    mt.tagData.bufferSize = 8;
    mt.tagData.valueLength = 0;
    ((dword *)mt.transferData)[0] = 0;
    ((dword *)mt.transferData)[1] = 0;


    if (!mailbox_process(0, (MailboxTag *)&mt, 12 + 8)) {
        log_println("FAILED video_update_info 1");
        return;
    }

    screenInfo.width = ((dword *)mt.transferData)[0];
    screenInfo.height = ((dword *)mt.transferData)[1];

    mt.tagData.tagId = PROPTAG_GET_PITCH;
    mt.tagData.bufferSize = 4;
    mt.tagData.valueLength = 0;
    ((dword *)mt.transferData)[0] = 0;
    ((dword *)mt.transferData)[1] = 0;

    if (!mailbox_process(0, (MailboxTag *)&mt, 12 + 4)) {
        log_println("FAILED video_update_info 2");
        return;
    }

    screenInfo.pitch = ((dword *)mt.transferData)[0];

    mt.tagData.tagId = PROPTAG_GET_DEPTH;
    mt.tagData.bufferSize = 4;
    mt.tagData.valueLength = 0;
    ((dword *)mt.transferData)[0] = 0;
    ((dword *)mt.transferData)[1] = 0;

    if (!mailbox_process(0, (MailboxTag *)&mt, 12 + 4)) {
        log_println("FAILED video_update_info 3");
        return;
    }

    screenInfo.bpp = ((dword *)mt.transferData)[0];

    log_println(" ");
    log_println("VIDEO_UPDATE_INFO: W: %d, H: %d, P: %d, D: %d, S: %d (%d) (%d) ", screenInfo.width, screenInfo.height, screenInfo.pitch, screenInfo.bpp, 
        screenInfo.size, screenInfo.width * screenInfo.height * 4, (screenInfo.pitch) * screenInfo.height);
    log_println(" ");
}

void video_set_brush(dword color) {
    screenInfo.brushColor = color;
}

void video_draw_line(VPoint from, VPoint to) {

}

void video_set_resolution(dword x, dword y, dword bpp) {
    log_println("VID SET: %d, %d, %d", x, y, bpp);

    memcpy(&fbinfo, &fbinfoDEFAULT, sizeof(fbinfo));

    fbinfo.xres = x;
    fbinfo.yres = y;
    fbinfo.xres_virtual = x;
    fbinfo.yres_virtual = y;
    
    log_println("CALLING MAILBOX");

    if (!mailbox_process(0, (MailboxTag *)&fbinfo, sizeof(FrameBufferRequest))) {
        log_println("FAILED TO SET VIDEO");
        return false;
    }

    log_println("OK SET IT: BASE: %8X", fbinfo.base);

    log_println("BASE: %8X", fbinfo.base);

    screenInfo.size = fbinfo.screen_size;

    video_update_info();

    return true;
}

void video_draw_char(char c, dword posX, dword posY) {
    for (int y=0; y<font_get_height(); y++) {
        for (int x=0; x<font_get_width(); x++) {
            bool yes = font_get_pixel(c, x, y);
            draw_pixel(posX + x, posY + y, yes ? screenInfo.textColor : screenInfo.backColor);
        }
    }
}

void video_draw_string(char *sz, dword inX, dword inY) {
    int posX = inX;
    int posY = inY;
    for (int i=0; sz[i] != 0; i++, posX += (font_get_width() + 2)) {
        for (int y=0; y<font_get_height(); y++) {
            for (int x=0; x<font_get_width(); x++) {
                bool yes = font_get_pixel(sz[i], x, y);
                draw_pixel(posX + x, posY + y, yes ? screenInfo.textColor : screenInfo.backColor);
            }
        }
    }
}

