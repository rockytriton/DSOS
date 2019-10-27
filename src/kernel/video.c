#include "mm.h"
#include "mailbox.h"
#include "printf.h"

static int pt[8192] __attribute__((aligned(16)));
static int pt_index = 0;

#define PT_OSIZE 0
#define T_OVALUE 3
#define PT_OREQUEST_OR_RESPONSE 1

#define TAG_ALLOCATE_BUFFER  0x40001
#define     TAG_RELEASE_BUFFER  0x48001
#define     TAG_BLANK_SCREEN  0x40002
#define     TAG_GET_PHYSICAL_SIZE  0x40003
#define     TAG_TEST_PHYSICAL_SIZE  0x44003
#define     TAG_SET_PHYSICAL_SIZE  0x48003
#define     TAG_GET_VIRTUAL_SIZE  0x40004
#define    TAG_TEST_VIRTUAL_SIZE  0x44004
#define     TAG_SET_VIRTUAL_SIZE  0x48004
#define     TAG_GET_DEPTH  0x40005
#define     TAG_TEST_DEPTH  0x44005
#define     TAG_SET_DEPTH  0x48005
#define TAG_GET_PITCH  0x40008

#define TAG_GET_FIRMWARE_VERSION  0x1
#define TAG_GET_BOARD_MODEL  0x10001
#define TAG_GET_BOARD_REVISION 0x10002
#define TAG_GET_BOARD_MAC_ADDRESS 0x10003
#define TAG_GET_BOARD_SERIAL 0x10004

#define TAG_GET_MAX_CLOCK_RATE  0x30004
#define TAG_SET_CLOCK_RATE  0x38002

#define TAG_GET_CLOCK_RATE 0x00030002

#define TAG_CLOCK_ARM  3


void addTag(int tag, int arg1, int arg2) {
    pt[pt_index++] = tag;

    if (tag == TAG_ALLOCATE_BUFFER) {
        pt[pt_index++] = 8;
        pt[pt_index++] = 8; /* Request */
        pt[pt_index++] = 4096;
        pt_index += 1;
    } else if (tag == TAG_SET_PHYSICAL_SIZE || tag == TAG_SET_VIRTUAL_SIZE) {
        pt[pt_index++] = 8;
        pt[pt_index++] = 8; /* Request */
        pt[pt_index++] = arg1; /* Width */
        pt[pt_index++] = arg2; /* Height */
    } else if (tag == TAG_SET_DEPTH) {
        pt[pt_index++] = 4;
        pt[pt_index++] = 4; /* Request */
        pt[pt_index++] = arg1;
    } else if (tag == TAG_GET_PITCH) {
        pt[pt_index++] = 4;
        pt[pt_index++] = 4; /* Request */
        pt_index += 1;
    } else if (tag == TAG_GET_PHYSICAL_SIZE ) {
        pt[pt_index++] = 8;
        pt[pt_index++] = 8; /* Request */
        pt_index += 2;
    } else if (tag == TAG_GET_DEPTH ) {
        pt[pt_index++] = 4;
        pt[pt_index++] = 4; /* Request */
        pt_index += 1;
    } else if (tag == TAG_GET_BOARD_MODEL || 
               tag == TAG_GET_BOARD_REVISION ||  
               tag == TAG_GET_FIRMWARE_VERSION ||  
               tag == TAG_GET_BOARD_MAC_ADDRESS ||  
               tag == TAG_GET_BOARD_SERIAL) {
        pt[pt_index++] = 8;
        pt[pt_index++] = 8; /* Request */
        pt_index += 2;
    } else if (tag == TAG_GET_MAX_CLOCK_RATE || tag == TAG_GET_CLOCK_RATE) {
        pt[pt_index++] = 8;
        pt[pt_index++] = 0; /* Request */
        pt[pt_index++] = arg1;
        pt[pt_index++] = 0;
    } else if (tag == TAG_SET_CLOCK_RATE ) {
        pt[pt_index++] = 12;
        pt[pt_index++] = 0; /* Request */
        pt[pt_index++] = arg1;
        pt[pt_index++] = arg2;
        pt[pt_index++] = 0;
    } 

    pt[pt_index] = 0;
}

#define T_ORESPONSE 2

int getTag(int tag) {
    int index = 2;
    int val = 0;

    printf("\r\n");

    while( index < ( pt[PT_OSIZE] >> 2 ) )
    {
        if( pt[index] == tag ) {
            printf("FOUND TAG: %d\r\n", tag);
            int *tag_buffer = &pt[index];
            int len = tag_buffer[T_ORESPONSE] & 0xFFFF;

            printf("\tBUFFER: %d, LEN: %d\r\n", tag_buffer, len);

            for (int i=0; i<len; i++) {
                printf("\t%d = %d\r\n", i, tag_buffer[i]);
            }

            int *pData = &tag_buffer[T_OVALUE];
            val = pData[0];

            break;
        }

        index += ( pt[index + 1] >> 2 ) + 3;
    }
            
    printf("TAG NOT FOUND: %d\r\n", tag);

    return val;
}

void *getTagData(int tag) {
    int index = 2;
    int val = 0;

    printf("\r\n");

    while( index < ( pt[PT_OSIZE] >> 2 ) )
    {
        if( pt[index] == tag ) {
            printf("FOUND TAG: %d\r\n", tag);
            int *tag_buffer = &pt[index];
            int len = tag_buffer[T_ORESPONSE] & 0xFFFF;

            printf("\tBUFFER: %d, LEN: %d\r\n", tag_buffer, len);

            for (int i=0; i<len; i++) {
                printf("\t%d = %d\r\n", i, tag_buffer[i]);
            }

            return &tag_buffer[T_OVALUE];
        }

        index += ( pt[index + 1] >> 2 ) + 3;
    }
            
    printf("TAG NOT FOUND: %d\r\n", tag);

    return 0;
}

void initTags() {
    for (int i=0; i<8192; i++) {
        pt[i] = 0;
    }

    /* Fill in the size on-the-fly */
    pt[PT_OSIZE] = 12;

    /* Process request (All other values are reserved!) */
    pt[PT_OREQUEST_OR_RESPONSE] = 0;

    /* First available data slot */
    pt_index = 2;

    /* NULL tag to terminate tag list */
    pt[pt_index] = 0;
}

void processTags() {
    pt[PT_OSIZE] = ( pt_index + 1 ) << 2;
    pt[PT_OREQUEST_OR_RESPONSE] = 0;

    for (int i=0; i<10; i++) {
        printf(" - %d", pt[i]);
    }
    
    printf("VC SENDING: %d\r\n", pt);

	mailboxWrite(MAIL_TAGS, pt);
    int result = mailboxRead(MAIL_TAGS);

    printf("VC RECEIVED: %d\r\n", result);
}

void doVideoCheck() {
    printf("oh checking: %d\r\n", pt);

    initTags();
    
    addTag( TAG_GET_BOARD_MODEL, 0, 0 );
    addTag( TAG_GET_BOARD_REVISION, 0, 0 );
    addTag( TAG_GET_FIRMWARE_VERSION, 0, 0 );
    addTag( TAG_GET_BOARD_MAC_ADDRESS, 0, 0 );
    addTag( TAG_GET_BOARD_SERIAL, 0, 0 );

    processTags();

    getTag( TAG_GET_BOARD_MODEL);
    getTag( TAG_GET_BOARD_REVISION);
    getTag( TAG_GET_FIRMWARE_VERSION);
    getTag( TAG_GET_BOARD_MAC_ADDRESS);
    getTag( TAG_GET_BOARD_SERIAL);

    initTags();

    addTag( TAG_ALLOCATE_BUFFER, 4096, 0 );
    addTag( TAG_SET_PHYSICAL_SIZE, 1280, 1024 );
    addTag( TAG_SET_VIRTUAL_SIZE, 1280, 2048 );
    addTag( TAG_SET_DEPTH, 32, 0 );
    addTag( TAG_GET_PITCH, 0, 0 );
    addTag( TAG_GET_PHYSICAL_SIZE, 0, 0 );
    addTag( TAG_GET_DEPTH, 0, 0 );

    processTags();

    getTag(TAG_GET_PHYSICAL_SIZE);
    getTag(TAG_GET_DEPTH);
    getTag(TAG_GET_PITCH);

    volatile unsigned char* fb = (unsigned char*)getTag(TAG_ALLOCATE_BUFFER);

    printf("ALLOC BUFFER: %d\r\n", fb);

    fb = (unsigned char *)((int)fb & 0x3FFFFFFF);

    printf("ALLOC BUFFER: %d\r\n", fb);

    initTags();
    addTag( TAG_GET_CLOCK_RATE, TAG_CLOCK_ARM, 0 );
    processTags();

    int cr = pt[6];
    printf("CR: %d\r\n", cr);

    initTags();
    addTag( TAG_GET_MAX_CLOCK_RATE, TAG_CLOCK_ARM, 0 );
    processTags();

    getTag(TAG_GET_MAX_CLOCK_RATE);
    cr = pt[6];

    printf("SETTING CR: %d\r\n", cr);

    initTags();
    addTag( TAG_SET_CLOCK_RATE, TAG_CLOCK_ARM, cr );
    processTags();

    initTags();
    addTag( TAG_GET_CLOCK_RATE, TAG_CLOCK_ARM, 0 );
    processTags();

    cr = pt[6];
    printf("CR: %d\r\n", cr);

    int pixel_offset = 0;
    int pitch = 5120;

    int rr = 150;
    int gg = 100;
    int bb = 50;
    int aa = 255;

    

    int frames = 0;

    while(1) {
        frames++;
        printf("FRAMES: %d\r\n", frames);

        bb++;

        if (bb > 255) {
            bb = 0;
        }

        for(int y = 0; y < 1024; y++ )
        {
            rr += 1;

            if (rr > 255) {
                rr = 0;
            }
            //rr += ( 1.0 / 1024 );
            //current_colour.b = 0;

            for(int x = 0; x < 1280; x++ ) {
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
    }
/*
    frame_count++;
    if( calculate_frame_count )
    {
        calculate_frame_count = 0;

        // Number of frames in a minute, divided by seconds per minute 
        float fps = (float)frame_count / 60;
        printf( "FPS: %.2f\r\n", fps );

        frame_count = 0;
    }*/
}