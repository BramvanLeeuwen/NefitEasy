#ifndef JSONPARSER_HPP
#define JSONPARSER_HPP

#include <stdio.h>
#include <string.h>
#include <sys/queue.h>
#include <json-c/json.h>

#define NEFIT_OUTDOOR_TEMPERATURE 0
#define NEFIT_USR_INTERFACESTATUS 1
#define NEFIT_SYSTEM_PRESSURE     2

void value_obtained(struct nefit_easy *easy, json_object *obj);
void print_json_value(json_object *jobj);
void json_parse_array( json_object *jobj, char *key) ;
void json_parse(json_object * jobj);

#endif // JSONPARSER_HPP
