#include "Arduino.h"
#include "mmwave_for_xiao.h"

#include <cstdio>
#include <cstring>
#include <vector>

static unsigned long g_millis = 0;
static int g_failures = 0;
static int g_passed = 0;

unsigned long millis()
{
    g_millis += 50;
    return g_millis;
}

void delay(unsigned long)
{
}

HardwareSerial Serial;

static void resetClock()
{
    g_millis = 0;
}

#define CHECK(cond, name)                                                       \
    do                                                                          \
    {                                                                           \
        if (cond)                                                               \
        {                                                                       \
            g_passed++;                                                         \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            g_failures++;                                                       \
            std::printf("FAIL: %s (%s:%d)\n", name, __FILE__, __LINE__);        \
        }                                                                       \
    } while (0)

class MockStream : public Stream
{
public:
    std::vector<uint8_t> rx;
    std::vector<std::vector<uint8_t> > replies;
    size_t replyIndex;
    size_t rxPos;

    MockStream() : replyIndex(0), rxPos(0) {}

    void queueReply(const std::vector<uint8_t> &frame)
    {
        replies.push_back(frame);
    }

    int available() override
    {
        return (int)(rx.size() - rxPos);
    }

    int read() override
    {
        if (rxPos >= rx.size())
        {
            return -1;
        }
        return rx[rxPos++];
    }

    size_t write(uint8_t) override
    {
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        (void)buffer;
        if (replyIndex < replies.size())
        {
            rx.insert(rx.end(), replies[replyIndex].begin(), replies[replyIndex].end());
            replyIndex++;
        }
        return size;
    }
};

static std::vector<uint8_t> makeFrame(const uint8_t *payload, size_t payloadLen)
{
    std::vector<uint8_t> frame;
    const uint8_t header[] = {0xFD, 0xFC, 0xFB, 0xFA};
    const uint8_t tail[] = {0x04, 0x03, 0x02, 0x01};
    frame.insert(frame.end(), header, header + 4);
    frame.insert(frame.end(), payload, payload + payloadLen);
    frame.insert(frame.end(), tail, tail + 4);
    return frame;
}

static std::vector<uint8_t> successAckFrame()
{
    const uint8_t payload[] = {0x08, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x01, 0x00, 0x40, 0x00};
    return makeFrame(payload, sizeof(payload));
}

static std::vector<uint8_t> failedAckFrame()
{
    const uint8_t payload[] = {0x08, 0x00, 0xFF, 0x01, 0x01, 0x00, 0x01, 0x00, 0x40, 0x00};
    return makeFrame(payload, sizeof(payload));
}

static std::vector<uint8_t> headerOnlyFrame()
{
    const uint8_t header[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x08, 0x00, 0xFF, 0x01};
    return std::vector<uint8_t>(header, header + sizeof(header));
}

static std::vector<uint8_t> shortCompleteFrame()
{
    const uint8_t frame[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x03, 0x02, 0x01};
    return std::vector<uint8_t>(frame, frame + sizeof(frame));
}

static std::vector<uint8_t> versionFrame()
{
    const uint8_t payload[] = {
        0x0C, 0x00,
        0xA0, 0x01,
        0x00, 0x00,
        0x00,
        0x01,
        0x02,
        0x00, 0x00, 0x00, 0x03};
    return makeFrame(payload, sizeof(payload));
}

static std::vector<uint8_t> configFrame()
{
    std::vector<uint8_t> payload(30, 0);
    payload[0] = 0x1C;
    payload[1] = 0x00;
    payload[2] = 0x61;
    payload[3] = 0x01;
    payload[4] = 0x00;
    payload[5] = 0x00;
    payload[7] = 0x08;
    payload[8] = 0x07;
    payload[9] = 0x06;
    payload[10] = 0x11;
    payload[28] = 0x2A;
    payload[29] = 0x00;
    return makeFrame(payload.data(), payload.size());
}

static std::vector<uint8_t> resolutionFrame()
{
    const uint8_t payload[] = {0x04, 0x00, 0xAB, 0x01, 0x00, 0x00, 0x01, 0x00};
    return makeFrame(payload, sizeof(payload));
}

static void testTimeoutReturnsErrorWithoutCrash()
{
    resetClock();
    MockStream stream;
    Seeed_HSP24 radar(stream);

    CHECK(radar.enableConfigMode() == Seeed_HSP24::AskStatus::Error, "enableConfigMode timeout");
    CHECK(radar.disableConfigMode() == Seeed_HSP24::AskStatus::Error, "disableConfigMode timeout");
    CHECK(radar.getVersion() == String("Into AT Mode Error!"), "getVersion timeout before config mode");
    CHECK(radar.setDetectionDistance(3, 5) == Seeed_HSP24::AskStatus::Error, "setDetectionDistance timeout");
    CHECK(radar.setGatePower(3, 40, 40) == Seeed_HSP24::AskStatus::Error, "setGatePower timeout");
    CHECK(radar.getConfig().detectionDistance == -1, "getConfig timeout keeps defaults");
    CHECK(radar.setResolution(1) == Seeed_HSP24::AskStatus::Error, "setResolution timeout");
    CHECK(radar.getResolution().resolution == -1, "getResolution timeout keeps defaults");
    CHECK(radar.rebootRadar() == Seeed_HSP24::AskStatus::Error, "rebootRadar timeout");
    CHECK(radar.refactoryRadar() == Seeed_HSP24::AskStatus::Error, "refactoryRadar timeout");
    CHECK(radar.enableEngineeringModel() == Seeed_HSP24::AskStatus::Error, "enableEngineeringModel timeout");
    CHECK(radar.disableEngineeringModel() == Seeed_HSP24::AskStatus::Error, "disableEngineeringModel timeout");
}

static void testSendCommandTimeoutLeavesBufferUnreadable()
{
    resetClock();
    MockStream stream;
    Seeed_HSP24 radar(stream);
    const byte command[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};

    Seeed_HSP24::DataResult result = radar.sendCommand(command, sizeof(command));
    CHECK(result.length < 0, "timeout length is negative");
    CHECK(result.resultBuffer == nullptr, "timeout resultBuffer is null");
}

static void testIncompleteFrameIsSafe()
{
    resetClock();
    MockStream stream;
    stream.queueReply(headerOnlyFrame());
    Seeed_HSP24 radar(stream);
    CHECK(radar.enableConfigMode() == Seeed_HSP24::AskStatus::Error, "incomplete frame is Error");
}

static void testShortCompleteFrameDoesNotReadPastEnd()
{
    resetClock();
    MockStream stream;
    stream.queueReply(shortCompleteFrame());
    Seeed_HSP24 radar(stream);
    CHECK(radar.enableConfigMode() == Seeed_HSP24::AskStatus::Error, "8-byte frame cannot supply index 8");
}

static void testSuccessAndFailedAck()
{
    resetClock();
    MockStream okStream;
    okStream.queueReply(successAckFrame());
    Seeed_HSP24 okRadar(okStream);
    CHECK(okRadar.enableConfigMode() == Seeed_HSP24::AskStatus::Success, "valid ACK is Success");

    resetClock();
    MockStream failStream;
    failStream.queueReply(failedAckFrame());
    Seeed_HSP24 failRadar(failStream);
    CHECK(failRadar.enableConfigMode() == Seeed_HSP24::AskStatus::Error, "ACK status 1 is Error");
}

static void testResultBufferSurvivesReturn()
{
    resetClock();
    MockStream stream;
    stream.queueReply(successAckFrame());
    Seeed_HSP24 radar(stream);
    const byte command[] = {0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xFF, 0x00, 0x01, 0x00, 0x04, 0x03, 0x02, 0x01};

    Seeed_HSP24::DataResult result = radar.sendCommand(command, sizeof(command));
    CHECK(result.resultBuffer != nullptr, "success resultBuffer is present");
    CHECK(result.length >= 9, "success length covers index 8");

    byte snapshot[BUFFER_SIZE];
    memset(snapshot, 0, sizeof(snapshot));
    memcpy(snapshot, result.resultBuffer, result.length);

    byte stackNoise[256];
    memset(stackNoise, 0xAA, sizeof(stackNoise));
    (void)stackNoise;

    CHECK(result.resultBuffer[8] == 0x00, "index 8 still readable after return");
    CHECK(memcmp(snapshot, result.resultBuffer, result.length) == 0, "payload unchanged after stack reuse");
}

static void testSecondCommandTimeoutAfterConfigMode()
{
    resetClock();
    MockStream stream;
    stream.queueReply(successAckFrame());
    Seeed_HSP24 radar(stream);
    CHECK(radar.getVersion() == String("Get Version Error!"), "version query timeout after entering config mode");
}

static void testGetVersionAndConfigParse()
{
    resetClock();
    MockStream versionStream;
    versionStream.queueReply(successAckFrame());
    versionStream.queueReply(versionFrame());
    versionStream.queueReply(successAckFrame());
    Seeed_HSP24 versionRadar(versionStream);
    CHECK(versionRadar.getVersion() == String("V1.02.000003"), "getVersion parses firmware bytes");

    resetClock();
    MockStream configStream;
    configStream.queueReply(successAckFrame());
    configStream.queueReply(configFrame());
    configStream.queueReply(successAckFrame());
    Seeed_HSP24 configRadar(configStream);
    Seeed_HSP24::RadarStatus status = configRadar.getConfig();
    CHECK(status.detectionDistance == 0x08, "getConfig detectionDistance");
    CHECK(status.moveSetDistance == 0x07, "getConfig moveSetDistance");
    CHECK(status.staticSetDistance == 0x06, "getConfig staticSetDistance");
    CHECK(status.radarMovePower.moveGate[0] == 0x11, "getConfig first move gate");
    CHECK(status.noTargrtduration == 0x2A, "getConfig unoccupied duration");

    resetClock();
    MockStream resolutionStream;
    resolutionStream.queueReply(successAckFrame());
    resolutionStream.queueReply(resolutionFrame());
    resolutionStream.queueReply(successAckFrame());
    Seeed_HSP24 resolutionRadar(resolutionStream);
    CHECK(resolutionRadar.getResolution().resolution == 0x01, "getResolution reads index 10");
}

int main()
{
    testTimeoutReturnsErrorWithoutCrash();
    testSendCommandTimeoutLeavesBufferUnreadable();
    testIncompleteFrameIsSafe();
    testShortCompleteFrameDoesNotReadPastEnd();
    testSuccessAndFailedAck();
    testResultBufferSurvivesReturn();
    testSecondCommandTimeoutAfterConfigMode();
    testGetVersionAndConfigParse();

    std::printf("%d passed, %d failed\n", g_passed, g_failures);
    return g_failures == 0 ? 0 : 1;
}
