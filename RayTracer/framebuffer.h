#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

class Framebuffer
{
public:
	static constexpr size_t kBytesPerPixels = 3;

	Framebuffer(size_t width, size_t height)
		:mWidth(width), mHeight(height),
		mData((uint8_t*)malloc(width* height* kBytesPerPixels))
	{}

	~Framebuffer() { free(mData); }

	void setPixel(size_t row, size_t col, uint8_t r, uint8_t g, uint8_t b)
	{
		const size_t index = kBytesPerPixels * (row * mWidth + col);
		mData[index + 0] = b;
		mData[index + 1] = g;
		mData[index + 2] = r;
	}

	/**
	* Save framebuffer to a TGA file
	*/
	void saveToFile(const char* filepath)
	{
		FILE* file = fopen(filepath, "wb");
		assert(file);
		putc(0, file);
		putc(0, file);
		putc(2, file); /* uncompressed RGB */
		putc(0, file); putc(0, file);
		putc(0, file); putc(0, file);
		putc(0, file);
		putc(0, file); putc(0, file); /* X origin */
		putc(0, file); putc(0, file); /* y origin */
		putc((mWidth & 0x00FF), file);
		putc((mWidth & 0xFF00) / 256, file);
		putc((mHeight & 0x00FF), file);
		putc((mHeight & 0xFF00) / 256, file);
		putc(24, file); /* 24 bit bitmap */
		putc(0, file);
		fwrite(mData, kBytesPerPixels, mWidth * mHeight, file);
		fclose(file);
	}

	size_t width() const { return mWidth; }
	size_t height() const { return mHeight; }

private:
	uint8_t* mData;
	size_t		mWidth;
	size_t		mHeight;
};
