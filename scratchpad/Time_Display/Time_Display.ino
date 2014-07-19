//digital pin each binary is connected to 
const int Bin1 = 12;
const int Bin2 = 8;
const int Bin3 = 9;
const int Bin4= 11;

void setup () 
{
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Bin3, OUTPUT);
  pinMode(Bin4, OUTPUT);
  
}

void loop() //should write "1" to display
{
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
}
  
  
