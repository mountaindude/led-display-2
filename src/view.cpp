#include <view.h>
#include "debug.h"
#include <arduino.h>

VIEW::VIEW ()
{
  // Initialize
  id = -1;
  name = "";
  value = "";
}


VIEW::VIEW (byte newId, String newName, String newValue)
{
  // Initialize
  setView(newId, newName, newValue);
}


void VIEW::setView(byte newId, String newName, String newValue)
{
  #ifdef DEBUG
    // Print values
    Serial.println("Updating view:");
    Serial.println("   ID=" + (String) newId + ", Name=" + newName + ", Value=" + newValue);
  #endif


  this->id = newId;
  this->name = newName;
  this->value = newValue;
}


void VIEW::dumpViewToSerial()
{
  Serial.println("View contents: ID=" + (String) id + ", Name=" + name + ", Value=" + value);
}



// ------------------
VIEWS::VIEWS()
{
  for(byte i=0; i<=MAX_VIEW; i++) {
    views[i].setView(i, "", "");
  }
}

// (VIEW *) VIEWS::getView(byte viewId)
// {
//   return &(views[viewId]);
// }

void VIEWS::setView(byte newId, String newName, String newValue)
{
  views[newId].setView(newId, newName, newValue);
}

byte VIEWS::getViewCount()
{
  #ifdef DEBUG
    Serial.println("# of views=" + NELEMS(views));
  #endif

  return NELEMS(views);
}

void VIEWS::dumpToSerial()
{
  for (byte i=0; i<NELEMS(views); i++) {

  }
}

void VIEWS::dumpToMQTT(char *topic)
{

}
