/*
 * Bitmap.h
 * Namespace bm: Contains various functions for working with .BMP image format.
 */
#pragma once
#include <iostream>
#include <fstream>

typedef unsigned char byte;

namespace bm {
    struct Pixel {
        public:
            Pixel() : rgba(nullptr) {}
            Pixel(unsigned int bytesPerPixel)
                : bytesPerPixel(bytesPerPixel)
            {
                this->rgba = new byte[bytesPerPixel];
            }
            ~Pixel() {
                if (this->rgba) delete[] this->rgba;
            }
            inline void setRGBA(byte* rgba) { this->rgba = rgba; }
            inline byte* getRGBA() const { return this->rgba; }
            unsigned int getAverage() const {
                return (rgba[bytesPerPixel - 1] + rgba[bytesPerPixel - 2] + rgba[bytesPerPixel - 3]) / 3;
            }
        private:
            unsigned int bytesPerPixel;
            byte* rgba;
    };
    bool Array2dToBMP(uint32_t** array, const uint32_t& width, const uint32_t& height, const char* filename);
    bool AsciiArt(const std::string& filename);
};
