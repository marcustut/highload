#!/bin/bash

PLATFORM=linux/amd64
NAME=highload
IMAGE=ubuntu

# Check if the container is already running
if docker ps -a | grep -q $NAME; then
    # Start the container if it's not running
    docker start $NAME
else
    # Create the container if it doesn't exist
    docker run \
        -it \
        -d \
        --platform $PLATFORM \
        -v ./:/app \
        --name $NAME \
        $IMAGE
fi

# Open a shell in the container
docker exec -it $NAME /bin/bash

