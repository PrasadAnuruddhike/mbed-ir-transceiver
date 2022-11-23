#include <mbed.h>
#include "IRremote.h"

#define IR_REC_PIN  P0_23
#define IR_REGION1 P2_5
#define IR_REGION2 P2_4
#define IR_REGION3 P2_3
#define IR_REGION4 P2_2
#define IR_REGION5 P2_0
#define IR_REGION6 P2_1
#define IR_FREQ 38

// #define IR_RECEIVING_TIMEOUT 3s  // 3 second

typedef struct HDR_DATA {
  int16_t HDR_MARK;
  int16_t HDR_SPACE;
  int16_t BIT_MARK;
  int16_t ZERO_SPACE;
  int16_t ONE_SPACE;
  int8_t ir_bit_count;
} hdr_data_t;

typedef struct IR_HEX {
  unsigned long long int third;
  unsigned long long int second;
  unsigned long long int first;
  /* data */
} ir_hex_val_t;

int16_t *buf;
volatile int counter = 0;
int8_t temp_inc = 0;

// Variables for raw IR signal
volatile int currTime, trigTime, gap, endTime, refTime;

void ir_receiving(uint32_t remote_id, uint8_t cmd, uint8_t communication_id);
void interrupt();


DigitalOut indicator_ir_ready   (LED1);
DigitalOut indicator_ir_receive (LED2);
DigitalOut indicator_ir_success (LED3);
DigitalOut indicator_ir_fail    (LED4);

DigitalIn button_learn_start    (P0_9);

Timer time_track;
InterruptIn ir_rec(IR_REC_PIN, PullUp);

IRsend ir_send_R1 (IR_REGION1);
IRsend ir_send_R2 (IR_REGION2);
IRsend ir_send_R3 (IR_REGION3);
IRsend ir_send_R4 (IR_REGION4);
IRsend ir_send_R5 (IR_REGION5);
IRsend ir_send_R6 (IR_REGION6);

uint32_t remote_id = 0x01;
uint8_t command_id = 0x02;
uint8_t comm_id    = 0xd4;


int main() {
  // put your setup code here, to run once:
  printf("IR Sample Test\n");

  while(1) {

    if(button_learn_start.read() == 1)
    {

      ir_receiving(remote_id, command_id, comm_id);
      // indicator_ir_success = 1;
      wait_us(5000000);
    }
    // else
    // {
    //   // indicator_ir_success = 0;
    // }
    // put your main code here, to run repeatedly:
    // indicator_ir_ready = 1;
    // wait_us(2000000);
    // indicator_ir_ready = 0;
    // wait_us(1000000);
  }
}

void ir_receiving(uint32_t remote_id, uint8_t cmd, uint8_t communication_id) 
{
  for (int8_t i = 0; i < 3; i++) 
  {
    buf = new int16_t[500];
    hdr_data_t _ir_hdr_data_value;
    uint8_t temp_color_data[4] = {0};
  
    indicator_ir_receive  = 0;
    indicator_ir_ready    = 0;
    indicator_ir_fail     = 0;
    indicator_ir_success  = 0;

    indicator_ir_ready  = 1;
    printf("\nReady to Receive \n");
    // _eeprom_controller->read_ir_receive_color(temp_color_data);

    // leds((temp_color_data[0] * temp_color_data[3]) / 25500.0, (temp_color_data[1] * temp_color_data[3]) / 25500.0,
    //      (temp_color_data[2] * temp_color_data[3]) / 25500.0);
    // INFO("REceiving mode color %d %d %d
    // %d",temp_color_data[0],temp_color_data[1],temp_color_data[2],temp_color_data[3]);

    _ir_hdr_data_value.HDR_MARK = 0;
    _ir_hdr_data_value.HDR_SPACE = 0;
    _ir_hdr_data_value.BIT_MARK = 0;
    _ir_hdr_data_value.ZERO_SPACE = 0;
    _ir_hdr_data_value.ONE_SPACE = 0;
    _ir_hdr_data_value.ir_bit_count = 0;

    int temp_ONE_SPACE = 0;
    int8_t ERROR = 80;
    int8_t len_raw_buf = 0;

    counter = 0;
    trigTime = 0;
    endTime = 0;

    time_track.start();
    printf("Now Fire IR\n");
    ir_rec.enable_irq();
    ir_rec.rise(&interrupt);

    printf("Reading IR signal\n");
    // ThisThread::sleep_for(IR_RECEIVING_TIMEOUT);
    wait_us(5000000);
    ir_rec.disable_irq();

    printf("Reading completed %d\n", counter);

    time_track.stop();

    // Remote data receive indication
    if (counter > 0) 
    {
      indicator_ir_receive  = 0;
      indicator_ir_ready    = 0;
      indicator_ir_fail     = 0;
      indicator_ir_success  = 0;

      indicator_ir_receive  = 1;

      printf("IR Received\n");
      // leds(0, 0, 0);
      // _eeprom_controller->read_ir_receive_indication_color(temp_color_data);
      // leds((temp_color_data[0] * temp_color_data[3]) / 25500.0, (temp_color_data[1] * temp_color_data[3]) / 25500.0,
      //      (temp_color_data[2] * temp_color_data[3]) / 25500.0);
      // // INFO("REceive indication  color %d %d %d %d", temp_color_data[0], temp_color_data[1], temp_color_data[2],
      // //      temp_color_data[3]);
      ThisThread::sleep_for(500ms);
    }

    // Remote Data read success indication
    if (counter > 50) 
    {
      _ir_hdr_data_value.HDR_MARK = buf[temp_inc + 1];
      _ir_hdr_data_value.HDR_SPACE = buf[temp_inc + 2];
      _ir_hdr_data_value.BIT_MARK = buf[temp_inc + 3];
      _ir_hdr_data_value.ZERO_SPACE = buf[temp_inc + 4];
      len_raw_buf = counter;  // sizeof(buf) / sizeof(buf[0]);
      int temp_count = 0;

      for (int i = (temp_inc + 4); i < (counter - 1); i = i + 2) 
      {
        if (buf[i] > _ir_hdr_data_value.HDR_MARK) 
        {
          break;
        }
        if (!(buf[i] < _ir_hdr_data_value.ZERO_SPACE + ERROR && buf[i] > _ir_hdr_data_value.ZERO_SPACE - ERROR)) 
        {
          if (buf[i] > 0) 
          {
            temp_ONE_SPACE += buf[i];
            // INFO("%d", buf[i]);
            temp_count++;
          }
        }
      }

      _ir_hdr_data_value.ONE_SPACE = temp_ONE_SPACE / temp_count;
      printf("%d \t: \n", buf[0]);
      printf("HDR_MARK \t: %d\n", _ir_hdr_data_value.HDR_MARK);
      printf("HDR_SPACE \t: %d\n", _ir_hdr_data_value.HDR_SPACE);
      printf("BIT_MARK \t: %d\n", _ir_hdr_data_value.BIT_MARK);
      printf("ZERO_SPACE\t: %d\n", _ir_hdr_data_value.ZERO_SPACE);
      printf("ONE_SPACE \t: %d\n", _ir_hdr_data_value.ONE_SPACE);

      temp_count = 0;
      ir_hex_val_t hex_val;
      hex_val.first = 0;
      hex_val.second = 0;
      hex_val.third = 0;
      if ((_ir_hdr_data_value.HDR_MARK < 0) | (_ir_hdr_data_value.HDR_MARK == 0) | (_ir_hdr_data_value.HDR_SPACE < 0) |
          (_ir_hdr_data_value.HDR_SPACE == 0) | (_ir_hdr_data_value.BIT_MARK < 0) | (_ir_hdr_data_value.BIT_MARK == 0) |
          (_ir_hdr_data_value.ZERO_SPACE < 0) | (_ir_hdr_data_value.ZERO_SPACE == 0) |
          (_ir_hdr_data_value.ONE_SPACE < 0) | (_ir_hdr_data_value.ONE_SPACE == 0)) 
      {
        
        indicator_ir_receive  = 0;
        indicator_ir_ready    = 0;
        indicator_ir_fail     = 0;
        indicator_ir_success  = 0;

        indicator_ir_fail     = 1;
        printf("IR Receive Fail\n");
        // _eeprom_controller->read_ir_receive_failure_color(temp_color_data);
        // leds((temp_color_data[0] * temp_color_data[3]) / 25500.0, (temp_color_data[1] * temp_color_data[3]) / 25500.0,
        //      (temp_color_data[2] * temp_color_data[3]) / 25500.0);

        // INFO("REceive failure color %d %d %d
        // %d",temp_color_data[0],temp_color_data[1],temp_color_data[2],temp_color_data[3]);
        ThisThread::sleep_for(1s);
        delete[] buf;
      } 
      else 
      {
        // leds(0, 0, 0);
        indicator_ir_receive  = 0;
        indicator_ir_ready    = 0;
        indicator_ir_fail     = 0;
        indicator_ir_success  = 0;

        indicator_ir_success = 1;

        printf("IR Receive Success\n");
        // _eeprom_controller->read_ir_receive_success_color(temp_color_data);
        // INFO("REceive suucees color %d %d %d
        // %d",temp_color_data[0],temp_color_data[1],temp_color_data[2],temp_color_data[3]);
        // leds((temp_color_data[0] * temp_color_data[3]) / 25500.0, (temp_color_data[1] * temp_color_data[3]) / 25500.0,
        //      (temp_color_data[3] * temp_color_data[3]) / 25500.0);
        for (int i = (temp_inc + 4); i < counter; i = i + 2) 
        {
          if ((buf[i] > (_ir_hdr_data_value.HDR_MARK - ERROR)) | (buf[i] > (_ir_hdr_data_value.HDR_SPACE - ERROR))) 
          {
            hex_val.first = 0;
            hex_val.second = 0;
            hex_val.third = 0;
            temp_count = 0;
            continue;
          }

          if (buf[i] > _ir_hdr_data_value.ZERO_SPACE - ERROR && buf[i] < _ir_hdr_data_value.ZERO_SPACE + ERROR) 
          {
            // bit value ZERO
          } 
          else if (buf[i] > _ir_hdr_data_value.ONE_SPACE - ERROR && buf[i] < _ir_hdr_data_value.ONE_SPACE + ERROR) 
          {
            // bit value ONE
            if (temp_count < 64) 
            {
              hex_val.first = (1ull << temp_count) | hex_val.first;
            } 
            else if (temp_count < 128) 
            {
              hex_val.second = (1ull << (temp_count - 64)) | hex_val.second;
            } 
            else 
            {
              hex_val.third = (1ull << (temp_count - 128)) | hex_val.third;
            }
          }
          temp_count++;
        }

        int8_t ir_bit_count = temp_count + 2;
        _ir_hdr_data_value.ir_bit_count = ir_bit_count;
        printf("Number of Bits %d \n", temp_count);
        printf("First Hex Value \t: %llx \n", hex_val.first);
        printf("Second Hex Value \t: %llx\n", hex_val.second);
        printf("Third Hex Value \t: %llx\n", hex_val.third);

        // send_ir_remote_packet(&remote_id, &cmd, &communication_id, &_ir_hdr_data_value, &hex_val);

        counter = 0;
        temp_count = 0;

        ThisThread::sleep_for(500ms);

        // for (int i = 0; i < counter; i++) {
        //   // INFO("%d", buf[i]);
        //   buf[i] = 0;
        // }
        delete[] buf;
        break;
      }

    } 
    else 
    {
      indicator_ir_receive  = 0;
      indicator_ir_ready    = 0;
      indicator_ir_fail     = 0;
      indicator_ir_success  = 0;

      indicator_ir_fail = 1;

      printf("IR Receive Fail\n");
      // leds(0, 0, 0);
      // _eeprom_controller->read_ir_receive_failure_color(temp_color_data);
      // leds((temp_color_data[0] * temp_color_data[3]) / 25500.0, (temp_color_data[1] * temp_color_data[3]) / 25500.0,
      //      (temp_color_data[2] * temp_color_data[3]) / 25500.0);

      // INFO("REceive failure color %d %d %d
      // %d",temp_color_data[0],temp_color_data[1],temp_color_data[2],temp_color_data[3]);
      ThisThread::sleep_for(1s);
      delete[] buf;
    }
  }
}


void interrupt() 
{
  currTime = time_track.elapsed_time().count();

  if (counter != 0) 
  {
    gap = currTime - trigTime;
  } 

  else 
  {
    refTime = currTime;
    gap = 2;
    counter++;
  }

  if (gap > 40 && counter < 500) 
  {
    if (counter > 2) 
    {
      buf[counter++] = trigTime - endTime;
      buf[counter++] = currTime - trigTime;
      endTime = currTime - 30;
    }
    else 
    {
      buf[counter++] = trigTime - refTime;
      buf[counter++] = currTime - trigTime;
      endTime = currTime;
    }
  }
  trigTime = currTime;
}