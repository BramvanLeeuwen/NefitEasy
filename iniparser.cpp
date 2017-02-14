
#include <iniparser.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include<iostream>
#include<fstream>


using namespace std;
extern int      tcp_ip_port;

bool ReadIniFile()
{   string str_access,str_passw,str_serial;
    if (!ReadIniFile(str_access,str_passw,str_serial))
        return false;
    setenv(NEFIT_AK,str_access.c_str(),1);
    setenv(NEFIT_PW,str_passw.c_str(),1);
    setenv(NEFIT_SN,str_serial.c_str(),1);
    return true;
}

bool ReadIniFile(string& str_access,string& str_passw,string& str_serial)
{	char buffer[1024];
    snprintf(buffer, sizeof(buffer),INIFILE_NAME );
    ifstream inputstream(buffer);
    if (inputstream.is_open())
    {  ifstream inputstream(buffer);
        string key;
        ReadIniString(key=NEFIT_AK,str_access, inputstream) ;
        ReadIniString(key=NEFIT_PW,str_passw, inputstream) ;
        ReadIniString(key=NEFIT_SN,str_serial, inputstream) ;
        tcp_ip_port =ReadIniInt(key=NEFIT_LP,inputstream);
        return true;
    }
    else
    {    cout << "Ini file not found!" << endl;
        return false;
    }
}


bool ReadIniString(string& key, string& result, ifstream& inputstream)
{     size_t pos;
      string::size_type l,eq_pos;
      string line;
      pos=0;
      while(inputstream.good() && pos!=string::npos)
      { pos=0;
        getline(inputstream,line); // get line from file
        if (line.length()){
            line.erase(line.find_first_of(";"));   //comment, begins with semicolon

            pos=line.find(key); // search
            eq_pos=line.find_first_of('=');
            if ((pos != string::npos ) && (eq_pos!=string::npos) ) // string::npos is returned if string is not found
            {	l=line.find_first_of('"');
                if(l==string::npos)  //quotation not found
                    line=line.substr(eq_pos+1,string::npos);
                else
                { line=line.substr((l+1),string::npos);

                line.erase(line.find_first_of('"'));   //skip on next quotation
                }
                result=line.erase( line.find_last_not_of(' ')+1);  //skip trailing spaces

    //        cout<<"line:"<<line<<"**result:**"<<result<<"**\"" <<endl;
            return true;
            }
          }
      }
      cout << "Key\" "<< key<< "\" not found not in ini file."<< endl;
      return false;
}

int  ReadIniInt(string& key, ifstream& inputstream)
{   string result;
    ReadIniString(key, result, inputstream);
    return atoi(result.c_str());
}

