    #include "mbed.h"
    #include "bbcar.h"
#include "drivers/DigitalOut.h"
    
#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "blink_led_server.h"
/**
 * Macros for setting console flow control.
 */
#define CONSOLE_FLOWCONTROL_RTS     1
#define CONSOLE_FLOWCONTROL_CTS     2
#define CONSOLE_FLOWCONTROL_RTSCTS  3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;

/** LED service */
LEDBlinkService_service led_service;




    #define velocity 60
    #define CENTER_BASE 1500

    //BufferedSerial serdev(D1, D0, 9600);

    Ticker servo_ticker;
    Ticker encoder_ticker;
    Thread qti;
    Thread Move;
    Thread Distance;
    Thread PING;

    Thread eRPC;
    PwmOut pin5(D10), pin6(D11);
    DigitalIn encoder(D3); // encoder
    DigitalInOut pin10(D9); // ping 
    volatile int steps;
    volatile int last;
    BBCar car(pin5, pin6, servo_ticker);

    // QTI Sensor
    DigitalInOut pin1(D4);
    DigitalInOut pin2(D5);
    DigitalInOut pin3(D6);
    DigitalInOut pin4(D7);
    int Sensor1;
    int Sensor2;
    int Sensor3;
    int Sensor4;
    int US = 20;
    int wL, wR;
    // Branch
    int right_turn = 0;
    int left_turn = 0;
    bool erpc_check = 0;

    int get_speed(){
        float init;
        float fin;
        int speed;
        init = steps * 6.5 * 3.14 / 32;
        ThisThread::sleep_for(100ms);
        fin = steps * 6.5 * 3.14 / 32;
        speed = (fin - init) * 10;
        return speed;
    }

    mbed::DigitalOut led3(LED3, 1);
    mbed::DigitalOut* leds[] = { &led3 };
    void led_on(uint8_t led) {
        erpc_check = 1;
    }
    void led_off(uint8_t led) {
    }
    // QTI Sensing System in thread
    void qti_sensing(){
        while (true) {
            pin1.output();
            pin2.output();
            pin3.output();
            pin4.output();

            pin1.write(1);
            pin2.write(1);
            pin3.write(1);
            pin4.write(1);

            ThisThread::sleep_for(1ms);

            pin1.input();
            pin2.input();
            pin3.input();
            pin4.input();

            pin1.write(0);
            pin2.write(0);
            pin3.write(0);
            pin4.write(0);

            wait_us(400);
            Sensor1 = pin1.read();
            Sensor2 = pin2.read();
            Sensor3 = pin3.read();
            Sensor4 = pin4.read();
 //           printf("%d, %d, %d, %d \n", Sensor1,Sensor2, Sensor3,Sensor4);
        }
    }
    void ping(){
        parallax_ping  ping1(pin10);
        while(1){
        US = (float)ping1;
        ThisThread::sleep_for(300ms);
        }
    }
    // encoder
    void encoder_control() {
        int value = encoder;
        if (!last && value) steps++;
        last = value;
    }

    // Servo Control System
    void servo0_control(int speed) {
        if (speed > 200)       speed = 200;
        else if (speed < -200) speed = -200;

        pin5 = (CENTER_BASE + speed)/20000.0f;
    }
    void servo1_control(int speed) {
        if (speed > 200)       speed = 200;
        else if (speed < -200) speed = -200;

        pin6 = (CENTER_BASE + speed)/20000.0f;
    }

    // Moving Thread
    void Moving(){
        int time = 0;
        pin5.period_ms(20);
        pin6.period_ms(20);
        while (1) {
            wL = 0;
            wR = 0;
            if (US < 25)
            {
                wL = 60;
                wR = 60*0.6;
                time = 200000;
            }
            else if (Sensor1 == 1 && Sensor2 == 0 && Sensor3 == 0 && Sensor4 == 0){
                wL = velocity * 0.05;
                wR = -velocity;
                time = 1;
            }
            else if (Sensor1 == 1 && Sensor2 == 1 && Sensor3 == 0 && Sensor4 == 0){
                wL = velocity * 0.2;
                wR = -velocity;
                time = 1;
            }

            else if (Sensor1 == 1 && Sensor2 == 1 && Sensor3 == 1 && Sensor4 == 0){
                time = 40000;
                wL = 100 * 0.1;
                wR = -100;
            }
            else if (Sensor1 == 0 && Sensor2 == 1 && Sensor3 == 1 && Sensor4 == 1){
                time = 40000;
                wL = 100;
                wR = -100 * 0.1;
            }

            else if (Sensor1 == 0 && Sensor2 == 1 && Sensor3 == 1 && Sensor4 == 0){
                wL = velocity;
                wR = -velocity;
                time = 1;
                }

            else if (Sensor1 == 0 && Sensor2 == 0 && Sensor3 == 1 && Sensor4 == 1){
                wL = velocity;
                wR = -velocity * 0.2;
                time = 1;
            } 

            else if (Sensor1 == 0 && Sensor2 == 1 && Sensor3 == 0 && Sensor4 == 0) {
                wL = velocity * 0.8;
                wR = -velocity ;
                time = 1;
            }
            else if (Sensor1 == 0 && Sensor2 == 0 && Sensor3 == 1 && Sensor4 == 0) {
                wL = velocity;
                wR = -velocity * 0.8;
                time = 1;
            }

            else if (Sensor1 == 0 && Sensor2 == 0 && Sensor3 == 0 && Sensor4 == 1){
                wL = velocity;
                wR = -velocity * 0.05;
                time = 1000;
            }  
            else if (Sensor1 == 0 && Sensor2 == 0 && Sensor3 == 0 && Sensor4 == 0) {
                wL = 0;
                wR = 0;
                time = 1;
            }
            else if (Sensor1 == 1 && Sensor2 == 1 && Sensor3 == 1 && Sensor4 == 1) {
                    wL = 0;
                    wR = 0;
                    time = 1;
            }
            for (int j = 1 ; j <= time ; j++) {
                servo0_control(wL);
                servo1_control(wR);
            }
        }
    }
    void erpc_server()
    {
        rpc_server.run();
    }
    // main() runs in its own thread in the OS
    int main()
    {
        int dist;
        int speed_test;
        char instr[32];
        uart_transport.setCrc16(&crc16);

	// Set up hardware flow control, if needed
#if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
	uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
	uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
	uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
#endif
	
    printf("Initializing server.\n");
	rpc_server.setTransport(&uart_transport);
	rpc_server.setCodecFactory(&basic_cf);
	rpc_server.setMessageBufferFactory(&dynamic_mbf);

	// Add the led service to the server
        printf("Adding LED server.\n");
	    rpc_server.addService(&led_service);

	// Run the server. This should never exit
        printf("Running server.\n");
	    eRPC.start(erpc_server);   

        encoder_ticker.attach(&encoder_control, 10ms);
        qti.start(qti_sensing);
        Move.start(Moving);
        PING.start(ping);
        memset(instr, '\0', 32); //clear buffer
        while(1){
            printf("while \n");
            if(erpc_check == 1){
                eRPC.terminate();
                while(1){
                    printf("success erpc");
                    dist = steps * 6.5 * 3.14 / 32 ;
                    sprintf(instr, "Distance traveled : %d cm\n", dist);
                    uart_transport.write(instr, strlen(instr));
                    ThisThread::sleep_for(1s);
                    memset(instr, '\0', 32);

                    speed_test = get_speed();
                    sprintf(instr, "Current speed : %d cm\n", speed_test);
                    uart_transport.write(instr, strlen(instr));
                    ThisThread::sleep_for(1s);
                    memset(instr, '\0', 32);
                }
            }
        }
    }


