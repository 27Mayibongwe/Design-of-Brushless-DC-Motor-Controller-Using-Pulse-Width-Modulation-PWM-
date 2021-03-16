#define hall1 2
#define hall2 3
#define hall3 4
int stepstate;
bool hall1state;
bool hall2state;
bool hall3state;
byte Throttle;

void ExInt0() {
  hall1state = digitalRead(hall1);
  NextStep();
}

void ExInt1() {
  hall2state = digitalRead(hall2);
  NextStep();
}

ISR(PCINT2_vect) {
  hall3state = digitalRead(hall3);
  NextStep();
}
//This function Nextstep will run whenever the state of any of the hall sensors is changed via the use of interrupts

void NextStep() {
  if ((hall1state == 1) && (hall2state == 0) && (hall3state == 1)) {
    stepstate = 1;
  }
  if ((hall1state == 0) && (hall2state == 0) && (hall3state == 1)) {
    stepstate = 2;
  }
  if ((hall1state == 0) && (hall2state == 1) && (hall3state == 1)) {
    stepstate = 3;
  }
  if ((hall1state == 0) && (hall2state == 1) && (hall3state == 0)) {
    stepstate = 4;
  }
  if ((hall1state == 1) && (hall2state == 1) && (hall3state == 0)) {
    stepstate = 5;
  }
  if ((hall1state == 1) && (hall2state == 0) && (hall3state == 0)) {
    stepstate = 6;
    }
     TCNT1 =790;
  }
//The functions below control how the phases A, B AND C will be energized
void Energize_Phase_A_high_B_low(){
  PORTB = B00000010;
  PORTD = B00100000;
  TCCR2A =  0;            // Phase A high_side PWM
  TCCR1A =  0x81;         //
}
void Energize_Phase_A_high_C_low(){
  PORTB = B00000010;
  PORTD = B00100000;
  TCCR2A =  0;            // Phase A high_side PWM
  TCCR1A =  0x81;         //
}
void Energize_Phase_B_high_C_low(){
  PORTB = B00000100;
  PORTD = B00100000;
  TCCR2A =  0;            // Phase B highside PWM
  TCCR1A =  0x21;         //
}
void Energize_Phase_B_high_A_low(){
  PORTB =  B00000100;
  PORTD =  B10000000;
  TCCR2A =  0;            // Phase B highside PWM
  TCCR1A =  0x21;         //
}
void Energize_Phase_C_high_A_low(){
  PORTB = B00001000;
  PORTD = B10000000;
  TCCR1A =  0;            // Phase C highside PWM
  TCCR2A =  0x81;         //
}
void Energize_Phase_C_high_B_low(){
  PORTB = B00001000;
  PORTD = B01000000;
  TCCR1A =  0;            // Phase C highside PWM
  TCCR2A =  0x81;         //
}
//This function below loads and updates the duty cycle as per potentiometer setting
void Duty_Cycle_Setting(byte duty){
  OCR1A  = duty;                   // loads and updates the Phase A PWM duty cycle
  OCR1B  = duty;                   // loads and updates the Phase B PWM duty cycle
  OCR2A  = duty;                   // loads and updates the Phase C PWM duty cycle
}

 void bldc_move(){
  //This is the commutation sequence for the BLDC
  switch(stepstate){
    case 1:
      Energize_Phase_A_high_B_low();
      break;
    case 2:
      Energize_Phase_A_high_C_low();
      break;
    case 3:
      Energize_Phase_B_high_C_low();
      break;
    case 4:
      Energize_Phase_B_high_A_low();
      break;
    case 5:
      Energize_Phase_C_high_A_low();
      break;
    case 6:
      Energize_Phase_C_high_B_low();
      break;
    default:
      PORTD = 0;
      break;
  }
 }

void setup() {
  DDRD = B11100000; //configures pin 7,6,5 as ouputs and the rest as inputs
  DDRB = B00001110; //configures pin 9,10,11 as outputs
  TCNT1 = 0;
  // below is the Timer1 module setting: set clock source to clkI/O / 1 (no prescaling)
  TCCR1A = 0;
  TCCR1B = 0x01;
  // below is the Timer2 module setting: set clock source to clkI/O / 1 (no prescaling)
  TCCR2A = 0;
  TCCR2B = 0x01;
 // ADC module configuration
  ADMUX  = 0x60;                     // Configures ADC module and select channels 0 as potentiometer/Throttle input
  ADCSRA = 0x84;                     // Enable ADC module with 16 division factor (ADC clock = 1MHz)
 PCICR |= 0b00000100;    // initialize interrupt on hall3
  PCMSK2 |= 0b00010000;   // turn on PD4 (D4)
  attachInterrupt(0, ExInt0, CHANGE);// interrupt on hall2
  attachInterrupt(1, ExInt1, CHANGE);// interrupt on hall1 

  hall1state = digitalRead(hall1);
  hall2state = digitalRead(hall2);
  hall3state = digitalRead(hall3);
  NextStep();
  bldc_move();// 1st BLDC step
  
}

void loop() {
  ADCSRA |= 1 << ADSC;               // Start conversion
  while(ADCSRA & 0x40);              // Wait for conversion complete
  Throttle = ADCH;                // Read ADC data (8 bits)
  Duty_Cycle_Setting(Throttle);
}
