// #define USING_CAPTOUCH
#define USING_BUTTONS

//#define Y_ABSOLUTE
#define Y_RELATIVE

#define DEBUG

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

// CHANGABLE #DEFINES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define X_BOOST 2

// XY low pass filters.  Smaller number is faster response
#define LPF_INPUT 1
#define LPF_ZERO 4096

// Main loop delay, smaller is faster motion
#define DELAY 20

// UNCHANGABLE #DEFINES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Fixed point quotient
#define QUOTIENT_MAX 4096

const int led_pin = 13;
const int battery_adc = A13; /*half voltage*/
// ADC 1 pins are: A2, A3, A4, A12 (Red LED), A9, A7

#include "mpu9250.hpp"
MPU9250 mpu = MPU9250();
//int16_t min_ir = 32767;
//int16_t max_ir = -32767;
const int ir_adc = A2;
const int irled_pin = 12;

#ifdef USING_BUTTONS
  const int button1_pin = 14;
  const int button2_pin = 15;
#endif

/*Touch Sensor Pin Layout
   T0 = GPIO4
   T1 = GPIO0
   T2 = GPIO2
   T3 = GPIO15
   T4 = GPIO13
   T5 = GPIO12
   T6 = GPIO14 // connected
   T7 = GPIO27
   T8 = GPIO33 //conected
   T9 = GPIO32 // connected
   functions: touchRead
   touchAttachInterrupt(T9, gotTouch3, threshold);
   threshold was nice at 60 with kapton tape, maybe too sensitive
   */

#ifdef USING_CAPTOUCH
  const int pad1_pin = T8;
  const int pad2_pin = T9;
  const int pad3_pin = T6;
#endif

bool connected = false;
BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

#ifdef Y_RELATIVE
const uint8_t kMouseHidDescriptor[] = {
  0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
  0x09, 0x02,                         // USAGE (Mouse)                    2
  0xa1, 0x01,                         // COLLECTION (Application)         4
  0x85, 0x01,                         //   REPORT_ID (Mouse)              6
  0x09, 0x01,                         //   USAGE (Pointer)                8
  0xa1, 0x00,                         //   COLLECTION (Physical)          10
  0x05, 0x09,                         //     USAGE_PAGE (Button)          12
  0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
  0x29, 0x03,                         //     USAGE_MAXIMUM (Button 3)     16
  0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
  0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
  0x75, 0x01,                         //     REPORT_SIZE (1)              22
  0x95, 0x03,                         //     REPORT_COUNT (3)             24
  0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
  0x95, 0x05,                         //     REPORT_COUNT (5)             28
  0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30
  0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
  0x09, 0x30,                         //     USAGE (X)                    34
  0x09, 0x31,                         //     USAGE (Y)                    36
  0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       38
  0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        40
  0x75, 0x08,                         //     REPORT_SIZE (8)              42
  0x95, 0x02,                         //     REPORT_COUNT (2)             44
  0x81, 0x06,                         //     INPUT (Data,Var,Rel)         46
  0xc0,                               //   END_COLLECTION                 48
  0xc0                                // END_COLLECTION                   49/50
};
 
typedef struct
{
  uint8_t  ButtonOne  : 1;  // bool
  uint8_t  ButtonTwo  : 1;  // bool
  uint8_t  ButtonThree: 1;  // bool
  uint8_t  X;               // int8_t
  uint8_t  Y;               // int8_t
} InputMouse_t;
#endif // Y_RELATIVE

#ifdef Y_ABSOLUTE
const uint8_t kMouseHidDescriptor[] = {
  0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)     0
  0x09, 0x02,                         // USAGE (Mouse)                    2
  0xa1, 0x01,                         // COLLECTION (Application)         4
  0x85, 0x01,                         //   REPORT_ID (Mouse)              6
  0x09, 0x01,                         //   USAGE (Pointer)                8
  0xa1, 0x00,                         //   COLLECTION (Physical)          10
  
  0x05, 0x09,                         //     USAGE_PAGE (Button)          12
  0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)     14
  0x29, 0x03,                         //     USAGE_MAXIMUM (Button 3)     16
  0x15, 0x00,                         //     LOGICAL_MINIMUM (0)          18
  0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)          20
  0x75, 0x01,                         //     REPORT_SIZE (1)              22
  0x95, 0x03,                         //     REPORT_COUNT (3)             24
  0x81, 0x02,                         //     INPUT (Data,Var,Abs)         26
  0x95, 0x05,                         //     REPORT_COUNT (5)             28
  0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)         30

  0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 32
  0x09, 0x30,                         //     USAGE (X)                    34
  0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)       36
  0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)        38
  0x75, 0x08,                         //     REPORT_SIZE (8)              40
  0x95, 0x01,                         //     REPORT_COUNT (1)             42
  0x81, 0x02,                         //     INPUT (Data,Var,Abs)         44

  0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop) 46
  0x09, 0x31,                         //     USAGE (Y)                    48
  0x16, 0x01, 0xf8,                   //     LOGICAL_MINIMUM (-2047)      50
  0x26, 0xff, 0x07,                   //     LOGICAL_MAXIMUM (2047)       53
  0x75, 0x10,                         //     REPORT_SIZE (16)             56
  0x95, 0x01,                         //     REPORT_COUNT (1)             58
  0x81, 0x02,                         //     INPUT (Data,Var,Abs)         60

  0xc0,                               //   END_COLLECTION                 62
  0xc0                                // END_COLLECTION                   63/64
};
 
typedef struct
{
  uint8_t  ButtonOne  : 1;  // bool
  uint8_t  ButtonTwo  : 1;  // bool
  uint8_t  ButtonThree: 1;  // bool
  int8_t  X;                // int8_t
  int16_t Y;                // int16_t
} InputMouse_t;
#endif // Y_ABSOLUTE

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    connected = true;
    BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(true);
  }

  void onDisconnect(BLEServer* pServer){
    connected = false;
    BLE2902* desc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    desc->setNotifications(false);
  }
};

void taskServer(void*){
  BLEDevice::init("MiniMouse1");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyCallbacks());

  hid = new BLEHIDDevice(pServer);
  input = hid->inputReport(1); // <-- input REPORTID from report map
  output = hid->outputReport(1); // <-- output REPORTID from report map

  std::string name = "MidKnights";
  hid->manufacturer()->setValue(name);

  hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
  hid->hidInfo(0x00,0x02);

  hid->reportMap((uint8_t*)kMouseHidDescriptor, sizeof(kMouseHidDescriptor));
  hid->startServices();


  BLESecurity *pSecurity = new BLESecurity();
//  pSecurity->setKeySize();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);


  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setAppearance(HID_MOUSE);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  hid->setBatteryLevel(7);

  delay(portMAX_DELAY);
}

int16_t SampleIR()
{
  int16_t val_on, val_off;
  // Turn on IR LED
  digitalWrite(irled_pin, HIGH);
  delay(1);
  // sample
  val_on = analogRead(ir_adc);
  // Turn off IR LED
  digitalWrite(irled_pin, LOW); // PFET turns off when gate pulled high
  delay(1);
  // sample baseline
  val_off = analogRead(ir_adc);
  return val_on-val_off;
}

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  pinMode(led_pin, OUTPUT);

  #ifdef USING_BUTTONS
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  #endif

  #ifdef DEBUG
  Serial.println("Starting MPU");
  #endif
  mpu.begin();
  mpu.set_gyro_range(RANGE_GYRO_250); // slowest, since fingers are slow
  pinMode(irled_pin, OUTPUT);
  digitalWrite(irled_pin, LOW); // PFET turns on when gate pulled low

  #ifdef DEBUG
  Serial.println("Starting BLE");
  #endif
  xTaskCreate(taskServer, "server", 20000, NULL, 5, NULL);

  #ifdef DEBUG
  Serial.println("Running loop");
  #endif
}

void loop() {
  // Persistant variables
  static int32_t x_filt = 0;
  static int32_t y_filt = 0;
  static int32_t x_zero = 0;
  static int32_t y_zero = 0;
  static int8_t b1_last = 0;
  static int8_t b2_last = 0;
  static int8_t b3_last = 0;
  static int8_t b3_toggle = 0;
  static int16_t y_min = 32767;
  static int16_t y_max = -23767;
  static int8_t y_boost = 0;

  // Temporary variables
  int8_t send_report = 0;

  delay(DELAY);

  // Check battery voltage
  if ((analogRead(battery_adc)*2)<3.3){
    digitalWrite(led_pin, !digitalRead(led_pin));
  }

  // Buttons, cap buttons
  #ifdef USING_BUTTONS
  // Buttons
  int8_t b1_now = !digitalRead(button1_pin);
  int8_t b2_now = !digitalRead(button2_pin);
  int8_t b3_now = 0;
  #endif

  #ifdef USING_CAPTOUCH //pad pin 3 pin gets randomly triggered
  int8_t b1_now = touchRead(pad1_pin)<60;
  int8_t b2_now = touchRead(pad1_pin)<60;
  int8_t b3_now = touchRead(pad1_pin)<30;
  #endif
  
  int8_t b1 = b1_now;
  int8_t b2 = b2_now;
  int8_t b3 = b3_now;
  if(b1_now != b1_last)
  {
    send_report = 1;
  }
  b1_last = b1_now;
  
  if(b2_now != b2_last)
  {
    send_report = 1;
  }
  b2_last = b2_now;

  // I can't get this to work for middle click...
  // might as well use it for something else
  if(b3_now == 1 && b3_last == 0)
  {
    send_report = 1;
    b3_toggle = !b3_toggle;
  }
  b3_last = b3_now;

  // Gyro
  mpu.get_gyro();
  int32_t x_now = (mpu.gz)>>4; // 16bit, natively int16_t, shift to 12bit
  // Limit inputs to expected values (who knows what can happen to the signals)
  x_now = constrain(x_now*X_BOOST,-2047,2047);
  // To SIQ number
  x_now = x_now*QUOTIENT_MAX; 
  // Process input through filter and zero
  // The /? is the decay to zero.  If ? is large, the decay is small.
  x_filt += (x_now - x_filt)/LPF_INPUT; 
  x_zero += (x_now - x_zero)/LPF_ZERO;
  int8_t x = constrain((x_filt - x_zero)/(QUOTIENT_MAX*16),-127,127);
  
  // IR
#ifdef Y_RELATIVE
  static int16_t last_IR = 0;
  int16_t this_IR = SampleIR();
  int32_t y_now = (this_IR - last_IR); // 12 bit, make signed
  last_IR = this_IR;
  if(this_IR < y_min)
    y_min = this_IR;
  if(this_IR > y_max)
    y_max = this_IR;

    /*
  // Limit inputs to expected values (who knows what can happen to the signals)
  int16_t y_range = constrain(y_max-y_min,1,4096);
  y_boost = 4096/y_range;
  y_now = constrain(y_now*y_boost,-2047,2047);
  // To SIQ number
  y_now = y_now*QUOTIENT_MAX;  
  // Process input through filter and zero
  // The /? is the decay to zero.  If ? is large, the decay is small.
  y_filt += (y_now - y_filt)/LPF_INPUT; 
  y_zero += (y_now - y_zero)/LPF_ZERO;
  int8_t y = constrain((y_filt - y_zero)/(QUOTIENT_MAX*16),-127,127);
  */
  int8_t y = constrain(y_now,-127,127);
#endif // Y_RELATIVE

#ifdef Y_ABSOLUTE
  int32_t y_now = SampleIR();
  if(y_now < y_min)
    y_min = y_now;
  if(y_now > y_max)
    y_max = y_now;
    
  // To SIQ number
  y_now = y_now*QUOTIENT_MAX;  
  // Process input through filter and zero
  // The /? is the decay to zero.  If ? is large, the decay is small.
  y_filt += (y_now - y_filt)/LPF_INPUT; 
  int16_t y = 0;
  if(y_max - y_min > 0)
    y = constrain((y_filt - y_min*QUOTIENT_MAX)/(y_max - y_min),0,QUOTIENT_MAX-1)-2047;
#endif // Y_ABSOLUTE

  // Extra delay for slow motion
  /*int32_t additional_delay = 30*((512/SENSITIVITY_DIV) - abs(x) - abs(y))/(512/SENSITIVITY_DIV);
  if(additional_delay > 0)
    delay(additional_delay);*/
  
  if(x != 0 || y != 0)
    send_report = 1;

  if(connected && send_report){
    InputMouse_t mouse_input_now{};
    mouse_input_now.X = x;
    mouse_input_now.Y = y;
    mouse_input_now.ButtonOne = b1;
    mouse_input_now.ButtonTwo = b2;
    mouse_input_now.ButtonThree = b3;
    input->setValue((uint8_t*)&mouse_input_now,sizeof(mouse_input_now));
    input->notify();

#ifdef DEBUG
    uint8_t* output = (uint8_t*)&mouse_input_now;
    Serial.print(output[0]);
    Serial.print(", ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print(y);
//    Serial.print(", ");
//    Serial.print(DELAY+additional_delay);
    Serial.println();
#endif
  }
}
