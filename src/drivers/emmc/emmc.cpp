#include "emmc.h"
#include "peripherals/base.h"
#include "mailbox.h"
#include "logger.h"
#include "../mailbox/mailbox.h"
#include "timer.h"
#include "utils.h"

#define DEVICE_ID_SD_CARD	0
#define DEVICE_ID_USB_HCD	3
#define POWER_STATE_OFF		(0 << 0)
#define POWER_STATE_ON		(1 << 0)
#define POWER_STATE_WAIT	(1 << 1)
#define POWER_STATE_NO_DEVICE	(1 << 1)	// in response

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID         400000
#define SD_CLOCK_NORMAL     25000000
#define SD_CLOCK_HIGH       50000000
#define SD_CLOCK_100        100000000
#define SD_CLOCK_208        208000000
#define SD_COMMAND_COMPLETE     1
#define SD_TRANSFER_COMPLETE    (1 << 1)
#define SD_BLOCK_GAP_EVENT      (1 << 2)
#define SD_DMA_INTERRUPT        (1 << 3)
#define SD_BUFFER_WRITE_READY   (1 << 4)
#define SD_BUFFER_READ_READY    (1 << 5)
#define SD_CARD_INSERTION       (1 << 6)
#define SD_CARD_REMOVAL         (1 << 7)
#define SD_CARD_INTERRUPT       (1 << 8)


namespace dsos {
namespace emmc {
    const dword EMMC_BASE = PBASE + 0x00300000;

    EmmcRegs *EMMC = (EmmcRegs *)EMMC_BASE;

    static Logger &logger = Logger::inst();

    enum CommandResponseType {
        RTNone,
        RT136,
        RT48,
        RT48Busy
    };

    struct EmmcCommand {
        reg8 resA : 1;
        reg8 blockCount : 1;
        reg8 autoCommand : 2;
        reg8 direction : 1;
        reg8 multiBlock : 1;
        reg16 resB : 10;
        reg8 responseType : 2;
        reg8 res0 : 1;
        reg8 crcCheckEnable : 1;
        reg8 idxCheckEnable : 1;
        reg8 isData : 1;
        reg8 type : 2;
        reg8 index : 6;
        reg8 res1 : 2;
    };

    void printCommand(EmmcCommand cmd) {
        logger << "EMMC COMMAND:" << endl
               << "    " << cmd.resA << endl
               << "    " << cmd.blockCount << endl
               << "    " << cmd.autoCommand << endl
               << "    " << cmd.direction << endl
               << "    " << cmd.multiBlock << endl
               << "    " << cmd.resB << endl
               << "    " << cmd.responseType << endl
               << "    " << cmd.res0 << endl
               << "    " << cmd.crcCheckEnable << endl
               << "    " << cmd.idxCheckEnable << endl
               << "    " << cmd.isData << endl
               << "    " << cmd.type << endl
               << "    " << cmd.index << endl
               << "    " << cmd.res1 << endl
               << endl;

    }

    const dword ERR_VALUE = 0xFFFFFFFF;
    const EmmcCommand RES_CMD = *(EmmcCommand *)(&ERR_VALUE);

    EmmcCommand commands[] = {
        {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 0, 0},
        RES_CMD,
        {0, 0, 0, 0, 0, 0, RT136, 0, 1, 0, 0, 0, 2, 0},
        {0, 0, 0, 0, 0, 0, RT48,  0, 1, 0, 0, 0, 3, 0},
        {0, 0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 4, 0},
    };

    enum CommandType {
        GO_IDLE
    };

    struct EmmcStatus {
        reg8 commandInhibit : 1;
        reg8 dataInhibit : 1;
        reg8 dataActive : 1;
        reg8 res0 : 4;
        reg8 writeTransfer : 1;
        reg8 readTransfer : 1;
        reg8 reg1 : 10;
        reg8 data3to0 : 4;
        reg8 commandLevel : 1;
        reg8 data7to4 : 4;
        reg8 res2 : 3;
    };

    struct EmmcControl0 {
        reg8 res0 : 1;
        reg8 lines4 : 1;
        reg8 highSpeed : 1;
        reg8 res1 : 2;
        reg8 lines8 : 1;
        reg8 res2 : 10;
        reg8 gapStop : 1;
        reg8 gapRestart : 1;
        reg8 readWait : 1;
        reg8 gapSdioIrq : 1;
        reg8 spiMode : 1;
        reg8 bootMode : 1;
        reg8 altBoot : 1;
        reg8 res4 : 9;
    };

    struct EmmcControl1 {
        reg8 intClockEnable : 1;
        reg8 sdClockStable : 1;
        reg8 clockEnable : 1;
        reg8 res0 : 2;
        reg8 clockGenMode : 1;
        reg8 clockDivHigh : 2;
        reg8 clockDivLow;
        reg8 timeoutExponent : 4;
        reg8 res1 : 4;
        reg8 resetComplete : 1;
        reg8 resetCommand : 1;
        reg8 resetData : 1;
        reg8 reg2 : 5;
    };

    struct EmmcIrqFlags {
        reg8 cmdDone : 1;
        reg8 dataDone : 1;
        reg8 blockGap : 1;
        reg8 res0 : 1;
        reg8 writeReady : 1;
        reg8 readReady : 1;
        reg8 cardInsert : 1;
        reg8 cardRemove : 1;
        reg8 card : 1;
        reg8 res2 : 3;
        reg8 retune : 1;
        reg8 bootAck : 1;
        reg8 endBoot : 1;
        reg8 err : 1;
        reg8 timeoutCommandErr : 1;
        reg8 crcErr : 1;
        reg8 endErr : 1;
        reg8 indexErr : 1;
        reg8 timeoutDataErr : 1;
        reg8 crcDataErr : 1;
        reg8 dataEndErr : 1;
        reg8 res3 : 1;
        reg8 autoCommandErr : 1;
        reg8 res4 : 7;
    };

    struct EmmcControl2 {
        reg8 autoCommandNoEx : 1;
        reg8 autoCommandTimeout : 1;
        reg8 autoCommandCrcErr : 1;
        reg8 autoCommandEndErr : 1;
        reg8 autoCommandIndexErr : 1;
        reg8 res0 : 2;
        reg8 autoCommand12Err : 1;
        reg8 res1; 
        reg8 speedMode : 3;
        reg8 res2 : 3;
        reg8 tuneOn : 1;
        reg8 tuned : 1;
        reg8 res3;
    };

    struct PowerData : public mailbox::Tag {
        dword deviceId;
        dword state;
    };

    inline reg32 structToReg(void *p) {
        return *((reg32 *)p);
    }

    dword getClockDivider(dword baseClock, dword targetRate) ;

    bool waitTimeout(reg32 *reg, dword mask, dword value, dword timeout = 2000) {
        volatile int cycles = 0;
        int MAX_CYCLES = timeout;

        for (; cycles <= MAX_CYCLES; cycles++) {
            if ((*reg & mask) ? value : !value) {
                return true;
            }

            delayMs(1);
        }

        return false;
    }

    void setBlockSizeCount(word blockSize, word count) {
        EMMC->blockSizeCount = blockSize | (count << 16);
    }

    void sentCommand(EmmcCommand command) {
        EMMC->commandTransferMode = structToReg(&command);
    }

    void setStatus(EmmcStatus status) {
        EMMC->status = structToReg(&status);
    }

    bool powerOn() {
        Logger &logger = Logger::inst();
        logger.println("Powering on SDCard...");

        logger.print("STATUS 0: ");
        logger.printBinary(EMMC->status);
        logger.println(" ");

        PowerData pd;
        pd.deviceId = DEVICE_ID_SD_CARD;
        pd.state = POWER_STATE_ON | POWER_STATE_WAIT;
        mailbox::Mailbox::inst()->processTag(PROPTAG_SET_POWER_STATE, pd, sizeof(pd));

        if (pd.state & POWER_STATE_NO_DEVICE || !(pd.state & POWER_STATE_ON)) {
            logger << "Failed to power on\r\n";
            return false;
        }

        logger.println("SDCard Powered On");
        return true;
    }

    dword getClockRate() {

        return dsos::getClockRate(CTEmmc);

    }



    void handleInterrupts() {
        logger.print("IRQ 1: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        EmmcIrqFlags flags;
        memcpy((byte *)&flags, (byte *)EMMC->irqFlags, 4);

        logger.print("HANDLING INTERRUPTS: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        if (flags.cmdDone) {
            logger.println("command complete interrupt");
            flags.cmdDone = 0;
        }

        if (flags.dataDone) {
            logger.println("data complete interrupt");
            flags.dataDone = 0;
        }

        if (flags.blockGap) {
            logger.println("blockGap interrupt");
            flags.blockGap = 0;
        }

        if (flags.res0) {
            logger.println("res0 interrupt");
            flags.res0 = 0;
        }

        if (flags.writeReady) {
            logger.println("writeReady interrupt");
            flags.writeReady = 0;
        }

        if (flags.readReady) {
            logger.println("readReady interrupt");
            flags.readReady = 0;
        }

        if (flags.card) {
            logger.println("readReady interrupt");
            flags.readReady = 0;
        }

        if (flags.cardInsert) {
            logger.println("cardInsert interrupt");
            flags.cardInsert = 0;
        }

        if (flags.cardRemove) {
            logger.println("cardRemove interrupt");
            flags.cardRemove = 0;
        }

        if (EMMC->irqFlags & 0x8000) {
            logger.println("ERR INT");
            EMMC->irqFlags |= 0xFFFF0000;
        }

        EMMC->irqFlags = structToReg(&flags);

        logger.print("IRQ 2: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");
    }

    bool Emmc::issueCommand(dword commandReg, dword arg, dword timeout) {
        //todo save last reg...

        //todo look into should poll status reg?

        if (transferBlocks > 0xFFFF) {
            logger << "transferBlocks too large: " << transferBlocks << endl;
            return false;
        }

        EMMC->blockSizeCount = blockSize | (transferBlocks << 16);
        EMMC->arg1 = arg;
        EMMC->commandTransferMode = commandReg;

        logger.print("HANDLING INTERRUPTS: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        waitTimeout(&EMMC->irqFlags, 0x8001, 1, 5000);
        dword irpts = EMMC->irqFlags;


        logger.print("HANDLING INTERRUPTS: ");
        logger.printBinary(irpts);
        logger.println(" ");

        EMMC->irqFlags = 0xFFFF0001;

        logger.print("HANDLING INTERRUPTS: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        if ((irpts & 0xFFFF0001) != 1) {
            logger.println("Error waiting for command interrupt complete");
            return false;
        }

        logger.println("Done Sending?");

        return true;
    }

    bool Emmc::command(dword command, dword arg, dword timeout) {
        handleInterrupts();

        //todo: check for card removed

        if (command & 0x80000000) {
            //todo check for app command and handle it differently...
            return false;
        }

        //todo: validate command
        //todo: save last command

        reg32 commandReg = structToReg(&commands[command]);
        return issueCommand(commandReg, arg, timeout);
    }

    bool Emmc::cardReset() {
        union {
            EmmcControl1 ctrl1;
            dword n = EMMC->control[1];
        } u;

        u.ctrl1.resetComplete = 1;
        u.ctrl1.clockEnable = 0;
        u.ctrl1.intClockEnable = 0;

        EMMC->control[1] = u.n;

        volatile int cycles = 0;
         int MAX_CYCLES = 10000000;



        logger.print("CONTROL1 1: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        logger.println("CARD RESETING...");

        if (!waitTimeout(&EMMC->control[1], (7 << 24), 0)) {
            logger.println("CARD REST TIMEOUT");
            return false;
        }

        logger.print("STATUS 1: ");
        logger.printBinary(EMMC->status);
        logger.println(" ");

        logger.println("Checking for inserted card....");

        if (!waitTimeout(&EMMC->status, (1 << 16), (1 << 16))) {
            logger.println("Not card inserted!");
            return false;
        }

        logger.print("STATUS 2: ");
        logger.printBinary(EMMC->status);
        logger.println(" ");


        logger.println("Found A Card");

        EMMC->control2 = 0;

        dword rate = getClockRate();

        logger.print("CONTROL1 2a: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        u.n = EMMC->control[1];

        logger.print("CONTROL1 2b: ");
        logger.printBinary(u.n);
        logger.println(" ");

        u.ctrl1.intClockEnable = 1;

        logger.print("CONTROL1 2c: ");
        logger.printBinary(u.n);
        logger.println(" ");

        u.n |= getClockDivider(dsos::getClockRate(CTEmmc), SD_CLOCK_ID);

        logger.print("CONTROL1 2d: ");
        logger.printBinary(u.n);
        logger.println(" ");

        u.n &= ~(0xf << 16);

        logger.print("CONTROL1 2e: ");
        logger.printBinary(u.n);
        logger.println(" ");

        u.n |= (11 << 16);

        logger.print("CONTROL1 2f: ");
        logger.printBinary(u.n);
        logger.println(" ");

        logger.print("CONTROL1 2: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        EMMC->control[1] = u.n;

        logger.print("CONTROL1 3: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        if (!waitTimeout(&EMMC->control[1], 0b10, 1)) {
            logger.println("SD Clock Not Stable");
            return false;
        }

        logger.print("CONTROL1 4: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        logger.println("Enabling SD Clock...");

        delayMs(3);
        EMMC->control[1] |= 4;
        delayMs(3);

        logger.print("CONTROL1 5: ");
        logger.printBinary(EMMC->control[1]);
        logger.println(" ");

        logger.print("IRQ 00: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");
        
        EMMC->irqFlags = 0xFFFFFFFF;

        logger.print("IRQ 01: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        EMMC->irqEnable = 0;
        EMMC->irqFlags = 0xFFFFFFFF;
        dword irptMask = 0xFFFFFFFF & ~(SD_CARD_INTERRUPT);
        logger.print("IRQ 0a: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        logger.print("IRQ 0aa: ");
        logger.printBinary(irptMask);
        logger.println(" ");

        dword d = 0xFEFEFEFE;
        memcpy((byte *)&d, (byte *)0x3F300030, 4);
        logger.print("IRQ 0ab: ");
        logger.printBinary(d);
        logger.println(" ");
        logger.print("IRQ 0ac: ");
        d = 0xFFFFFFFF;
        memcpy((byte *)0x3F300030, (byte *)&d, 4);
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");
        logger.print("IRQ 0ad: ");
        logger.printBinary(0b11000000000000000000000000000001);
        logger.println(" ");
        logger.print("IRQ 0ae: ");
        logger.printBinary(0b1000001000100110);
        logger.println(" ");
        logger.print("IRQ 0af: ");
        logger.printBinary(0b1000001000100111);
        logger.println(" ");

        EMMC->irqFlags = irptMask;

        logger.print("IRQ 0b: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");
        delayMs(3);

        logger.print("IRQ 0c: ");
        logger.printBinary(EMMC->irqFlags);
        logger.println(" ");

        transferBlocks = 0;
        lastCommandReg = 0;
        lastSuccess = 0;
        blockSize = 0;

        //printCommand(commands[0]);
        //printCommand(commands[1]);
        //printCommand(commands[2]);
        //printCommand(commands[3]);
        ////printCommand(commands[4]);

        logger.print("STATUS 3: ");
        logger.printBinary(EMMC->status);
        logger.println(" ");
        
        command(GO_IDLE, 0, 2000);

        logger.print("STATUS 4: ");
        logger.printBinary(EMMC->status);
        logger.println(" ");

        //CommandType

        return true;
    }

    dword getClockDivider(dword baseClock, dword targetRate) {
        dword targetDiv = 1;

        if (targetRate <= baseClock) {
            targetDiv = baseClock / targetRate;

            if (baseClock % targetRate) {
                targetDiv = 0;
            }
        }

        int div = -1;
        for (int fb = 31; fb >= 0; fb--) {
            dword bt = (1 << fb);

            if (targetDiv & bt) {
                div = fb;
                targetDiv &= ~(bt);

                if (targetDiv) {
                    div++;
                }

                break;
            }
        }

        if (div == -1) {
            div = 31;
        }

        if (div >= 32) {
            div = 31;
        }

        if (div != 0) {
            div = (1 << (div - 1));
        }

        if (div >= 0x400) {
            div = 0x3FF;
        }

        dword freqSel = div & 0xff;
        dword upper = (div >> 8) & 0x3;
        dword ret = (freqSel << 8) | (upper << 6) | (0 << 5);

        return ret;
    }

    bool Emmc::cardInit() {
        if (!powerOn()) {
            return false;
        }

        byte vendor = (EMMC->slotIrqStatus >> 24) & 0xFF;
        byte sdVersion = (EMMC->slotIrqStatus >> 16) & 0xFF;
        byte slotStatus = EMMC->slotIrqStatus & 0xFF;

        Logger &logger = Logger::inst();
        logger << "SDCard INFO: \r\n"
               << "    VENDOR : " << vendor << "\r\n"
               << "    VERSION: " << sdVersion << "\r\n"
               << "    STATUS : " << slotStatus << "\r\n"
               << "\r\n";

        return cardReset();
    }
}
}

