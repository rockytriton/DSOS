#include "video.h"
#include "mailbox/mailbox.h"
#include "log.h"
#include <sys/timer.h>
#include "mm.h"
#include "../dma/dmachannel.h"

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

#define SCREEN_WIDTH 1824
#define SCREEN_HEIGHT 984

typedef struct {
	MailboxTag tag1;
	dword xres, yres;
	MailboxTag tag2;
	dword xres_virtual, yres_virtual;
	MailboxTag tag3;
	dword bpp;
	MailboxTag tag4;
	dword xoffset, yoffset;
	MailboxTag tag5;
	dword base, screen_size;
	MailboxTag tag6;
	dword pitch;
} FrameBufferRequest;

FrameBufferRequest fbinfo = {
    .tag1 = { PROPTAG_SET_PHYS_WIDTH_HEIGHT,
            8, 0, },
        .xres = SCREEN_WIDTH,
        .yres = SCREEN_HEIGHT,
    .tag2 = { PROPTAG_SET_VIRT_WIDTH_HEIGHT,
            8, 0, },
        .xres_virtual = SCREEN_WIDTH,
        .yres_virtual = SCREEN_HEIGHT,
    .tag3 = { PROPTAG_SET_DEPTH, 4, 0 },
        .bpp = 32,
    .tag4 = { PROPTAG_SET_VIRTUAL_OFFSET, 8, 0 },
        .xoffset = 0,
        .yoffset = 0,
    .tag5 = { PROPTAG_ALLOCATE_BUFFER, 4096, 0 },
        .base = 0,
        .screen_size = 0,
    .tag6 = { PROPTAG_GET_PITCH, 4, 0 },
        .pitch = 0,
};

byte *screenRed;
byte *sreenBlack;
byte *screenBlue;
byte *screenWhite;
byte *screenGreen;
DmaChannel *ch;

void video_clear_screen(dword color) {
    byte *preBuffer = allocPage();
    int pixel_offset = 0;
    int pitch = 4 * SCREEN_WIDTH; //5120;

    int r = (color & 0xFF000000) >> 24;
    int g = (color & 0x00FF0000) >> 16;
    int b = (color & 0x0000FF00) >> 8;
    int a = 0xFF;
    
    for(int y = 0; y < SCREEN_HEIGHT; y++ )
    {
        for(int x = 0; x < SCREEN_WIDTH; x++ ) {
            pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );

            preBuffer[ pixel_offset++ ] = r;
            preBuffer[ pixel_offset++ ] = g;
            preBuffer[ pixel_offset++ ] = b;
            preBuffer[ pixel_offset++ ] = a;
        }
    }

    byte *p = (byte *)fbinfo.base;
    qword ct2 = clock_get_ticks();
    int total = (SCREEN_HEIGHT * SCREEN_WIDTH) * 4;
    dword start = 0;

    while(total > 0) {
        int numBytes = total;

        if (numBytes > 0xFFFF) {
            numBytes = 0xFFFF;
        }

        dma_setup_mem_copy(ch, p + start, preBuffer + start, numBytes, 2, false);
        dma_start(ch);
        dma_wait(ch);

        start += 0xFFFF;
        total -= 0xFFFF;
    }
}

void draw_pixel(int x, int y, dword color) {
    int pitch = 4 * SCREEN_WIDTH; //5120;
    int pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );
    volatile unsigned char* fb = (unsigned char*)((fbinfo.base | 0x40000000) & ~0xc0000000);

    int r = (color & 0xFF000000) >> 24;
    int g = (color & 0x00FF0000) >> 16;
    int b = (color & 0x0000FF00) >> 8;
    int a = 0xFF;

    fb[ pixel_offset++ ] = r;
    fb[ pixel_offset++ ] = g;
    fb[ pixel_offset++ ] = b;
    fb[ pixel_offset++ ] = a;
}

void video_init() {
    MailboxTransfer mt;

    mt.tagData.tagId = PROPTAG_GET_DISPLAY_DIMENSIONS;
    mt.tagData.bufferSize = 8;
    mt.tagData.valueLength = 8;
    dword *pData = (dword *)mt.transferData;
    pData[0] = 0;
    pData[1] = 0;
    pData[2] = 0;

    log_println("DONG REQUEST");

    if (!mailbox_process(mt.tagData.tagId, (MailboxTag *)&mt, sizeof(mt.tagData) + 8)) {
        log_println("FAILED TO GET VIDEO");
        return;
    }

    log_println("VIDEO SIZE: %d, %d, %d, %d", pData[0], pData[1], pData[2], pData[3]);

    if (!mailbox_process(mt.tagData.tagId, (MailboxTag *)&fbinfo, sizeof(FrameBufferRequest))) {
        log_println("FAILED TO SET VIDEO");
        return;
    }

    log_println("PITCH: %d", fbinfo.pitch);
    log_println("BASE: %X - %X - %X", fbinfo.base, fbinfo.base | 0x40000000, (fbinfo.base | 0x40000000) & ~0xc0000000);
    log_println("SCREENSIZE: %d", fbinfo.screen_size);
    log_println(" ");
    log_println("VIDEO INITIALIZED: %d, %d", fbinfo.yres, fbinfo.xres);
    timer_delay(5000);

    int pixel_offset = 0;
    int pitch = 4 * SCREEN_WIDTH; //5120;

    int rr = 150;
    int gg = 50;
    int bb = 50;
    int aa = 255;

    volatile unsigned char* fb = (unsigned char*)((fbinfo.base | 0x40000000) & ~0xc0000000);
    //fb = (unsigned char *)((int)fb & 0x3FFFFFFF);

    int frames = 0;

    byte *preBuffer = allocPage();

    log_println("DRAW 1");
    qword ct1 = clock_get_ticks();

    for(int y = 0; y < SCREEN_HEIGHT; y++ )
    {
        //rr += ( 1.0 / 1024 );
        //current_colour.b = 0;

        for(int x = 0; x < SCREEN_WIDTH; x++ ) {
            pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );

            int r = rr;
            int g = gg;
            int b = bb;
            int a = aa;

            preBuffer[ pixel_offset++ ] = r;
            preBuffer[ pixel_offset++ ] = g;
            preBuffer[ pixel_offset++ ] = b;
            preBuffer[ pixel_offset++ ] = a;
        }
    }
    int total = (SCREEN_WIDTH * SCREEN_HEIGHT) * 4;
    dword start = 0;
    byte *p = (byte *)fbinfo.base;
    qword ct2 = clock_get_ticks();

    //log_println("DOING DMA: %d", total);

    ch = dma_open_channel(DCNormal);

    while(total > 0) {
        int numBytes = total;

        if (numBytes > 0xFFFF) {
            numBytes = 0xFFFF;
        }

        dma_setup_mem_copy(ch, p + start, preBuffer + start, numBytes, 2, false);
        dma_start(ch);
        dma_wait(ch);

        start += 0xFFFF;
        total -= 0xFFFF;
    }

    qword ct3 = clock_get_ticks();
    log_println("DONE 1: %d, %d, %d", (ct2 - ct1), (ct3 - ct2), (ct3 - ct1));

    video_clear_screen(0xFFFFFF00);
    video_clear_screen(0x00000000);
    video_clear_screen(0x0000FF00);
    video_clear_screen(0x0000FF00);
    video_clear_screen(0x00FFFF00);
    video_clear_screen(0xFFFFFF00);
    video_clear_screen(0xFF000000);
    video_clear_screen(0xFFFF0000);
    video_clear_screen(0x0000FF00);
    video_clear_screen(0x00FF0000);

    rr = 50;
    gg = 100;
    bb = 150;

    ct1 = clock_get_ticks();
    for(int y = 0; y < SCREEN_HEIGHT; y++ )
    {
        //rr += ( 1.0 / 1024 );
        //current_colour.b = 0;

        for(int x = 0; x < SCREEN_WIDTH; x++ ) {
            pixel_offset = ( x * ( 32 >> 3 ) ) + ( y * pitch );

            int r = rr;
            int g = gg;
            int b = bb;
            int a = aa;

            fb[ pixel_offset++ ] = r;
            fb[ pixel_offset++ ] = g;
            fb[ pixel_offset++ ] = b;
            fb[ pixel_offset++ ] = a;
        }
    }
    ct2 = clock_get_ticks();
    log_println("DONE 2: %d,", (ct2 - ct1));

    video_clear_screen(0x00000000);

    timer_delay(5000);

    char *sz = "This is a test, with a long string of character, and some special... #&$_!#$)(* END.";

    int posX = 10;
    int posY = 10;

    int yess = 0;
    int nos = 0;

    for (int i=0; sz[i] != 0; i++, posX += (font_get_width() + 2)) {
        for (int y=0; y<font_get_height(); y++) {
            for (int x=0; x<font_get_width(); x++) {
                bool yes = font_get_pixel(sz[i], x, y);
                draw_pixel(posX + x, posY + y, yes ? 0xFFFFFFFF : 0x00000000);

                if (yes) {
                    yess++;
                } else {
                    nos++;
                }
            }
        }
    }

    log_println("OK DONE: %d, %d", yess, nos);
    timer_delay(5000);
}

