// importante funçao trim()



#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MFRC522.h>
#include <SPI.h>


#define SCREEN_WIDTH 128 // Largura do display
#define SCREEN_HEIGHT 32 // Altura do display

#define RAIO_CILINDRO 4.9
#define CONSTANTE_PI 3.142
#define ALTURA_MAX_CILINDRO 242

#define GPIO_ELETROMAGNET 5
#define GPIO_LED_ALARM 6
#define GPIO_BUTTON 3

#define SECONDS_FOR_DELETE_CARD 3

#define SS_PIN 10
#define RST_PIN 9

// Variaveis globais

#define SCREEN_ADDRESS 0x3C // Endereço do display (tente trocar por 0x3D se não funcionar)

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

int cont_button = 0;
bool flag_press = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


enum MachineStates
{
  waiting_card = 0,
  register_card,
  delete_card
};

MachineStates CurrentState;


enum StateDisplay
{
  default_screen = 0,
  access_allowed_screen,
  access_failure_screen,
  register_screen,
  delete_screen,
  action_success_screen,
  action_fail_screen,
  open_door_screen,
  close_door_screen
};

StateDisplay CurrentStateScreen;


MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Definindo as dimensões da matriz
const int tamanho = 12;

// Declarando a matriz
String meuArray [tamanho];


String valorCartao = ""; // Variável para armazenar o valor do cartão lido

int posicaoArray = 0;

bool authorized = false;

// Declaracao dos prtotipos de funcoes 

void ExecuteMachineState();
void intIOs();
void checkButton();
void screenDisplay(int state);
void configTimer1();
void waiting_card_function();
void pisca_led(int delay_time);

//-------------------------------------------

ISR(TIMER1_OVF_vect){                          //interrupção do TIMER1 
  TCNT1 = 0xC2F7;                                 // Renicia TIMER
  checkButton();
}

void setup()
{
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); 

  intIOs();

  configTimer1();

  CurrentStateScreen = default_screen;
  screenDisplay(CurrentStateScreen);

  SPI.begin();          // Inicia comunicação SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522

}



void loop(){

  ExecuteMachineState();
  }


 void intIOs(){
  
  pinMode(GPIO_BUTTON, INPUT);
  pinMode(GPIO_ELETROMAGNET, OUTPUT);
  pinMode(GPIO_LED_ALARM, OUTPUT);

  digitalWrite(GPIO_ELETROMAGNET, HIGH);
  digitalWrite(GPIO_LED_ALARM, HIGH);
  }

void ExecuteMachineState(){


  switch (CurrentState){

  case waiting_card:

  CurrentStateScreen = default_screen;
  screenDisplay(CurrentStateScreen);

  waiting_card_function();
  break;  


  case register_card:

    CurrentStateScreen = register_screen;
    screenDisplay(CurrentStateScreen);
    delay(1000);
    Serial.println("register_card");

      // Verifica se há cartões presentes
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Lê o UID do cartão RFID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    Serial.print("UID do cartão: ");
    Serial.println(uid);

    // Salva o UID do cartão na variável valorCartao
    valorCartao = uid;
    valorCartao.trim();
    Serial.print("valorCartao");
    Serial.println(valorCartao);
    // Pausa por um momento para evitar leituras repetidas

    posicaoArray++;

    if(posicaoArray <= 5){
      meuArray[posicaoArray] = valorCartao;
      CurrentStateScreen = action_success_screen;
      screenDisplay(CurrentStateScreen);
      pisca_led(250);
      delay(2000);
    }

    
 





    delay(1000);
    CurrentState = waiting_card;
  }

  break;  


  case delete_card:

    CurrentStateScreen = delete_screen;
    screenDisplay(CurrentStateScreen);
    delay(1000);
    Serial.println("delete_card");

    // Verifica se há cartões presentes
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Lê o UID do cartão RFID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    Serial.print("UID do cartão: ");
    Serial.println(uid);

    // Salva o UID do cartão na variável valorCartao
    valorCartao = uid;
    valorCartao.trim();
    Serial.print("valorCartao");
    Serial.println(valorCartao);
    // Pausa por um momento para evitar leituras repetidas



    for (int i = 0; i <= 5; i++){
    
      if(meuArray[i] == valorCartao){
      meuArray[i] = "0";
      posicaoArray--;

      CurrentStateScreen = action_success_screen;
      screenDisplay(CurrentStateScreen);
      pisca_led(250);
      }
    }


    delay(2000);
    CurrentState = waiting_card;


  break;  

  }
  }
}

void screenDisplay(int state){

  switch (state){

  case default_screen:

  display.clearDisplay();
  display.display();
  break; 


  case access_allowed_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(10, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("ACCESS OK");

  display.display();
  break;  


  case access_failure_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(10, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("NO ACCESS");

  display.display();
  break;  

  case register_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(15, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("REGISTER");

  display.display();

  break; 

  case delete_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(25, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("DELETE");

  display.display();

  break;

  case action_success_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(10, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("SUCCESS!");

  display.display();
  break;  

  case action_fail_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(10, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("FAIL!");

  display.display();
  break;  

  case open_door_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(30, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("OPEN");

  display.display();
  break;  

case close_door_screen:

  display.clearDisplay();

  display.setTextSize(1); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(23, 0);
  display.print("ACCESS CONTROL");

  display.setTextSize(2); // Tamanho
  display.setTextColor(WHITE); // Cor
  display.setCursor(30, 13); // Cursor pode ir de 0,0 a 128,32
  display.print("CLOSE");

  display.display();
  break;  


  }

}

void configTimer1(){

   // Configuração do timer1 
  TCCR1A = 0;                        //confira timer para operação normal pinos OC1A e OC1B desconectados
  TCCR1B = 0;                        //limpa registrador
  TCCR1B |= (1<<CS10)|(1 << CS12);   // configura prescaler para 1024: CS12 = 1 e CS10 = 1
 
  TCNT1 = 0xC2F7;                    // incia timer com valor para que estouro ocorra em 1 segundo
                                     // 65536-(16MHz/1024/1Hz) = 49911 = 0xC2F7
  
  TIMSK1 |= (1 << TOIE1);           // habilita a interrupção do TIMER1
}

void checkButton(){



  if(!digitalRead(GPIO_BUTTON)){
    flag_press = true;
    cont_button++;
  }

  if(digitalRead(GPIO_BUTTON) && flag_press){
    flag_press = false;
    cont_button = 0;

    CurrentState = register_card;
  }

  if(cont_button >= SECONDS_FOR_DELETE_CARD){
    flag_press = false;
    cont_button = 0;

    CurrentState = delete_card;    
  }
}

void waiting_card_function(){

 // Verifica novos cartões
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona um dos cartões
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  
  // Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);

    

     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase(); //Converte uma String para sua versão composta apenas de letras maiúsculas

  authorized = false;
  

  for(int i = 0; i <= 5; i++){

  Serial.print("Leitura da Posicaoo: ");
  Serial.println(i);

    if (conteudo.substring(1) == meuArray[i]) //UID 1 - Chaveiro
  {
    Serial.println("Chaveiro identificado!");
    Serial.println();
    digitalWrite(GPIO_ELETROMAGNET, LOW);
    CurrentStateScreen = open_door_screen;
    screenDisplay(CurrentStateScreen);
    delay(3000);
    digitalWrite(GPIO_ELETROMAGNET, HIGH); 
    authorized = true; 
  }
}

if (!authorized){
  CurrentStateScreen = close_door_screen;
  screenDisplay(CurrentStateScreen);

    pisca_led(700);
}



}

void pisca_led(int delay_time){

  digitalWrite(GPIO_LED_ALARM, LOW);
   delay(delay_time);

   digitalWrite(GPIO_LED_ALARM, HIGH);
   delay(delay_time); 
   digitalWrite(GPIO_LED_ALARM, LOW);
   delay(delay_time);

   digitalWrite(GPIO_LED_ALARM, HIGH);
   delay(delay_time); 
   digitalWrite(GPIO_LED_ALARM, LOW);
   delay(delay_time);
   digitalWrite(GPIO_LED_ALARM, HIGH);


}