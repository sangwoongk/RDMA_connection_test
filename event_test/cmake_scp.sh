#!/bin/bash
INVOKERS=("caslab@10.150.21.198")
MAKE_CMD="make -j"

for INVOKER in "${INVOKERS[@]}"
	do
		ssh $INVOKER rm -rf ~/workspace/rdma-connect-test
		rm -f *.o pickme *~
		scp -r ../rdma-connect-test/ $INVOKER:~/workspace/
		echo "[$INVOKER] Compiling..."
		# remove build directory
		ssh $INVOKER "rm -rf ~/workspace/rdma-connect-test/build/*;"
		ssh $INVOKER "cd ~/workspace/rdma-connect-test/build; cmake ..; $MAKE_CMD"
	done

echo "[Controller] Compiling..."
cd build
cmake ..
$MAKE_CMD
