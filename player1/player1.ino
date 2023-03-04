// Include Arduino Wire library for I2C
#include <Wire.h>
// Include LCD display library for I2C
#include <LiquidCrystal_I2C.h>
// Include Keypad library
#include <Keypad.h>
// Include LED matrix library
#include <LedControl.h>

// Array to represent keys on keypad
char hexaKeys[4][3] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// Connections to Arduino
byte rowPins[4] = {8, 7, 6, 5}; //pt ca avem 4 randuri
byte colPins[3] = {4, 3, 2};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, 4, 3);

LiquidCrystal_I2C lcd(0x27 , 16, 2);

LedControl lc = LedControl(11, 9, 10, 0);

unsigned long lcdTimer = 0;
unsigned long lcdInterval = 10000;

unsigned long delaytime = 600;
int xPos;
int yPos;
int PLAYER;
int WON = 0;

//  MAPS
int A[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
int R[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

int me = 0; //randul meu

// BACKUP MAPS
//salvez matricea nemodificata sa o afisez la final
int saveR[8][8];

int x = 0;

int row = 0;
void setup() {
  // put your setup code here, to run once:
  randomSeed(analogRead(0));  //pt a merge random corect


  Serial.begin(9600);
  lcd.backlight(); //DE CE NE TREBE?
  lcd.init();

  lc.shutdown(0, false);
  lc.setIntensity(0, 4);
  lc.clearDisplay(0);

  lcd.begin(16, 2);
  lcd.clear();
  xPos = 0;
  yPos = 7;

  // display on LED matrix
  lc.setRow(0, 0, B10000000);

  // generate maps and backup maps
  generateMaps();

  //displayMapOnLED(A);

  // Pornim busul I2C ca și slave la adresa 9
  Wire.begin(9);
  Wire.begin();
  // Atașăm o funcție care să se declanșeze atunci când primim
  //ceva

  Wire.onReceive(receiveEvent);

  //delay(700);

  // game is won when WON != -1
  WON = -1;
}

int data;
int raul=0;
void receiveEvent(int bytes) {

  data = (int)Wire.read();
  if (data == 2)
  {
    me = 1; //e randul meu
  }
  if(data == 3)
  {
    raul=1;
    Serial.print(raul);
  }
  while ( Wire.available())
  {
    if (data == 0 || data == 1)
    {
      R[row][0] = data;
      for (int j = 1; j < 8; j++)
      {
        R[row][j] = (int) Wire.read();
      }

      row++;
      me = 1;

    }
  }
}


int getMax (int x, int y) {
  if (x > y) return x;
  return y;
}
int getMin (int x, int y) {
  if (x < y)  return x;
  return y;
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'

int onSegment(int px, int py, int qx, int qy, int rx, int ry)
{
  if (qx <= getMax(px, rx) && qx >= getMin(px, rx) &&
      qy <= getMax(py, ry) && qy >= getMin(py, ry))
    return 1; //true

  return 0; //false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise

int orientation(int px, int py, int qx, int qy, int rx, int ry)
{
  // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
  // for details of below formula.
  int val = (qy - py) * (rx - qx) -
            (qx - px) * (ry - qy);

  if (val == 0) return 0;  // colinear

  return (val > 0) ? 1 : 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.

int doIntersection(int p1x, int p1y, int q1x, int q1y, int p2x, int p2y, int q2x, int q2y)
{
  // Find the four orientations needed for general and
  // special cases
  int o1 = orientation(p1x, p1y, q1x, q1y, p2x, p2y);
  int o2 = orientation(p1x, p1y, q1x, q1y, q2x, q2y);
  int o3 = orientation(p2x, p2y, q2x, q2y, p1x, p1y);
  int o4 = orientation(p2x, p2y, q2x, q2y, q1x, q1y);

  // General case
  if (o1 != o2 && o3 != o4)
    return 1; //true

  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && onSegment(p1x, p1y, p2x, p2y, q1x, q1y)) return 1;// true;

  // p1, q1 and q2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && onSegment(p1x, p1y, q2x, q2y, q1x, q1y)) return 1;// true;

  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && onSegment(p2x, p2y, p1x, p1y, q2x, q2y)) return 1;//true;

  // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && onSegment(p2x, p2y, q1x, q1y, q2x, q2y)) return 1;// true;

  return 0;//false; // Doesn't fall in any of the above cases
}

void generateMap(int matrix[8][8]) {
  int i;
  // store data about previusly placed ships
  int sx[3] = { -1, -1, -1}; // x start
  int sy[3] = { -1, -1, -1}; // y start
  int d[3] = { -1, -1, -1}; // direction of ship
  int ex[3] = { -1, -1, -1}; // x end
  int ey[3] = { -1, -1, -1}; // y end

  // generate 3 ships of length 2, 3, 4
  for (i = 0; i < 3; i++) {
    int l = i + 2;
    int ok = 0;
    int xEnd, yEnd;
    while (ok == 0) {
      //get random direction : 0=up/1=down/2=left/3=right
      int dir = random(0, 4);
      // get random starting point: x & y
      int xStart = random(0, 8);
      int yStart = random(0, 8);

      if (sx[i] == xStart && sy[i] == yStart) {
        continue;
      }

      int lx = 0;
      int ly = 0;

      if (dir == 0) {
        // UP
        lx = -1;
      } else if (dir == 1) {
        // DOWN
        lx = 1;
      } else if (dir == 2) {
        // LEFT
        ly = -1;
      } else if (dir == 3) {
        // RIGHT
        ly = 1;
      }

      xEnd = xStart + lx * l;
      yEnd = yStart + ly * l;

      if (xEnd < 0 || yEnd < 0 || xEnd > 7 || yEnd > 7)
        continue;

      int j;
      int okInt = 0;
      for (j = 0; j < 3 && okInt == 0; j++) {
        if (d[j] == -1 || j == i) {
          continue;
        }

        okInt = doIntersection(xStart, yStart, xEnd, yEnd, sx[j], sy[j], ex[j], ey[j]);
      }
      if (okInt == 0) {
        // current ship doesn't intersec with previously placed ships
        ok = 1;
        // store data about the new ship
        sx[i] = xStart;
        sy[i] = yStart;
        ex[i] = xEnd;
        ey[i] = yEnd;
        d[i] = dir;
      }
    }
  }
  // mapping of the ships on the matrix
  for (i = 0; i < 3; i++ ) {
    if (ex[i] < sx[i] ) {
      int aux = ex[i];
      ex[i] = sx[i];
      sx[i] = aux;
    }
    if (ey[i] < sy[i]) {
      int aux = ey[i];
      ey[i] = sy[i];
      sy[i] = aux;
    }

    int l = i + 2;
    // writing "1"'s on  matrix
    int r, c;

    if (sx[i] == ex[i]) {
      for (c = sy[i]; c < ey[i]; c++) {
        matrix[sx[i]][c] = 1;
      }
    } else {
      for (r = sx[i]; r < ex[i]; r++) {
        matrix[r][sy[i]] = 1;
      }
    }
  }
}

int generated = 0;
void generateMaps () {

  generateMap(A);

  generated = 1;

  //print maps
  Serial.println("HARTA A");
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++)
      Serial.print(A[i][j]);
    Serial.println();
  }

  //displayMapOnLED(A);
}


int checkHit(int x, int y) {
  // translate from the position selected by the player on the LED matrix to
  // the position mapping on the integer matrix with the actual map:
  // y is changing (the indexing is form right to left on the LED matrix)

  y = 7 - y;
  if (R[x][y] == 1) { // hit
    // modifiy initial matrix
    R[x][y] = 0;
    return 1;
  }
  else return 0;

}

int checkWin() {
  // if the matrix is full of 0's, then current player win

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (R[i][j] == 1) {
        // no win yet
        return 0;
      }
    }
  }
  // win
  return 1;
}
int transmis = 0;
int recieved = 0;
int xHit[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1};
int yHit[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1};
int hitContor = 0;

void loop() {
  byte LEDboard[8] = { B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000 };
  if (transmis == 0 && generated == 1)
  {
    for (int i = 0; i < 8; i++)
    {
      Wire.beginTransmission(9);
      for (int j = 0; j < 8; j++)
      {
        Wire.write(A[i][j]);
      }
      Wire.endTransmission();
    }
    transmis = 1;
  }

  if (row == 8)
  {
    //print maps
    Serial.println("HARTA R");
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++)
        Serial.print(R[i][j]);
      Serial.println();

      //displayMapOnLED(R);
    }
    row = 0;
  }

   if(raul ==1)
    {
      lcdTimer = 0;
      if (millis() - lcdTimer >= lcdInterval)
      {
        lcdTimer = millis();
        lcd.setCursor(0, 0);
        lcd.print("RAUL WON        ");

      }
    }

    if ( WON == 1) {
      for (int i = 0; i < 9; i++)
        if (xHit[i] != -1 && yHit[i] != -1)
          LEDboard[xHit[i]] |= 1 << yHit[i];

      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, LEDboard[i]);
      }
      
      lcdTimer = 0;
      if (millis() - lcdTimer >= lcdInterval)
      {
        lcdTimer = millis();
        lcd.setCursor(0, 0);
        lcd.print("ANDREEA WON      ");

      }
    }

  for (int i = 0; i < 9; i++)
    if (xHit[i] != -1 && yHit[i] != -1)
      LEDboard[xHit[i]] |= 1 << yHit[i];
      
  // while game isn't won
  if (WON == -1 && me == 1) {

    lcd.clear();
    lcdTimer = 0;
    if (millis() - lcdTimer >= lcdInterval)
    {
      lcdTimer = millis();
      lcd.setCursor(0, 0);
      lcd.print("Andreea's turn");
      lcd.setCursor(0, 1);
    }

    char customKey = customKeypad.getKey();

    if (customKey == '2') { //UP
      if (xPos > 0)
        xPos--;
    } else if (customKey == '8') { // DOWN
      if (xPos < 7)
        xPos++;
    } else if (customKey == '4') { // LEFT
      if (yPos < 7)
        yPos++; // theindexing on the LED matrix is from right to left
    } else if (customKey == '6') { //RIGHT
      if (yPos > 0)
        yPos--;
    } else if (customKey == '5') { // ATTACK   //daca tasta e 9

      // HIT or MISS
      if (checkHit(xPos, yPos) == 1) {
        xHit[hitContor] = xPos;
        yHit[hitContor] = yPos;
        hitContor++;

        // WIN?
        if (checkWin() == 1) {
          WON = 1;
        }
      }

      //imi reactualizeaza pozitia pe matrice la (0,0)

      LEDboard[xPos] = B00000000;
      xPos = 0;
      yPos = 7;
      delay(500);
      LEDboard[xPos] |= 1 << yPos;
      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, LEDboard[i]);
      }  
      if (WON != 1)
      {
        Wire.beginTransmission(9); // transmitem spre device #9
        Wire.write(2); // trimitem 2 sa stie ca e randul lui
        Wire.endTransmission(); // oprim transmisia
        me = 0; //nu mai e randul meu
      }
    }
     // display current position of current player on LED matrix
      LEDboard[xPos] |= 1 << yPos;
      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, LEDboard[i]);
      }
  }
  else {
    if (me == 0 && raul == 0)
    {    
      for (int i = 0; i < 9; i++)
        if (xHit[i] != -1 && yHit[i] != -1)
          LEDboard[xHit[i]] |= 1 << yHit[i];

      for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, LEDboard[i]);
      }
      lcdTimer = 0;
      if (millis() - lcdTimer >= lcdInterval)
      {
        lcdTimer = millis();
        lcd.setCursor(0, 0);
        lcd.print("Raul's turn   ");
      }
    } 
  }
}
