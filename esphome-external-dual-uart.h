
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
    Sensor *sensor_fanSpeed;
    Sensor *sensor_pm25;
    Sensor *sensor_roomSize;
   
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
        
        sensor_fanSpeed->publish_state(NAN);
        sensor_pm25->publish_state(NAN);
        sensor_roomSize->publish_state(NAN);
        
        
        textsensor_Power->publish_state("Unknown");      
        textsensor_mcuFW->publish_state("Unknown");    
        textsensor_FanMode->publish_state("Unknown");    
        textsensor_DisplayLit->publish_state("Unknown");    
        textsensor_DisplayLocked->publish_state("Unknown");    
        textsensor_FanAutoMode->publish_state("Unknown");    
        
        set_update_interval(200);
    }

    void update() override
    {

        while ( available() ) {
            read_byte(&b);

 //          if( (b == 0xe2) && (last_b==0xe1 || last_b==0xe3) )  {
 //              //uart::UARTDebug::log_hex(uart::UARTDirection::UART_DIRECTION_RX , rx_buf, ' ');
 //              process_packet();                
 //              rx_buf.clear();            
 //          }
 //          
 //          last_b=b;
 //          rx_buf.push_back(b);
 //          
 //          if(rx_buf.size() == rx_buf.capacity() )  {
 //              ESP_LOGV(TAG, "rx_buf was filled!");
 //              uart::UARTDebug::log_hex(uart::UARTDirection::UART_DIRECTION_RX , rx_buf, ':');
 //              rx_buf.clear();            
 //          }
                
        }
        
 
     }

        
  private: 
    uint8_t b=0;
    uint8_t last_b=0;
    unsigned long millisProgress=0;
    uint8_t erd=0;
    
    std::vector<uint8_t> rx_buf; 
   
    //std::vector<uint8_t> erd2050= {0XE2, 0XC0, 0X0B, 0XBB, 0XF0, 0X01, 0X20, 0X50, 0XC4, 0XCB, 0xE3};    //Dryer Heat Setting  

    component_core300sUART(UARTComponent *parent) : PollingComponent(200), UARTDevice(parent) 
    {


        this->sensor_fanSpeed  = new Sensor();
        this->sensor_pm25  = new Sensor();
        this->sensor_roomSize  = new Sensor();

        this->textsensor_Power  = new TextSensor();
        this->textsensor_mcuFW  = new TextSensor();
        this->textsensor_FanMode  = new TextSensor();
        this->textsensor_DisplayLit  = new TextSensor();
        this->textsensor_DisplayLocked  = new TextSensor();
        this->textsensor_FanAutoMode  = new TextSensor();
        
    }
    
        
    void process_packet()  {
    }
};

