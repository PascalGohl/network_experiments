/*
 * DevMem.h
 *
 *  Created on: Jul 31, 2013
 *      Author: pascal
 */

#ifndef DEVMEM_H_
#define DEVMEM_H_

class DevMem {
public:
	DevMem();
	DevMem(off_t offset, size_t size);
	virtual ~DevMem();

	int write(off_t offset, size_t length, unsigned char * data);
	int write_byte(off_t offset, char data);
	int write_word(off_t offset, off_t data);
	int read(off_t offset, size_t length, unsigned char * data);
	char read_byte(off_t offset);
	off_t read_word(off_t offset);

private:
	void open_dev(off_t base_address, size_t size);

private:
	int memfd;
	void *mapped_dev_base;
	void *mapped_base;
	size_t map_size;
};

#endif /* DEVMEM_H_ */
