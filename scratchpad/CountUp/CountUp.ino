const int Bin1 = 12;
const int Bin2 = 8;
const int Bin3 = 9;
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
  //sets up all pins as outputs
  
  
}

void loop()
{
  //the first 4 "for" groups are testing binary pins
 for (mask1 = 0001; mask1>0; mask1<=1);
   if (x & mask1) 
   {digitalWrite(Bin1, 0);}//if result of test is false, set Bin1 to 0 
   else{digitalWrite(Bin1, 1);}//if result of test is true, set Bin1 to 1 
   
 for (mask2 = 0010; mask2>1; mask2<=2);
   if (x & mask2) 
   {digitalWrite(Bin2, 0);}//if result of test is false, set Bin2 to 0  
   else{digitalWrite(Bin2, 1);}//if result of test is true, set Bin2 to 1 
   
 for (mask3 = 0100; mask3>3; mask3<=4);
   if (x & mask3) 
   {digitalWrite(Bin3, 0);}//if result of test is false, set Bin3 to 0
   else{digitalWrite(Bin3, 1);}//if result of test is true, set Bin3 to 1
   
 for (mask4 = 1000; mask4>7; mask4<=8);
   if (x & mask4)
   {digitalWrite(Bin4, 0);}//if result of test is false, set Bin4 to 0
   else{digitalWrite(Bin4, 1);}//if result of test is true, set Bin4 to 1
   
//end of "mask for" groups. 

  for(int x=0; x<= 9; x++);//this groups sets the x value to test against the binary pins. This should cycle through 0-9 in the ones place. 
  digitalWrite(digit1, HIGH);
  digitalWrite(digit2, HIGH);
  digitalWrite(digit3, LOW);
  delay(1000);
  
  
  
}


  
