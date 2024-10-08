simple SeedNode
{
    parameters:
        int ns = default(1);  // Total number of seed nodes
        int np = default(1);  // Total number of peer nodes
    gates:
        input in[np];
        output out[np];
}

simple PeerNode
{
    parameters:
        int ns = default(1);  // Total number of seed nodes
        int np = default(1);  // Total number of peer nodes
        int adv=default(0);
        int iat=default(2);
    gates:
        input in[ns];
        output out[ns];
        input peerin[np];
        output peerout[np];
    
        
}

network Net
{
    parameters:
        int numSeeds;
        int numPeers;
        int adv=intuniform(0,numPeers-1);
        int iat;
    submodules:
        seed[numSeeds]: SeedNode {
            parameters:
                ns = parent.numSeeds;
                np = parent.numPeers;
              
        }
        peer[numPeers]: PeerNode {
            parameters:
                ns = parent.numSeeds;
                np = parent.numPeers;
                adv= parent.adv;
                iat= parent.iat;
        }
    connections allowunconnected:
        // Connect peers to seeds
//        for i=0..numPeers-1, for j=0..numSeeds-1 {
//            peer[i].out++ --> {delay=100ms;} --> seed[j].in++;
//            peer[i].in++ <-- {delay=100ms;} <-- seed[j].out++;
//        }
        // Connect peers to peers (will be established dynamically in the simulation)

}