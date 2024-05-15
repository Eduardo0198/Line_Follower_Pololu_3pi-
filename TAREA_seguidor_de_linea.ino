


#include <Pololu3piPlus32U4.h>
#include <PololuMenu.h>

using namespace Pololu3piPlus32U4;

// Change next line to this if you are using the older 3pi+
// with a black and green LCD display:
// LCD display;
OLED display;

Buzzer buzzer;
LineSensors lineSensors;
Motors motors;
ButtonA buttonA;
ButtonB buttonB;
ButtonC buttonC;

int16_t lastError = 0;

#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];

/* Configuration for specific 3pi+ editions: the Standard, Turtle, and
Hyper versions of 3pi+ have different motor configurations, requiring
the demo to be configured with different parameters for proper
operation.  The following functions set up these parameters using a
menu that runs at the beginning of the program.  To bypass the menu,
you can replace the call to selectEdition() in setup() with one of the
specific functions.
*/

// This is the maximum speed the motors will be allowed to turn.
// A maxSpeed of 400 would let the motors go at top speed, but
// the value of 200 here imposes a speed limit of 50%.

// Note that making the 3pi+ go faster on a line following course
// might involve more than just increasing this number; you will
// often have to adjust the PID constants too for it to work well.
uint16_t maxSpeed;
int16_t minSpeed;

// This is the speed the motors will run when centered on the line.
// Set to zero and set minSpeed to -maxSpeed to test the robot
// without.
uint16_t baseSpeed;
uint16_t calibrationSpeed;
bool calibrateForWhiteLine = false; // Variable para indicar si se va a calibrar para línea blanca  
                                    // estaba true


// PID configuration: This example is configured for a default
// proportional constant of 1/4 and a derivative constant of 1, which
// seems to work well at low speeds for all of our 3pi+ editions.  You
// will probably want to use trial and error to tune these constants
// for your particular 3pi+ and line course, especially if you
// increase the speed.

uint16_t proportional; // coefficient of the P term * 256
uint16_t derivative; // coefficient of the D term * 256


void selectTortuga()
{
  maxSpeed = 200;
  minSpeed = 0;
  baseSpeed = maxSpeed;
  calibrationSpeed = 60;
  proportional = 64; // P coefficient = 1/4
  derivative = 256; // D coefficient = 1
}

void selectFlash()
{
  maxSpeed = 400;
  minSpeed = 100; // INTENTAR CON ESTAS SPEEDS
  baseSpeed = 200; //190 // 13 srgo pista 1
                   //190 // 10 seg pista 2
  calibrationSpeed = 100;
  proportional = 64; // P coefficient = 1/4
  derivative = 256; // D coefficient = 1
}


void selectLineColor()
{
  display.clear();
  display.gotoXY(0,0);
  display.print("A -> NEGRO");
  display.gotoXY(0,1);
  display.print("B -> BLANCO");

  while (true){
    if (buttonA.getSingleDebouncedPress()){
      calibrateForWhiteLine = false; // Calibrar para línea negr
      break;
    }
    else if (buttonB.getSingleDebouncedPress()){
      calibrateForWhiteLine = true; // Calibrar para línea blanca
      break;
    }
  }
}

PololuMenu<typeof(display)> menu;

void selectEdition()
{
  display.clear();
  display.print(F("Eliga un"));
  display.gotoXY(0,1);
  display.print(F("modo"));
  delay(1000);

  static const PololuMenuItem items[] = {
    { F("Tortuga"), selectTortuga },
    { F("Flash"), selectFlash },
  };

  menu.setItems(items, 2);
  menu.setDisplay(display);
  menu.setBuzzer(buzzer);
  menu.setButtons(buttonA, buttonB, buttonC);

  while(!menu.select());
  
  selectLineColor();

  display.gotoXY(0,1);
  display.print(" OK LETS GO. XD");
}

// Sets up special characters in the LCD so that we can display
// bar graphs.
void loadCustomCharacters()
{
  static const char levels[] PROGMEM = {
    0, 0, 0, 0, 0, 0, 0, 63, 63, 63, 63, 63, 63, 63
  };
  display.loadCustomCharacter(levels + 0, 0);  // 1 bar
  display.loadCustomCharacter(levels + 1, 1);  // 2 bars
  display.loadCustomCharacter(levels + 2, 2);  // 3 bars
  display.loadCustomCharacter(levels + 3, 3);  // 4 bars
  display.loadCustomCharacter(levels + 4, 4);  // 5 bars
  display.loadCustomCharacter(levels + 5, 5);  // 6 bars
  display.loadCustomCharacter(levels + 6, 6);  // 7 bars
}

void printBar(uint8_t height)
{
  if (height > 8) { height = 8; }
  const char barChars[] = {' ', 0, 1, 2, 3, 4, 5, 6, (char)255};
  display.print(barChars[height]);
}

void calibrateSensors()
{
  display.clear();

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 80; i++)
  {
    if (i > 20 && i <= 60)
    {
      motors.setSpeeds(-(int16_t)calibrationSpeed, calibrationSpeed);
    }
    else
    {
      motors.setSpeeds(calibrationSpeed, -(int16_t)calibrationSpeed);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

// Displays the estimated line position and a bar graph of sensor
// readings on the LCD. Returns after the user presses B.
void showReadings()
{
  display.clear();

  while(!buttonB.getSingleDebouncedPress())
  { 
    uint16_t position = lineSensors.readLineWhite(lineSensorValues);
    // uint16_t position = lineSensors.readLineBlack(lineSensorValues);

    display.gotoXY(0, 0);
    display.print(position);
    display.print("    ");
    display.gotoXY(0, 1);
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
      // uint8_t barHeight = map(lineSensorValues[i], 0, 1000, 0, 8);
      // printBar(barHeight);
    }

    delay(50);
  }
}

void setup()
{
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  loadCustomCharacters();

  // Play a little welcome song
  buzzer.play(">g32>>c32");

  // To bypass the menu, replace this function with
  // selectHyper(), selectStandard(), or selectTurtle().
  selectEdition();

  // Wait for button B to be pressed and released.
  display.clear();
  display.print(F("Pulse B ->"));
  display.gotoXY(0, 1);
  display.print(F("Calibrar"));
  while(!buttonB.getSingleDebouncedPress());

  calibrateSensors();

  showReadings();

  // Play music and wait for it to finish before we start driving.
  display.clear();
  display.print(F(":)  :D"));
  buzzer.play("L16 cdegreg4");
  while(buzzer.isPlaying());
} 
   

void followBlackLine(){
  // Get the position of the line.  Note that we *must* provide
  // the "lineSensorValues" argument to readLineBlack() here, even
  // though we are not interested in the individual sensor
  // readings.
  int16_t position = lineSensors.readLineBlack(lineSensorValues);

  // Our "error" is how far we are away from the center of the
  // line, which corresponds to position 2000.
  int16_t error = position - 2000;

  // Get motor speed difference using proportional and derivative
  // PID terms (the integral term is generally not very useful
  // for line following).
  int16_t speedDifference = error * (int32_t)proportional / 256  + (error - lastError) * (int32_t)derivative / 256;

  lastError = error;

  int16_t leftSpeed = (int16_t)baseSpeed + speedDifference;
  int16_t rightSpeed = (int16_t)baseSpeed - speedDifference;

  
  leftSpeed = constrain(leftSpeed, minSpeed, (int16_t)maxSpeed);
  rightSpeed = constrain(rightSpeed, minSpeed, (int16_t)maxSpeed);

  motors.setSpeeds(leftSpeed, rightSpeed);
}

void followWhiteLine(){
  
  int16_t position = lineSensors.readLineWhite(lineSensorValues);

  // Our "error" is how far we are away from the center of the
  // line, which corresponds to position 2000.
  int16_t error = position - 2000;


  int16_t speedDifference = error * (int32_t)proportional / 256  + (error - lastError) * (int32_t)derivative / 256;

  lastError = error;

  int16_t leftSpeed = (int16_t)baseSpeed + speedDifference;
  int16_t rightSpeed = (int16_t)baseSpeed - speedDifference;

  leftSpeed = constrain(leftSpeed, minSpeed, (int16_t)maxSpeed);
  rightSpeed = constrain(rightSpeed, minSpeed, (int16_t)maxSpeed);

  motors.setSpeeds(leftSpeed, rightSpeed);
}

void loop() {
  if (calibrateForWhiteLine) {
    followWhiteLine();
  } 
  else {
    followBlackLine();
  }
}
