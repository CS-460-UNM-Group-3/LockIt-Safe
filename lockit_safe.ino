#include <Keypad.h>
#include <Queue.h>

class UserInput{
 private:

  char hexaKeys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
  };

  byte rowPins[4] = {9, 8, 7, 6}; 
  byte colPins[4] = {5, 4, 3, 2}; 

  Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, 4, 4); 

  Queue<char> keys = Queue<char>(100);

 public:
  char getkey(){
    return keys.pop();
  }
  
  void start(){
    keys.push(customKeypad.getKey());
  }

  void clearQueue(){
    while(keys.count() > 0){
      keys.pop();
    }
  }

 
};


class Gate{
  private:
    int gateLed = 13;
    int limitSwitch = 22;
    int limitSwitchState = 0;
 
  public:
    Gate(){
      pinMode(gateLed, OUTPUT);
      pinMode(limitSwitch, INPUT);
    }

  void unlock(){
    digitalWrite(gateLed, HIGH);
  }

  void lock(){
    digitalWrite(gateLed, LOW);
  }

  boolean isGateUnlocked(){
      limitSwitchState = digitalRead(limitSwitch);

      if(limitSwitchState == HIGH){
        Serial.println("Open");
        return true;
      }
      else{
        Serial.println("close");
        return false;
      } 
  }
  
};


class Led{
  private:
    int greenLed = 10;
    int yellowLed = 11;
    int redLed = 13;

  public:
    Led(){
      pinMode(greenLed, OUTPUT);
      pinMode(yellowLed, OUTPUT);
      pinMode(redLed, OUTPUT);
    }
    void on(String color){
      if(color == "green")
        digitalWrite(greenLed, HIGH);
      else if(color == "yellow")
        digitalWrite(yellowLed, HIGH);
      else if(color == "red")
        digitalWrite(redLed, HIGH);
    }

    void off(String color){
      if(color == "green")
        digitalWrite(greenLed, LOW);
      else if(color == "yellow")
        digitalWrite(yellowLed, LOW);
      else if(color == "red")
        digitalWrite(redLed, LOW);
    }
  
};

class Buzzer{
  private:
    int buzzerPin = 12;

  public:
    Buzzer(){
      pinMode(buzzerPin, OUTPUT);
    }

    void start(){
      analogWrite(buzzerPin, 20);
    }

    void stop(){
      digitalWrite(buzzerPin, 0);
    }
};

class Notification{
  private:
    Led led;
    Buzzer buzzer;

  public:
    void startBuzzer(){
      buzzer.start();
    }

    void stopBuzzer(){
      buzzer.stop();
    }

    void blinkLed(String color){
      led.on(color);
      delay(500);
      led.off(color);
    }
};

class Timer{
  private:
    double startTime = millis();
    boolean isReset = true;
    //boolean isTimeout = false;

  public:
    void setTimer(){
      if(isReset){
        startTime = millis();
      }
      isReset = false;
    }

    void resetTimer(){
      isReset = true;
    }

    boolean isTimeout(){
      if (!isReset){
        return millis()-startTime>10000.0;
      }
      return false;
    }
};


class PasscodeVerifier{
  private:
    String managerPasscode = "00000";
    String userPasscode = "0000";

  public:
    PasscodeVerifier(){
      //read from SD card
    }

    boolean isManagerPasscodeCorrect(String code){
      return managerPasscode == code;
    }

    boolean isUserPasscodeCorrect(String code){
      return userPasscode == code;
    }

    void setNewManagerPasscode(String code){
      managerPasscode = code;
    }

    void setNewUserPasscode(String code){
      userPasscode = code;
    }
};




Notification notif;
PasscodeVerifier pv;
Timer t;
Gate gate;
UserInput userInput;
String passcode = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  userInput.start();
  char k = userInput.getkey();
  if(k==0){
    while(k==0){
      userInput.start();
      k = userInput.getkey();
    }
  }
  if(k=='*'){
    notif.blinkLed("yellow");
    t.setTimer();
    if(getPasscode(5)){
      if(pv.isManagerPasscodeCorrect(passcode)){
        notif.blinkLed("green");
        passcode == "";
        userInput.clearQueue();
        t.setTimer();
        k == 0;
        while(k == 0){
          if(!t.isTimeout()){
            userInput.start();
            k = userInput.getkey();
          }else{
            break;
          }
        }
        if(k=='*'){
          notif.blinkLed("yellow");
          t.setTimer();
          if (getPasscode(5) == true){
            pv.setNewManagerPasscode(passcode);
            notif.blinkLed("green");
            goIdle();
          }
        }else if(k>47 && k<58){
          notif.blinkLed("yellow");
          passcode += k;
          t.setTimer();
          if (getPasscode(3) == true){
            pv.setNewUserPasscode(passcode);
            notif.blinkLed("green");
            goIdle();
          }
        }else{
          notif.blinkLed("red");
          buzzerAlert();
          goIdle();
        }
      }else{
        notif.blinkLed("red");
        buzzerAlert();
        goIdle();
      }
    }
  }else if(k>47 && k<58){
    notif.blinkLed("yellow");
    passcode += k;
    t.setTimer();
    if (getPasscode(3) == true){
      if(pv.isUserPasscodeCorrect(passcode)){
        notif.blinkLed("green");
        gate.unlock();
        passcode ="";
        t.setTimer();
        while(!t.isTimeout()){
          delay(500);
        }
        if(gate.isGateUnlocked()){
          notif.startBuzzer();
          while(gate.isGateUnlocked()){
            delay(100);
          }
          notif.stopBuzzer();
          gate.lock();
          goIdle();
        }else{
          gate.lock();
          goIdle();
        }
      }else{
        notif.blinkLed("red");
        buzzerAlert();
        goIdle();
      }
    }
  }else{
    notif.blinkLed("red");
    buzzerAlert();
    goIdle();
  }
}


boolean getPasscode(int len){
   while(true){
     if(!t.isTimeout()){
      userInput.start();
      char k = userInput.getkey();
      if(k!=0){
        if (k>47 && k<58){
          passcode += k;
          notif.blinkLed("yellow");
          if(passcode.length() == len){
            t.resetTimer();
            return true;
          }
          t.setTimer();
        }else{
          notif.blinkLed("red");
          buzzerAlert();
          goIdle();
          return false; 
        }
      } 
    }else{
      notif.blinkLed("red");
      buzzerAlert();
      goIdle();
      return false;
    }
  }
}


void goIdle(){
  t.resetTimer();
  passcode = "";
  userInput.clearQueue();
}


void buzzerAlert(){
  notif.startBuzzer();
  delay(250);
  notif.stopBuzzer();
}
