
#ifndef HELPERS_H
#define HELPERS_H

//
// Check the Values is between 0-255
//
boolean checkRange(String Value)
{
   if (Value.toInt() < 0 || Value.toInt() > 255)
   {
     return false;
   }
   else
   {
     return true;
   }
}

  
char* GetMacAddress()
{
  uint8_t mac[6];
    char macStr[18] = {0};
  WiFi.macAddress(mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],  mac[1], mac[2], mac[3], mac[4], mac[5]);
    return  macStr;
}

// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
byte h2int(char c)
{
   if(c=='0') return 0;
   if(c=='1') return 1;
   if(c=='2') return 2;
   if(c=='3') return 3;
   if(c=='4') return 4;
   if(c=='5') return 5;
   if(c=='6') return 6;
   if(c=='7') return 7;
   if(c=='8') return 8;
   if(c=='9') return 9;
   if(c=='a') return 10;
   if(c=='b') return 11;
   if(c=='c') return 12;
   if(c=='d') return 13;
   if(c=='e') return 14;
   if(c=='f') return 15;
    return(0);
}

char c2h(char c)
{  return "0123456789ABCDEF"[0x0F & (unsigned char)c];
}

int StringToInt(String txt){
  int buf;
  buf=txt.toInt();
  return buf;
}

const char* StringToChar(String txt){
 char out[txt.length()+1];
 txt.toCharArray(out, txt.length()+1);
 //Serial.println(out);
 //Serial.println(txt.length());
 return out; 
}

char* StringToCharH(String txt){
 char out[txt.length()+1];
 txt.toCharArray(out, txt.length()+1);
 //Serial.println(out);
 //Serial.println(txt.length());
 return out; 
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

String clearString(String inp){
  String out=inp;
  //for (int i = 0; i < maxBytes; i++) {
  //Serial.println(inp);
  //Serial.println(inp.length());
  return out;
}

float StringToFloat(String chislo){
  char simw;
  String tmp="";
  float tmp_f;
  float out;
  byte i;
  byte str_l;
  
  //Serial.println(chislo);
  str_l=chislo.length();
//разбиваем число на целое и дробные части
  i=0;
  simw=chislo[i]; 
  while ((simw!='.')&&(simw!=',')&&(i<str_l)){
    i++;
    simw=chislo[i];   
  }
  if(str_l==i){
    out=tmp.toInt();
    return out;
  }
  
//просчет целой части
  chislo=chislo+"00";
  tmp=chislo.substring(0,i);
  out=tmp.toInt();

//ghjcxtn lhj,yjq xfcnb
  tmp=chislo.substring(i+1,i+3);
  tmp_f=tmp.toInt();
  tmp_f=tmp_f/100;

//объединяем результат
  out=out+tmp_f;
  
  return out;
}

#endif
