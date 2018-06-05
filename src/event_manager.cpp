#include <cll/event_manager.h>

using namespace cll;

EventManager::EventManager(std::string const& iKey) : iKey(iKey) {
    config.addDefaultConfigurations(iKey);
}