#include "lcd.h"
#include "../gpio/gpio.h"
#include "../spi/spi.h"
#include <sys/timer.h>
#include <log.h>
#include "mm.h"
#include "../dma/dmachannel.h"

#define LCD_X_MAXPIXEL  480  //LCD width maximum memory 
#define LCD_Y_MAXPIXEL  320 //LCD height maximum memory
#define LCD_X	 0
#define LCD_Y	 0

#define LCD_WIDTH  (LCD_X_MAXPIXEL - 2 * LCD_X)  //LCD width
#define LCD_HEIGHT  LCD_Y_MAXPIXEL //LCD height

#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define BRED           0XF81F
#define GRED 		   0XFFE0
#define GBLUE		   0X07FF
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define BROWN 		   0XBC40
#define BRRED 		   0XFC07
#define GRAY  		   0X8430


#define LCD_RESET 25
#define LCD_DC 24
#define LCD_CS 8

word dmaData[(320 * 480) + 0xFF];

byte *cmds[] = {
    {0xCF, 0x00, 0x83, 0x30},
    {0xED, 0x64, 0x03, 0x12, 0x81},
    {0xE8, 0x85, 0x01, 0x79},
    {0xCB, 0x39, 0X2C, 0x00, 0x34, 0x02},
    {0xF7, 0x20},
    {0xEA, 0x00, 0x00},
    {0xC0, 0x26},
    {0xC1, 0x11},
    {0xC5, 0x35, 0x3E},
    {0xC7, 0xBE},
    {0x3A, 0x55},
    {0xB1, 0x00, 0x1B},
    {0x26, 0x01},
    {0xB7, 0x07},
    {0xB6, 0x0A, 0x82, 0x27, 0x00},
    {0x11},
    //wait 100sm
    {0x29},
    //wait 20ms
};

static int init_sequence[] = {
    -1, 0xF0, 0xC3,
    -1, 0xF0, 0x96,
	-1, 0x36, 0x68,
	-1, 0x3A, 0x05,
	-1, 0xB0, 0x80,
	-1, 0xB6, 0x20, 0x02,
	-1, 0xB5, 0x02, 0x02, 0x00, 0x04,
	-1, 0xB1, 0x80, 0x10,
	-1, 0xB4, 0x00,
	-1, 0xB7, 0xC6,
	-1, 0xC5, 0x5D,
	-1, 0xE4, 0x31,
	-1, 0xE8, 0x40, 0x8A, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33,
	-1, 0xC2,
	-1, 0xA7,
	-1, 0xE0, 0xF0, 0x09, 0x13, 0x12, 0x12, 0x2B, 0x3C, 0x44, 0x4B, 0x1B, 0x18, 0x17, 0x1D, 0x21,
	-1, 0xE1, 0xF0, 0x09, 0x13, 0x0C, 0x0D, 0x27, 0x3B, 0x44, 0x4D, 0x0B, 0x17, 0x17, 0x1D, 0x21,
	-1, 0x36, 0xEC,
	-1, 0xF0, 0x3C,
	-1, 0xF0, 0x69,
	-1, 0x13,
	-1, 0x11,
	-1, 0x29,
//Set Var stuff...
    -1, 0x36, 0xE8,
    -3
};

static int clear_sequence[] = {
    -1, 0x2A, 0, 0, 479 >> 8, 479 & 0xFF,
    -1, 0x2B, 0, 0, 319 >> 8, 319 & 0xFF,
    -1, 0x2C,
    -3
};

/*
static int clear_sequence[] = {
    -1, 0x2A, 0 >> 8, 0 & 0xFF, 480 >> 8, 480 & 0xFF,
    -1, 0x2B, 0 >> 8, 0 & 0xFF, 320 >> 8, 320 & 0xFF,
    -1, 0x2C,
    -3
};
*/

static int default_init_sequence[] = {
	/* Interface Mode Control */
	-1, 0xb0, 0x0,
	/* Sleep OUT */
	-1, 0x11,
	-2, 250,
	/* Interface Pixel Format */
	-1, 0x3A, 0x55,
	/* Power Control 3 */
	-1, 0xC2, 0x44,
	/* VCOM Control 1 */
	-1, 0xC5, 0x00, 0x00, 0x00, 0x00,
	/* PGAMCTRL(Positive Gamma Control) */
	-1, 0xE0, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
	          0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
	/* NGAMCTRL(Negative Gamma Control) */
	-1, 0xE1, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	/* Digital Gamma Control 1 */
	-1, 0xE2, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	/* Sleep OUT */
	-1, 0x11,
	/* Display ON */
	-1, 0x29,
	/* end marker */
	-3
};

void write_byte_16(byte b) {
    //gpio_write(LCD_CS, false);
    spi_send_byte(0);
    spi_send_byte(b);
    //gpio_write(LCD_CS, true);
}

void write_word_16(word w) {
    //gpio_write(LCD_CS, false);
    spi_send_byte(w >> 8);
    spi_send_byte(w & 0xFF);
    //gpio_write(LCD_CS, true);
}

void write_byte_16_master(byte b) {
    word n = b;
    spimaster_write(1, &n, 2);
}

void write(byte *b, int len) {
    spi_start_transaction();
    spi_command();
    write_byte_16_master(b[0]);

    log_print("SPI_WRITE: %4X ", b[0]);

    if (len > 1) {
        spi_data();
    }

    word commandData[64];
    
    for (int i=1; i<len; i++) {
        commandData[i - 1] = b[i];
        //write_byte_16_master(b[i]);
        //log_print("%4X ", b[i]);
    }

    spimaster_write(1, commandData, len - 1);

    log_println(" ");
    spi_end_transaction();
}

void write_old(byte *b, int len) {
    spi_start_transaction();
    spi_command();
    write_byte_16(b[0]);

    log_print("SPI_WRITE: %4X ", b[0]);

    if (len > 1) {
        spi_data();
    }

    for (int i=1; i<len; i++) {
        write_byte_16(b[i]);
        log_print("%4X ", b[i]);
    }

    log_println(" ");
    spi_end_transaction();
}

void lcd_reset() {
    gpio_write(LCD_RESET, true);
    timer_delay(500);
    gpio_write(LCD_RESET, false);
    timer_delay(500);
    gpio_write(LCD_RESET, true);

    gpio_write(LCD_CS, 0);
}

void write_reg(byte b) {
    spi_command();
    
    //gpio_write(LCD_DC, false);
    //gpio_write(LCD_CS, false);
    spi_send_byte(b >> 0);
    spi_send_byte(b & 0xFF);
    //gpio_write(LCD_CS, true);
}

void write_data(byte b) {
    spi_data();
    //gpio_write(LCD_DC, true);
    //gpio_write(LCD_CS, false);
    spi_send_byte(b >> 8);
    spi_send_byte(b & 0xFF);
    //gpio_write(LCD_CS, true);
    //timer_delay(10);
}

void lcd_set_window(word Xstart, word Ystart, word Xend, word Yend)
{
    //set the X coordinates
    write_reg(0x2A);
    write_data(Xstart >> 8);	 				//Set the horizontal starting point to the high octet
    write_data(Xstart & 0xff);	 				//Set the horizontal starting point to the low octet
    write_data((Xend - 1) >> 8);	//Set the horizontal end to the high octet
    write_data((Xend - 1) & 0xff);	//Set the horizontal end to the low octet

    //set the Y coordinates
    write_reg(0x2B);
    write_data(Ystart >> 8);
    write_data(Ystart & 0xff );
    write_data((Yend - 1) >> 8);
    write_data((Yend - 1) & 0xff);
    write_reg(0x2C);
}

static void LCD_Write_AllData(uint16_t Data, uint32_t DataLen)
{
    uint32_t i;
    gpio_write(LCD_DC, true);
    gpio_write(LCD_CS, false);
    for(i = 0; i < DataLen; i++) {
        spi_send_byte(Data >> 8);
        spi_send_byte(Data & 0XFF);
    }
    gpio_write(LCD_CS, true);
}

void LCD_SetColor(word Color , word Xpoint, word Ypoint)
{
    LCD_Write_AllData(Color , (uint32_t)Xpoint * (uint32_t)Ypoint);
}

void play_sequence(int *p, dword size) {
    dword max = size * 2;
    int j = 0;
    byte buf[max];

    for (int i=0; i<max;) {
        //log_println("LCD:> %2X", i);

        if (p[i] == -3) {
            break;
        }

        switch(p[i]) {
            case -1:
            {
                i++;

                j = 0;

                while(p[i] != -1 && p[i] != -3) {
                    if (j > 63) {
                        log_println("BAD VAL: %d, %d, %d", i, j, p[i]);
                        assert(false, "LCD FAIL 1");
                    }
                    //log_println("READING DATA ITEM: %d", p[i]);
                    buf[j++] = p[i++];
                }

                write(buf, j);
            } break;

            case -2: {
                log_println("NEG 2");
                i++;
                timer_delay(p[i++]);
            } break;

            case -3: {
                log_println("DONE");
                break;
            }

            default:
                log_println("BAD VALUE: %d - %d", i, p[i]);
                assert(false, "BAD VALUE");
                break;
        }
    }
}



void write_color(word color) {

    spi_data();

    log_println("BEGIN TX");

    word *screenData = allocPage();
    
    for (int i=0; i<(320*480); i++) {
        screenData[i] = color;
    }

    spimaster_write(1, screenData, 320 * 480);

    log_println("END TX");
}

void write_color_old(word color) {

    spi_data();
    
    //log_println("START TX");

    for (int i=0; i<(320*480); i++) {
        //dmaData[i] = color;
        
    spi_start_transaction();
    //gpio_write(LCD_CS, false);
        spi_send_byte(color >> 8);
        spi_send_byte(color & 0xFF);
    //gpio_write(LCD_CS, true);
    spi_end_transaction();

        if (i % 1000 == 0) {
            //log_println("SENT %d BYTES", i);
        }
        
    }

    //spi_send_dma(dmaData, 320 * 480);

    log_println("END TX");
}

bool lcd_init() {
    log_println("STARTING UP LCD 6");
    timer_delay(3000);

    spimaster_init();
    gpio_pin_mode(24, PMOut); //RS (DC?)
    gpio_pin_mode(25, PMOut); //Reset

    log_println("INIT SPI");
    timer_delay_ms(1000);
/*
    log_println("TESTING DMA");

    byte *b1 = allocPage();
    byte *b2 = allocPage();

    for (int i=0; i<200; i++) {
        b1[i] = i;
        log_println("B1 [%d] = %d", i, b1[i]);
    }

    timer_delay(3000);

    for (int i=0; i<20; i++) {
        log_println("B2 [%d] = %d - %d", i, b2[i], b1[i]);
    }

    log_println("WAS BEFORE DMA");

    timer_delay(3000);

    DmaChannel *ch = dma_open_channel(DCNormal);
    dma_setup_mem_copy(ch, b2, b1, 512, 2, false);
    dma_start(ch);
    dma_wait(ch);

    log_println("DONE DMA");
    timer_delay(2000);


    for (int i=0; i<200; i++) {
        log_println("B2 [%d] = %d - %d", i, b2[i], b1[i]);
    }

    timer_delay(5000);
*/

    spi_reset();
    timer_delay(200);

    //spi_activate_transfer();
    int max = 1000;
    int j=0;
    byte buf[64];
    log_println("\r\n\r\nSTARTING LCD...");

    play_sequence(init_sequence, sizeof(init_sequence));

    timer_delay(3000);

    spi_reset();
    timer_delay(200);
    log_println("\r\n\r\nSTARTING LCD...");

    play_sequence(init_sequence, sizeof(init_sequence));

    timer_delay(3000);
    log_println("\r\n\r\nCLEARING LCD...");

    play_sequence(clear_sequence, sizeof(clear_sequence));
    log_println("\r\n\r\nWRITING RED LCD...");
    write_color(RED);
    log_println("\r\n\r\nWROTE RED LCD...");

    timer_delay(3000);
    log_println("\r\n\r\nCLEARING LCD...");

    play_sequence(clear_sequence, sizeof(clear_sequence));
    write_color(BLUE);
    
    timer_delay(3000);
    log_println("\r\n\r\nCLEARING LCD...");

    play_sequence(clear_sequence, sizeof(clear_sequence));
    write_color(YELLOW);
    

    log_println("LCD STARTED\r\n\r\n");

    timer_delay(5000);


//old init seq:
/*
    log_println("LDC:> Soft Reset");
    spi_send_byte(MIPI_DCS_SOFT_RESET);
    log_println("sent");
    log_println("");
    timer_delay(5);

    log_println("LDC:> Display Off");
    spi_send_byte(MIPI_DCS_SET_DISPLAY_OFF);
    log_println("sent");
    log_println("");

    write(cmds[0], 4);
    write(cmds[1], 5);
    write(cmds[2], 4);
    write(cmds[3], 6);
    write(cmds[4], 2);
    write(cmds[5], 3);
    write(cmds[6], 2);
    write(cmds[7], 2);
    write(cmds[8], 3);
    write(cmds[9], 2);
    write(cmds[10],2 );
    write(cmds[11],3 );
    write(cmds[12],2 );
    write(cmds[13], 2);
    write(cmds[14], 4);
    write(cmds[15], 1);
    timer_delay(100);
    write(cmds[16], 1);
    timer_delay(20);

    //timer_delay(2000);
*/

}
