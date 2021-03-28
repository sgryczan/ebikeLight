#include <Adafruit_NeoPixel.h>

#define LEFT_OUT 3
#define RIGHT_OUT 4
#define BRAKE 0
#define LBLINK 1
#define RBLINK 2
#define LENGTH 55

int brakeVal = 0, lBlinkVal = 0, rBlinkVal = 0, blinkState = 0, currentFrame = 0;

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

Adafruit_NeoPixel l_strip = Adafruit_NeoPixel(LENGTH, LEFT_OUT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel r_strip = Adafruit_NeoPixel(LENGTH, RIGHT_OUT, NEO_GRB + NEO_KHZ800);

const long blinkInterval = 500;           // interval at which to blink (milliseconds)
unsigned long previousMillis = 0;          // will store last time LED was updated

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }
 
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }
 
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};
 
void Stick1Complete();
void Stick2Complete();

NeoPatterns Stick1(55, LEFT_OUT, NEO_GRB + NEO_KHZ800, &Stick1Complete);
NeoPatterns Stick2(55, RIGHT_OUT, NEO_GRB + NEO_KHZ800, &Stick2Complete);

void setup() {
  // brightness is 0-255
  l_strip.setBrightness(255);
  r_strip.setBrightness(255);
  l_strip.begin();
  r_strip.begin();
  l_strip.show();
  r_strip.show();

  // input pins
  pinMode(BRAKE, INPUT_PULLUP);
  pinMode(LBLINK, INPUT_PULLUP);
  pinMode(RBLINK, INPUT_PULLUP);

  Stick1.RainbowCycle(0, REVERSE);
  Stick2.RainbowCycle(0);
}

void loop() {
  brakeVal = digitalRead(BRAKE);
  lBlinkVal = digitalRead(LBLINK);
  rBlinkVal = digitalRead(RBLINK);
  
  Serial.print(brakeVal);

  // Brake Lever is pulled
  if (brakeVal == LOW){
    uint32_t red = l_strip.Color(255, 0, 0);
    // Only show brake light if blinker is not on
    if (lBlinkVal != LOW) {
      l_strip.fill(red,0,LENGTH);
      l_strip.show();
    }
    if (rBlinkVal != LOW) {
      r_strip.fill(red,0,LENGTH);
      r_strip.show();
    }
    
    
    //brakeFill(l_strip, r_strip);


    // smoother effect
    //for (int i=0; i<LENGTH; i++) {
    //  l_strip.setPixelColor(i,255,0,0);
    //  r_strip.setPixelColor(i,255,0,0);
    //  l_strip.show();
    //  r_strip.show();
    //}
  }

  // Left Blinker

  if (lBlinkVal == LOW){
    // check to see if it's time to blink the LED; that is, if the difference
    // between the current time and last time you blinked the LED is bigger than
    // the interval at which you want to blink the LED.
    unsigned long currentMillis = millis();
  
    if (currentMillis - previousMillis >= blinkInterval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      if (blinkState == 0) {
        blinkState = 1;
        
        //uint32_t orange = l_strip.Color(255, 80, 0);
        //l_strip.fill(orange,0,LENGTH);
        //l_strip.show();

        for (int i=0; i<LENGTH; i++) {
          l_strip.setPixelColor(i,255,80,0);
          l_strip.show();
        }
      } else {
          blinkState = 0;
   
          uint32_t black = l_strip.Color(0, 0, 0);
          l_strip.fill(black,0,LENGTH);
          l_strip.show();
      }
    }
  }

  // Right Blinker

  if (rBlinkVal == LOW){
    unsigned long currentMillis = millis();
  
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;

      if (blinkState == 0) {
        blinkState = 1;
        
        //uint32_t orange = r_strip.Color(255, 80, 0);
        //r_strip.fill(orange,0,LENGTH);
        //r_strip.show();

        for (int i=0; i<LENGTH; i++) {
          r_strip.setPixelColor(i,255,80,0);
          r_strip.show();
        }
      } else {
          blinkState = 0;
   
          uint32_t black = r_strip.Color(0, 0, 0);
          r_strip.fill(black,0,LENGTH);
          r_strip.show();
      }
    }
  }


  // Brake Lever Released
  if (brakeVal == HIGH && lBlinkVal == HIGH && rBlinkVal == HIGH) {

    // All signals are idle
    // TODO Party light pattern goes here?
    //Stick1.ActivePattern = FADE;
    Stick1.Update();
    Stick2.Update();

    // turn off
    //uint32_t black = r_strip.Color(0, 0, 0);
    //r_strip.fill(black,0,LENGTH);
    //r_strip.show();
  }
  if (brakeVal == HIGH){
    if (lBlinkVal == HIGH) {
      uint32_t black = l_strip.Color(0, 0, 0);
      l_strip.fill(black,0,LENGTH);
      l_strip.show();
    }
    if (rBlinkVal == HIGH) {
      uint32_t black = r_strip.Color(0, 0, 0);
      r_strip.fill(black,0,LENGTH);
      r_strip.show();
    }
  }
}

// TODO - figure out why these wont return correctly

void brakeFill(Adafruit_NeoPixel x, Adafruit_NeoPixel y) {
  uint32_t red = x.Color(255, 0, 0);
  x.fill(red,0,LENGTH);
  y.fill(red,0,LENGTH);
  x.show();
  y.show();
}

void blinkerFill(Adafruit_NeoPixel x) {
  uint32_t orange = x.Color(255, 80, 0);
  x.fill(orange,0,LENGTH);
  x.show();
}

void clearStrip(Adafruit_NeoPixel x) {
  uint32_t black = x.Color(0, 0, 0);
  x.fill(black,0,LENGTH);
  x.show();
}

void wipeClear(Adafruit_NeoPixel x) {
  for (int i=0; i<LENGTH; i++) {
      x.setPixelColor(i,0,0,0);
      x.show();
    }
}

// Stick Completion Callback
void Stick1Complete()
{
    // Random color change for next scan
    Stick1.Color1 = Stick1.Wheel(random(255));
}

void Stick2Complete()
{
    // Random color change for next scan
    Stick2.Color1 = Stick2.Wheel(random(255));
}

int numPixels() {
  return LENGTH;
}
