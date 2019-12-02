#include "lcd.h"
#include "../gpio/gpio.h"
#include "../spi/spi.h"
#include <sys/timer.h>
#include <log.h>

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

void write(byte *b, int len) {
    for (int i=0; i<len; i++) {
        spi_send_byte(b[i]);
    }
}

#define LCD_RESET 25
#define LCD_DC 24
#define LCD_CS 8

void lcd_reset() {
    gpio_write(LCD_RESET, true);
    timer_delay(500);
    gpio_write(LCD_RESET, false);
    timer_delay(500);
    gpio_write(LCD_RESET, true);
}

void write_reg(byte b) {
    gpio_write(LCD_DC, false);
    gpio_write(LCD_CS, false);
    spi_send_byte(b);
    gpio_write(LCD_CS, true);
}

void write_data(byte b) {
    gpio_write(LCD_DC, true);
    gpio_write(LCD_CS, false);
    //spi_send_byte(b >> 8);
    spi_send_byte(b & 0xFF);
    gpio_write(LCD_CS, true);
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

bool lcd_init() {
    log_println("STARTING UP LCD");

    lcd_reset();

    log_println("\r\n\r\nSTARTUP SEQUENCE:");
    write_reg(0XF9);
    write_data(0x00);
    write_data(0x08);

    write_reg(0xC0);
    write_data(0x19);//VREG1OUT POSITIVE
    write_data(0x1a);//VREG2OUT NEGATIVE

    write_reg(0xC1);
    write_data(0x45);//VGH,VGL    VGH>=14V.
    write_data(0x00);

    write_reg(0xC2);	//Normal mode, increase can change the display quality, while increasing power consumption
    write_data(0x33);

    write_reg(0XC5);
    write_data(0x00);
    write_data(0x28);//VCM_REG[7:0]. <=0X80.

    write_reg(0xB1);//Sets the frame frequency of full color normal mode
    write_data(0xA0);//0XB0 =70HZ, <=0XB0.0xA0=62HZ
    write_data(0x11);

    write_reg(0xB4);
    write_data(0x02); //2 DOT FRAME MODE,F<=70HZ.

    write_reg(0xB6);//
    write_data(0x00);
    write_data(0x42);//0 GS SS SM ISC[3:0];
    write_data(0x3B);

    write_reg(0xB7);
    write_data(0x07);

    write_reg(0xE0);
    write_data(0x1F);
    write_data(0x25);
    write_data(0x22);
    write_data(0x0B);
    write_data(0x06);
    write_data(0x0A);
    write_data(0x4E);
    write_data(0xC6);
    write_data(0x39);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    write_data(0x00);
    

    write_reg(0XE1);
    write_data(0x1F);
    write_data(0x3F);
    write_data(0x3F);
    write_data(0x0F);
    write_data(0x1F);
    write_data(0x0F);
    write_data(0x46);
    write_data(0x49);
    write_data(0x31);
    write_data(0x05);
    write_data(0x09);
    write_data(0x03);
    write_data(0x1C);
    write_data(0x1A);
    write_data(0x00);

    write_reg(0XF1);
    write_data(0x36);
    write_data(0x04);
    write_data(0x00);
    write_data(0x3C);
    write_data(0x0F);
    write_data(0x0F);
    write_data(0xA4);
    write_data(0x02);

    write_reg(0XF2);
    write_data(0x18);
    write_data(0xA3);
    write_data(0x12);
    write_data(0x02);
    write_data(0x32);
    write_data(0x12);
    write_data(0xFF);
    write_data(0x32);
    write_data(0x00);

    write_reg(0XF4);
    write_data(0x40);
    write_data(0x00);
    write_data(0x08);
    write_data(0x91);
    write_data(0x04);

    write_reg(0XF8);
    write_data(0x21);
    write_data(0x04);

    write_reg(0X3A);	//Set Interface Pixel Format
    write_data(0x55);


    log_println("\r\n\r\nGAMMA SEQUENCE:");
//gammastuff
    write_reg(0xB6);
    write_data(0X00);
    write_data(0x62);
    write_reg(0x36);
    write_data(0x28);

    log_println("\r\n\r\nTURNING ON:");
    timer_delay(200);

    write_reg(0x11);
    timer_delay(120);
    write_reg(0x29);

    log_println("LCD BACK ON");

    timer_delay(2000);
    //clear
    lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LCD_SetColor (BLACK , LCD_WIDTH, LCD_HEIGHT);
    log_println("CLEARED BLACK");

    timer_delay(2000);
    //clear
    lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LCD_SetColor (RED , LCD_WIDTH, LCD_HEIGHT);
    log_println("CLEARED RED");

    timer_delay(2000);
    //clear
    lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
    LCD_SetColor (BLUE , LCD_WIDTH, LCD_HEIGHT);
    log_println("CLEARED BLUE");

    timer_delay(2000);
/*
    spi_activate_transfer();
    int max = 1000;
    int j=0;
    byte buf[64];
    log_println("\r\n\r\nSTARTING LCD...");

    for (int i=0; i<max;) {
        //log_println("LCD:> %2X", i);

        if (default_init_sequence[i] == -3) {
            break;
        }

        switch(default_init_sequence[i]) {
            case -1:
            {
                i++;


                j = 0;

                while(default_init_sequence[i] >= 0) {
                    if (j > 63) {
                        assert(false, "LCD FAIL 1");
                    }
                    buf[j++] = default_init_sequence[i++];
                }

                log_println("WRITING DATA: ");
                log_dump(buf, j, 8);

                write(buf, j);
            } break;

            case -2: {
                i++;
                timer_delay(default_init_sequence[i++]);
            } break;

            case -3: {
                log_println("DONE");
                break;
            }

            default:
                assert(false, "BAD VALUE");
                break;
        }
    }

    log_println("LCD STARTED\r\n\r\n");

    timer_delay(5000);
*/

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
