#include <arduino.h>

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

#define MAX_VIEW 2


// Define view class
class VIEW {
  public:
    VIEW();
    VIEW(byte newId, String newName, String newValue);
    void setView(byte newId, String newName, String newValue);
    void dumpViewToSerial();

  private:
    byte id;
    String name;
    String value;
};


class VIEWS {
  public:
    VIEWS();
    // (VIEW *) getView(byte viewId);
    void setView(byte newId, String newName, String newValue);
    byte getViewCount();
    void dumpToSerial();
    void dumpToMQTT(char *topic);

  private:
    // Declare array of views
    VIEW views[MAX_VIEW];

};
