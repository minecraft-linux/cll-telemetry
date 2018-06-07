#include "event_serializer_extensions.h"

using namespace cll;

OsInfoExtension::OsInfoExtension() {
    data["ver"] = "1.0";
    data["locale"] = getLocale();
}

std::string OsInfoExtension::getLocale() const {
    std::string cppName = std::locale("").name(); // example: en_US.utf8
    if (cppName.length() < 5)
        return std::string();
    cppName = cppName.substr(0, 5); // cut the suffix
    if (cppName[2] == '_')
        cppName = '-';
    if (cppName[2] != '-') // check for invalid locale
        return std::string();
    return cppName;
}