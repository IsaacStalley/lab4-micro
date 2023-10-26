
#ifndef _SPI_MEMS_H
#define _SPI_MEMS_H

void write_reg(uint8_t reg, uint8_t value);
void read_xyz(int16_t vecs[3]);
void mems_init(void);

#define SSF_250 9
#define SSF_2000 70
#endif