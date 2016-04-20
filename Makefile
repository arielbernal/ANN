SOURCES := main.cpp

PROGRAM := ANN
CXX := g++ 
#LDFLAGS := -L/media/ajbernal/SSD1/ocv/oclTest/beignet
#LDFLAGS := -L/opt/intel/opencl
LDFLAGS +=  -O3 -lOpenCL 
#-lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs -lopencv_objdetect
CXXFLAGS := -I ./include
CXXFLAGS += -std=c++11 -Wall -O3 -mfpmath=sse -msse4.1
CXXFLAGS += -fpermissive

OBJECTS := $(SOURCES:.cpp=.o)

BIN := bin

build: $(BIN)/$(PROGRAM)

all: clean run

run: $(BIN)/$(PROGRAM)
	./$(BIN)/$(PROGRAM) $(PARAMS)

$(OBJECTS):
	@mkdir -p $(BIN)
	@echo "Compiling: $(subst .o,.cpp, $@) -> $(addprefix $(BIN)/, $(notdir $@))"
	@$(CXX) -c $(subst .o,.cpp, $@) -o $(addprefix $(BIN)/,$(notdir $@)) $(CXXFLAGS)

$(BIN)/$(PROGRAM): $(OBJECTS)
	@mkdir -p $(BIN)
	@echo "Linking: $(addprefix $(BIN)/, $(notdir $^)) -> $@"
	@$(CXX) $(addprefix $(BIN)/, $(notdir $^)) -o $@ $(CXXFLAGS) $(LDFLAGS)

clean:
	@rm -f *.o $(BIN)/$(PROGRAM)
