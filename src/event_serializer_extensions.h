#pragma once

#include <cll/event_serializer.h>

namespace cll {

class UserInfoExtension : public EventSerializer::Extension {

private:
    nlohmann::json data;

public:
    UserInfoExtension() {
        data["ver"] = "1.0";
    }

    void setLocalId(std::string const& localId) { data["localId"] = localId; }
    void clearLocalId() { data.erase("localId"); }

    std::string getName() const override { return "user"; }
    nlohmann::json build(Event const&) override { return data; }
};

class OsInfoExtension : public EventSerializer::Extension {

private:
    nlohmann::json data;

    std::string getLocale() const;

public:
    OsInfoExtension();

    std::string getName() const override { return "os"; }
    nlohmann::json build(Event const&) override { return data; }
};

class DeviceInfoExtension : public EventSerializer::Extension {

private:
    nlohmann::json data;

public:
    DeviceInfoExtension() {
        data["ver"] = "1.0";
    }

    void setLocalId(std::string const& localId) { data["localId"] = localId; }
    void clearLocalId() { data.erase("localId"); }

    std::string getName() const override { return "device"; }
    nlohmann::json build(Event const&) override { return data; }
};

class AndroidExtension : public EventSerializer::Extension {

public:
    std::string getName() const override { return "android"; }
    nlohmann::json build(Event const& ev) override;
};

}