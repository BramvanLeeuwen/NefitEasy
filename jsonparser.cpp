#include "easyheader.hpp"
#include <jsonparser.hpp>
#include <string>

//#define LOGPARSER

using namespace  std;
extern char *paths[];

string unescape(const string& s);

void value_obtained(struct nefit_easy *easy, json_object *obj)
//in easy connect defined as the function where the nefit easy server calls back on my request
{   char *str;
    string strg;
    struct json_object *subobj;

    /* Allocate room for the string. */
    str = (char *)malloc(sizeof(char) * (strlen(json_object_to_json_string(obj)) + 1));
    /* Copy the contents of the string. */
    strcpy(str, json_object_to_json_string(obj));
    //putenv((char*)"NEFIT_ROOMTEMP='21.0'");



    EASY_UNUSED(easy);
    json_object * jobj = json_tokener_parse(str);
    json_parse(jobj);
    subobj = json_object_object_get(jobj, "id");
    json_object_get_string(subobj);
    strg=unescape(json_object_to_json_string(subobj));
    //strg=strg.substr(1,strg.length()-1);
     //printf("%s\n", strg.c_str());
     // printf("%s\n", paths[1]);
   if (strg==paths[NEFIT_USR_INTERFACESTATUS])
    {   subobj = json_object_object_get(json_object_object_get(jobj, "value"), "IHT");
        printf("roomtemp: %s deg C\n", json_object_get_string(subobj));
    }
    if (strg==paths[NEFIT_SYSTEM_PRESSURE])
    {   subobj = json_object_object_get(jobj, "value");
        printf("pressure: %s bar\n", json_object_get_string(subobj));
    }
    if (strg==paths[NEFIT_OUTDOOR_TEMPERATURE])
    {   subobj = json_object_object_get(jobj, "value");
        printf("outdoor temperature: %s degC\n", json_object_get_string(subobj));
    }

}


//printing the value corresponding to boolean, double, integer and strings

void print_json_value(json_object *jobj){
  enum json_type type;
  type = json_object_get_type(jobj); //Getting the type of the json object
  switch (type) {
    case json_type_boolean: printf("json_type_boolean");
                         printf("value: %s\n", json_object_get_boolean(jobj)? "true": "false");
                         break;
    case json_type_double: printf("json_type_double");
                        printf("          value: %lf\n", json_object_get_double(jobj));
                         break;
    case json_type_int: printf("json_type_int");
                        printf("          value: %d\n", json_object_get_int(jobj));
                         break;
    case json_type_string: printf("json_type_string");
                         printf("          value: %s\n", json_object_get_string(jobj));
                         break;
    default: break;
  }

}

void json_parse_array( json_object *jobj, char *key) {
  void json_parse(json_object * jobj); //Forward Declaration
  enum json_type type;

  json_object *jarray = jobj; //Simply get the array
  if(key) {
    jarray = json_object_object_get(jobj, key); //Getting the array if it is a key value pair
    //printf("; key_array= %s  ***",key);
  }

  int arraylen = json_object_array_length(jarray); //Getting the length of the array
  printf("Array Length: %d\n",arraylen);
  int i;
  json_object * jvalue;

  for (i=0; i< arraylen; i++){
    jvalue = json_object_array_get_idx(jarray, i); //Getting the array element at position i
    type = json_object_get_type(jvalue);
    if (type == json_type_array) {
      json_parse_array(jvalue, NULL);
    }
    else if (type != json_type_object) {
      //printf("# %i  ",i);
      //print_json_value(jvalue);
    }
    else {
      //printf("#jval %i  ",i);
      json_parse(jvalue);
    }
  }
}


//Parsing the json object

void json_parse(json_object * jobj) {
  enum json_type type;
  json_object_object_foreach(jobj, key, val) { //Passing through every array element
    type = json_object_get_type(val);
    switch (type) {
      case json_type_boolean:
      case json_type_double:
      case json_type_int:
      case json_type_string: //printf("key= \"%10s\"\t",key);

                             //   print_json_value(val);
                           break;
      case json_type_object: //printf("json_type_object");
                           jobj = json_object_object_get(jobj, key);
                           //printf("; objectkey= \"%s\"\n",key);
                           json_parse(jobj);
                           break;
      case json_type_array: //printf("type: json_type_array");
                            //printf("; arraykey= %s  ",key);
                            json_parse_array(jobj, key);
                          break;
      default: break;
    }
  }
}

string unescape(const string& s)
{ string res;
  string::const_iterator it = s.begin()+2;
  while (it != s.end())
  { char c = *it++;
    if (c != '\\' && it != s.end())
    res += c;
  }
  return res;
}


