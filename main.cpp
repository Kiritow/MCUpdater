#include "NetworkWrapper.h"
#include <iostream>
using namespace std;

const string UPDATE_SITE = "kiritow.com";

int main()
{
    HTTPConnection h;
    h.setVerbos(true);
    h.setOutputFile("mcup_latest.json");
    h.setURL(UPDATE_SITE+"/mc/latest.json");
    h.perform();

    return 0;
}
