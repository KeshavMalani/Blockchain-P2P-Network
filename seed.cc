#include <omnetpp.h>
#include <vector>
#include <string>
#include "MyMessage.h"

#include <functional>

#include <sstream>
#include <iomanip>


using namespace omnetpp;
using namespace std;


class SeedNode : public cSimpleModule
{
private:
    std::vector<std::pair<std::string, int>> peerList;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

public:
    set<int> peerlist;

};

Define_Module(SeedNode);
std::string to_hex(size_t hash) {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return ss.str();
}
std::string cal_hash(std::string str){
    std::hash<std::string> hasher;
    size_t hash_value = hasher(str);
    std::string hex_hash = to_hex(hash_value);
    return hex_hash;
}
void SeedNode::initialize() {
    string test="navneet";
    EV<< "Hash of " << test << " is " << cal_hash(test) << endl;

}

void SeedNode::handleMessage(cMessage *msg)
{
    MyMessage *m = check_and_cast<MyMessage *>(msg);
    if (m->messageType == "register") {
        peerlist.insert(m->address);
        EV<< "Peer " << m->address << " registered with seed" <<getIndex()<<endl;

        MyMessage *registerMessage = new MyMessage();
        registerMessage->messageType = "registered";
        registerMessage->content = "";
        registerMessage->address = getIndex();
        send(registerMessage, "out", m->address);

    }
    else if (m->messageType == "peerList") {
        EV<< "peer " << m->address << "  asking peer list from seed " << getIndex()<< endl;
        // send the peer list to the peer
//        for(auto peer : peerlist) {
//            EV << "Peer " << peer <<" ";
//        }
//        EV << endl;
//        EV<< "at seed " << getIndex() << endl;
        MyMessage *peerListMessage = new MyMessage();
        peerListMessage->messageType = "peerList";
        peerListMessage->content = "";
        peerListMessage->address = getIndex();
        peerListMessage->peerlist = peerlist;
        send(peerListMessage, "out", m->address);
    }
    else if(m->messageType == "Dead Node"){
        // remove the dead peer from the peer list
        if(peerlist.find(m->address)!=peerlist.end()){
            peerlist.erase(m->address);
            EV<< "Peer " << m->sender_address << " notified seed " << getIndex() << " of dead peer " << m->address << endl;
        }
    }


    delete m;
}

