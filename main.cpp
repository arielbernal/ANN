#include <ann_dataset.h>

int main() {
  printf("Testing\n");
  ann::MNISTDataset dataset("./Data/MNIST/train-images.idx3-ubyte", "./Data/MNIST/train-labels.idx1-ubyte");
}

