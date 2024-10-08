#ifndef MYMESSAGE_H
#define MYMESSAGE_H

#include <omnetpp.h>
#include <string>
#include <vector>
#include "Block.h"

using namespace omnetpp;
using namespace std;
class MyMessage : public cMessage {
public:
    std::string messageType;  // Type of the message (e.g., "register", "peerList")
    std::string content;      // Main content of the message (e.g., peer list as a string)
    int address;          // Address of the peer/seed
    set<int> peerlist;    // List of peers
    simtime_t timestamp;     // Time at which the message was sent
    int sender_address;   // Address of the sender
    Block block;          // Block object
    vector<Block> blockchain; // Blockchain

    MyMessage() : cMessage() {}

};

#endif // MYMESSAGE_H
