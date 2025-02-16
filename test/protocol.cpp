/*
 * Test the protocol implementation
 */

#include "catch2.hpp"
#include "tipsy/tipsy.h"

#include <cstring>
#include <iostream>

TEST_CASE("Protocol Encode Simple String")
{
    INFO("FIXME - make this test assert state transitions");
    const char * mimeType{"application/text"};
    const char * message{"I am the very model of a modern major general"};

    tipsy::ProtocolEncoder pe;

    // don't forget null termination
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool done{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);

        if (done)
        {
            REQUIRE(st == tipsy::EncoderResult::DORMANT);
        }
        else
        {
            REQUIRE(((st == tipsy::EncoderResult::ENCODING_MESSAGE) ||
                     (st == tipsy::EncoderResult::MESSAGE_COMPLETE)));
        }
        if (st == tipsy::EncoderResult::MESSAGE_COMPLETE)
        {
            done = true;
        }
    }
}

TEST_CASE("Encode Decode String")
{
    const char *mimeType{"application/text"};
    const char *message{"I am the very model of a modern major general"};

    unsigned char buffer[2048];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 2048);

    // don't forget null termination
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        auto rf = pd.readFloat(nf);

        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(std::string((const char *)buffer) == std::string(message));
            gotBody = true;
        }
    }
}

TEST_CASE("Require a MIME type")
{
    tipsy::ProtocolEncoder pe;
    auto status = pe.initiateMessage(nullptr, 0, (const unsigned char *)"");
    REQUIRE(status == tipsy::EncoderResult::ERROR_MISSING_MIME_TYPE);
}

TEST_CASE("Buffer too small")
{
    const char *mimeType{"application/text"};
    const char *message{"I am the very model of a modern major general"};

    unsigned char buffer[20];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 20);

    // don't forget the terminating null
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        auto rf = pd.readFloat(nf);

        //  we expect this error when the message exceeds the buffer
        if (rf == tipsy::DecoderResult::ERROR_DATA_TOO_LARGE)
        {
            return;
        }
        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(std::string((const char *)buffer) == std::string(message));
            gotBody = true;
        }
    }
}


TEST_CASE("Encode Decode Empty Message")
{
    const char * mimeType{"application/text"};
    const char * message{""};

    unsigned char buffer[2048];

    tipsy::ProtocolEncoder pe;
    tipsy::ProtocolDecoder pd;
    pd.provideDataBuffer(buffer, 2048);

    // don't forget the terminating null
    auto status = pe.initiateMessage(mimeType, strlen(message) + 1, (const unsigned char *)message);
    REQUIRE(status == tipsy::EncoderResult::MESSAGE_INITIATED);
    bool gotHeader{false}, gotBody{false};
    for (int i = 0; i < 50; ++i)
    {
        float nf;
        auto st = pe.getNextMessageFloat(nf);
        auto rf = pd.readFloat(nf);

        REQUIRE(!tipsy::ProtocolDecoder::isError(rf));

        if (gotBody && gotHeader)
        {
            REQUIRE(rf == tipsy::DecoderResult::DORMANT);
        }
        if (rf == tipsy::DecoderResult::HEADER_READY)
        {
            REQUIRE(std::string(pd.getMimeType()) == std::string(mimeType));
            gotHeader = true;
        }
        if (rf == tipsy::DecoderResult::BODY_READY)
        {
            REQUIRE(0 == buffer[0]);
            gotBody = true;
        }
    }
}