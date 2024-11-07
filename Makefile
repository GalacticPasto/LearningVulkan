CFLAGS = -std=c++17 -O2
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

VulkanTest: src/main.cpp
	g++ $(CFLAGS) src/main.cpp -o build/VulkanTest $(LDFLAGS)

.PHONY: test clean

test:VulkanTest
	 ./build/VulkanTest

clean:
	 rm -f build/VulkanTest
