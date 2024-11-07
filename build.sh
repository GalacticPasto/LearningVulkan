#!/bin/bash

DEFINES="-std=c++17 -O2"
LDFLAGS="-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi"

g++ $CFLAGS src/main.cpp -o build/VulkanTest $LDFLAGS

