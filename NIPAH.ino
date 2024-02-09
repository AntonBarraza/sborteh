double ch_scale = 2.5; 
double ONE_LINE_Y = 11;
double ONE_CHAR_X = 6.5;


int WRITE_SPEED = 30;
int MOVE_SPEED = 100;

int NB_EMBOSS_REP = 3;
int EMBOSS_DELAY_REP = 50; 

int EMBOSS_DELAY_BEFORE = 20;
int EMBOSS_DURATION = 20;
int EMBOSS_DELAY_AFTER = 50;

int PAGE_W = 210;
int PAGE_H = 297;
int MARGIN_X = 40, MARGIN_Y = 30;
int MIN_X = 0 + MARGIN_X/2;
int MAX_X = PAGE_W - MARGIN_X/2 ;
int MIN_Y = 0 + MARGIN_Y/2;
int MAX_Y = PAGE_H - MARGIN_Y/2;


#define WITH_MOTORS
#define WITH_SOLENOIDE
#define WITH_X0_SWITCH
#define DEBUG


int PIN_X0 = 10;
int PIN_SOLENOID = 2;
int PIN_LED = 13;

double MOTOR_SCALE_X = 10;
double MOTOR_SCALE_Y = 8;
#define MOTOR_STEP_MODE INTERLEAVE 


int fullAsciiToBraille[] = {0,46,16,60,43,41,47,4,55,62,33,44,32,36,40,12,52,2,6,18 ,50,34,22,54,38,20,49,48,35,63, 28,57,8, 1, 3, 9,25,17,11,27,19,10,26,5,7,13,29,21,15,31,23,14,30,37,39,58,45,61,53,42,51,59,24,56,0,1,3,9,25,17,11,27,19,10,26,5,7,13,29,21,15,31,23,14,30,37,39,58,45,61,53,0,0,0,0,0};

#ifdef WITH_MOTORS
  #include <AFMotor.h>
#endif

#define BACK -1
#define FORW 1

 
#ifdef WITH_MOTORS
  AF_Stepper motor1(200, 2);
  AF_Stepper motor2(200, 1);
#endif

double x0 = MARGIN_X, 
       y0 = MARGIN_Y;
double pxLast = x0, pyLast = y0;

int isXEnd = 0;
char data;
boolean brailleOn = false;

int brailleOrder[] = { 1,2,3,7,8,6,5,4 };

void setup() {
  pinMode(PIN_LED, OUTPUT);
  
  Serial.begin(9600);
  Serial.setTimeout(50);

  pinMode(PIN_X0, INPUT_PULLUP);
  pinMode(PIN_SOLENOID,OUTPUT);

#ifdef WITH_MOTORS
  motor1.setSpeed(WRITE_SPEED);  
  motor2.setSpeed(WRITE_SPEED);
#endif

}


void motorX(double st_mm) {
#ifdef WITH_MOTORS
  st_mm = -st_mm;
  motor1.step(abs(st_mm)*MOTOR_SCALE_X, st_mm>=0?FORWARD:BACKWARD, MOTOR_STEP_MODE);
#endif
}

void motorY(double st_mm) {
#ifdef WITH_MOTORS
  motor2.step(abs(st_mm)*MOTOR_SCALE_Y, st_mm>=0?FORWARD:BACKWARD, MOTOR_STEP_MODE);
#endif
}

void moveXToStart() {
#ifdef WITH_MOTORS
    motor1.setSpeed(MOVE_SPEED);
    while (digitalRead(PIN_X0)) {
      motorX(-1);
    }
    motor1.setSpeed(WRITE_SPEED);
#endif   
    pxLast = 0;
    x0 = MARGIN_X;
}

void moveXToEnd() {
#ifdef WITH_MOTORS
    motor1.setSpeed(MOVE_SPEED);

    while (digitalRead(PIN_X0)) {
      motorX(1);
    }
    motorX(-(PAGE_W-MARGIN_X));
    pxLast = MARGIN_X;
    
    motor1.setSpeed(WRITE_SPEED);
#endif   
    x0 = MARGIN_X;
}

void nextLine() {
    motorY(ONE_LINE_Y);
    x0 = MARGIN_X;
    y0 += ONE_LINE_Y;
    pyLast += ONE_LINE_Y;    
}

void moveTo(double chx, double chy) {
    double dx, dy,
           x = x0+ch_scale*chx, 
           y = y0+ch_scale*chy;  

    if (x != pxLast && x>=MIN_X && x<=MAX_X) {
        dx = x-pxLast;
        motorX(dx); 
        pxLast = x;
    }
    
    if (y != pyLast && y>=MIN_Y && y<=MAX_Y) {
        dy = y-pyLast;
        motorY(dy); 
        pyLast = y;
    }
}

void embosse() {
  #ifdef WITH_SOLENOIDE
     delay(EMBOSS_DELAY_BEFORE);
     for (int i=0; i<NB_EMBOSS_REP; i++) {
        if (i>0) delay(EMBOSS_DELAY_REP);
        digitalWrite(PIN_SOLENOID, HIGH);       
        delay(EMBOSS_DURATION);
        digitalWrite(PIN_SOLENOID, LOW);
     }
     delay(EMBOSS_DELAY_AFTER); 
  #endif
}
void drawBrailleChar(int bch) {
#ifdef DEBUG
    Serial.print("BCH:");
    Serial.println(bch);
#endif    
    int id;
    for (int i=0; i<8; i++) {
       id = brailleOrder[i];
       if ((bch & (1<<(id-1))) != 0) {
          moveTo(id<7?(id-1)/3:id-7, id<7?((id-1)%3):3);
          embosse();
#ifdef DEBUG
          Serial.print("ID:");
          Serial.println(id);          
          Serial.print("moveTo(");
          Serial.print(id<7?(id-1)/3:id-7);
          Serial.print(",");
          Serial.print(id<6?((id-1)%3):3);
          Serial.println(");");
#endif          
       }
    }
}



int convertToBraille(char ascii) {
  int id = ascii-32;
  if (id <0 || id >= 98) {
    id = 0;
  }
  Serial.println("value at");
  Serial.println(id);
  Serial.println(fullAsciiToBraille[id]);
  
  int braille = fullAsciiToBraille[id];
  return braille;
}


void loop() {



  
  if (Serial.available()>0){
/*
   // Serial.print("⠂⠃⠄⠅⠆⠇⠈⠉⠊⠋⠌⠍⠎⠏");
   String command = Serial.readString();

   // data = Serial.read();
    Serial.println(command);
    //Serial.print("---");
    
  }
  */
   
      data = Serial.read();
     if (data == '\n' || data == '\r') {
     
     } else if (data == '{') {
        #ifdef DEBUG
          Serial.println("Init position");
        #endif
        moveXToEnd();
        
        #ifdef DEBUG
          Serial.println("Braille On");
        #endif

        #ifdef DEBUG
          Serial.println("Braille On");
        #endif
          brailleOn = true;
     
     } else if (data == '}') {
        #ifdef DEBUG
          Serial.println("Braille Off");
        #endif
          brailleOn = false;
     } else if (data == '#') {
        nextLine();
 #ifdef WITH_MOTORS
        motor1.release();
        motor2.release();
 #endif
     } else {

        if (brailleOn) {
          if (data < '~') {
            Serial.print("---------------------");
            Serial.println(data);
            data = convertToBraille(data);       
            drawBrailleChar(data);
          }else {
          
            drawBrailleChar((data-0x2800));
          }
          x0 += ONE_CHAR_X;
          
        } else {

      
        }
     
      }
  } else {
     #ifdef WITH_MOTORS
        motor1.release();
        motor2.release();
    #endif
  }
}

const int bSize = 32; 
char Buffer[bSize];
char Command[10];
char Data[15];   
int ByteCount;
int timedRead(int _timeout) {
  int c;
  int _startMillis = millis();
  do {
    c = Serial.read();
    if (c >= 0) return c;
  } while(millis() - _startMillis < _timeout);
  return -1;
}

size_t readBytesUntil(char terminator, char *buffer, size_t length){
  if (length < 1) return 0;
  size_t index = 0;
  while (index < length) {
    int c = timedRead(1000);
    if (c < 0 || c == terminator) break;
    *buffer++ = (char)c;
    index++;
  }
  return index;
}

String readLine(char terminator){
  size_t sz = readBytesUntil(terminator, Buffer, bSize);
  String str(Buffer);
  return str;
}
