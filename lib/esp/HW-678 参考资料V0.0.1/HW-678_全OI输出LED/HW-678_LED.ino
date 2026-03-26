int led_io[50]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49};

void setup() {
  // put your setup code here, to run once:
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
pinMode(4,OUTPUT);
pinMode(5,OUTPUT);
pinMode(6,OUTPUT);
pinMode(7,OUTPUT);
pinMode(8,OUTPUT);
pinMode(9,OUTPUT);
pinMode(10,OUTPUT);
pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(14,OUTPUT);
pinMode(15,OUTPUT);
pinMode(16,OUTPUT);
pinMode(17,OUTPUT);
pinMode(18,OUTPUT);
  pinMode(19,OUTPUT);
pinMode(20,OUTPUT);
 pinMode(21,OUTPUT);
 // pinMode(22,OUTPUT);
//  pinMode(23,OUTPUT);
//  pinMode(24,OUTPUT);
//  pinMode(25,OUTPUT);
//  pinMode(26,OUTPUT);
//  pinMode(27,OUTPUT);
//  pinMode(28,OUTPUT);
//  pinMode(29,OUTPUT);
//  pinMode(30,OUTPUT);
//  pinMode(31,OUTPUT);
//  pinMode(32,OUTPUT);
//  pinMode(33,OUTPUT);
//  pinMode(34,OUTPUT);
 pinMode(35,OUTPUT);
 pinMode(36,OUTPUT);
 pinMode(37,OUTPUT);
 pinMode(38,OUTPUT);
 pinMode(39,OUTPUT);
 pinMode(40,OUTPUT);
 pinMode(41,OUTPUT);
 pinMode(42,OUTPUT);
 pinMode(43,OUTPUT);
 pinMode(44,OUTPUT);
 pinMode(45,OUTPUT);
 pinMode(46,OUTPUT);
 pinMode(47,OUTPUT);
 pinMode(48,OUTPUT);
//  pinMode(49,OUTPUT);

}

void loop() {
  int i;
  for(i=0;i<50;i++)
  {
    digitalWrite(led_io[i],HIGH);
  }
  delay(500);
  for(i=0;i<50;i++)
  {
    digitalWrite(led_io[i],LOW);
  }
  delay(500);
}
