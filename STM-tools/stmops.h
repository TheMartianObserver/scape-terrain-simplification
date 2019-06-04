/*
 * This code is mean to be a simple interface for programs
 * that want to do simple manipulations of STM terrain files.
 *
 */

int stmMatchOrder(char orderBytes[4]);
void stmSwapOrder(unsigned short *data, int length);
void stmReadHeader(FILE *in, int *width, int *height, int *shouldSwap);

void stmWriteHeader(FILE *out,int width, int height);
void stmWriteData(FILE *out, unsigned short *data, int length);

typedef struct {
    unsigned short *data;
    int width, height;
} STMdata;

#define stmRef(stm,i,j) ((stm)->data[(j)*(stm)->width + (i)])

STMdata *stmRead(FILE *);
void stmWrite(FILE *, STMdata *);
