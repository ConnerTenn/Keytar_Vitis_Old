
/**
* Hello World program using XUartPs driver in polled mode
*
* The example uses the default setting in the XUartPs driver:
*	. baud rate 115200
*	. 8 bit data
*	. 1 stop bit
*	. no parity
*
* @note
* This example requires an external SchmartModule connected to the pins for
* the device to display the 'Hello World' message onto a hyper-terminal.
*
*/

#include "../../Synth_system/Common.h"

#include "xpseudo_asm.h"
#include "xil_mmu.h"

#include "xil_cache.h"
#include "xgpiops_hw.h"

// #include "xil_exception.h"
// #include "xscugic.h"

#include "sleep.h"

#include "Synth.h"

#define KEYBOARD_PRINT 0//1

#define CPU1STARTADDRREG 0xfffffff0
#define CPU1IMAGEADR 0x02000000

int InitCPU1()
{
    print("CPU0: writing startaddress for CPU1\n");
    Xil_Out32(CPU1STARTADDRREG, CPU1IMAGEADR);
    dmb(); //Wait for write to complete

    print("CPU0: sending the SEV to wake up CPU1\n");
    sev();

    return XST_SUCCESS;
}

int InitUart(u16 DeviceId)
{
    int status;

    Out32(PRINT_MUTEX_ADDR, 0);

    //Initialize the UART driver so that it's ready to use
    //Look up the configuration in the config table and then initialize it.
    XUartPs_Config *Config = XUartPs_LookupConfig(DeviceId);
    if (Config == NULL) { return XST_FAILURE; }

    XUartPs Uart_Ps; //The instance of the UART Driver
    status = XUartPs_CfgInitialize(&Uart_Ps, Config, Config->BaseAddress);
    if (status != XST_SUCCESS) { return XST_FAILURE; }

    XUartPs_SetBaudRate(&Uart_Ps, 115200);
    if (status != XST_SUCCESS) { return XST_FAILURE; }

    return XST_SUCCESS;
}

int InitAnalog()
{
    PRINT("CPU0: Initalizing DAC/ADC\n");
    //Initialize device
    ANALOG_CTL_RESET_REG = 1;
    ANALOG_CTL_WRITE(0x7, 0x03);

    PRINT("CPU0: Configuring DAC/ADC\n");

    //Mode=0b10; Double speed (50khz-100khz)
    //Ratio=0b00; 128x (Slave Mode)
    //Master/Slave=0b0; Slave Mode
    //Mode=0b001 (I2S 24 bit)
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x1, 0b10000001);

    //Auto Mute=0b0; Off
    //Filter select=0b0; Fast rolloff
    //De-Emphasis=0b00; Disabled
    //Soft Ramp-Up=0b0; Ramp is immediate
    //Soft Ramp-Down=0b0; Ramp is immediate
    //Invert Polarity=0b00; Disabled
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x2, 0b00000000);

    //Reserved=0b0
    //Combined channel volume=0b1; Combined (Controlled by channel A)
    //Soft Ramp or Zero Cross=0b01; Zero cross enabled
    //Channel mixing and muting=0b1001; A=Left, B=Right
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x3, 0b01011001);

    //Mute=0b0
    //Volume=0b0000000; -0dB
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x4, 0b00000000);

    //Mute=0b0
    //Volume=0b0000000; -0dB
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x5, 0b00000000);

    //Reserved=0b00
    //Dither=0b0; Dither disabled
    //ADC Format=0b1; Format 1
    //ADC Mute=0b00
    //High pass filter disable=0b11; Disabled both channels
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x6, 0b00010011);

    //Reserved=0b000
    //Loopback=0b0
    //AMute and BMute=0b0; Not combined
    //Freeze=0b0
    //Control Port Enable=0b1
    //Power Down=0b0; Power Up!
    ANALOG_CTL_SYNC();
    ANALOG_CTL_WRITE(0x7, 0b00000010);


    PRINT("CPU0: DAC/ADC Configured\n");

    return XST_SUCCESS;
}

int InitFilter(u8 bank)
{
    PRINT("CPU0: InitFilter[%d]\n", bank);

    SYNTH_FILTER_COEFF_REG(bank, 0) = 0x100000;
    for (u16 i=1; i<MAX_FILTER_DEPTH; i++)
    {
        SYNTH_FILTER_COEFF_REG(bank, i) = 0x000000;
    }

    // PRINT("CPU0: Filter Coeff: ");
    // for (u16 i=0; i<MAX_FILTER_DEPTH; i++)
    // {
    //     PRINT("0x%06X ", SYNTH_FILTER_COEFF_REG(bank, i));
    // }
    // PRINT("\n");

    PRINT("CPU0: Filter Swap\n");
    SYNTH_FILTER_SWAP(bank);

    return XST_SUCCESS;
}

int Init()
{
    //Disable cache on OCM
    Xil_SetTlbAttributes(SHARED_ADDR, 0x14de2); //S=b1 TEX=b100 AP=b11, Domain=b1111, C=b0, B=b0
    //Disable cache on DDR segment
    Xil_SetTlbAttributes(0x30000000, NORM_NONCACHE);


    if (InitUart(XPAR_XUARTPS_0_DEVICE_ID) != XST_SUCCESS)
    {
        PRINT("CPU0: Failed to initialize UART\n");
        return XST_FAILURE;
    }

    if (InitCPU1() != XST_SUCCESS)
    {
        PRINT("CPU0: Filed to initialize CPU 1\n");
        return XST_FAILURE;
    }

    if (InitAnalog() != XST_SUCCESS)
    {
        PRINT("CPU0: Filed to initialize the DAC/ADC\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int main()
{
    if (Init() != XST_SUCCESS)
    {
        PRINT("CPU0: Init Failed\n");
        return XST_FAILURE;
    }

    // XGpioPs_WriteReg(XPAR_PS7_GPIO_0_BASEADDR, XGPIOPS_DIRM_OFFSET, 0);

    usleep(500*1000); //500ms
    
    PRINT("CPU0: Begin\n");

    for (u8 k=0; k<sizeof(KeyState); k++)
    {
        KeyState[k] = 0;
        KeyBankChannel[k][0] = (u8)-1;
        KeyBankChannel[k][1] = (u8)-1;
    }

    for (u8 b=0; b<MAX_BANKS-1; b++)
    {
        InitFilter(b);

        SYNTH_WAVETYPE_REG(b) = 1;
        SYNTH_PULSEWIDTH_REG(b) = 0;

        SYNTH_ATTACK_REG(b) = 10000;
        SYNTH_DECAY_REG(b) = 50;
        SYNTH_SUSTAIN_REG(b) = 0xEF0000;
        SYNTH_RELEASE_REG(b) = 300;

        SYNTH_LFOINCR_REG(b) = 1000;
        SYNTH_LFOAMP_REG(b) = 0x000100u;
        SYNTH_LFORUN_REG(b) = 1;
        SYNTH_LFOWAVETYPE_REG(b) = 2;
        SYNTH_LFOSELECTION_STRUCT(b) = (LfoSelection){.Increment=1,.PulseWidth=0};
    }

    SYNTH_WAVETYPE_REG(MAX_BANKS-1) = 1;
    SYNTH_PULSEWIDTH_REG(MAX_BANKS-1) = 0;

    SYNTH_ATTACK_REG(MAX_BANKS-1) = 1000;
    SYNTH_DECAY_REG(MAX_BANKS-1) = 0;
    SYNTH_SUSTAIN_REG(MAX_BANKS-1) = 0xEFFFFF;
    SYNTH_RELEASE_REG(MAX_BANKS-1) = 50;


#if KEYBOARD_PRINT
    u8 clear = 0;
    u32 count = 0;
#endif

    u32 dly = 0;

    while (1)
    {
        dly++;
        //Keyboard
        for (u8 i=0; i<8; i++)
        {
            KEYBOARD_REG = (1<<i);
            dsb(); //Wait for write to complete
            usleep(5);
            u8 keys = KEYBOARD_REG;
            dsb(); //Wait for Read to complete

            // PRINT("0x%02X:0x%02X  ", KEYBOARD_DRIVE_REG, keys); dmb(); //Wait for Read to complete

            KEYBOARD_REG = 0;
            // dsb(); //Wait for write to complete

            // if (i == 5 && ((dly%10000)>=5000))
            // {
            //     keys = keys | 1;
            // }

            // if (i == 3 && ((dly%4000)>=2000))
            // {
            //     keys = keys | 1;
            // }

            for (u8 k=0; k<8 && i*8+k<sizeof(KeyState); k++)
            {
                u8 *state = &KeyState[i*8+k];
                u8 pressed = (keys>>k)&1;
                switch (*state)
                {
                case 0: //Unpressed
                    *state = pressed ? 1 : 0;
                    break;
                case 1: //KeyDown
                case 2: //KeyPressed
                    *state = pressed ? 2 : 3;
                    break;
                case 3: //KeyUp
                    *state = pressed ? 1 : 0;
                    break;
                default: //Unknown
                    *state = pressed ? 1 : 3;
                    break;
                }
            }
        }

        for (u8 b=0; b<MAX_BANKS; b++)
        {
            if (KeyState[0] == 1) { SYNTH_WAVETYPE_REG(b) = 0; }
            else if (KeyState[1] == 1) { SYNTH_WAVETYPE_REG(b) = 1; }
            else if (KeyState[2] == 1) { SYNTH_WAVETYPE_REG(b) = 2; }
            else if (KeyState[3] == 1) { SYNTH_WAVETYPE_REG(b) = 3; }
        }
        for (u8 k=4; k<sizeof(KeyState); k++)
        {
            if (KeyState[k] == 1) //KeyDown
            {
                u8 bank = 0;
                u8 channel = 0;
                u8 found = 0;

                //Find an unused oscillator
                while (bank<MAX_BANKS-1 && channel<MAX_CHANNELS && !found)
                {
                    if (SYNTH_RUNNING_REG(bank,channel)==0)
                    {
                        found = 1;
                    }
                    else
                    {
                        channel++;
                        if (channel==MAX_CHANNELS)
                        {
                            bank++;
                            channel=0;
                        }
                    }
                }

                //Set the oscillator to play the note
                if (bank<MAX_BANKS && channel<MAX_CHANNELS)
                {
                    KeyBankChannel[k][0] = bank;
                    KeyBankChannel[k][1] = channel;

                    SYNTH_INCR_REG(bank,channel) = NoteIncrs[k + KeyOctaveOffset];
                    SYNTH_GATE_REG(bank,channel) = 1;

                    PRINT("CPU0: " TERM_MAGENTA "Gate ON   Bank:%d  Channel:%d  Incr:%d\n" TERM_RESET, bank, channel, NoteIncrs[k + KeyOctaveOffset]);
#if KEYBOARD_PRINT
                    clear = 0;
#endif
                }
            }
            else if (KeyState[k] == 3) //KeyUP
            {
                u8 bank = KeyBankChannel[k][0];
                u8 channel = KeyBankChannel[k][1];
                if (bank!=(u8)-1 && channel!=(u8)-1)
                {
                    SYNTH_GATE_REG(bank,channel) = 0;
                    KeyBankChannel[k][0] = (u8)-1;
                    KeyBankChannel[k][1] = (u8)-1;

                    PRINT("CPU0: " TERM_MAGENTA "Gate OFF  Bank:%d  Channel:%d\n" TERM_RESET, bank, channel);
#if KEYBOARD_PRINT
                    clear = 0;
#endif
                }
            }
        }

        if ((s32)TOUCHSTRIP_PRESSURE_REG>(s32)(0.5*0x7FFFFF))
        {
            u8 bank = MAX_BANKS-1;
            u8 channel = 0;

            s32 pos = (s32)TOUCHSTRIP_POSITION_REG;
            float upper = NoteIncrs[12*4+11];
            float lower = NoteIncrs[12*4];
            u32 incr = (float)pos/(0.7f*0x7FFFFF) * (upper-lower)/2.0f + (upper+lower)/2.0f;

//             if (SYNTH_RUNNING_REG(bank,channel)==0)
//             {
//                 PRINT("CPU0: " TERM_MAGENTA "Gate ON   Bank:%d  Channel:%d  Incr:%d\n" TERM_RESET, bank, channel, incr);
// #if KEYBOARD_PRINT
//                 clear = 0;
// #endif
//             }

            //Set the oscillator to play the note
            SYNTH_INCR_REG(bank,channel) = incr;
            SYNTH_GATE_REG(bank,channel) = 1;

        }
        else
        {
            u8 bank = MAX_BANKS-1;
            u8 channel = 0;

//             if (SYNTH_RUNNING_REG(bank,channel)==1)
//             {
//                 PRINT("CPU0: " TERM_MAGENTA "Gate OFF  Bank:%d  Channel:%d\n" TERM_RESET, bank, channel);
// #if KEYBOARD_PRINT
//                 clear = 0;
// #endif
//             }

            SYNTH_GATE_REG(bank,channel) = 0;
        }


#if KEYBOARD_PRINT
        if (count == 0)
        {
            PRINT_GETLOCK;

            static u8 nl = 0;
            if (clear) { TERM_MOVE_UP(nl); }
            nl = 0;

            PRINT_NOLOCK("\n"); nl++;
            for (u8 k=0; k<sizeof(KeyState); k++)
            {
                PRINT_NOLOCK("C \0C#\0D \0D#\0E \0F \0F#\0G \0G#\0A \0A#\0B \0"+3*(k%12));
            }
            PRINT_NOLOCK("\n"); nl++;
            for (u8 k=0; k<sizeof(KeyState); k++)
            {
                PRINT_NOLOCK("%s%d ", KeyState[k]?TERM_CYAN:TERM_RESET, KeyState[k]);
            }
            PRINT_NOLOCK(TERM_RESET "\n"); nl++;

            for (u8 b=0; b<MAX_BANKS; b++)
            {
                for (u8 c=0; c<MAX_CHANNELS; c++)
                {
                    if (SYNTH_RUNNING_REG(b,c))
                    {
                        PRINT_NOLOCK("[%d,%2d]:" TERM_CYAN "%d:%d:%6d  " TERM_RESET, b, c, SYNTH_GATE_REG(b,c), SYNTH_ADSR_STATE_REG(b,c), SYNTH_INCR_REG(b,c));
                    }
                    else
                    {
                        PRINT_NOLOCK("[%d,%2d]:OFF         ", b, c);
                    }
                    if (c%8==7)
                    {
                        PRINT_NOLOCK("\n"); nl++;
                    }
                }
                PRINT_NOLOCK("\n"); nl++;
            }

            PRINT_NOLOCK("\n"); nl++;

            s32 position = TOUCHSTRIP_POSITION_REG;
            s32 pressure = TOUCHSTRIP_PRESSURE_REG;
            PRINT_NOLOCK("Strip Position: %9d %08X  %c0.%04d\n", position, position, position>=0?'+':'-', (s32)abs(10000.0f*(float)position/(float)0x7FFFFF)); nl++;
            PRINT_NOLOCK("Strip Pressure: %9d %08X  %c0.%04d\n", pressure, pressure, pressure>=0?'+':'-', (s32)abs(10000.0f*(float)pressure/(float)0x7FFFFF)); nl++;

            PRINT_NOLOCK("\n"); nl++;
            PRINT_NOLOCK("\n"); nl++;
            
            clear = 1;
            
            PRINT_RELEASELOCK;
        }

        count=count<200?count+1:0;
#endif
    }


    return XST_SUCCESS;
}


