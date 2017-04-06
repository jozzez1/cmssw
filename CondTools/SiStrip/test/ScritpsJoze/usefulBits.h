#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <iostream>

class CStringTokenizer {
    public:
        CStringTokenizer (const char* tokens_){
            tokens = string(tokens_);
        }

        std::vector<std::string> tokenize (const char* cstring);
    private:
        std::string tokens;
};

std::vector<std::string>
CStringTokenizer::tokenize (const char* cstring)
{
    std::vector<std::string> result;
    std::string part = "";
    for (unsigned int i=0; cstring[i] != '\n'; i++){
        bool match = false;
        for (auto token : tokens){
            if (cstring[i] == token) match = true;
        }

        if (!match)
            part += cstring[i];
        if (match){
            result.push_back (part);
            part = "";
        }
    }

    return result;
}

class HexToDecimal {
    public:
        HexToDecimal (size_t _len) : len (_len) {};

        size_t getLen () { return len; }

    private:
        size_t len;

        uint32_t hexCharToInt (char c){
            if      (c >= 48 && c <=  57) return static_cast<uint32_t> (c - 48);
            else if (c >= 97 && c <= 102) return static_cast<uint32_t> (c - 87);
            else if (c >= 65 && c <=  70) return static_cast<uint32_t> (c - 55);
            else return 16;
        }

    public:
        uint32_t hex2dec (const char* hex, size_t Length=0, std::vector<uint32_t>* digits=NULL) {
            uint32_t toReturn = 0;
            if (Length==0) Length = len;
            for (int i=Length; --i && hex[i] != 'x';){
                uint32_t hexDigit = hexCharToInt(hex[i]);
                toReturn += hexDigit * pow (16, Length-i-1);

                if (digits) digits->push_back(hexDigit);

                if (hexDigit > 15){
                    std::cerr << "Warning! Not in hex format!" << std::endl;
                    break;
                }
            }
            return toReturn;
        }
};

char* getProgressBar (double percentage, unsigned int barLength)
{
   char* bar = new char [barLength+8];
   sprintf (bar, "[");
   unsigned int i = 0;
   for (; i+1 < (unsigned int) (percentage*barLength); i++)
      sprintf (bar, "%s=", bar);
   sprintf (bar, "%s%c", bar, percentage<0.99?'>':'=');
   for (; i < barLength-1; i++)
      sprintf (bar, "%s ", bar);
   sprintf (bar, "%s] %3u %% ", bar, (unsigned int) (100*percentage));
   return bar;
}

void printProgressBar (const char* bar, const char* prefix)
{
   if (prefix)
      fprintf (stdout, "\r%s%s", prefix, bar);
   else
      fprintf (stdout, "\r%s", bar);
   fflush (stdout);
}

