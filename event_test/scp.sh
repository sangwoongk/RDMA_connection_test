#!/bin/bash
INVOKERS=("caslab@10.150.21.198")
MAKE_CMD="make -j"
UPPER_DIR="/home/caslab/workspace/rdma_playground/"
DIR="/home/caslab/workspace/rdma_playground/event_test"

for INVOKER in "${INVOKERS[@]}"
	do
		ssh $INVOKER "rm -r $DIR"
		make clean
		scp -r $DIR $INVOKER:$UPPER_DIR
		echo "[$INVOKER] Compiling..."
		ssh $INVOKER "cd $DIR; $MAKE_CMD"
	done

# echo "[Controller] Compiling..."
# cd build
# cmake ..
$MAKE_CMD