#include <cll/event_serializer.h>
#include <cll/event.h>
#include <gtest/gtest.h>

using namespace cll;

TEST(EventSerializerTest, BasicTest) {
    EventSerializer serializer;
    serializer.setIKey("test-ikey");
    serializer.setApp("my-app", "1.0.0");
    serializer.setEpoch(12345678L);

    Event testEvent ("Event Name", {{"some", "data"}});

    nlohmann::json json = serializer.createEnvelopeFor(testEvent);
    printf("%s\n", json.dump().c_str());
}
