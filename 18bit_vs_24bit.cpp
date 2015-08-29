#include <stdio.h>
#include <stdint.h>

struct Image
{
    uint8_t *_texels; // 8
    size_t   _area  ; // 8
    uint16_t _width ; // 2
    uint16_t _height; // 2
    uint16_t _bpp   ; // 2

    void Alloc( int width, int height, int bpp )
    {
        _width  = width;
        _height = height;
        _bpp    = bpp;
        _area = width * height * (bpp >> 3);
        _texels = new uint8_t[ _area ];
    }

    void Free()
    {
        delete [] _texels;
    }

    uint8_t* Get( int x, int y )
    {
        return _texels + (y * _width * _bpp) + (x * _bpp);
    }
};


// ========================================================================
void BMP_WriteColor24bit( const char * filename, const uint8_t *texelsRGB, const int width, const int height )
{
    uint32_t headers[13]; // 54 bytes == 13 x int32
    FILE   * pFileSave;
    int x, y, i;

    // Stupid Windows BMP must have each scanline width padded to 4 bytes
    int      nExtraBytes = (width * 3) % 4;
    int      nPaddedSize = (width * 3 + nExtraBytes) * height;
    uint32_t nPlanes     =  1      ; // 1 plane
    uint32_t nBitcount   = 24 << 16; // 24-bit RGB; 32-bit packed for writing

    // Header: Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".
    headers[ 0] = nPaddedSize + 54;    // bfSize (total file size)
    headers[ 1] = 0;                   // bfReserved1 bfReserved2
    headers[ 2] = 54;                  // bfOffbits
    headers[ 3] = 40;                  // biSize BITMAPHEADER
    headers[ 4] = width;               // biWidth
    headers[ 5] = height;              // biHeight
    headers[ 6] = nBitcount | nPlanes; // biPlanes, biBitcount
    headers[ 7] = 0;                   // biCompression
    headers[ 8] = nPaddedSize;         // biSizeImage
    headers[ 9] = 0;                   // biXPelsPerMeter
    headers[10] = 0;                   // biYPelsPerMeter
    headers[11] = 0;                   // biClrUsed
    headers[12] = 0;                   // biClrImportant

    pFileSave = fopen(filename, "wb");
    if( pFileSave )
    {
        // Output Headers
        fprintf(pFileSave, "BM");
        for( i = 0; i < 13; i++ )
        {
           fprintf( pFileSave, "%c", ((headers[i]) >>  0) & 0xFF );
           fprintf( pFileSave, "%c", ((headers[i]) >>  8) & 0xFF );
           fprintf( pFileSave, "%c", ((headers[i]) >> 16) & 0xFF );
           fprintf( pFileSave, "%c", ((headers[i]) >> 24) & 0xFF );
        }

        // Stupid Windows BMP are written upside down
        for( y = height - 1; y >= 0; y-- )
        {
            const uint8_t* scanline = &texelsRGB[ y*width*3 ];
            for( x = 0; x < width; x++ )
            {
                // swizzle rgb -> brg
                uint8_t r = *scanline++;
                uint8_t g = *scanline++;
                uint8_t b = *scanline++;

                // Stupid Windows BMP are written BGR
                fprintf( pFileSave, "%c", b );
                fprintf( pFileSave, "%c", g );
                fprintf( pFileSave, "%c", r );
           }

           if( nExtraBytes ) // See above - BMP lines must be of lengths divisible by 4 bytes.
              for( i = 0; i < nExtraBytes; i++ )
                 fprintf( pFileSave, "%c", 0 );
        }

        fclose( pFileSave );
    }
}


int main()
{
    Image image;

    int s = 6;
    int t = 16; // 16 pixels/row
    int w = 256*s;
    int h = 8 * t; // 2xRGBW * pixels/row
    image.Alloc( 2, 1, 24 );

    uint8_t *p = image.Get( 0, 0 );
    uint8_t *d = p;

    int y, x;
    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = x/s; // red
            *d++ = 0;
            *d++ = 0;
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = x/s & ~3; // red
            *d++ = 0;
            *d++ = 0;
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = 0;
            *d++ = x/s; // green;
            *d++ = 0;
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = 0;
            *d++ = x/s & ~3; // green
            *d++ = 0;
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = 0;
            *d++ = 0;
            *d++ = x/s; // blue
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = 0;
            *d++ = 0;
            *d++ = x/s & ~3; // blue
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            *d++ = x/s; // white
            *d++ = x/s; // white
            *d++ = x/s; // white
        }

    for( y = 0; y < t; y++ )
        for( x = 0; x < w; x++ )
        {
            uint8_t k = x/s & ~3;
            *d++ = k; // white
            *d++ = k; // white
            *d++ = k; // white
        }

    BMP_WriteColor24bit( "18bit_vs_24bit.bmp", p, w, h );

    image.Free();

    return 0;
}

