
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
        tx_buf.reserve(32);
        
        sensor_pm25->publish_state(NAN);
        sensor_roomSize->publish_state(NAN);
        
        textsensor_fanSpeed->publish_state("Unknown");        
        textsensor_Power->publish_state("Unknown");      
        textsensor_mcuFW->publish_state("Unknown");    
        textsensor_FanMode->publish_state("Unknown");    
        textsensor_DisplayLit->publish_state("Unknown");    
        textsensor_DisplayLocked->publish_state("Unknown");    
        textsensor_FanAutoMode->publish_state("Unknown");    
        
        set_update_interval(5);

        send_command(cmd_get_status);
  
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
    
    void power_on(void) {
        ESP_LOGD(TAG, "power_on");
        send_command(cmd_power_on);
    }
        
    void power_off(void) {
        ESP_LOGD(TAG, "power_off");
        send_command(cmd_power_off);    
    }

    void set_fan_manual_high(void) {
        ESP_LOGD(TAG, "set_fan_manual_high");
        send_command(cmd_set_fan_manual_high);
    }
        
    void set_fan_manual_medium(void) {
        ESP_LOGD(TAG, "set_fan_manual_medium");
        send_command(cmd_set_fan_manual_medium);    
    }
        
    void set_fan_manual_low(void) {
        ESP_LOGD(TAG, "set_fan_manual_low");
        send_command(cmd_set_fan_manual_low);
    }

    void set_fan_mode_manual(void) {
        ESP_LOGD(TAG, "set_fan_mode_manual");
        send_command(cmd_set_fan_mode_manual);
    }

    void set_fan_mode_sleep(void) {
        ESP_LOGD(TAG, "set_fan_mode_sleep");
        send_command(cmd_set_fan_mode_sleep);
    }
        
    void set_fan_mode_auto(void) {
        ESP_LOGD(TAG, "set_fan_mode_auto");
        send_command(cmd_set_fan_mode_auto);    
    }
        
    void set_fan_auto_ppm(void) {
        ESP_LOGD(TAG, "cmd_set_fan_auto_ppm");
        send_command(cmd_set_fan_auto_ppm);
    }
        
    void set_fan_auto_ppmQuiet(void) {
        ESP_LOGD(TAG, "set_fan_auto_ppmQuiet");
        send_command(cmd_set_fan_auto_ppmQuiet);
    }
        
    void set_fan_auto_area(void) {
        ESP_LOGD(TAG, "set_fan_auto_area");
        cmd_set_fan_auto_area[11]=areaL;
        cmd_set_fan_auto_area[12]=areaH;
        send_command(cmd_set_fan_auto_area);    
    }

    void lock_display(void) {
        ESP_LOGD(TAG, "lock_display");
        send_command(cmd_lock_display);
    }

    void unlock_display(void) {
        ESP_LOGD(TAG, "unlock_display");
        send_command(cmd_unlock_display);
    }
        
    void display_on(void) {
        ESP_LOGD(TAG, "display_on");
        send_command(cmd_display_on);    
    }
        
    void display_off(void) {
        ESP_LOGD(TAG, "display_off");
        send_command(cmd_display_off);
    }

    void wifi_led_off(void) {
        if (wifi_led==0) {
            return;
        }
        ESP_LOGD(TAG, "wifi_led_off");
        send_command(cmd_wifi_led_off);
        wifi_led=0;
    }
        
    void wifi_led_flash(void) {
        if (wifi_led==2) {
            return;
        }
        ESP_LOGD(TAG, "wifi_led_flash");
                send_command(cmd_wifi_led_flash);    
        wifi_led=2;
    }
        
    void wifi_led_on(void) {
        if (wifi_led==1) {
            return;
        }
        ESP_LOGD(TAG, "wifi_led_on");
        send_command(cmd_wifi_led_on);
        wifi_led=1;
    }      

    void filter_led_on(void) {
        ESP_LOGD(TAG, "filter_led_on");
        send_command(cmd_filter_led_on);    
    }
        
    void filter_led_off(void) {
        ESP_LOGD(TAG, "filter_led_off");
        send_command(cmd_filter_led_off);
    }              
          
  private: 
    uint8_t b=0;
    std::vector<uint8_t> rx_buf,tx_buf;
    uint8_t tx_seq_num = 0;
    uint8_t wifi_led = 0;
    uint8_t areaH=0x01, areaL=0x3B;

    std::vector<uint8_t> cmd_get_status            = {0XA5, 0X22, 0XFF, 0X04, 0X00, 0XFF, 0X01, 0X31, 0X40, 0X00};

    std::vector<uint8_t> cmd_power_on              = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X00, 0XA0, 0X00, 0X01};
    std::vector<uint8_t> cmd_power_off             = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X00, 0xA0, 0X00, 0X00};  

    std::vector<uint8_t> cmd_set_fan_manual_high   = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0X60, 0XA2, 0X00, 0X00, 0X01, 0X03};
    std::vector<uint8_t> cmd_set_fan_manual_medium = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0X60, 0XA2, 0X00, 0X00, 0X01, 0X02};
    std::vector<uint8_t> cmd_set_fan_manual_low    = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0X60, 0XA2, 0X00, 0X00, 0X01, 0X01};

    std::vector<uint8_t> cmd_set_fan_auto_ppm      = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0XE6, 0XA5, 0X00, 0X00, 0X00, 0X00};
    std::vector<uint8_t> cmd_set_fan_auto_ppmQuiet = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0XE6, 0XA5, 0X00, 0X01, 0x00, 0X00};
    std::vector<uint8_t> cmd_set_fan_auto_area     = {0XA5, 0X22, 0XFF, 0X07, 0X00, 0XFF, 0X01, 0XE6, 0XA5, 0X00, 0X02, 0X3B, 0X01};    
    
    std::vector<uint8_t> cmd_set_fan_mode_manual   = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0XE0, 0XA5, 0X00, 0X00};  //Not used by vesync app but works to restore last fan speed
    std::vector<uint8_t> cmd_set_fan_mode_sleep    = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0XE0, 0XA5, 0X00, 0X01};
    std::vector<uint8_t> cmd_set_fan_mode_auto     = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0XE0, 0XA5, 0X00, 0X02};

    std::vector<uint8_t> cmd_lock_display          = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X00, 0XD1, 0X00, 0X01};
    std::vector<uint8_t> cmd_unlock_display        = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X00, 0XD1, 0X00, 0X00};    

    std::vector<uint8_t> cmd_display_on            = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X05, 0XA1, 0X00, 0X64};
    std::vector<uint8_t> cmd_display_off           = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0X05, 0XA1, 0X00, 0X00}; 

    std::vector<uint8_t> cmd_wifi_led_off          = {0XA5, 0X22, 0XFF, 0X0A, 0X00, 0XFF, 0X01, 0X29, 0XA1, 0X00, 0X00, 0XF4, 0X01, 0XF4, 0X01, 0X00};    
    std::vector<uint8_t> cmd_wifi_led_on           = {0XA5, 0X22, 0XFF, 0X0A, 0X00, 0XFF, 0X01, 0X29, 0XA1, 0X00, 0X01, 0X7D, 0X00, 0X7D, 0X00, 0X00};
    std::vector<uint8_t> cmd_wifi_led_flash        = {0XA5, 0X22, 0XFF, 0X0A, 0X00, 0XFF, 0X01, 0X29, 0XA1, 0X00, 0X02, 0XF4, 0X01, 0XF4, 0X01, 0X00};

    std::vector<uint8_t> cmd_filter_led_on         = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0XE2, 0XA5, 0X00, 0X01};
    std::vector<uint8_t> cmd_filter_led_off        = {0XA5, 0X22, 0XFF, 0X05, 0X00, 0XFF, 0X01, 0XE2, 0xA5, 0X00, 0X00};      
    
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

    void send_command(std::vector< uint8_t > &cmd) {
        cmd[2]=tx_seq_num++;
        cmd[5]=0; //zero checksum byte
        uint8_t cs=255;
        for(int i=0; i<cmd.size(); i++) {
            cs -= cmd[i];
        }
        cmd[5]=cs;
        write_array(cmd);
    }

       
    void acknowledge_packet() {
        tx_buf.clear();
        tx_buf.assign(rx_buf.begin(),rx_buf.begin()+9);
        tx_buf.push_back(0);
        tx_buf[1]=0x12;  //ack byte
        tx_buf[3]=4; //payload lenght
        tx_buf[5]=0; //checksum byte
        uint8_t cs=255;
        for(int i=0; i<tx_buf.size(); i++) {
            cs -= tx_buf[i];
        }
        tx_buf[5]=cs;
        write_array(tx_buf);
    }

    void process_packet()  {
        //uart::UARTDebug::log_hex(uart::UARTDirection::UART_DIRECTION_RX , rx_buf, '-');
        char buf[32];        
        unsigned int packet_type=0;
        packet_type = (rx_buf[6] << 16) + (rx_buf[7] << 8) + rx_buf[8];
        ESP_LOGD(TAG, "process_packet type %X", packet_type);
        
        if(rx_buf[1]==0x22)
            acknowledge_packet();

        switch (packet_type) {
            case 0x013040:  //Status packet
            case 0x013140:
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
                                        
                if(rx_buf[22]==0xFF && rx_buf[21]==0xFF)  //0xFFFF returned when off, samples every 15 minutes while off.
                    sensor_pm25->publish_state(NAN);                
                else
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
                    textsensor_FanAutoMode->publish_state("Room Size");
                    break;                

                    default:
                    sprintf(buf, "Error %X", rx_buf[24]);
                    textsensor_FanAutoMode->publish_state(buf);
                }

                areaL=rx_buf[25];
                areaH=rx_buf[26];
                sensor_roomSize->publish_state( (rx_buf[26]<<8) + rx_buf[25]);

            break;
        }

    }

};

