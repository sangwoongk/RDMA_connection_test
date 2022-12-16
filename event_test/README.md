# RDMA event test
Source code to understand event-based RDMA.

## Setup and prerequisites
- Mellanox HCAs
- Mellanox OFED 4.9 or 5.4
- g++ 7.3 or higher
- libboost-dev
- Two or more servers are connected via Infiniband Switch
- OpenSM is running in the network

## Build
```
make -j
```

## Run
```
./test
```
