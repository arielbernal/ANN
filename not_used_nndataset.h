#ifndef NNDATASET
#define NNDATASET

#include <vector>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <random>
#include <algorithm>

template <typename DataType = double,
          typename FileDataType = uint8_t,
          typename LabelType = uint8_t>
struct NNSample {
  NNSample(size_t DataSize,
           size_t OutputSize,
           size_t id,
           LabelType label,
           FileDataType* data)
      : Id(id), Label(label), MSE(0), Error(false) {
    Data = new FileDataType[DataSize];
    Input = new DataType[DataSize];
    Output = new DataType[OutputSize];
    for (size_t j = 0; j < DataSize; ++j)
      Data[j] = data[j];
  }

  NNSample(size_t DataSize, size_t OutputSize)
      : Id(0), Label(0), MSE(0), Error(false) {
    Data = new FileDataType[DataSize];
    Input = new DataType[DataSize];
    Output = new DataType[OutputSize];
  }

  ~NNSample() {
    delete Data;
    delete Input;
    delete Output;
  }

  FileDataType* Data;
  DataType* Input;
  DataType* Output;
  size_t Id;
  LabelType Label;
  DataType MSE;
  bool Error;
};

template <typename DataType = double,
          typename FileDataType = uint8_t,
          typename LabelType = uint8_t>
class NNDataset {
 public:
  typedef NNSample<DataType, FileDataType, LabelType> SampleType;
  typedef typename std::vector<
      NNSample<DataType, FileDataType, LabelType>*>::iterator Iterator;

  NNDataset(size_t Rows, size_t Cols, size_t OutputSize)
      : MeanSample(0),
        N(0),
        Size(Rows * Cols),
        Cols(Cols),
        Rows(Rows),
        OutputSize(OutputSize),
        CurrentId(0) {}

  NNDataset(size_t Size, size_t OutputSize)
      : MeanSample(0),
        N(0),
        Size(Size),
        Cols(Size),
        Rows(1),
        OutputSize(OutputSize),
        CurrentId(0) {}

  size_t getN() { return Filter.size(); }
  size_t getSize() { return Size; }
  size_t getRows() { return Rows; }
  size_t getCols() { return Cols; }
  size_t getOutputSize() { return OutputSize; }
  size_t getNSamples() { return N; }

  Iterator begin() { return Filter.begin(); }
  Iterator end() { return Filter.end(); }

  bool load(size_t x0,
            size_t n,
            const std::string& DataFile,
            const std::string& LabelsFile,
            size_t DataOffset = 0,
            size_t LabelsOffset = 0) {
    std::vector<FileDataType> Data(n * Size);
    std::vector<LabelType> Labels(n);

    std::ifstream ifsd(DataFile.c_str(),
                       std::ifstream::in | std::ifstream::binary);
    if (!ifsd.is_open()) {
      std::cout << "Error Data file no found : " << DataFile << std::endl;
      return false;
    }
    ifsd.seekg(DataOffset + x0 * Size * sizeof(FileDataType));
    ifsd.read((char*)&Data[0], n * Size * sizeof(FileDataType));
    ifsd.close();

    std::ifstream ifsl(LabelsFile.c_str(),
                       std::ifstream::in | std::ifstream::binary);
    if (!ifsl.is_open()) {
      std::cout << "Error Labels file no found : " << LabelsFile << std::endl;
      return false;
    }
    ifsl.seekg(LabelsOffset + x0 * sizeof(uint8_t));
    ifsl.read((char*)&Labels[0], n * sizeof(uint8_t));
    ifsl.close();

    N = n;

    Samples.resize(N);
    Filter.resize(N);

    for (size_t i = 0; i < N; ++i) {
      SampleType* S = new SampleType(
          Size, OutputSize, i, Labels[i], (uint8_t*)&Data[i * Size]);
      Samples[i] = S;
      Filter[i] = S;
    }

    generateOutputs();
    return true;
  }

  void preProcessingInputs(const SampleType* Sample = 0) {
    normalizeInputs();
    if (!Sample)
      computeMeanVector();
    else
      setMeanSample(Sample);
    subtractMeanInput();
  }

  SampleType* getMeanSample() { return MeanSample; }

  void setMeanSample(const SampleType* Sample) {
    if (MeanSample)
      delete MeanSample;
    MeanSample = new SampleType(Size, OutputSize);
    for (size_t i = 0; i < Size; ++i)
      MeanSample->Input[i] = Sample->Input[i];
  }

  void computeMeanVector() {
    if (MeanSample)
      delete MeanSample;
    MeanSample = new SampleType(Size, OutputSize);
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < Size; ++j)
        MeanSample->Input[j] += Samples[i]->Input[j];
    for (size_t j = 0; j < Size; ++j)
      MeanSample->Input[j] /= N;
  }

  void generateOutputs() {
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < OutputSize; ++j)
        Samples[i]->Output[j] = Samples[i]->Label == j ? 1 : -1;
  }

  void normalizeInputs() {
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < Size; ++j)
        Samples[i]->Input[j] = DataType(Samples[i]->Data[j]) / 256.0f;
  }

  void subtractMeanInput() {
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < Size; ++j)
        Samples[i]->Input[j] -= MeanSample->Input[j];
  }

  void addNoiseNormal(double sigma = 64) {
    std::default_random_engine generator;
    std::normal_distribution<double> normal(0, sigma);
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < Size; ++j) {
        int v = Samples[i]->Data[j] + int(normal(generator));
        if (v < 0)
          v = 0;
        else if (v > 255)
          v = 255;
        Samples[i]->Data[j] = v;
      }
  }

  void addNoiseSaltPepper(double P = 0.1) {
    std::default_random_engine generator;
    std::uniform_int_distribution<size_t> d1(0, Size);
    std::uniform_int_distribution<size_t> d2(0, 1);
    for (size_t i = 0; i < N; ++i)
      for (size_t j = 0; j < Size; ++j)
        if (d1(generator) < Size * P) {
          if (d2(generator) > 0)
            Samples[i]->Data[j] = 255;
          else
            Samples[i]->Data[j] = 0;
        }
  }

  void blurXY() {
    int radius = 1;
    int st = (2 * radius + 1) * (2 * radius + 1);
    for (size_t i = 0; i < N; ++i) {
      for (size_t y = 0; y < Rows; ++y) {
        for (size_t x = 0; x < Cols; ++x) {
          int y0 = y >= radius ? y - radius : 0;
          int y1 = y + radius < Rows ? y + radius + 1 : Rows;
          int x0 = x >= radius ? x - radius : 0;
          int x1 = x + radius < Cols ? x + radius + 1 : Cols;
          float s = 0;
          int nn = 0;
          for (int yr = y0; yr < y1; ++yr)
            for (int xr = x0; xr < x1; ++xr) {
              s += Samples[i]->Data[yr * Cols + xr];
              nn++;
            }
          if (nn < 4)
            std::cout << "Errrprrrrrrr " << y0 << " " << y1 << " " << x0 << " "
                      << x1 << std::endl;
          s /= nn;
          Samples[i]->Data[y * Cols + x] = s;
        }
      }
    }
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

 private:
  std::vector<SampleType*> Samples;
  std::vector<SampleType*> Filter;
  SampleType* MeanSample;

  size_t N;
  size_t Size;
  size_t Cols;
  size_t Rows;
  size_t OutputSize;
  size_t CurrentId;
};

#endif  // NNDATASET
