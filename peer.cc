#include <omnetpp.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <random>
#include "MyMessage.h"
#include <functional>
#include <iomanip>
#include "Block.h"
#include "MiningPowers.h"
#include <queue>
#include <fstream> // Include this header for file operations


using namespace omnetpp;
using namespace std;





class PeerNode : public cSimpleModule
{
private:

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

public:
    
    int numSeeds;
    int numPeers;
    int maxPeersSel;
    bool isadversary;
    int interArrivalTime;

    void registerWithSeed(int seedIndex);
    void PeerListasker();
    void connectToPeers();
    void setuplivness();
    void livnessTest();

    void gossip();
    void handleGossip(MyMessage *msg);

    vector<int>selectedSeeds;
    set<int> peerlist;
    set<int> connectedpeers;
    int receivedNumPlist=0;
    map<int,int>livness;
    map<int,simtime_t>livnessTimestamp;
    int gossip_gen=0;

    map<int,bool>ML;

    // blockchain
    vector<Block> blockchain;
    queue<Block> pendingQueue;
    void setupblockchain();
    void mineBlock();
    void receiveBlock(MyMessage *msg);
    void startMining();
    void validateAndAddBlock(Block block);
    void broadcastBlock(Block block);
    void logBlockDetails(Block block);
    Block* createNewBlock();
    string calculateBlockHash(Block& block);
     
    
    int blockheight=0;
    string currentHash;
    double hashPower;


    simtime_t nextminingtime;


   
    

};

Define_Module(PeerNode);



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

void PeerNode::initialize()
{   
    string test="navneet";
    EV<< "Hash of " << test << " is " << cal_hash(test) << endl;
    int adv_index=par("adv");
    EV<<"adv_index "<<adv_index<<endl;
    if(adv_index==getIndex()){
        isadversary=true;
    }
    else{
        isadversary=false;
    }
    

    numSeeds = par("ns");
    numPeers = par("np");
    interArrivalTime = par("iat");

    maxPeersSel = 4;



    // make [n/2] + 1 random connections to seeds
    vector<int> seedIndices(numSeeds);
    for(int i = 0; i < numSeeds; i++) {
        seedIndices[i] = i;
    }
    std::random_device rd; // Obtain a random number from hardware
    std::default_random_engine eng(rd()); // Seed the generator

    shuffle(seedIndices.begin(), seedIndices.end(), eng);
    EV<< "Seed indices: ";
    for (int i = 0; i < numSeeds; i++) {
        EV << seedIndices[i] << " ";
    }
    EV << endl;



    for (int i = 0; i < numSeeds/2 + 1; i++) {

        selectedSeeds.push_back(seedIndices[i]);

        cModule *seed = getParentModule()->getSubmodule("seed", seedIndices[i]);
        // add gate and connect to seed
    
        gate("out", seedIndices[i])->connectTo(seed->gate("in",getIndex()));
 
        seed->gate("out", getIndex())->connectTo(gate("in", seedIndices[i]));

        registerWithSeed(seedIndices[i]);

    }

    

   
        MyMessage *askpeerlist = new MyMessage();
        askpeerlist->messageType = "askPeerList";
        askpeerlist->content = "peer";

        // delay for 1 second and ask for the peer list
        scheduleAt(simTime() + 1, askpeerlist);







}

void PeerNode::handleMessage(cMessage *msg)
{
    MyMessage *m = check_and_cast<MyMessage *>(msg);
    EV << "Received message of type: " << m->messageType ;

    if (m->messageType == "peerList") {
        // union the received peer list with the current peer list
        peerlist.insert(m->peerlist.begin(), m->peerlist.end());
        receivedNumPlist++;

        if(receivedNumPlist == selectedSeeds.size()){
            EV << "final Peer list at peer " << getIndex() << " : ";
            for (int peer : peerlist) {
                EV << peer << " ";
            }
            EV << endl;

            connectToPeers();
        }
    }
    else if(m->messageType == "askPeerList") {
        PeerListasker();
        setuplivness();
        setupblockchain();
    }
    else if(m->messageType == "do livness test"){
        livnessTest();
        EV << "livness test " << getIndex() << endl;
        MyMessage *livnessMessage = new MyMessage();
        livnessMessage->messageType = "do livness test";
        livnessMessage->address = getIndex();
        scheduleAt(simTime() + 13, livnessMessage);
    }
    else if(m->messageType == "livness Request"){
        MyMessage *livnessMessage = new MyMessage();
        livnessMessage->messageType = "livness Reply";
        livnessMessage->address = getIndex();
        livnessMessage->timestamp = m->timestamp;
        livnessMessage->sender_address = m->address;
        send(livnessMessage, "peerout", m->address);
    }
    else if(m->messageType == "livness Reply"){
        EV << "livness Reply from " << m->address << endl;
        if(livnessTimestamp[m->address] == m->timestamp && m->sender_address == getIndex()){
            livness[m->address] = 0;
        }
    }
    else if(m->messageType == "do gossip"){
        EV << "do gossip " << getIndex() << endl;
        gossip();
    }
    else if(m->messageType == "gossip"){
        EV << "gossip received from " << m->address << " : " << m->content << endl;
        gossip();
    }
    else if (m->messageType == "do mining" and simTime()>=nextminingtime) {
        EV << "mining " << getIndex() << endl;
        mineBlock();
    }
    else if (m->messageType == "newBlock") {
        receiveBlock(m);
    }
    else if(m->messageType == "blockchainRequest"){
        
        MyMessage *blockchainReply = new MyMessage();
        blockchainReply->messageType = "blockchainReply";
        blockchainReply->address = getIndex();
        blockchainReply->blockchain = blockchain;
        send(blockchainReply, "peerout", m->address);
    
    }
    else if(m->messageType == "blockchainReply"){
        // check if the received blockchain is longer than the current blockchain
        
        if(m->blockchain.size() > blockchain.size()){

            blockchain = m->blockchain;
            
        }
        
    }
    
    

    delete m;   
}
void PeerNode::setupblockchain() {
    // create genesis block
    Block genesisBlock("0x0000", "0x00000", getIndex(),1);
    // blockchain.push_back(genesisBlock);
    currentHash = "0x9e1c";
    blockheight = 1;
    // logBlockDetails(genesisBlock);
    // Initialize hash power
    hashPower = MINING_POWERS[getIndex() % MINING_POWERS.size()];
    // ask from peers for blockchain
    for (int peer : peerlist) {
        
        if(peer==getIndex()){
            continue;
        }
        MyMessage *blockchainRequest = new MyMessage();
        blockchainRequest->messageType = "blockchainRequest";
        blockchainRequest->address = getIndex();
        send(blockchainRequest, "peerout", peer);
    }
    for(Block& block : blockchain) {
        if(block.blockheight>blockheight and block.prevHash!="0x0000"){
            blockheight=block.blockheight;
            currentHash = calculateBlockHash(block);
        }

        logBlockDetails(block);
            
    }

    // Start mining
    startMining();
}

void PeerNode::startMining() {
    EV << "Start mining called at   " << getIndex() << endl;
    MyMessage *miningMsg = new MyMessage();
    miningMsg->messageType = "do mining";
    miningMsg->address = getIndex();

    double meanTk = 1.0 / interArrivalTime;
    double lambda = (hashPower * meanTk) / 100.0;
    simtime_t Tk = simTime() + exponential(1/lambda);
    // schedule next mining
    EV<< "Next mining time: " << Tk << endl;

    nextminingtime=max(Tk,nextminingtime);
    
    scheduleAt(Tk, miningMsg);
}

void PeerNode::mineBlock() {
    EV<<"peer"<<getIndex()<<" is mining"<<endl;
    Block* newBlock = createNewBlock();
    blockchain.push_back(*newBlock);
    logBlockDetails(*newBlock);
   

    if(newBlock->blockheight>blockheight){
        blockheight=newBlock->blockheight;
        currentHash = calculateBlockHash(*newBlock);
    }
    
    
    broadcastBlock(*newBlock);
    startMining();  // Start mining next block
}

void PeerNode::receiveBlock(MyMessage *msg) {
    pendingQueue.push(msg->block);
    validateAndAddBlock(pendingQueue.front());
    pendingQueue.pop();
    
}

void PeerNode::validateAndAddBlock(Block block) {
    
    startMining();
    // Check if the block is valid
    // check if any existing block has less than 1 hour difference and extend any previous block
    // if yes, add the block to the blockchain
    // if no, ignore the block
    for(Block& prev : blockchain) {
        if (block.prevHash == calculateBlockHash(prev) && block.blockheight==prev.blockheight+1 and block.timestamp-prev.timestamp<=3600) {
            blockchain.push_back(block);
            logBlockDetails(block);
            break;
        }
    }

    
}

void PeerNode::broadcastBlock(Block block) {
    for (int peer : connectedpeers) {
        MyMessage *blockMsg = new MyMessage();
        blockMsg->messageType = "newBlock";
        blockMsg->block = block;
        blockMsg->address = getIndex();
        blockMsg->timestamp = simTime();
        send(blockMsg, "peerout", peer);
    }
}

std::string PeerNode::calculateBlockHash(Block& block) {
    std::string blockstr=block.toString();
    return "0x"+cal_hash(blockstr).substr(0,4);
}

Block* PeerNode::createNewBlock() {
    string prevHash = currentHash;
    string merkleRoot = "0x" ;
    int miner = getIndex();
    int h=blockheight+1;
    return new Block(prevHash, merkleRoot, miner,h);
}



void PeerNode::handleGossip(MyMessage *msg){
    // hash and keep in map hash,true
    // if hash already exists, ignore
    // if hash does not exist,
    //    hash,true
    int hashval = hash<string>{}(SIMTIME_STR(msg->timestamp)+to_string(msg->address)+msg->content);
    if(ML.find(hashval)==ML.end()){
        ML[hashval]=true;
        // forward the gossip to all adjacent peers
        for(int peerIndex : connectedpeers){
            if(peerIndex==getIndex()){
                continue;
            }
            MyMessage *gossipMessage = new MyMessage();
            gossipMessage->messageType = "gossip";
            gossipMessage->timestamp = simTime();
            gossipMessage->address = getIndex();
            // generate random gossip content

            gossipMessage->content = "gossip content"+to_string(rand()%100);
            send(gossipMessage, "peerout", peerIndex);
            
        }
    }


    // forward the gossip to all adjacent peers
   
}
void PeerNode::gossip(){
    gossip_gen++;
    if(gossip_gen<=10){
        // send gossip to all adjacent peers
        for(int peerIndex : peerlist){
              if(peerIndex==getIndex()){
                continue;
            }
            MyMessage *gossipMessage = new MyMessage();
            gossipMessage->messageType = "gossip";
            gossipMessage->timestamp = simTime();
            gossipMessage->address = getIndex();
            // generate random gossip content

            gossipMessage->content = "gossip content"+to_string(rand()%100);
            send(gossipMessage, "peerout", peerIndex);
            
        }
    }
    // schedule next gossip
    MyMessage *gossipMessage = new MyMessage();
    gossipMessage->messageType = "do gossip";
    gossipMessage->address = getIndex();
    scheduleAt(simTime() + 5, gossipMessage);
}
void PeerNode::livnessTest(){
    

    for(int peerIndex : connectedpeers){
        if(peerIndex==getIndex()){
            continue;
        }
        livness[peerIndex]++;
        if(livness[peerIndex]>3){
            
            // notify the seeds of the dead peer
            for(int seedIndex : selectedSeeds) {
                MyMessage *peerListMessage = new MyMessage();
                peerListMessage->messageType = "Dead Node";
                peerListMessage->content = "";
                peerListMessage->address = getIndex();
                peerListMessage->sender_address = peerIndex;
                send(peerListMessage, "peerout", seedIndex);

            }
            peerlist.erase(peerIndex);

            continue;
        }
        livnessTimestamp[peerIndex]=simTime();
        MyMessage *livnessMessage = new MyMessage();
        livnessMessage->messageType = "livness Request";
        livnessMessage->address = getIndex();


        livnessMessage->timestamp =livnessTimestamp[peerIndex];
        

        send(livnessMessage, "peerout", peerIndex);
    }
}
void PeerNode::setuplivness(){
    for(int peerIndex : peerlist){
        
        livness[peerIndex]=0;
    }
    MyMessage *livnessMessage = new MyMessage();
    livnessMessage->messageType = "do livness test";
    livnessMessage->address = getIndex();
    scheduleAt(simTime() + 1, livnessMessage);

}
void PeerNode::connectToPeers(){
    // Ramdomly select maxPeersSel peers to connect to
    maxPeersSel = min(4,(int)peerlist.size());
    vector<int> peerIndices((int)peerlist.size());

    for(int i = 0; i < peerlist.size(); i++) {
        peerIndices[i] = i;
    }
    std::random_device rd; // Obtain a random number from hardware
    std::default_random_engine eng(rd()); // Seed the generator

    shuffle(peerIndices.begin(), peerIndices.end(), eng);

    for (int i = 0; i < maxPeersSel; i++) {
        int peerIndex = peerIndices[i];
        connectedpeers.insert(peerIndex);
        cModule *peer = getParentModule()->getSubmodule("peer", peerIndex);
        // add gate and connect to peer
        // gate("peerout", peerIndex)->connectTo(peer->gate("peerin",getIndex()));
    
        // peer->gate("peerout", getIndex())->connectTo(gate("peerin", peerIndex));
        // if not already connected
        if(gate("peerout", peerIndex)->getPathEndGate()->getPreviousGate() == nullptr) {
            gate("peerout", peerIndex)->connectTo(peer->gate("peerin", getIndex()));
        }
        if(peer->gate("peerout", getIndex())->getPathEndGate()->getPreviousGate() == nullptr) {
            peer->gate("peerout", getIndex())->connectTo(gate("peerin", peerIndex));
        }
    }


}
void PeerNode:: PeerListasker() {
    for(int seedIndex : selectedSeeds) {
        MyMessage *peerListMessage = new MyMessage();
        peerListMessage->messageType = "peerList";
        peerListMessage->content = "";
        peerListMessage->address = getIndex();
        send(peerListMessage, "out", seedIndex);
    }
    

}


void PeerNode::registerWithSeed(int seedIndex) {
    // send a message to the seed to register
    MyMessage *msg = new MyMessage();
    msg->messageType = "register";
    msg->content = "peer";
    msg->address = getIndex();
    
    send(msg, "out", seedIndex);
}
// Function to log block details to a file
void PeerNode::logBlockDetails(Block block) {
    std::ofstream outfile;
    
    outfile.open("blockchain_log.txt", std::ios_base::app); // Open file in append mode
    outfile << "peer " << getIndex() << " " <<
     "BlockHash: " <<calculateBlockHash(block) <<" PrevHash: " << block.prevHash << " BlockMiner: " << block.blockminer << " BlockHeight: " << block.blockheight << " Timestamp: " << block.timestamp << "\n";
    outfile.close();
}