
CXXFLAGS += -std=c++17 -Wall -fPIC 
OUTPUT = build

SRC_INC = \
	abs_camera.h \
	com_camera.h \
	v4l2_camera.h \
	ov7670.h

SRC_LIB = \
	com_camera.cpp \
	v4l2_camera.cpp \
	ov7670.cpp 

SRC_DEMO = \
	demo.cpp 

OBJ_LIB := $(addprefix $(OUTPUT)/, $(SRC_LIB:.cpp=.o))
OBJ_DEMO := $(addprefix $(OUTPUT)/, $(SRC_DEMO:.cpp=.o))

STATIC_LIB := $(OUTPUT)/lib/libcamera.a
DYNAMIC_LIB := $(OUTPUT)/lib/libcamera.so
DEMO := $(OUTPUT)/demo

.PHONY: all clean demo

all: $(STATIC_LIB) $(DYNAMIC_LIB) $(DEMO)
	echo "Build all done!"

# 生成静态库
$(STATIC_LIB): $(OBJ_LIB)
	ar rcs $@ $^

# 生成动态库
$(DYNAMIC_LIB): $(OBJ_LIB)
	$(CXX) -shared -o $@ $^

# 编译源文件生成目标文件
$(OUTPUT)/%.o: %.cpp $(SRC_INC) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p $(OUTPUT)/lib

$(DEMO): $(OBJ_DEMO) | $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(OBJ_DEMO) $(STATIC_LIB) -lpthread -o $@

demo: $(DEMO)
	@echo "Demo build OK!"

clean:
	rm -rf $(OUTPUT)

#$CXX -std=c++17 com_camera.cpp v4l2_camera.cpp ov7670.cpp test.cpp -o test