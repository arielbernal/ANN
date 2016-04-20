#ifndef ANN_DATASET_H
#define ANN_DATASET_H

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <algorithm>

namespace ann {

template <typename NNType = double,
          typename FileDataType = uint8_t,
          typename LabelType = uint8_t>
struct DatasetSample {
  DatasetSample() : Id(0), Label(0), Error(false) {}

  DatasetSample(size_t DataSize, size_t OutputSize)
      : Id(0), Data(DataSize), Label(0), Input(DataSize), Output(OutputSize), Error(false) {
  }

  DatasetSample(size_t Id,  LabelType Label, FileDataType* data, size_t DataSize, size_t OutputSize)
      : Id(Id), Data(DataSize), Label(Label), Input(DataSize), Output(OutputSize), Error(false) {
    setData(data);
  }

  void setData(FileDataType* data) {
    for (size_t j = 0; j < Data.size(); ++j)
      Data[j] = data[j];
  }

  void zeroAll() {
    for (size_t j = 0; j < Data.size(); ++j) {
      Data[j] = 0;
      Input[j] = 0;
    }
    for (size_t j = 0; j < Output.size(); ++j) 
      Output[j] = 0;
  }

  void create(size_t Id,  LabelType Label, size_t DataSize, size_t OutputSize) {
    clear();
    this->Id = Id;
    this->Label = Label;
    Data.resize(DataSize);
    Input.resize(DataSize);
    Output.resize(OutputSize);
  }

  void clear() {
    Data.clear();
    Input.clear();
    Output.clear();
  }

  void print() {
    std::cout << "Sample Id = " << Id << " Label = " << int(Label) << " DataSize = " 
              << Data.size() << " OutputSize = " << Output.size() << " Error = " << Error << std::endl;
  }

  size_t Id; // Sample Id
  std::vector<FileDataType> Data; // File data
  LabelType Label; // Label for classification

  std::vector<NNType> Input; // NN Input vector
  std::vector<NNType> Output; // NN Output vector
    
  bool Error;
};

uint32_t reverse_bytes(uint32_t x)
{
  char* px = (char*) &x;
  char buff = px[0];
  px[0] = px[3];
  px[3] = buff;
  buff = px[1];
  px[1] = px[2];
  px[2] = buff;
  return x;
}

template <typename NNType = double,
          typename FileDataType = uint8_t,
          typename LabelType = uint8_t>
class BaseDataset {
public:
  typedef DatasetSample<NNType, FileDataType, LabelType> SampleType;

  BaseDataset(uint32_t OutputSize) 
    : N(0),
      SampleSize(0),
      Cols(0),
      Rows(0),
      OutputSize(OutputSize)
  {}

  BaseDataset(uint32_t Rows, uint32_t Cols, uint32_t OutputSize)
      : N(0),
        SampleSize(Rows * Cols),
        Cols(Cols),
        Rows(Rows),
        OutputSize(OutputSize)
  {}

  BaseDataset(uint32_t SampleSize, uint32_t OutputSize)
      : N(0),
        SampleSize(SampleSize),
        Cols(SampleSize),
        Rows(1),
        OutputSize(OutputSize)
  {}
  
  size_t getSampleSize() { return SampleSize; }
  size_t getRows() { return Rows; }
  size_t getCols() { return Cols; }
  size_t getOutputSize() { return OutputSize; }
  size_t getNSamples() { return N; }

  virtual bool load(const std::string& DataFile, const std::string& LabelsFile) = 0;

  void clear() {
    if (!Samples.empty()) {
      for (auto I = Samples.begin(), E = Samples.end(); I != E; ++I)
        delete (*I);
    }
    Samples.clear();
    N = 0;
  }

  void computeMeanVector() {
    MeanSample.create(0, 0, SampleSize, OutputSize);
    MeanSample.zeroAll();
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < SampleSize; ++j)
        MeanSample.Input[j] += Samples[i]->Input[j];
    for (size_t j = 0; j < SampleSize; ++j)
      MeanSample.Input[j] /= N;
  }

  // Scale normalization 
  void normalizeInputs(NNType vNorm) { 
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < SampleSize; ++j)
        Samples[i]->Input[j] = NNType(Samples[i]->Data[j]) / vNorm;
  }

  // Mean normalization
  void subtractMeanInput() {
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < SampleSize; ++j)
        Samples[i]->Input[j] -= MeanSample.Input[j];
  }


  void addFilterByLabel(LabelType Label) {
    std::vector<SampleType*> FilterTemp = Filter;
    Filter.clear();
    for (auto& e : FilterTemp)
      if (e->Label == Label)
        Filter.push_back(e);
  }

  void addFilterByError() {
    std::vector<SampleType*> FilterTemp = Filter;
    Filter.clear();
    for (auto& e : FilterTemp)
      if (e->Error)
        Filter.push_back(e);
  }

  void clearFilter() {
    Filter.clear();
    for (auto& e : Samples)
      Filter.push_back(e);
  }

  void sortByMSE() {
    std::sort(Filter.begin(),
              Filter.end(),
              [](const SampleType* a, const SampleType* b)
                  -> bool { return a->MSE > b->MSE; });
  }

  void randomizeOrder() {
    std::default_random_engine generator;
    std::uniform_int_distribution<size_t> d(0, Filter.size() - 1);
    for (size_t i = 0; i < Filter.size(); ++i) {
      size_t iRand = d(generator);
      std::swap(Filter[i], Filter[iRand]);
    }
  }

protected:
  std::vector<SampleType*> Samples;
  std::vector<SampleType*> Filter;
  uint32_t N;
  uint32_t SampleSize;
  uint32_t Cols;
  uint32_t Rows;
  uint32_t OutputSize;

  SampleType MeanSample;  
};


class MNISTDataset : public BaseDataset<double, uint8_t, uint8_t> {
public:
  MNISTDataset() : BaseDataset<double, uint8_t, uint8_t>(10) {}

  MNISTDataset(const std::string& DataFile, const std::string& LabelsFile ) 
  : BaseDataset<double, uint8_t, uint8_t>(10) {
    load(DataFile, LabelsFile);
  }

  bool load(const std::string& DataFile, const std::string& LabelsFile) {
    clear();
    std::ifstream ifsd(DataFile.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!ifsd.is_open()) {
      std::cout << "Error loading MNIST Data file no found : " << DataFile << std::endl;
      return false;
    }
    uint32_t magic;
    ifsd.read((char*) &magic, 4);
    ifsd.read((char*) &N, 4);
    ifsd.read((char*) &Rows, 4);
    ifsd.read((char*) &Cols, 4);
    magic = reverse_bytes(magic);
    N = reverse_bytes(N);
    Rows = reverse_bytes(Rows);
    Cols = reverse_bytes(Cols);
    if (magic != 2051)  {
      printf("Error loading MNIST Data file corrupted, magic = %d, NSamples = %d, Rows = %d, Cols = %d\n", magic, N, Rows, Cols);      
      return false;
    }
    SampleSize = Rows * Cols;
    printf("Loading Data File '%s'\nN = %d, Rows = %d,  Cols = %d\n", DataFile.c_str(), N, Rows, Cols);
    std::vector<uint8_t> Data(N * SampleSize);
    ifsd.read((char*)&Data[0], N * SampleSize * sizeof(uint8_t));
    ifsd.close();

    std::ifstream ifsl(LabelsFile.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!ifsl.is_open()) {
      std::cout << "Error loading MNIST Labels file no found : " << LabelsFile << std::endl;
      return false;
    }

    uint32_t NL;
    ifsl.read((char*) &magic, 4);
    ifsl.read((char*) &NL, 4);
    magic = reverse_bytes(magic);
    NL = reverse_bytes(NL);
    if (magic != 2049 || NL != N)  {
      printf("Error loading MNIST Labels file corrupted, magic = %d, NLabels = %d, NSamples = %d\n", magic, NL, N);
      return false;
    }
    printf("Loading Labels File '%s'\nN = %d\n", LabelsFile.c_str(), N);
    std::vector<uint8_t> Labels(N);
    ifsl.read((char*)&Labels[0], N * sizeof(uint8_t));
    ifsl.close();

    for (size_t i = 0; i < N; ++i) {
      SampleType* S = new SampleType(i, Labels[i], (uint8_t*)&Data[i * SampleSize], SampleSize, OutputSize);
      Samples.push_back(S);
    }

    generateInputs();
    generateOutputs();

    return true;
  }

  void generateInputs() {
    normalizeInputs(255.0f);
    computeMeanVector();
    subtractMeanInput();
  }

  void generateOutputs() {
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < OutputSize; ++j)
        Samples[i]->Output[j] = Samples[i]->Label == j ? 1 : -1;
  }

};


} // namespace ann

#endif // ANN_DATASET_H
