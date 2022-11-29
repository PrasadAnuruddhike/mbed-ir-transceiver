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

// int16_t *buf;

uint16_t *buf;

volatile int counter = 0;
int8_t temp_inc = 0;

// Variables for raw IR signal
volatile int currTime, trigTime, gap, endTime, refTime;

void ir_receiving(uint32_t remote_id, uint8_t cmd, uint8_t communication_id);
void interrupt();
void hex_cmd(char region, uint8_t _ir_bit_count, int HDR_MARK, int HDR_SPACE, int BIT_MARK, int ZERO_SPACE,
                         int ONE_SPACE, unsigned long long int first, unsigned long long int second,
                         unsigned long long int third);
void invert_value(unsigned long long *value);

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

// DigitalOut IR_LED_1(IR_REGION1);

uint32_t remote_id = 0x01;
uint8_t command_id = 0x02;
uint8_t comm_id    = 0xd4;


int main() {
  // put your setup code here, to run once:
  printf("IR Sample Test\n");

  // printf("----------------IR TRIGGER in 5 seconds-------------\n");
  // wait_us(5000000);
  hdr_data_t _ir_hdr_data_value;
  ir_hex_val_t hex_val;
  char region;

  // LG Remote 24 C
  // _ir_hdr_data_value.HDR_MARK     = 3154;
  // _ir_hdr_data_value.HDR_SPACE    = 9848;
  // _ir_hdr_data_value.BIT_MARK     = 522;
  // _ir_hdr_data_value.ZERO_SPACE   = 1559;
  // _ir_hdr_data_value.ONE_SPACE    = 602;
  // _ir_hdr_data_value.ir_bit_count = 30;

  // hex_val.first   = 0x7f6efee;
  // hex_val.second  = 0x00;
  // hex_val.third   = 0x00;


  // LG Remote 24 C --> Original Codes
  // _ir_hdr_data_value.HDR_MARK     = 8000;
  // _ir_hdr_data_value.HDR_SPACE    = 4000;
  // _ir_hdr_data_value.BIT_MARK     = 600;
  // _ir_hdr_data_value.ZERO_SPACE   = 550;
  // _ir_hdr_data_value.ONE_SPACE    = 1600;
  // _ir_hdr_data_value.ir_bit_count = 28;

  // hex_val.first   = 0x8091011;
  // hex_val.second  = 0x00;
  // hex_val.third   = 0x00;  

  // TV Box Volume Down
  // Working
  // _ir_hdr_data_value.HDR_MARK     = 9031;
  // _ir_hdr_data_value.HDR_SPACE    = 4536;
  // _ir_hdr_data_value.BIT_MARK     = 555;
  // _ir_hdr_data_value.ZERO_SPACE   = 1712;
  // _ir_hdr_data_value.ONE_SPACE    = 580;
  // _ir_hdr_data_value.ir_bit_count = 34;

  // hex_val.first   = 0x17e8bfbf;

  // // 0xb24d4040 --> Capture for Power On/Off
  // hex_val.first   = 0xb24d4040;

  // // // 0xbc434040 --> Capture for Mute
  // hex_val.first   = 0xbc434040;

  // // 0xe8174040 --> Capture for volume down
  // hex_val.first   = 0xe8174040;

  // // // 0xf40b4040 --> Capture for key up
  // hex_val.first   = 0xf40b4040;

  // // 0xef104040 --> Capture for key left
  // hex_val.first   = 0xef104040;

  // invert_value(&hex_val.first);
  // hex_val.second  = 0x00;
  // hex_val.third   = 0x00;

  
  
  // // Working
  // _ir_hdr_data_value.HDR_MARK     = 9031;
  // _ir_hdr_data_value.HDR_SPACE    = 4536;
  // _ir_hdr_data_value.BIT_MARK     = 555;
  // _ir_hdr_data_value.ZERO_SPACE   = 580;
  // _ir_hdr_data_value.ONE_SPACE    = 1712;
  // _ir_hdr_data_value.ir_bit_count = 34;
  // hex_val.first   = 0xe8174040;

  // invert_value(&hex_val.first);
  // printf("invert 0x%llX\n", hex_val.first);


  // // TV BOX with final bits
  // _ir_hdr_data_value.HDR_MARK     = 9037;
  // _ir_hdr_data_value.HDR_SPACE    = 4536;
  // _ir_hdr_data_value.BIT_MARK     = 555;
  // _ir_hdr_data_value.ZERO_SPACE   = 580;
  // _ir_hdr_data_value.ONE_SPACE    = 1712;
  // _ir_hdr_data_value.ir_bit_count = 34;

  // hex_val.first   = 0xe8174040;
  // hex_val.second  = 0x00;
  // hex_val.third   = 0x00;

  // region = 1;


  // for(int i = 0; i < 3; i++)
  // {
  //   hex_cmd(region, _ir_hdr_data_value.ir_bit_count, _ir_hdr_data_value.HDR_MARK, _ir_hdr_data_value.HDR_SPACE,
  //                   _ir_hdr_data_value.BIT_MARK, _ir_hdr_data_value.ZERO_SPACE, _ir_hdr_data_value.ONE_SPACE,
  //                   hex_val.first, hex_val.second, hex_val.third);
  //   wait_us(2000000);  
  // }
  while(1) {
    // IR_LED_1 = 1;
    // wait_us(100000);
    // IR_LED_1 = 0;
    // wait_us(1500000);
    
    ir_receiving(remote_id, command_id, comm_id);
    // wait_us(50000000);
  
    // if(button_learn_start.read() == 1)
    // {

    //   ir_receiving(remote_id, command_id, comm_id);
    //   // indicator_ir_success = 1;
    //   wait_us(5000000);
    // }
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
    buf = new uint16_t[500];
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

      printf("--------------------------------------\n");
      for(int i = 0; i < counter; i++)
      {
        printf("%d, ",buf[i]);
      }
      printf("\n--------------------------------------\n");

      for (int i = (temp_inc + 4); i < (counter - 1); i = i + 2) 
      {
        if (buf[i] > _ir_hdr_data_value.HDR_MARK) 
        {
          printf("\ncounter: %d,  i: %d", counter, i);
          counter = i;
          break;
        }
        if (!(buf[i] < _ir_hdr_data_value.ZERO_SPACE + ERROR && buf[i] > _ir_hdr_data_value.ZERO_SPACE - ERROR)) 
        {
          if (buf[i] > 0) 
          {
            temp_ONE_SPACE += buf[i];
            printf("%d ,", buf[i]);
            temp_count++;
          }
        }
      }
      printf("\n");

      _ir_hdr_data_value.ONE_SPACE = temp_ONE_SPACE / temp_count;
      printf("start: %d end: %d \t: \n", buf[0], buf[counter - 1]);
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
            printf("hdr\n");
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
          else if ((buf[i] > (_ir_hdr_data_value.ONE_SPACE - ERROR)) && (buf[i] < (_ir_hdr_data_value.ONE_SPACE + ERROR)))
          {
            printf("1 - %d\n", temp_count);
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
        printf("Number of Bits %d \n", _ir_hdr_data_value.ir_bit_count);
        printf("First Hex Value \t: %llx \n", hex_val.first);
        printf("Second Hex Value \t: %llx\n", hex_val.second);
        printf("Third Hex Value \t: %llx\n", hex_val.third);

        // send_ir_remote_packet(&remote_id, &cmd, &communication_id, &_ir_hdr_data_value, &hex_val);

        counter = 0;
        temp_count = 0;

        // invert_value(&hex_val.first);
        // printf("invert hex 0x%llX\n", hex_val.first);

        // char region_address[4] = {1, 2, 4, 8};

        for(uint8_t i = 1; i < 3; i ++)
        {
          char region = i;
          printf("----------------IR %d TRIGGER in 2 seconds-------------\n", i);
          wait_us(2000000);

          hex_cmd(region, _ir_hdr_data_value.ir_bit_count, _ir_hdr_data_value.HDR_MARK, _ir_hdr_data_value.HDR_SPACE,
                    _ir_hdr_data_value.BIT_MARK, _ir_hdr_data_value.ZERO_SPACE, _ir_hdr_data_value.ONE_SPACE,
                    hex_val.first, hex_val.second, hex_val.third);
          
        }

        /*

        _ir_hdr_data_value.ir_bit_count = _ir_hdr_data_value.ir_bit_count - 2;

        for(uint8_t i = 1; i < 3; i ++){
          char region = i;
          printf("----------------IR %d TRIGGER Reduced bit in 2 seconds-------------\n", i);
          wait_us(2000000);

          hex_cmd(region, _ir_hdr_data_value.ir_bit_count, _ir_hdr_data_value.HDR_MARK, _ir_hdr_data_value.HDR_SPACE,
                    _ir_hdr_data_value.BIT_MARK, _ir_hdr_data_value.ZERO_SPACE, _ir_hdr_data_value.ONE_SPACE,
                    hex_val.first, hex_val.second, hex_val.third);
        }
        */

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

void invert_value(unsigned long long *value)
{
  printf("input 0x%llX\n", *value);
  unsigned long long temp = 0;
  for (int i = 0; i < 32; i++)
  {
    if((*value >> i) & 0x01 == 0x01)
    {
      temp = temp | (0x00 << i);
    }
    else
    {
      temp = temp | (0x01 << i);
    }
  }

  temp = temp & 0xFFFFFFFF;
  // printf("invert 0x%llX\n", temp);
  *value = temp;
}

void hex_cmd(char region, uint8_t _ir_bit_count, int HDR_MARK, int HDR_SPACE, int BIT_MARK, int ZERO_SPACE,
                         int ONE_SPACE, unsigned long long int first, unsigned long long int second,
                         unsigned long long int third) 
{
  // ir_blaster_status = STATUS::ir_transmitting_mode;
  // INFO("==============IR blaster status %i",ir_blaster_status);
  
  ir_hex_val_t _ir_data;
  _ir_data.first = first;
  _ir_data.second = second;
  _ir_data.third = third;

  printf("Ir bit count %d\n", _ir_bit_count);
  printf("hex cmd First %llx\n", _ir_data.first);
  printf("hex cmd Second %ll\nx", _ir_data.second);
  printf("hex cmd Third %llx\n", _ir_data.third);
  // _ir_data.first = 0x8032090004e02002;
  // _ir_data.second = 0x890000e00e000035;
  // _ir_data.third = 0x6d0000;

  int16_t *buf = new int16_t[_ir_bit_count * 2];
  int data_buf_counter = 0;

  int temp_count = 0;
  buf[data_buf_counter++] = HDR_MARK;
  buf[data_buf_counter++] = HDR_SPACE;

  temp_count = 0;

  for (int i = 0; i < _ir_bit_count - 2; i++)  // 29
  { 
    if (i < 64) 
    {
      if ((_ir_data.first & (1ull << temp_count)) >> temp_count) 
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ONE_SPACE;
        // serial.printf("1");

      } 
      else
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ZERO_SPACE;
        // serial.printf("0");
      }

    } 
    else if (i < 128) 
    {
      if ((_ir_data.second & (1ull << (temp_count - 64))) >> (temp_count - 64)) 
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ONE_SPACE;
        // serial.printf("1");
      } 
      else 
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ZERO_SPACE;
        // serial.printf("0");
      }

    } 
    else 
    {
      if ((_ir_data.third & (1ull << (temp_count - 128))) >> (temp_count - 128)) 
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ONE_SPACE;
        // serial.printf("1");
      } 
      else 
      {
        buf[data_buf_counter++] = BIT_MARK;
        buf[data_buf_counter++] = ZERO_SPACE;
        // serial.printf("0");
      }
    }

    temp_count++;
  }
  buf[data_buf_counter++] = BIT_MARK;
  // buf[data_buf_counter++] = 5000;
  buf[data_buf_counter++] = 32766;      // overflow
  
  // buf[data_buf_counter++] = 46400;

  buf[data_buf_counter++] = HDR_MARK;
  buf[data_buf_counter++] = ONE_SPACE;
  buf[data_buf_counter++] = BIT_MARK;
  buf[data_buf_counter++] = 5000;

  printf("FIrst %d, Second %d, last %d and last %d data_but-count %d\n", buf[0], buf[1], buf[data_buf_counter - 2],
       buf[data_buf_counter - 1], data_buf_counter);
  printf("Third %d, Forth %d, Fifth %d and sixth %d seventh %d\n", buf[2], buf[3], buf[4], buf[5], buf[6]);

  if ((region & 0x01) == 1) {
    // contorl_leds(1, 0, 1, 0);
    ir_send_R1.sendRaw(buf, data_buf_counter, IR_FREQ);
    printf("Fire Zone 1 \n");
    ThisThread::sleep_for(500ms);
  }
  if ((region & 0x02) == 2) {
    // contorl_leds(2, 0, 1, 0);
    ir_send_R2.sendRaw(buf, data_buf_counter, IR_FREQ);
    printf("Fire Zone 2 \n");
    ThisThread::sleep_for(500ms);
  }
  if ((region & 0x04) == 4) {
    // contorl_leds(3, 0, 1, 0);
    ir_send_R3.sendRaw(buf, data_buf_counter, IR_FREQ);
    printf("Fire Zone 3 \n");
    ThisThread::sleep_for(500ms);
  }
  if ((region & 0x08) == 8) {
    // contorl_leds(4, 0, 1, 0);
    ir_send_R4.sendRaw(buf, data_buf_counter, IR_FREQ);
    printf("Fire Zone 4 \n");
    ThisThread::sleep_for(500ms);
  }

  delete[] buf;
  temp_count = 0;
  data_buf_counter = 0;
  
}