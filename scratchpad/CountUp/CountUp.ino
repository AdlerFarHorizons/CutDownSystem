const int Bin1 = 12;
const int Bin2 = 8;
const int Bin3 = 9;
const int Bin4 = 11;
int digit1 = 7;
int digit2 = 6;
int digit3 = 5;
int x = 0;
byte mask1 = 1;
byte mask2 = 2;
byte mask3 = 4;
byte mask4 = 8;
byte display1 = 0;
byte display2 = 0;


void setup()
{
  //sets up all pins as outputs
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Bin3, OUTPUT);
  pinMode(Bin4, OUTPUT);
  pinMode(digit1, OUTPUT); // Farthest right
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT); // Farthest left
  
  Serial.begin(9600);
}

void loop()
{
  
  for(int x=0; x<= 99; x++)
  {
    if (display1 < 9) 
    {
      display1++;
    }
    
    else
    {
      display1 = 0;
      //OLD: display1++; This made the code skip every multiple of ten
      
      if (display2 < 9)
      {
        display2++;
      }
      
      else
      {
        display2 = 0;
      }
    }
    Serial.print(display2);
    Serial.println(display1);
    
    // I switched the order (display highest number first). This was causing the counter to display the display2 value before the one it should be.
    // Display is active LOW
    digitalWrite(digit1, HIGH);
    digitalWrite(digit2, LOW);
    digitalWrite(digit3, HIGH);

    if (display2 & mask1)
    {
      digitalWrite(Bin1, 1);
    }
    else
    {
      digitalWrite(Bin1, 0);
    }
   
    if (display2 & mask2)
    {
      digitalWrite(Bin2, 1);
    }
    else
    {
      digitalWrite(Bin2, 0);
    }
   
    if (display2 & mask3)
    {
      digitalWrite(Bin3, 1);
    }
    else
    {
      digitalWrite(Bin3, 0);
    }
   
    if (display2 & mask4)
    {
      digitalWrite(Bin4, 1);
    }
    else
    {
      digitalWrite(Bin4, 0);
    }
   
    delay(1000);
    
    // Second number
    digitalWrite(digit1, LOW);
    digitalWrite(digit2, HIGH);
    digitalWrite(digit3, HIGH);
    
    if (display1 & mask1)
    {
      digitalWrite(Bin1, 1);
    }
    else
    {
      digitalWrite(Bin1, 0); //OLD: digitalWrite(Bin1, 1); This created the double odd numbers
    }
   
    if (display1 & mask2)
    {
      digitalWrite(Bin2, 1);
    }
    else
    {
      digitalWrite(Bin2, 0);
    }
   
    if (display1 & mask3)
    {
      digitalWrite(Bin3, 1);
    }
    else
    {
      digitalWrite(Bin3, 0);
    }
  
    if (display1 & mask4)
    {
      digitalWrite(Bin4, 1);
    }
    else
    {
      digitalWrite(Bin4, 0);
    }
   
    delay(1000);
   
    
    
  } // End of for loop  
} // End of void loop


  
