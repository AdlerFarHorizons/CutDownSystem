const int Bin1= 12;
const int Bin2= 8;
const int Bin3= 9;
const int Bin4= 11;
int digit1= 7;
int digit2= 6;
int digit3= 5;

void setup()
{
  pinMode(Bin1, OUTPUT);
  pinMode(Bin2, OUTPUT);
  pinMode(Bin3, OUTPUT);
  pinMode(Bin4, OUTPUT);
  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  
}


void baseincrement(int x)
{

  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 1
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B0);
  digitalWrite(Bin2, B1);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 2
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B1);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 3
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B0);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B1);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 4
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B1);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 5
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B0);
  digitalWrite(Bin2, B1);
  digitalWrite(Bin3, B1);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 6
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B1);
  digitalWrite(Bin3, B1);
  digitalWrite(Bin4, B0);
  delay(500);
  //should write number 7
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B0);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B1);
  delay(500);
  //should write number 8
  
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B1);
  delay(500);
  //should write number 9
  
}

void loop()
{
  baseincrement(1);
  delay(1000);
  digitalWrite(digit1, HIGH);
  digitalWrite(digit2, LOW);
  digitalWrite(digit3, HIGH);
  digitalWrite(Bin1, B1);
  digitalWrite(Bin2, B0);
  digitalWrite(Bin3, B0);
  digitalWrite(Bin4, B0);
  delay(500)
  baseincrement(1);
}
  
