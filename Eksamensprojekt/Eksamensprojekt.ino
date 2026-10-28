//Inkluderer dhtnew.h biblioteket, som bruges til afmålning af Temperatur og Fugtighed.
#include <dhtnew.h>

//Inkluderer Wire.h biblioteket, som bruges til kommunikation med RTC.
#include <Wire.h>

//Inkluderer RTClib.h, som bruges til at hente og danne Timestamps. 
#include <RTClib.h>

//Inkluderer SD.h & SPI.h biblioteket for at SD-Kortet og Arduinoen kan snakke sammen. 


#include <SD.h>
#include <SPI.h>

//Definerer chipSelect (CS) til at være pin 53. 
const int chipSelect = 53; 

//Definerer at man kommer til at arbejde med en fil, og deklarer den specifikke myFile.
File myFile;

//Definerer Photoresistorens (LDR) pin;
const int lightPin = A0;

//Tildeler digitale pins til de to DHT Temperatur- og Fugtighedssensorer.
DHTNEW dht_INDE(2);
DHTNEW dht_UDE(3);

//Opretter et RTC-objekt til DS3231, så Arduinoen kan aflæse dato & tid.
RTC_DS3231 rtc;

//Variabler.
unsigned long lastLog = 0; //Variablen gemmer tidspunktet for sidste loging (ms). 
int readCount = 0; //Variablen tæller antal aflæsninger. 
unsigned long interval = 600000UL; //Aflæser Temperaturen & Fugtigheden hver 10min.

void setup(){
  Serial.begin(9600);

  lastLog = millis() - interval; //Gør at den ikke kører før intervallet er færdigt.

  pinMode(chipSelect, OUTPUT); //Sætter chipSelect til at være output. 

//Start delay på 5sek for at give Arduinoen og Serial Monitor tid til at starte op.
  delay(2000); 

//Definerer Photoresistorens pin som Input.
  pinMode(lightPin, INPUT);

//Tjekker om SD-Kortet er tilsluttet korrekt. 
//Stopper programmet i et uendeligt loop, så programmet ikke kører uden SD-Kortet for ikke at midste data.

  if(!SD.begin(chipSelect)){
    Serial.println("Fejl med SD-Kortet!");
    myFile.close();
    while(1); 
  }

//Tjekker om RTCen (DS3231) er tilsluttet korrekt.
//Stopper programmet i et loop, så programmet ikke kører uden RTCen.  //Logger at der er fejl med RTCen i filen DALO.csv.

  if(!rtc.begin()){
    Serial.println("Fejl med RTCen!");

    myFile = SD.open("DALO.csv", FILE_WRITE);
    if(myFile){
    myFile.println("Fejl med RTCen!");
    myFile.close();
    Serial.println("Fejlen er logget i DALO.csv");
    }else{
      Serial.println("Kunne ikke logge RTC-Fejlen i DALO.csv!");
    }
  }
//Sætter RTCen til den aktuelle tid ved opstart af programmet.

  rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop(){
//Styrer intervallet mellem aflæsninger.   
  if(millis() - lastLog >= interval){
    lastLog = millis(); // Opdaterer tidspunktet for seneste aflæsning.
    readCount ++; //Lægger 1 til variablen hver gang, der bliver aflæst. 

    DateTime now;

  //Opsætter en variabel, der tjekker om rtcen fejler imens programmet kører.
    bool rtcFejl = false;

  //Tjekker om rtcen fejler.
    if(!rtc.begin()){
      rtcFejl = true;
    }else{
      now = rtc.now();
    }
    
  //Printer Headeren i starten af aflæsningen, derefter hver 6ede aflæsning, altså hver time.
    if (readCount == 1 || (readCount - 1) % 6 == 0) {
      Serial.println("| Dato |  Tid | Temp_INDE |Fugt_INDE| Temp_UDE  | Fugt_UDE  | LYS_INTENSITET  |"); //Headeren.
   
   //Tjekker om filen er åben, hvis det er tilfældet vil den skrive headeren.
    myFile = SD.open("DALO.csv", FILE_WRITE); //Laver eller tilgår filen "DALO" og skriver i den.
      if(myFile){ 
        myFile.print("Dato"); myFile.print(";");
        myFile.print("Tid"); myFile.print(";");
        myFile.print("Temp_INDE"); myFile.print(";");
        myFile.print("Fugt_INDE"); myFile.print(";");
        myFile.print("Temp_UDE"); myFile.print(";");
        myFile.print("Fugt_UDE"); myFile.print(";");
        myFile.println("Lys_INTENSITET");
        myFile.close(); //Lukker filen. 
      }
    }

  //Aflæser Temperatur og Fugtighed fra både Inde- og Udesensorerne.
    dht_INDE.read();
    dht_UDE.read();

  //Gemmer de aflæste Temperatur- og Fugtighedsværdier fra Inde- og Udesensorerne, som float for at få decimaler på.
    float Temp_INDE = dht_INDE.getTemperature();
    float Fugt_INDE = dht_INDE.getHumidity();
    float Temp_UDE = dht_UDE.getTemperature();
    float Fugt_UDE = dht_UDE.getHumidity();

  //Omskriver den aflæste Temperatur- og Fugtighedsværdier til heltal. 
    int TEMP_INDE_RESULTAT = (Temp_INDE * 10);
    int FUGT_INDE_RESULTAT = (Fugt_INDE * 10);
    int TEMP_UDE_RESULTAT = (Temp_UDE * 10);
    int FUGT_UDE_RESULTAT = (Fugt_UDE * 10);


  //Aflæser Photoresistors (LDR) værdier. 
    int LDR_Value = analogRead(lightPin);

  //Hvis RTCen fejler vil den vise "0000:00:00", så den indikerer at rtcen fejler men stadig kører.
  //Hvis RTCen ikke fejler skal hente tiden fra RTC, så man får et rigtigt timestamp.
    String date = rtcFejl ? "0000:00:00": now.timestamp(DateTime::TIMESTAMP_DATE);
    String time = rtcFejl ? "00:00:00" : now.timestamp(DateTime::TIMESTAMP_TIME);

//Tjekker om den aflæste værdi er lig med -999, altså hvis aflæsningen fejler.
//Hvis det er tilfældet at den fejler vil den ikke printe noget. 
  if(Temp_INDE == -999 || Fugt_INDE == -999 || Temp_UDE == -999 || Fugt_UDE == -999){    

     }
      else{ //elsers skal den køre som normalt. 
   //Printer den aflæste data i Serial Monitoren i et txt-format.
    Serial.print(date); Serial.print(","); //Skriver Datoen.
    Serial.print(time); Serial.print(","); //Skriver Tiden.
    Serial.print(TEMP_INDE_RESULTAT); Serial.print("C,"); //Skriver den aflæste Temperatur fra IndeSensoren, samt viser værdien i grader (C).
    Serial.print(FUGT_INDE_RESULTAT); Serial.print("%,"); //Skriver den aflæste Fugtighed fra IndeSensoren, samt viser værdien i Procent (%).
    Serial.print(TEMP_UDE_RESULTAT); Serial.print("C,");  //Skriver den aflæste Temperatur fra UdeSensoren, samt viser værdien i grader (C).
    Serial.print(FUGT_UDE_RESULTAT) ; Serial.print("%,"); //Skriver den aflæste Fugtighed fra UdeSensoren, samt viser værdien i procent (%).
    Serial.println(LDR_Value); //Skriver den aflæste LysIntensitet.

   //Skriver den aflæste data i file "DALO.csv".
    myFile = SD.open("DALO.csv",FILE_WRITE);
    if(myFile){
      myFile.print(date); myFile.print(";");
      myFile.print(time); myFile.print(";");
      myFile.print(TEMP_INDE_RESULTAT); myFile.print("C;");
      myFile.print(FUGT_INDE_RESULTAT); myFile.print("%;");
      myFile.print(TEMP_UDE_RESULTAT); myFile.print("C;");
      myFile.print(FUGT_UDE_RESULTAT); myFile.print("%;");
      myFile.println(LDR_Value); 
      myFile.close();
      }
    }
  }
}




