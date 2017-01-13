#ifndef INIPARSER_HPP
#define INIPARSER_HPP

#include <stdlib.h>
#include <stdio.h>
#include <string>

#define INIFILE_NAME    "NefitEasy.ini"
#define NEFIT_AK        "NEFIT_ACCESS_KEY"
#define NEFIT_PW        "NEFIT_PASSWORD"
#define NEFIT_SN        "NEFIT_SERIAL_NUMBER"

using namespace std;
bool ReadIniFile();
bool ReadIniFile(string& str_access,string& str_passw,string& str_serial);
bool ReadIniString(string& key, string& result, ifstream& inputstream);
int  ReadIniInt(string& key, ifstream& inputstream);

#endif // INIPARSER_HPP
