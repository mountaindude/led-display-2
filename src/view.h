#include <arduino.h>
#include <PubSubClient.h>

// #define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define VIEW_COUNT 5
#define NAME_LENGTH 16
#define VALUE_LENGTH 16



// Define view class
class VIEW {
  public:
    VIEW();
    VIEW(byte newId, const char *newName, const char *newValue);
    void setView(byte newId, const char *newName, const char *newValue);
    void getIdNameValue(byte *id, char *name, char *value);

  private:
    byte id;
    char name[NAME_LENGTH];    // Names max 15 chars long
    char value[VALUE_LENGTH];   // Values max 15 chars long
};


class VIEWS {
  public:
    VIEWS();
    // (VIEW *) getView(byte viewId);
    void setView(byte newId, const char *newName, const char *newValue);
    // void getView(byte id, String &name, String &value);
    // void getView(byte id, VIEW *viewPtr);
    void getView(byte id, VIEW *viewPtr);
    byte getViewCount();

    byte getViewId(byte id);
    void getViewName(byte id, char *name);
    void getViewValue(byte id, char *value);
    void dumpToSerial();
    void dumpToMQTT(PubSubClient *client, const char *topic);


  private:
    // Declare array of views
    VIEW views[VIEW_COUNT];

};
