#include "mbed.h"
#include "USBKeyboard.h"
#include "uLCD_4DGL.h"
#include "PinDetect.h"
#include <string>


RawSerial  dev(p28,p27);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

uLCD_4DGL uLCD(p9, p10, p11);
PwmOut speaker(p21);
PinDetect pbMode(p20);
PinDetect pbDot(p24);
PinDetect pbDash(p23);
PinDetect pbSend(p22);
string fullMsg = "";
InterruptIn RPG_A(p14);
InterruptIn RPG_B(p15);

AnalogIn sig(p16);             // Gives the rectified and integrated EMG signal


char buffer[128];
volatile int mode = 0;   // 0 = text to morse, 1 = morse to text
string a = "";
volatile int write = 0;
volatile int stage = 0;

volatile float speed = 0.15;
volatile int old_enc = 0;
volatile int new_enc = 0;
volatile int enc_count = 0;
const int lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

void Enc_change_ISR(void)
{
    new_enc = RPG_A<<1 | RPG_B;
    enc_count = enc_count + lookup_table[old_enc<<2 | new_enc];
    old_enc = new_enc;
}

void adjSpeed() {
    if (enc_count > 9) enc_count = 9;
    if (enc_count < 0) enc_count = 1;
    speed = (float)enc_count / 100.0;
    uLCD.locate(8, 14);
    uLCD.printf(" ");
    uLCD.locate(0, 14);
    uLCD.color(RED);
    uLCD.printf("Speed: %d", enc_count);
    uLCD.color(GREEN);

   

}

void changeMode() {
    uLCD.locate(0,0);
    uLCD.printf("              ");
    stage++;
    mode = !mode;
    led2 = !led2;

    uLCD.cls();
    uLCD.locate(0,15);
    uLCD.color(WHITE);
    if(mode == 0) {uLCD.printf("Mode0: Txt to Mrs");}
    if(mode == 1) {uLCD.printf("Mode1: Mrs to Txt");}
    //if(mode == 2) {uLCD.printf("Mode2: EMG to Txt");}

    uLCD.color(GREEN);
    
    //uLCD.locate(0,0);

    fullMsg = "";
}

void clearBuffer(char* buf) {
    for(int i = 0; i < 128; i++) {
        buf[i] = 0;
    }
}

void dot() {a = a + ".";}
void dash() {a = a + "-";}
void sendMorse() {write = 1;}

void play(char* code) {
    int length = sizeof(code) - 1;
    speaker.period(1.0/1000.0);
    for(int i = 0; i <= length; i++) {
        if(code[i] == '.') {
            led1 = 1;
            speaker = 0.5;
            wait(speed);
            led1 = 0;
            speaker = 0.0;
        }
        if(code[i] == '-') {
            led1 = 1;
            speaker = 0.5;
            wait(speed * 2.0);
            led1 = 0;
            speaker = 0.0;
        }
        wait(speed);
    }
}


char* textToMorse(char c) {
    char* code;
    switch(c) {
        case 'a':
            code = ".- ";
            break;
        case 'b':
            code = "-... ";
            break;
        case 'c':
            code = "-.-. ";
            break;
        case 'd':
            code = "-.. ";
            break;
        case 'e':
            code = ". ";
            break;
        case 'f':
            code = "..-. ";
            break;
        case 'g':
            code = "--. ";
            break;
        case 'h':
            code = ".... ";
            break;
        case 'i':
            code = ".. ";
            break;
        case 'j':
            code = ".--- ";
            break;
        case 'k':
            code = "-.- ";
            break;
        case 'l':
            code = ".-.. ";
            break;
        case 'm':
            code = "-- ";
            break;
        case 'n':
            code = "-. ";
            break;
        case 'o':
            code = "--- ";
            break;
        case 'p':
            code = ".--. ";
            break;
        case 'q':
            code = "--.- ";
            break;
        case 'r':
            code = ".-. ";
            break;
        case 's':
            code = "... ";
            break;
        case 't':
            code = "- ";
            break;
        case 'u':
            code = "..- ";
            break;
        case 'v':
            code = "...- ";
            break;
        case 'w':
            code = ".-- ";
            break;
        case 'x':
            code = "-..- ";
            break;
        case 'y':
            code = "-.-- ";
            break;
        case 'z':
            code = "--.. ";
            break;
        case '0':
            code = "----- ";
            break;
        case '1':
            code = ".---- ";
            break;
        case '2':
            code = "..--- ";
            break;
        case '3':
            code = "...-- ";
            break;
        case '4':
            code = "....- ";
            break;
        case '5':
            code = "..... ";
            break;
        case '6':
            code = "-.... ";
            break;
        case '7':
            code = "--... ";
            break;
        case '8':
            code = "---.. ";
            break;
        case '9':
            code = "----. ";
            break;
        case ' ':
            code = "/ ";
            break;
        case '\n':
            code = "";
            break;
        default:
            code = "";
    }
    return code;
}

char morseToText(string a) {
    //uLCD.printf("%s",a);
    if(a == ".-") { return 'a';}
    if(a == "-...") { return 'b';}
    if(a == "-.-.") { return 'c';}
    if(a == "-..") { return 'd';}
    if(a == ".") { return 'e';}
    if(a == "..-.") { return 'f';}
    if(a == "--.") { return 'g';}
    if(a == "....") { return 'h';}
    if(a == "..") { return 'i';}
    if(a == ".---") { return 'j';}
    if(a == "-.-") { return 'k';}
    if(a == ".-..") { return 'l';}
    if(a == "--") { return 'm';}
    if(a == "-.") { return 'n';}
    if(a == "---") { return 'o';}
    if(a == ".--.") { return 'p';}
    if(a == "--.-") { return 'q';}
    if(a == ".-.") { return 'r';}
    if(a == "...") { return 's';}
    if(a == "-") { return 't';}
    if(a == "..-") { return 'u';}
    if(a == "...-") { return 'v';}
    if(a == ".--") { return 'w';}
    if(a == "-..-") { return 'x';}
    if(a == "-.--") { return 'y';}
    if(a == "--..") { return 'z';}
    if(a == ".----") { return '1';}
    if(a == "..---") { return '2';}
    if(a == "...--") { return '3';}
    if(a == "....-") { return '4';}
    if(a == ".....") { return '5';}
    if(a == "-....") { return '6';}
    if(a == "--...") { return '7';}
    if(a == "---..") { return '8';}
    if(a == "----.") { return '9';}
    if(a == "-----") { return '0';}
    if(a == ".---.") { return ' ';}
    return '$';
}


int main()
{
    //led2 = 0;
    dev.baud(9600);

    USBKeyboard keyboard;
    wait(0.5);

    pbMode.mode(PullDown);
    pbMode.attach_deasserted(&changeMode);
    pbMode.setSampleFrequency();

    pbDot.mode(PullUp);
    pbDot.attach_deasserted(&dot);
    pbDot.setSampleFrequency();

    pbDash.mode(PullUp);
    pbDash.attach_deasserted(&dash);
    pbDash.setSampleFrequency();

    pbSend.mode(PullUp);
    pbSend.attach_deasserted(&sendMorse);
    pbSend.setSampleFrequency();

    RPG_A.mode(PullUp);
    RPG_B.mode(PullUp);

    RPG_A.rise(&Enc_change_ISR);
    RPG_A.fall(&Enc_change_ISR);
    RPG_B.rise(&Enc_change_ISR);
    RPG_B.fall(&Enc_change_ISR);

    clearBuffer(buffer);

    uLCD.locate(0,15);
    uLCD.color(WHITE);
    uLCD.printf("Mode0: Txt to Mrs");
    uLCD.color(GREEN);


    while(1) {
        while(mode == 0) {
            while(!dev.readable() && mode == 0) {
                adjSpeed();
            }
            uLCD.locate(0,0);
            uLCD.printf("                 ");
            uLCD.locate(0,8);
            uLCD.printf("                                    ");

            if(mode == 1) {
                a="";
                continue;
            }
            uLCD.locate(0,0);
            clearBuffer(buffer);
            int i = 0;

            while(dev.readable()) {
                buffer[i] = dev.getc();
                i++;
            }
            string temp ="";
            for(int j = 0; j < 128; j++) {
                //char* code;
                if(buffer[j] != 0) {
                
                    char* translated_in_morse = textToMorse(buffer[j]);
                    temp = temp + translated_in_morse;
                    keyboard.printf(translated_in_morse);
                    uLCD.printf("%c", buffer[j]);
                }
            }
            uLCD.locate(0,8);
            uLCD.printf("%s",temp);


            for(int i = 0; i < 128; i++) {
                if(buffer[i] != 0) {
                    play(textToMorse(buffer[i]));
                }
            }
        }
        while(mode == 1) {
            //adjSpeed();
            if(dev.readable()) {
                for(int i = 0; i < 128; i++) {
                        buffer[i] = dev.getc();
                        i++;
                }   
                clearBuffer(buffer);
            }

            if(write == 1) {
                //uLCD.cls();
                uLCD.locate(0,8);
                uLCD.printf("              ");
                char c = morseToText(a);
                fullMsg = fullMsg + c;
                keyboard.printf("%c", c);
                uLCD.locate(0,0);
                uLCD.printf("%s", fullMsg);
                uLCD.locate(0,8);
                uLCD.printf("%s", a);
                write = 0;
                a = "";
                play(textToMorse(c));
                dev.puts(&c);


            }
        }
        

    }
}
