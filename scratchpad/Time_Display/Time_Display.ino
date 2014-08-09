//digital pin each binary is connected to 
const int Bin1 = 12;
const int Bin2 = 8;
const int Bin3 = 9;
const int Bin4= 11;
int digit1= 7;
int digit2= 6;
int digit3= 5;


void setup () 
{
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Bin3, OUTPUT);
  pinMode(Bin4, OUTPUT);
  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  
  
  
}

void loop() //should write "1" to display
{
  digitalWrite(digit1, HIGH);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, LOW);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
  delay(50);
  
  digitalWrite(digit1, HIGH);
  digitalWrite(digit2, LOW);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B0);
 digitalWrite(Bin2, B1);
digitalWrite(Bin3, B0);
digitalWrite(Bin4, B0);
delay(50);

digitalWrite(digit1, LOW);
digitalWrite(digit2, HIGH);
digitalWrite(digit3, HIGH);
digitalWrite(Bin1, B1);
digitalWrite(Bin2, B1);
digitalWrite(Bin3, B0);
digitalWrite(Bin4, B0);
delay(50);
}
  
  
