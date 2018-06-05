#include <string>
#include "event.h"

namespace cll {

class EventUploadSource {

public:
    virtual Event* top() = 0;
    virtual void pop() = 0;

};

}