#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <omnetpp.h>
#include <string>

using namespace omnetpp;
using namespace std;
class Block {
public:
    string prevHash;
    string merkleRoot;
    simtime_t timestamp;
    int blockminer;
    int blockheight;
    
    Block() {}
    
    Block(string prev, string merkle,int miner,int height) : 
        prevHash(prev), 
        merkleRoot(merkle), 
        timestamp(simTime()),
        blockminer(miner),
        blockheight(height) {}

    std::string toString() {
        return "Block: prevHash=" + prevHash + 
               ", merkleRoot=" + merkleRoot + 
               ", timestamp=" + SIMTIME_STR(timestamp) +
                ", blockminer=" + std::to_string(blockminer) +
                ", blockheight=" + std::to_string(blockheight);
    }
    
};

#endif // __BLOCK_H__