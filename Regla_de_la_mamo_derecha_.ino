

#include <Pololu3piPlus32U4.h>

using namespace Pololu3piPlus32U4;

class UI
{
public:
    void inicializarPantalla();
    void mostrarPantallaCarga();
    void mostrarCalibrando();
    void mostrarSensores();
};

class MazeSolver
{
public:
    MazeSolver();
    void solveMaze();

private:
    void turnLeft();
    void turnRight();
    void turnAround();
    void followSegment();
    void handleIntersection();
};

class LineFollower
{
public:
    LineFollower();
    void followSegment();
    void calibrateSensors();

private:
    int lastProportional;
    long integral;
};

// Declaración de objetos globales
OLED display;
Buzzer buzzer;
ButtonA buttonA;
ButtonB buttonB;
ButtonC buttonC;
LineSensors lineSensors;
BumpSensors bumpSensors;
Motors motors;
Encoders encoders;
LineFollower lineFollower;
UI ui;
MazeSolver mazeSolver;

// Implementación de la clase UI

void UI::inicializarPantalla() {
    display.setLayout21x8();
}

void UI::mostrarPantallaCarga() {
    display.noAutoDisplay();  
    display.clear();          

    display.gotoXY(3, 0);
    display.print("PololuPro");
    display.gotoXY(8, 1);
    display.print("LINEAS");
    display.gotoXY(5, 7);
    display.print("B CAMBIAR MODO");

    display.display();        
}

void UI::mostrarCalibrando() {
    display.noAutoDisplay();  
    display.clear();          
    display.gotoXY(3, 4);
    display.print("Preparando");
    display.display();        
}

void UI::mostrarSensores() {
    display.noAutoDisplay();  
    display.clear();          
    unsigned int sensorValues[5]; 

    lineSensors.read(sensorValues);

    for (int i = 0; i < 5; i++) {
        display.gotoXY(0, i);  
        display.print("Sensor ");
        display.print(i + 1);  
        display.print(": ");
        display.print(sensorValues[i]);  
    }

    display.display();  
}

// Implementación de la clase MazeSolver

MazeSolver::MazeSolver() {}

void MazeSolver::solveMaze() {
    while (true) {
        followSegment();      
        handleIntersection(); 
    }
}

void MazeSolver::turnLeft() {
    motors.setSpeeds(30, 30);  
    delay(120);                
    motors.setSpeeds(-60, 60); 
    delay(300);                
    motors.setSpeeds(0, 0);    
}

void MazeSolver::turnRight() {
    motors.setSpeeds(30, 30);  
    delay(120);                
    motors.setSpeeds(60, -60); 
    delay(300);                
    motors.setSpeeds(0, 0);    
}

void MazeSolver::turnAround() {
    motors.setSpeeds(30, 30);  
    delay(120);                
    motors.setSpeeds(60, -60); 
    delay(700);                
    motors.setSpeeds(0, 0);    
}

void MazeSolver::followSegment() {
    lineFollower.followSegment();
}

void MazeSolver::handleIntersection() {
    unsigned int sensors[5]; 
    lineSensors.read(sensors); 

    bool left = sensors[0] < 1200;
    bool right = sensors[4] < 1200;
    bool centerleft = sensors[1] < 1200;
    bool front = sensors[2] < 1000;
    bool centerright = sensors[3] < 1200;

    if (left && right) {
        turnRight(); 
    } else if (left && right && front && centerleft && centerright) {
        motors.setSpeeds(0, 0); 
        delay(20000);           
    } else if (left && front) {
        motors.setSpeeds(30, 30); 
        delay(100);
    } else if (left) {
        turnLeft(); 
    } else if (right) {
        turnRight(); 
    } else {
        turnAround(); 
    }
}

// Implementación de la clase LineFollower

LineFollower::LineFollower() : lastProportional(0), integral(0) {}

void LineFollower::calibrateSensors() {
    for (int i = 0; i < 80; i++) {
        if (i < 20 || i >= 60)
            motors.setSpeeds(40, -40);
        else
            motors.setSpeeds(-40, 40);

        lineSensors.calibrate(); 
        delay(20);               
    }
    motors.setSpeeds(0, 0); 
}

void LineFollower::followSegment() {
    unsigned int sensors[5]; 

    while (true) {
        unsigned int position = lineSensors.readLineWhite(sensors); 
        ui.mostrarSensores(); 

        int proportional = ((int)position) - 2000;
        int derivative = proportional - lastProportional;
        integral += proportional;
        lastProportional = proportional;

        float kp = 0.1;
        float ki = 0.0;
        float kd = 0.015;

        const int integralLimit = 1000;
        if (integral > integralLimit) integral = integralLimit;
        if (integral < -integralLimit) integral = -integralLimit;

        const int deadband = 40;
        if (abs(proportional) < deadband) proportional = 0;

        int powerDifference = kp * proportional + ki * integral + kd * derivative;

        const int maxSpeed = 100; 
        if (powerDifference > maxSpeed) powerDifference = maxSpeed;
        if (powerDifference < -maxSpeed) powerDifference = -maxSpeed;

        if (powerDifference < 0)
            motors.setSpeeds(maxSpeed + powerDifference, maxSpeed);
        else
            motors.setSpeeds(maxSpeed, maxSpeed - powerDifference);

        if (sensors[1] > 200 && sensors[2] > 200 && sensors[3] > 200) {
            return; 
        } else if (sensors[0] < 200 || sensors[4] < 200) {
            return; 
        }
    }
}

// Setup y loop

void setup() {
    ui.inicializarPantalla();
    ui.mostrarPantallaCarga();
    ui.mostrarCalibrando();
    lineFollower.calibrateSensors();
}

void loop() {
    mazeSolver.solveMaze();
}