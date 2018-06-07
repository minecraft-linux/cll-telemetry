#pragma once

#include <cll/event_serializer.h>

namespace cll {

class OsInfoExtension : public EventSerializer::Extension {

private:
    nlohmann::json data;

    std::string getLocale() const;

public:
    OsInfoExtension();

    std::string getName() const override { return "os"; }
    nlohmann::json build(Event const&) override { return data; }
};

}