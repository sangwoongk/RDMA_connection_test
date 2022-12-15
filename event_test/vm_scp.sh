#!/bin/bash
INVOKERS=("sriov@192.168.122.160" "sriov@192.168.122.73")
MAKE_CMD="make -j"

for INVOKER in "${INVOKERS[@]}"
	do
		sshpass -p 'sriov' ssh $INVOKER "rm -rf ~/rdma-connect-test"
		make clean
		sshpass -p 'sriov' scp -r ../rdma-connect-test/ $INVOKER:~/
		echo "[$INVOKER] Compiling..."
		sshpass -p 'sriov' ssh $INVOKER "cd ~/rdma-connect-test; $MAKE_CMD"
	done

# echo "[Controller] Compiling..."
# cd build
# cmake ..
# $MAKE_CMD