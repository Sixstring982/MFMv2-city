#define _BSD_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {false, true} bool;

#define BUFFER_LENGTH 1024

typedef struct {
  int epoch;
  int totalGas;
  float averageUsage;
  float percentArrived;
  int arrived;
  int outOfGas;
}NumRow;

void clearNumRow(NumRow* numRow) {
  numRow->epoch = numRow->totalGas =
    numRow->arrived = numRow->outOfGas = 0;

  numRow->averageUsage = numRow->percentArrived = 0.0f;
}

void fillNumRow(NumRow* numRow, const char* row) {
  sscanf(row, "%d %d %f %f %d %d", &(numRow->epoch),
	 &(numRow->totalGas), &(numRow->averageUsage),
	 &(numRow->percentArrived), &(numRow->arrived),
	 &(numRow->outOfGas));
}

void printNumRow(NumRow* numRow, FILE* outStream) {
  fprintf(outStream, "%d %d %f %f %d %d", numRow->epoch,
	 numRow->totalGas, numRow->averageUsage,
	 numRow->percentArrived, numRow->arrived,
	 numRow->outOfGas);
}

void averageNumRows(NumRow* toFill, NumRow* inputs, int count) {
  int i = 0;
  NumRow total;
  clearNumRow(&total);
  for (i = 0; i < count; i++) {
    total.epoch += inputs[i].epoch;
    total.totalGas += inputs[i].totalGas;
    total.averageUsage += inputs[i].averageUsage;
    total.percentArrived += inputs[i].percentArrived;
    total.arrived += inputs[i].arrived;
    total.outOfGas += inputs[i].outOfGas;
  }

  total.epoch /= 10;
  total.totalGas /= 10;
  total.averageUsage /= 10;
  total.percentArrived /= 10;
  total.arrived /= 10;
  total.outOfGas /= 10;

  *toFill = total;
}

FILE* openLocalFile(const char* mode, const char* format, ...) {
  char buffer[BUFFER_LENGTH];
  FILE* fp;
  char* suffix;
  int prefixLen;
  va_list ap;
  va_start(ap, format);
  snprintf(buffer, BUFFER_LENGTH, "/nfs/student/t/tsmall1/Documents/Git/MFMv2-city/"
	                          "tools/collector/");
  suffix = buffer + (prefixLen = strlen(buffer));
  vsnprintf(suffix, BUFFER_LENGTH - prefixLen, format, ap);
  va_end(ap);
  fp = fopen(buffer, mode);
  if (!fp) {
    perror("fopen: ");
    exit(1);
  }
  return fp;
}

void openFiles(FILE** fp, const char* prefix, int count) {
  int i;
  for (i = 0; i < count; i++) {
    fp[i] = openLocalFile("r", "mfm-city-%s-%i.dat", prefix, i + 1);
  }
}

void closeFiles(FILE** fp, int count) {
  int i;
  for (i = 0; i < count; i++) {
    fclose(fp[i]);
  }
}

void readLine(char* buffer, FILE* input) {
  int readC = fgetc(input);
  while ((readC != '\n') &&
	 (readC != EOF)) {
    *(buffer++) = readC;
    readC = fgetc(input);
  }
  *buffer = 0;
}

bool averageNextLines(FILE** inputs, FILE* output, int count) {
  NumRow inputRows[1024];
  NumRow average;
  char buffer[1024];
  bool atEof = true;
  int i;
  for (i = 0; i < count; i++) {
    readLine(buffer, inputs[i]);
    fillNumRow(inputRows + i, buffer);
    if (atEof) {
      atEof = !feof(inputs[i]);
    }
  }

  averageNumRows(&average, inputRows, count);
  printNumRow(&average, output);
  fputc('\n', output);
  return atEof;
}

void averageAndPrint() {
  int i;
  const char* prefixes[] = {"NOT_USED", "RANDOM_ROUTING", "CANAL_ROUTING"};
  const int PREFIX_COUNT = 3;
  for (i = 0; i < PREFIX_COUNT; i++) {
    FILE* files[10];
    FILE* output = openLocalFile("w", "%s-average.dat", prefixes[i]);
    openFiles(files, prefixes[i], 10);

    while(averageNextLines(files, output, 10));

    closeFiles(files, 10);
  }
}

int main (int argc, char** argv) {
  averageAndPrint();
  return 0;
}
