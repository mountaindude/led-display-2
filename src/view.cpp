#include <view.h>
#include "debug.h"

VIEW::VIEW ()
{
  // Initialize
  id = -1;
  strcpy(name, "--------");
  strcpy(value, "");
}

VIEW::VIEW (byte newId, const char *newName, const char *newValue)
{
  // Initialize
  setView(newId, newName, newValue);
}


void VIEW::setView(byte newId, const char *newName, const char *newValue)
{
  #ifdef DEBUG
    Serial.println("Updating view, new data:");
    Serial.print("   ID="); Serial.print(newId);
    Serial.print(", Name="); Serial.print(newName);
    Serial.print(", Value="); Serial.println(newValue);
  #endif

  id = newId;
  strncpy(name, newName, NAME_LENGTH);
  strncpy(value, newValue, VALUE_LENGTH);
}

void VIEW::getIdNameValue(byte *id, char *newName, char *newValue)
{
  *id = this->id;
  strcpy(newName, (const char*) name);
  strcpy(newValue, (const char*) value);
}

void VIEW::getName(char *name1)
{
  strcpy(name1, (const char *) name);
}

void VIEW::getValue(char *value1)
{
  strcpy(value1, (const char *) value);
}


// ------------------
//
// ------------------
VIEWS::VIEWS()
{
  // for(byte i=0; i<=VIEW_COUNT; i++) {
  //   views[i].setView(i, "", "");
  // }
}

void VIEWS::setView(byte newId, const char *newName, const char *newValue)
{
  views[newId].setView(newId, newName, newValue);
}



void VIEWS::getView(byte id, VIEW *viewPtr)
{
  memcpy(viewPtr, &(views[id]), sizeof(VIEW));
}


void VIEWS::getViewName(byte id, char *name)
{
  views[id].getName(name);
}


void VIEWS::getViewValue(byte id, char *value)
{
  views[id].getValue(value);
}



byte VIEWS::getViewCount()
{
  return VIEW_COUNT;
}

void VIEWS::dumpToSerial()
{
  byte id;
  char name[NAME_LENGTH];
  char value[VALUE_LENGTH];
  VIEW tmpView;

  for (int i=0; i < this->getViewCount(); i++ ) {
    this->getView(i, &tmpView);
    tmpView.getIdNameValue(&id, name, value);

    Serial.print("View id="); Serial.print(id);
    Serial.print(", name="); Serial.print(name);
    Serial.print(", value="); Serial.println(value);
  }
}

void VIEWS::dumpToMQTT(PubSubClient *client, const char *topic)
{
  byte id;
  char idStr[5];
  char name[NAME_LENGTH];
  char value[VALUE_LENGTH];
  VIEW tmpView;

  for (int i=0; i < this->getViewCount(); i++ ) {
    this->getView(i, &tmpView);
    tmpView.getIdNameValue(&id, name, value);

    // Build JSON response
    // There are 25 "structural" characters in desired JSON format:
    // {id:"<id>", name:"<name>", value:"123"}
    // Add 10 extra characters to be safe...

    itoa(id, idStr, 10);

    char json[25 + 5 + 16 + 16 + 10] = "{id:\"";
    strcat(json, idStr);
    strcat(json, "\", name:\"");
    strcat(json, name);
    strcat(json, "\", value:\"");
    strcat(json, value);
    strcat(json, "\"}");


    client->publish(topic, json);
  }
}
