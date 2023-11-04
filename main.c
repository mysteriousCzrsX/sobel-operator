#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

typedef enum {
    BM = 0x4D42, // Windows 3.1x, 95, NT, ... etc.
    BA = 0x4142, // OS/2 struct bitmap array
    CI = 0x4943, // OS/2 struct color icon
    CP = 0x5043, // OS/2 const color pointer
    IC = 0x4349, // OS/2 struct icon
    PT = 0x5450  // OS/2 pointer
} BMP_HeaderID;

#pragma pack(1)
typedef struct bmp_header{
	BMP_HeaderID type;           // 2 bytes
    int32_t fileSize;           // 4 bytes
    int16_t reserved1;          // 2 bytes
    int16_t reserved2;          // 2 bytes
    int32_t pixelDataOffset;    // 4 bytes 
	}BMP_Header;

#pragma pack(1)
typedef struct info_header{
	int32_t infoHeaderSize; 
	int32_t bmpWidth;
	int32_t bmpHeight;
	int16_t numberOfPlanes;
	int16_t bitsPerPixel;
	int32_t Compression; //program work only for no compressed bmp
	int32_t compressedSize; //It is valid to set this :0 if compression = 0
	int32_t XpixelsPerM;
	int32_t YpixelsPerM;
	int32_t colorsUsed;
	int32_t colorsImportant; // 0 = all
	 }INFO_HEADER;
 bool read_bmd_header (BMP_Header*,char*);
 bool read_info_header(INFO_HEADER*,char*);
 uint32_t convert_to_little_endian(unsigned char*);
 
 uint32_t convert_to_little_endian(unsigned char* buffer){
	 return (uint32_t)(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3]);
	 }
 

 
 bool read_bmd_header(BMP_Header* header, char* filename){
	  
	FILE* fp = fopen(filename, "rb");
	if(!fp){
		return 0;
		}

	unsigned char buffer[4];  // buffer to read bytes

	// Read type (2 bytes)
	fread(buffer, 1, 2, fp);
	header->type = (BMP_HeaderID)(convert_to_little_endian(buffer));
	// Read file size (4 bytes)
	fread(buffer, 1, 4, fp);
	header->fileSize = convert_to_little_endian(buffer);
	fseek(fp, 4, SEEK_CUR);
	// Read pixelDataOffset (4 bytes)
	fread(buffer, 1, 4, fp);
	header->pixelDataOffset =convert_to_little_endian(buffer);
	fclose(fp);
	return 1;
	}

bool read_info_header(INFO_HEADER* info , char* filename){
	FILE* fp = fopen(filename,"rb");
	if(!fp){
		return 0;
		}
	unsigned char buffer[4]; //buffer to read bytes
	
	fseek(fp, 14, SEEK_CUR); //skip BMP HEADER
	fread(buffer, 1, 4, fp);
	info->infoHeaderSize = convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->bmpWidth = convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->bmpHeight = convert_to_little_endian(buffer);
	fread(buffer, 1, 2, fp);
	info->numberOfPlanes =  convert_to_little_endian(buffer);
	fread(buffer, 1, 2, fp);
	info->bitsPerPixel =  convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->Compression =  convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->compressedSize =  convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->XpixelsPerM = convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->YpixelsPerM =  convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->colorsUsed =  convert_to_little_endian(buffer);
	fread(buffer, 1, 4, fp);
	info->colorsImportant =  convert_to_little_endian(buffer);
	return 1;
	}

struct pixel{           //pixel colour values
    unsigned char B;    //blue
    unsigned char G;    //green
    unsigned char R;    //red
};

int main(int argc, char *argv[]) {
	BMP_Header header; 
	INFO_HEADER info;
	char* fileName = "a.bmp";
	read_bmd_header(&header,fileName);
	read_info_header(&info,fileName);
	printf("Type: %x\n", header.type);
	printf("File Size: %" PRId32 "\n", header.fileSize);
	printf("Pixel Data Offset: %" PRId32 "\n", header.pixelDataOffset);
	printf("infoHeaderSize: %"PRId32"\n",info.infoHeaderSize);
	printf("bmpWidth: %"PRId32"\n",info.bmpWidth);
	printf("bmpHeight: %"PRId32"\n",info.bmpHeight);
	printf("numberOfPlanes: %"PRId16"\n",info.numberOfPlanes);
	printf("bitsPerPixel: %"PRId16"\n",info.bitsPerPixel);
	printf("Compression: %"PRId32"\n",info.Compression);
	printf("compressedSize: %"PRId32"\n",info.compressedSize);
	printf("XpixelPerM: %"PRId32"\n",info.XpixelsPerM);
	printf("YpixelPerM: %"PRId32"\n",info.YpixelsPerM);
	printf("colorUsed: %"PRId32"\n",info.colorsUsed);
	printf("colorsImportant: %"PRId32"\n",info.colorsImportant);
	return 0;
}
