
//class constructor based off of @mannkind
//ESPHomeRoombaComponent/ESPHomeRoombaComponent.h
//https://github.com/mannkind/ESPHomeRoombaComponent

#define USE_UART_DEBUGGER

#include "esphome.h"
#include <vector>
#include "esphome/components/uart/uart_debugger.h"


static const char *TAG = "component.core300s_UART";

class component_core300sUART : 
        public PollingComponent,
        public UARTDevice,
        public CustomAPIDevice {

  public:
    
    Sensor *sensor_pm25;
    Sensor *sensor_roomSize;
   
    TextSensor *textsensor_fanSpeed;
    TextSensor *textsensor_Power;  
    TextSensor *textsensor_mcuFW;
    TextSensor *textsensor_FanMode; 
    TextSensor *textsensor_DisplayLit;
    TextSensor *textsensor_DisplayLocked;
    TextSensor *textsensor_FanAutoMode;


    
    static component_core300sUART* instance(UARTComponent *parent)
    {
        static component_core300sUART* INSTANCE = new component_core300sUART(parent);
        return INSTANCE;
    }

    //delay setup() so that core300s board has booted completely before
    //starting to rx/tx on bus
    float get_setup_priority() const override 
    { 
        return esphome::setup_priority::LATE; 
    }

    void setup() override
    {
        ESP_LOGD(TAG, "setup().");
        
        rx_buf.reserve(128);
        
        sensor_pm25->publish_state(NAN);
        sensor_roomSize->publish_state(NAN);
        
        textsensor_fanSpeed->publish_state("Unknown");        
        textsensor_Power->publish_state("Unknown");      
        textsensor_mcuFW->publish_state("Unknown");    
        textsensor_FanMode->publish_state("Unknown");    
        textsensor_DisplayLit->publish_state("Unknown");    
        textsensor_DisplayLocked->publish_state("Unknown");    
        textsensor_FanAutoMode->publish_state("Unknown");    
        
        set_update_interval(200);

        //Services to allow home assistant to control air filter
        register_service(&component_core300sUART::set_fan_manual_high, "set_fan_manual_high");
        register_service(&component_core300sUART::set_fan_manual_medium, "set_fan_manual_medium");
        register_service(&component_core300sUART::set_fan_manual_low, "set_fan_manual_low");        
    }

    void update() override
    {

        //longest messages should take about 2msec to tx from MCU
        while ( available() ) {
            read_byte(&b);

            //Look for start byte
            if( (b == 0xA5) )  {
                rx_buf.clear();
                rx_buf.push_back(b);

                //Read until checksum
                for(int i=0; i<5; i++) {
                    if(read_byte(&b))
                        rx_buf.push_back(b);
                    else
                        break;
                }

                if(rx_buf.size() == 6) {
                    //Read rest of packet
                    for(int i=0; i<rx_buf[3]; i++) {
                        if(read_byte(&b))
                            rx_buf.push_back(b);
                        else
                            break;
                    }

                    //Process packet if payload was read complete
                    if(rx_buf.size() == (rx_buf[3]+6) ) {
                        process_packet();
                    }
                }
            }
          }
          
              
        }

        
  private: 
    uint8_t b=0;
    std::vector<uint8_t> rx_buf; 
   
    //std::vector<uint8_t> erd2050= {0XE2, 0XC0, 0X0B, 0XBB, 0XF0, 0X01, 0X20, 0X50, 0XC4, 0XCB, 0xE3};    //Dryer Heat Setting  

    component_core300sUART(UARTComponent *parent) : PollingComponent(200), UARTDevice(parent) 
    {
        this->sensor_pm25  = new Sensor();
        this->sensor_roomSize  = new Sensor();

        this->textsensor_fanSpeed  = new TextSensor();
        this->textsensor_Power  = new TextSensor();
        this->textsensor_mcuFW  = new TextSensor();
        this->textsensor_FanMode  = new TextSensor();
        this->textsensor_DisplayLit  = new TextSensor();
        this->textsensor_DisplayLocked  = new TextSensor();
        this->textsensor_FanAutoMode  = new TextSensor();
        
    }
        
    void set_fan_manual_high(void) {
        ESP_LOGD(TAG, "set_fan_manual_high");
        //write_str(buf);
    }
        
    void set_fan_manual_medium(void) {
        ESP_LOGD(TAG, "set_fan_manual_medium");
        //write_str(buf);
    }
        
    void set_fan_manual_low(void) {
        ESP_LOGD(TAG, "set_fan_manual_low");
        //write_str(buf);
    }


    void process_packet()  {
        //uart::UARTDebug::log_hex(uart::UARTDirection::UART_DIRECTION_RX , rx_buf, '-');
        char buf[32];        
        unsigned int packet_type=0;
        packet_type = (rx_buf[6] << 16) + (rx_buf[7] << 8) + rx_buf[8];
        ESP_LOGD(TAG, "process_packet type %X", packet_type);
        
        switch (packet_type) {
            case 0x013040:  //Status packet
                if (rx_buf.size() != 28)
                    return;

                sprintf(buf, "%d.%d.%d",rx_buf[12], rx_buf[11],rx_buf[10]);
                textsensor_mcuFW->publish_state(buf);    

                switch(rx_buf[13]) {  //Power
                    case 0x00:
                    textsensor_Power->publish_state("Off");
                    break;

                    case 0x01:
                    textsensor_Power->publish_state("On");
                    break;

                    default:
                    sprintf(buf, "Error %X", rx_buf[13]);
                    textsensor_Power->publish_state(buf);
                }

                switch(rx_buf[14]) {  //Fan Mode
                    case 0x00:
                    textsensor_FanMode->publish_state("Manual");
                    break;

                    case 0x01:
                    textsensor_FanMode->publish_state("Sleep");
                    break;

                    case 0x02:
                    textsensor_FanMode->publish_state("Auto");
                    break;                

                    default:
                    sprintf(buf, "Error %X", rx_buf[14]);
                    textsensor_FanMode->publish_state(buf);
                }   

                switch(rx_buf[18]) {  //Fan Speed
                    case 0x00:
                    textsensor_fanSpeed->publish_state("Sleep");
                    break;

                    case 0x01:
                    textsensor_fanSpeed->publish_state("Low");
                    break;

                    case 0x02:
                    textsensor_fanSpeed->publish_state("Medium");
                    break;  

                    case 0x03:
                    textsensor_fanSpeed->publish_state("High");
                    break;

                    case 0xFF:
                    textsensor_fanSpeed->publish_state("Off");
                    break;  

                    default:
                    sprintf(buf, "Error %X", rx_buf[18]);
                    textsensor_fanSpeed->publish_state(buf);
                } 

                switch(rx_buf[16]) {  //Screen
                    case 0x00:
                    textsensor_DisplayLit->publish_state("Off");
                    break;

                    case 0x64:
                    textsensor_DisplayLit->publish_state("On");
                    break;

                    default:
                    sprintf(buf, "Error %X", rx_buf[16]);
                    textsensor_DisplayLit->publish_state(buf);
                }
                                        
                sensor_pm25->publish_state( (rx_buf[22]<<8) + rx_buf[21]);

                switch(rx_buf[23]) {  //Display Lock
                    case 0x00:
                    textsensor_DisplayLocked->publish_state("Unlocked");
                    break;

                    case 0x01:
                    textsensor_DisplayLocked->publish_state("Locked");
                    break;

                    default:
                    sprintf(buf, "Error %X", rx_buf[23]);
                    textsensor_DisplayLocked->publish_state(buf);
                }

                switch(rx_buf[24]) {  //Fan Mode
                    case 0x00:
                    textsensor_FanAutoMode->publish_state("Default");
                    break;

                    case 0x01:
                    textsensor_FanAutoMode->publish_state("Quiet");
                    break;

                    case 0x02:
                    textsensor_FanAutoMode->publish_state("Efficient");
                    break;                

                    default:
                    sprintf(buf, "Error %X", rx_buf[24]);
                    textsensor_FanAutoMode->publish_state(buf);
                }

                sensor_roomSize->publish_state( (rx_buf[26]<<8) + rx_buf[25]);

            break;
        }

    }

};

