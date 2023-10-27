//#include <Wire.h>
//#include <SPI.h>
//#include <SD.h>
//#include <SerialFlash.h>

/**********Pulled from Teensy's (Part_1_03) Playing Music Example**********/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioPlaySdWav           playSdWav1;
AudioOutputI2S           i2s1;
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
/**************************************************************************/

int tCounter = 0;

//Pin assignments
int buttonInputPins[] = {0, 1, 2, 3, 4, 5, 9, 14};
int buttonLightPins[] = {16, 17, 22, 24, 25, 26, 27, 28};
int solenoidPins[] = {29, 30, 31, 32, 33, 34, 35, 36};

int semnoxPlayerCountInput = 37;
int semnoxTicketOutputPin = 38;
int semnoxTicketPulseOutput = 39;

int playerOneSpotLight = 40;
int playerTwoSpotLight = 41;

//test time (ms) during initialization
int buttonLightSolenoidTestTime = 2000;

boolean startGame = false;  //dictates the triggering of a new game

unsigned long initialGameStartTime = 0;   //stores game start time (ms)
unsigned long currentTime = 0;       //holds button trigger time

int parsedSongHolder[300][9];

int parsedSongText1[300][9];
int parsedSongText2[300][9];
int parsedSongText3[300][9];
int parsedSongText4[300][9];
int parsedSongText5[300][9];
int parsedSongText6[300][9];
int parsedSongText7[300][9];
int parsedSongText8[300][9];
int parsedSongText9[300][9];
int parsedSongText10[300][9];
int parsedSongText11[300][9];
int parsedSongText12[300][9];
int parsedSongText13[300][9];
int parsedSongText14[300][9];
int parsedSongText15[300][9];
int parsedSongText16[300][9];
int parsedSongText17[300][9];
int parsedSongText18[300][9];
int parsedSongText19[300][9];
int parsedSongText20[300][9];

int parsedSongText21[300][9];

int eLoc = 0;
int bOffset = 625;
int aOffset = 250;

int winPointsPlayer1 = 0;
int winPointsPlayer2 = 0;
int failurePointsPlayer1 = 0;     //count for incorrect button triggering
int failurePointsPlayer2 = 0;
int totalGamePointsPlayer1 = 0;          //holds the expected total amount of button triggering for the song
int totalGamePointsPlayer2 = 0;
float percentageCorrectPlayer1 = 0;
float percentageCorrectPlayer2 = 0;
float totalScorePercentage = 0;

int buttonMissedPlayer1 = 0;
int buttonMissedPlayer2 = 0;

unsigned long priorButtonHitTime[] = {0, 0, 0, 0, 0, 0, 0, 0}; //holds the time (ms) of prior button hits
unsigned long buttonHitTime[] = {0, 0, 0, 0, 0, 0, 0, 0}; //holds the time (ms) of the current button hits

//holds the amount of time (ms) for registering incorrect button hits
unsigned long buttonPressedInterval = 1000;

boolean songEnd = false;
int newGameFlag = 0;

File myFile;              //stores SD text file
const int chipSelect = BUILTIN_SDCARD;
char previousChar;        //char for comparison (prior char)
char charFromFile;        //stroes current char from file
String timeHolderString;  //holds time values in text file
String songNameHolder;

//counters for storing data in the button-triggering-time array
int firstArrayDimension = 0;
int secondArrayDimension = 0;

int serialFlag = 0;   //monitor the establishing of a serial connection

boolean turnOffLightsOnceFlag = true;
boolean playSongFlag = true;

int pauseForIntroMusic = 5000;

boolean attractMode = true;
boolean attractModeNotificationFlag = true;
boolean beginAttractModeSong = true;
unsigned long timeBetweenGames = 0;
unsigned long triggerAttractModeTime = 10000;

boolean activateSolenoidFlag = true;
unsigned long startTime = 0;

boolean freestyleModeOn = true;

char songTitleStringArray[21][9] = {{"1.wav"}, {"2.wav"}, {"3.wav"}, {"4.wav"}, {"5.wav"},
                                   {"6.wav"}, {"7.wav"}, {"8.wav"}, {"9.wav"}, {"10.wav"},
                                   {"11.wav"}, {"12.wav"}, {"13.wav"}, {"14.wav"}, {"15.wav"},
                                   {"16.wav"}, {"17.wav"}, {"18.wav"}, {"19.wav"}, {"20.wav"},
                                   {"21.wav"}};

int randomizeSongs = 0;
boolean randomSelectionOn = false;
int randomSongNumber = 0;
int songCounter = 0;

float percentageCorrect = 80.0;

int userParameters[] = {0, 0, 0, 0, 0, 0, 0, 0};
int parameterCounter = 0;

boolean listenToSemnoxPlayerCount = true;
int playerCount = 0;
unsigned long firstImpulseTime = 0;
boolean onePlayerGame = false;
boolean twoPlayerGame = false;

int highInputFlag = 1;
int pwOffset = 140;

boolean onePlayerWins = false;
boolean twoPlayersTie = false;
int ticketPulseCount = 50;
int ticketPulseDelayTime = 130;

int extraCredit = 0;

void setup()
{
  delay(1000);

  Serial.begin(9600);
  /*while (!Serial)
    {
    if (serialFlag == 0)
    {
      Serial.println("Waiting for Serial Connection");
      serialFlag = 1;
    }
    }*/

  InitializeGamePins();
  InitializeSDCardAndAudio();
  PullUserParameterFiles();
  ButtonLightSolenoidTest();
  ParseSongFiles();
}

void loop()
{
  if (listenToSemnoxPlayerCount == true)
  {
    HandleSemnoxPlayerCount();
  }

  //listen for game start triggering
  if (((onePlayerGame || twoPlayerGame) || extraCredit > 0) && newGameFlag == 0)
  {
    if (extraCredit > 0)
    {
      extraCredit--;
      twoPlayerGame = true;
    }

    Serial.print("\nCredit(s) Left: ");
    Serial.println(extraCredit);
    Serial.println("");
    
    Serial.println("\nStop Attract Mode");
    digitalWrite(playerOneSpotLight, LOW);
    digitalWrite(playerTwoSpotLight, LOW);

    eLoc = 0;

    Serial.println("Sending Stop Signal");
    StopAttractMode();

    randomlySelectSong();

    initialGameStartTime = millis() + pauseForIntroMusic;
    startGame = true;
    newGameFlag = 1;
    attractMode = false;
    attractModeNotificationFlag = false;
    beginAttractModeSong = false;
    playSongFlag = true;
    highInputFlag = 1;

    if (onePlayerGame == true)
    {
      Serial.println("Turn on Spotlight One");
      digitalWrite(playerOneSpotLight, HIGH);
      digitalWrite(playerTwoSpotLight, LOW);
    }

    if (twoPlayerGame == true)
    {
      Serial.println("Turn on Spotlights One and Two");
      digitalWrite(playerOneSpotLight, HIGH);
      digitalWrite(playerTwoSpotLight, HIGH);
    }
  }

  //game logic sequence
  if (startGame)
  {
    /*if (initialGameStartTime > 3000)
    {
      HandleExtraCredits();
    }*/  
    
    PlaySongBasedOnSelection();
        
    ActivateButtonLights();
    DisableActiveButtonLights();
    ListenForButtonTriggering();
    HandleSolenoids();

    if (playSdWav1.isPlaying() == false)
    {
      delay(100);

      timeBetweenGames = millis();
      HandleGameEndAndReset();

      newGameFlag = 0;
      attractMode = true;
      attractModeNotificationFlag = true;
      beginAttractModeSong = true;
      delay(100);
    }
  }

  if (attractMode)
  {
    if (attractModeNotificationFlag)
    {
      Serial.println("\nEntering Attract Mode State");

      CopyTimeArray(parsedSongText21);
      playSongFlag = true;

      timeBetweenGames = millis();

      attractModeNotificationFlag = false;
      Serial.println("Attract Mode Time: " + (String)triggerAttractModeTime + " ms");
    }

    if ((millis() - timeBetweenGames) > triggerAttractModeTime)
    {
      if (beginAttractModeSong)
      {
        Serial.println("Begin Attract Mode");
        initialGameStartTime = millis();

        PlaySong(songTitleStringArray[20]);
        //digitalWrite(playerOneSpotLight, HIGH);
        //digitalWrite(playerTwoSpotLight, HIGH);

        beginAttractModeSong = false;
        activateSolenoidFlag = true;
      }

      ActivateSolenoids();
      DeactivateSolenoids();

      if(playSdWav1.isPlaying() == false)
      {
        digitalWrite(playerOneSpotLight, LOW);
        digitalWrite(playerTwoSpotLight, LOW);

        delay(100);

        HandleGameEndAndReset();

        attractMode = true;
        attractModeNotificationFlag = true;
        beginAttractModeSong = true;
      }
    }
  }
}

/*****************************************************
  Function: InitializeGamePins
    -Sets pin Inputs/Outputs
*****************************************************/
void InitializeGamePins()
{
  Serial.println("Initializing Game");
  Serial.println("Assigning Pins Button and Light Pins");

  //Assign pins
  for (int i = 0; i < 8; i++)
  {
    pinMode(buttonInputPins[i], INPUT_PULLUP);
    pinMode(buttonLightPins[i], OUTPUT);
    pinMode(solenoidPins[i], OUTPUT);
  }

  pinMode(playerOneSpotLight, OUTPUT);
  pinMode(playerTwoSpotLight, OUTPUT);

  pinMode(semnoxPlayerCountInput, INPUT_PULLDOWN);
  pinMode(semnoxTicketOutputPin, OUTPUT);
  pinMode(semnoxTicketPulseOutput, OUTPUT);

  digitalWrite(playerOneSpotLight, LOW);
  digitalWrite(playerTwoSpotLight, LOW);
  digitalWrite(semnoxTicketPulseOutput, HIGH);
}

/*****************************************************
  Function: InitializeSDCardAndAudio
    -Sets pin Inputs/Outputs
    -Pulled from Teensy's Playing Music example
*****************************************************/
void InitializeSDCardAndAudio()
{
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) 
  {
    while (1) 
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
  delay(1000);
}

/*****************************************************
  Function: ButtonLightSolenoidTest
    -Triggers Button Lights and Solenoids for
     troubleshooting purposes
*****************************************************/
void ButtonLightSolenoidTest()
{
  Serial.println("\nTesting Button Lights and Solenoids");

  //Trigger lights
  for (int i = 0; i < sizeof(buttonLightPins) / sizeof(int); i++)
  {
    digitalWrite(buttonLightPins[i], HIGH);
    //digitalWrite(solenoidPins[i], HIGH);
  }

  digitalWrite(playerOneSpotLight, HIGH);
  digitalWrite(playerTwoSpotLight, HIGH);

  //hold triggered lights
  delay(buttonLightSolenoidTestTime);

  //Trigger lights
  for (int i = 0; i < sizeof(buttonLightPins) / sizeof(int); i++)
  {
    digitalWrite(buttonLightPins[i], LOW);
    //digitalWrite(solenoidPins[i], LOW);
  }

  digitalWrite(playerOneSpotLight, LOW);
  digitalWrite(playerTwoSpotLight, LOW);

  Serial.println("Testing Ended");
}

/*****************************************************
  Function: ActivateButtonLights
    -Handles button light activation based on trigger
     sequence
*****************************************************/
void ActivateButtonLights()
{
  currentTime = (millis() - initialGameStartTime);

  if (currentTime > (parsedSongHolder[eLoc][0] - bOffset) && currentTime < (parsedSongHolder[eLoc][0] + aOffset))
  {
    turnOffLightsOnceFlag = true;
    freestyleModeOn = false;

    Serial.println("\nActivate Button Lights for Next Sequence Step");

    Serial.print("Expected Triggering: " + (String)parsedSongHolder[eLoc][0] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][1] + ", " + (String)parsedSongHolder[eLoc][2] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][3] + ", " + (String)parsedSongHolder[eLoc][4] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][5] + ", " + (String)parsedSongHolder[eLoc][6] + ", ");
    Serial.println((String)parsedSongHolder[eLoc][7] + ", " + (String)parsedSongHolder[eLoc][8]);

    //loop through pins for light triggering
    for (int i = 0; i < sizeof(buttonLightPins) / sizeof(int); i++)
    {
      if (parsedSongHolder[eLoc][i + 1] == 1)
      {
        digitalWrite(buttonLightPins[i], HIGH);

        if (i < 4)
        {
          totalGamePointsPlayer1++;
        }
        else
        {
          totalGamePointsPlayer2++;
        }
      }
      else
      {
        //redundancy
        digitalWrite(buttonLightPins[i], LOW);
      }

      if (onePlayerGame == true)
      {
        if (i >= 3)
        {
          i = 8;
        }
      }
    }

    eLoc++; //increment to next triggering step
  }
}

/*****************************************************
  Function: DisableActiveButtonLights()
    -turn off button lights if they have not been
     utlized during the trigger time
*****************************************************/
void DisableActiveButtonLights()
{
  //checks the current game time in comparison to the prior trigger time (with offset)
  if (currentTime > (parsedSongHolder[eLoc - 1][0] + aOffset))
  {
    if (turnOffLightsOnceFlag)
    {
      buttonMissedPlayer1 = 0;
      buttonMissedPlayer2 = 0;

      //turn off all button lights
      for (int i = 0; i < sizeof(buttonLightPins) / sizeof(int); i++)
      {
        if (digitalRead(buttonLightPins[i]) == HIGH)
        {
          digitalWrite(buttonLightPins[i], LOW);
          //Serial.println("Button Missed");          //do a counter (w reset)
          if (i < 4)
          {
            buttonMissedPlayer1++;
          }
          else
          {
            buttonMissedPlayer2++;
          }
        }
      }
      Serial.println("Player 1 Buttons missed: " + (String)buttonMissedPlayer1);
      Serial.println("Player 2 Buttons missed: " + (String)buttonMissedPlayer2);

      //create a single instance of turning off the lights (per trigger time)
      turnOffLightsOnceFlag = false;

      Serial.print(parsedSongHolder[eLoc - 1][0]);
      Serial.print(": (Auto) Turn Button Lights Off: ");
      Serial.println(currentTime);

      Serial.print("Player 1 Misfires: ");
      Serial.println(failurePointsPlayer1);

      Serial.print("Player 2 Misfires: ");
      Serial.println(failurePointsPlayer2);
    }
  }
}

/*****************************************************
  Function: ListenForButtonTriggering
    -Captures button interaction
    -Handles point counting for game win/loss
*****************************************************/
void ListenForButtonTriggering()
{
  for (int i = 0; i < sizeof(buttonInputPins) / sizeof(int); i++)
  {
    if ((digitalRead(buttonInputPins[i]) == LOW) && (digitalRead(buttonLightPins[i]) == HIGH))
    {
      digitalWrite(buttonLightPins[i], LOW);

      priorButtonHitTime[i] = millis();

      if (i < 4)
      {
        winPointsPlayer1++;
      }
      else
      {
        winPointsPlayer2++;
      }
    }

    //handle button pressing during inactive light
    if ((digitalRead(buttonInputPins[i]) == LOW) && (digitalRead(buttonLightPins[i]) == LOW))
    {
      buttonHitTime[i] = millis();

      //handles both rapid button triggering and lengthy, pressed buttons
      if ((buttonHitTime[i] - priorButtonHitTime[i] > buttonPressedInterval) && !freestyleModeOn)
      {
        if (i < 4)
        {
          failurePointsPlayer1++;
        }
        else
        {
          failurePointsPlayer2++;
        }

        priorButtonHitTime[i] = millis();
      }
    }
  }
}

/*****************************************************
  Function: HandleSolenoids
    -Determines if game has ben won or lost
*****************************************************/
void HandleSolenoids()
{
  for (int i = 0; i < sizeof(buttonInputPins) / sizeof(int); i++)
  {
    if (digitalRead(buttonInputPins[i]) == HIGH)
    {
      digitalWrite(solenoidPins[i], LOW);
    }
    else
    {
      digitalWrite(solenoidPins[i], HIGH);
    }

    if (onePlayerGame == true)
    {
      if (i >= 3)
      {
        i = 8;
      }
    }
  }
}

/*****************************************************
  Function: PullSongTxtFile
    -Opens song txt file (trigger times) fron the SD
     and parses into a two dimensional array
*****************************************************/
void PullSongTxtFile(char txtFile[], int timeArrayy[][9])
{
  myFile = SD.open(txtFile);

  timeHolderString = "";

  if (myFile)
  {
    Serial.println("Reading Song Text File");

    // read from the file until the end
    while (myFile.available())
    {
      //store character from file
      charFromFile = myFile.read();

      //check to see if the character is a number
      if (isdigit(charFromFile))
      {
        previousChar = charFromFile;

        //add current char to string (for 'time' numbers)
        timeHolderString.concat(charFromFile);
      }
      else
      {
        //check if prior char was a digit
        if (isdigit(previousChar))
        {
          //store timer (ms) values to the array if not 1 or 0 (stores ms)
          if (timeHolderString.toInt() >= 2)
          {
            timeArrayy[firstArrayDimension][secondArrayDimension] = timeHolderString.toInt();
            //Serial.println(timeArrayy[firstArrayDimension][secondArrayDimension]);
          }
          //store 1's and 0's into array
          else
          {
            secondArrayDimension++;
            timeArrayy[firstArrayDimension][secondArrayDimension] = timeHolderString.toInt();
            //Serial.println(timeArrayy[firstArrayDimension][secondArrayDimension]);

            //reset counter if all button vals have been stored
            if (secondArrayDimension >= (sizeof(buttonLightPins) / sizeof(int)))
            {
              secondArrayDimension = 0;
              firstArrayDimension++;
            }
          }
          //clear string
          timeHolderString = "";
        }
      }
    }
    // close the file
    myFile.close();
  }
  else
  {
    // if the file didn't open
    Serial.println("error opening test.txt");
  }
}

/*****************************************************
  Function: PlaySong
    -
*****************************************************/
void PlaySong(char wavFile[])
{
  if (playSongFlag)
  {
    if (!beginAttractModeSong)
    {
      Serial.println("Pausing for Intro: " + (String)pauseForIntroMusic + " ms");
      delay(pauseForIntroMusic);
    }

    Serial.flush();
    Serial.println("Start Playing Song");
    delay(10);
    //playSdWav1.play(wavFile);

    songNameHolder = (String)wavFile;
    Serial.println(songNameHolder);

    playSdWav1.play(wavFile);

    delay(10); // wait for library to parse WAV info
    playSongFlag = false;
  }
}

/*****************************************************
  Function: HandleGameEndAndReset
    -Determines if game has ben won or lost
    -Resets variables for next gameplay instance
*****************************************************/
void HandleGameEndAndReset()
{
  //turn off button lights (redundancy) and reset button-hit array
  for (int i = 0; i < sizeof(buttonLightPins) / sizeof(int); i++)
  {
    digitalWrite(buttonLightPins[i], LOW);

    buttonHitTime[i] = 0;
    priorButtonHitTime[i] = 0;
  }

  percentageCorrectPlayer1 = (float)(winPointsPlayer1 - failurePointsPlayer1) / (float)totalGamePointsPlayer1;
  percentageCorrectPlayer2 = (float)(winPointsPlayer2 - failurePointsPlayer2) / (float)totalGamePointsPlayer2;

  Serial.println("\nPlayer 1: Total Possible Game Points: " + (String)totalGamePointsPlayer1);
  Serial.println("Player 1: Buttons Properly Triggered: " + (String)winPointsPlayer1);
  Serial.println("Player 1: Misfire Total: " + (String)failurePointsPlayer1);
  Serial.println("Player 1 Success Percentage:" + (String)(percentageCorrectPlayer1 * 100.0));

  if (onePlayerGame == false)
  {
    Serial.println("\nPlayer 2: Total Possible Game Points: " + (String)totalGamePointsPlayer2);
    Serial.println("Player 2: Buttons Properly Triggered: " + (String)winPointsPlayer2);
    Serial.println("Player 2: Misfire Total: " + (String)failurePointsPlayer2);
    Serial.println("Player 2 Success Percentage:" + (String)(percentageCorrectPlayer2 * 100.0));
  }

  totalScorePercentage = ((float)(winPointsPlayer1 - failurePointsPlayer1) + (float)(winPointsPlayer2 - failurePointsPlayer2)) / 
               ((float)(totalGamePointsPlayer1 + totalGamePointsPlayer2));

  Serial.println("Total Score Percentage:" + (String)(totalScorePercentage * 100.0));

  //HandleTicketPayout();

  startGame = false;
  songEnd = false;

  initialGameStartTime = 0;

  winPointsPlayer1 = 0;
  winPointsPlayer2 = 0;
  totalGamePointsPlayer1 = 0;
  totalGamePointsPlayer2 = 0;
  failurePointsPlayer1 = 0;
  failurePointsPlayer2 = 0;
  percentageCorrectPlayer1 = 0;
  percentageCorrectPlayer2 = 0;
  totalScorePercentage = 0;
  
  eLoc = 0;

  listenToSemnoxPlayerCount = true;
  playerCount = 0;

  Serial.flush();

  highInputFlag = 1;
  onePlayerWins = false;
  twoPlayersTie = false;
  onePlayerGame = false;
  twoPlayerGame = false;
  freestyleModeOn = true;

  digitalWrite(playerOneSpotLight, LOW);
  digitalWrite(playerTwoSpotLight, LOW);
}

/*****************************************************
  Function: AutomateSolenoids
    -
*****************************************************/
void ActivateSolenoids()
{
  currentTime = millis() - initialGameStartTime;

  if (currentTime > (parsedSongHolder[eLoc][0] - 10) && currentTime < (parsedSongHolder[eLoc][0] + 250))
  {
    Serial.print("Expected: " + (String)parsedSongHolder[eLoc][0] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][1] + ", " + (String)parsedSongHolder[eLoc][2] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][3] + ", " + (String)parsedSongHolder[eLoc][4] + ", ");
    Serial.print((String)parsedSongHolder[eLoc][5] + ", " + (String)parsedSongHolder[eLoc][6] + ", ");
    Serial.println((String)parsedSongHolder[eLoc][7] + ", " + (String)parsedSongHolder[eLoc][8]);

    //loop through pins for light triggering
    if (activateSolenoidFlag)
    {
      startTime = currentTime;

      for (int i = 0; i < sizeof(solenoidPins) / sizeof(int); i++)
      {
        if (parsedSongHolder[eLoc][i + 1] == 1)
        {
          digitalWrite(solenoidPins[i], HIGH);
          //Serial.println("Solenoid Pin " + (String)solenoidPins[i] + ": " + (String)(digitalRead(solenoidPins[i])));
        }
      }
      activateSolenoidFlag = false;
    }
    eLoc++; //increment to next triggering step
  }
}

/*****************************************************
  Function: DeactivateSolenoids
    -
*****************************************************/
void DeactivateSolenoids()
{
  if (currentTime > startTime + 500)
  {
    if (activateSolenoidFlag == false)
    {
      for (int i = 0; i < sizeof(solenoidPins) / sizeof(int); i++)
      {
        digitalWrite(solenoidPins[i], LOW);
      }
      activateSolenoidFlag = true;
    }
  }
}

/*****************************************************
  Function: StopAttractMode();
    -
*****************************************************/
void StopAttractMode()
{
  for (int i = 0; i < sizeof(solenoidPins) / sizeof(int); i++)
  {
    digitalWrite(solenoidPins[i], LOW);
    digitalWrite(buttonLightPins[i], LOW);
    delay(10);
  }

  Serial.flush();
  delay(1000);

  playSdWav1.stop();

  delay(1000);
}

/*****************************************************
  Function: CopyTimeArray()
    -
*****************************************************/
void CopyTimeArray(int timeArray[][9])
{
  for (int i = 0; i < 300; i++)
  {
    for (int j = 0; j < 9; j++)
    {
      parsedSongHolder[i][j] = timeArray[i][j];
    }
  }
}

/****************************************************
   Function: PullUserParamterFiles()
    -
 ****************************************************/
void PullUserParameterFiles()
{
  myFile = SD.open("userParameters.txt");

  timeHolderString = "";

  if (myFile)
  {
    Serial.println("Reading User Parameters File");

    // read from the file until the end
    while (myFile.available())
    {
      //store character from file
      charFromFile = myFile.read();

      //check to see if the character is a number
      if (isdigit(charFromFile))
      {
        previousChar = charFromFile;

        //add current char to string (for 'time' numbers)
        timeHolderString.concat(charFromFile);
      }
      else
      {
        //check if prior char was a digit
        if (isdigit(previousChar))
        {
          userParameters[parameterCounter] = timeHolderString.toInt();
          //Serial.println(userParameters[parameterCounter]);

          parameterCounter++;
        }

        //clear string
        timeHolderString = "";
      }
    }
    // close the file
    myFile.close();
  }
  else
  {
    // if the file didn't open
    Serial.println("error opening user parameters file");
  }

  buttonLightSolenoidTestTime = userParameters[0];
  bOffset = userParameters[1];
  aOffset = userParameters[2];
  buttonPressedInterval = userParameters[3];
  pauseForIntroMusic = userParameters[4];
  triggerAttractModeTime = userParameters[5];
  percentageCorrect = userParameters[6];
  randomizeSongs = userParameters[7];

  Serial.println("\nButton Light & Solenoid Time: " + (String)buttonLightSolenoidTestTime);
  Serial.println("Time Offset Before the Expected Trigger Time: " + (String)bOffset);
  Serial.println("Time Offset After the Expected Trigger Time:  " + (String)aOffset);
  Serial.println("Button Pressed Interval Threshold: " + (String)buttonPressedInterval);
  Serial.println("Pause Before the Song Begins: " + (String)pauseForIntroMusic);
  Serial.println("Time Interveral Between Attract Mode Song Triggering: " + (String)triggerAttractModeTime);
  Serial.println("Percentage of Successful Triggers to Win: " + (String)percentageCorrect);
  
  if (randomizeSongs)
  {
    randomSelectionOn = true;
    Serial.println("Randomize Songs: True");
  }
  else
  {
    randomSelectionOn = false;
    Serial.println("Randomize Songs: False");
  }
}

/****************************************************
   Function: randomlySelectSong()
    -
 ****************************************************/
void randomlySelectSong()
{
  if (randomSelectionOn)
  { 
    randomSongNumber = random(0, 20);
  } 
  else 
  {
    if(songCounter >= 20)
    {
      songCounter = 0;
    }
    
    randomSongNumber = songCounter;
    songCounter++;
  }

  switch (randomSongNumber)
  {
    case 0:
      CopyTimeArray(parsedSongText1);
      Serial.println("Song 1 Selected");
      break;
    case 1:
      CopyTimeArray(parsedSongText2);
      Serial.println("Song 2 Selected");
      break;
    case 2:
      CopyTimeArray(parsedSongText3);
      Serial.println("Song 3 Selected");
      break;
    case 3:
      CopyTimeArray(parsedSongText4);
      Serial.println("Song 4 Selected");
      break;
    case 4:
      CopyTimeArray(parsedSongText5);
      Serial.println("Song 5 Selected");
      break;
    case 5:
      CopyTimeArray(parsedSongText6);
      Serial.println("Song 6 Selected");
      break;
    case 6:
      CopyTimeArray(parsedSongText7);
      Serial.println("Song 7 Selected");
      break;
    case 7:
      CopyTimeArray(parsedSongText8);
      Serial.println("Song 8 Selected");
      break;
    case 8:
      CopyTimeArray(parsedSongText9);
      Serial.println("Song 9 Selected");
      break;
    case 9:
      CopyTimeArray(parsedSongText10);
      Serial.println("Song 10 Selected");
      break;
    case 10:
      CopyTimeArray(parsedSongText11);
      Serial.println("Song 11 Selected");
      break;
    case 11:
      CopyTimeArray(parsedSongText12);
      Serial.println("Song 12 Selected");
      break;
    case 12:
      CopyTimeArray(parsedSongText13);
      Serial.println("Song 13 Selected");
      break;
    case 13:
      CopyTimeArray(parsedSongText14);
      Serial.println("Song 14 Selected");
      break;
    case 14:
      CopyTimeArray(parsedSongText15);
      Serial.println("Song 15 Selected");
      break;
    case 15:
      CopyTimeArray(parsedSongText16);
      Serial.println("Song 16 Selected");
      break;
    case 16:
      CopyTimeArray(parsedSongText17);
      Serial.println("Song 17 Selected");
      break;
    case 17:
      CopyTimeArray(parsedSongText18);
      Serial.println("Song 18 Selected");
      break;
    case 18:
      CopyTimeArray(parsedSongText19);
      Serial.println("Song 19 Selected");
      break;
    case 19:
      CopyTimeArray(parsedSongText20);
      Serial.println("Song 20 Selected");
      break;
  }
}

/****************************************************
   Function: PullUserParamterFiles()
    -
 ****************************************************/
void ParseSongFiles()
{
  for (int i = 0; i < 300; i++)
  {
    for (int j = 0; j < (sizeof(buttonLightPins) / sizeof(int)) + 1; j++)
    {
      parsedSongHolder[i][j] = 0;
      
      parsedSongText1[i][j] = 0;
      parsedSongText2[i][j] = 0;
      parsedSongText3[i][j] = 0;
      parsedSongText4[i][j] = 0;
      parsedSongText5[i][j] = 0;
      parsedSongText6[i][j] = 0;
      parsedSongText7[i][j] = 0;
      parsedSongText8[i][j] = 0;
      parsedSongText9[i][j] = 0;
      parsedSongText10[i][j] = 0;
      parsedSongText11[i][j]  = 0;
      parsedSongText12[i][j] = 0;
      parsedSongText13[i][j] = 0;
      parsedSongText14[i][j] = 0;
      parsedSongText15[i][j] = 0;
      parsedSongText16[i][j] = 0;
      parsedSongText17[i][j] = 0;
      parsedSongText18[i][j] = 0;
      parsedSongText19[i][j] = 0;
      parsedSongText20[i][j] = 0;
      
      parsedSongText21[i][j] = 0;
    }
  }

  PullSongTxtFile("1.txt", parsedSongText1);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("2.txt", parsedSongText2);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("3.txt", parsedSongText3);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("4.txt", parsedSongText4);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("5.txt", parsedSongText5);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("6.txt", parsedSongText6);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("7.txt", parsedSongText7);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("8.txt", parsedSongText8);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("9.txt", parsedSongText9);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("10.txt", parsedSongText10);
  firstArrayDimension = 0;
  secondArrayDimension = 0;
  
  PullSongTxtFile("11.txt", parsedSongText11);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("12.txt", parsedSongText12);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("13.txt", parsedSongText13);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("14.txt", parsedSongText14);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("15.txt", parsedSongText15);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("16.txt", parsedSongText16);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("17.txt", parsedSongText17);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("18.txt", parsedSongText18);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("19.txt", parsedSongText19);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("20.txt", parsedSongText20);
  firstArrayDimension = 0;
  secondArrayDimension = 0;

  PullSongTxtFile("21.txt", parsedSongText21);
  firstArrayDimension = 0;
  secondArrayDimension = 0;
}


/****************************************************
   Function: PullUserParamterFiles()
    -
 ****************************************************/
void PlaySongBasedOnSelection()
{
  PlaySong(songTitleStringArray[randomSongNumber]);
  /*switch (randomSongNumber)
  {
    case 0:
      PlaySong(songTitleStringArray[randomSongNumber]);
      //Serial.println("Loading Song 1");
      break;
    case 1:
      PlaySong(songTitleStringArray[randomSongNumber]);
      //Serial.println("Loading Song 2");
      break;
    case 2:
      PlaySong(songTitleStringArray[randomSongNumber]);
      //Serial.println("Loading Song 3");
      break;
    case 3:
      PlaySong(songTitleStringArray[randomSongNumber]);
      //Serial.println("Loading Song 4");
      break;
  }*/
}


/****************************************************
   Function: HandleSemnoxPlayerCount()
    -
 ****************************************************/
void HandleSemnoxPlayerCount()
{
  if (digitalRead(semnoxPlayerCountInput) == LOW && highInputFlag == 1)
  {
    highInputFlag = 0;
    firstImpulseTime = millis();
    playerCount++;
  }

  if (digitalRead(semnoxPlayerCountInput) == HIGH && highInputFlag == 0 && (millis() - firstImpulseTime) > pwOffset)
  {
    highInputFlag = 1;
  }

  if ((((millis() - firstImpulseTime) > 1000) && playerCount == 1) || playerCount == 2)
  {
    if (playerCount == 1)
    {
      Serial.println("\n2-Player Game Selected (Formerly 1 Player Selection)\n");
      onePlayerGame = false;
      twoPlayerGame = true;
      listenToSemnoxPlayerCount = false;
    }

    if (playerCount == 2)
    {
      Serial.println("\n2-Player Game Selected\n");
      onePlayerGame = false;
      twoPlayerGame = true;
      listenToSemnoxPlayerCount = false;
    }
  }
}

/****************************************************
   Function: HandleTicketPayout()
    -
 ****************************************************/
void HandleTicketPayout()
{
  if (onePlayerGame == true || twoPlayerGame == true)
  {
    Serial.println("\nHold Spotlights\n");
    
    if(totalScorePercentage >= 0.8)
    {
      ticketPulseDelayTime = 131;
      ticketPulseCount = 100;
    }
    else if (totalScorePercentage >= 0.5)
    {
      ticketPulseDelayTime = 128;
      ticketPulseCount = 50;
    }
    else
    {
      ticketPulseDelayTime = 110;
      ticketPulseCount = 20;
    }
    Serial.println("Ticket payout: " + (String)ticketPulseCount);
    Serial.println("Ticket payout pulse-width: " + (String)ticketPulseDelayTime + "\n");
    
    Serial.println("Ticket Activation HIGH\n");
    digitalWrite(semnoxTicketOutputPin, HIGH);
  
    tCounter = 0;
  
    while (tCounter <= ticketPulseCount)
    {
      digitalWrite(semnoxTicketPulseOutput, LOW);
      delay(ticketPulseDelayTime);
      digitalWrite(semnoxTicketPulseOutput, HIGH);
      delay(ticketPulseDelayTime);
  
      Serial.print("Ticket Pulse: ");
      Serial.println(tCounter);  
  
      tCounter++;
    }
  
    digitalWrite(semnoxTicketPulseOutput, HIGH);
  
    delay(1000);
    digitalWrite(semnoxTicketOutputPin, LOW);
    Serial.println("\nTicket Activation LOW");
  
    delay(2000);
  
    Serial.println("Turn off Spotlights\n");
  
    digitalWrite(playerOneSpotLight, LOW);
    digitalWrite(playerTwoSpotLight, LOW);
  } 
}

/****************************************************
  Function: HandleExtraCredits()
    -place in game loop
****************************************************/
void HandleExtraCredits()
{
  if (digitalRead(semnoxPlayerCountInput) == LOW && highInputFlag == 1)
  {
    highInputFlag = 0;
    firstImpulseTime = millis();
  }

  if (digitalRead(semnoxPlayerCountInput) == HIGH && highInputFlag == 0)
  {
    if ((millis() - firstImpulseTime) > pwOffset)
    {
      extraCredit++;
      Serial.print("\n***Extra Credit Deposited***  Credits: ");
      Serial.print(extraCredit);
      Serial.println("\n");
    }
    
    highInputFlag = 1;
  }
}
  
