#include<fstream>
#include<iostream>
#include<cstdlib>
#include<stdint.h>

#define clear system("clear");

/*Data types declaration*/

struct bmp_header{      //file header data 
    char type[2];       //type of file
    int size;           //size of file
    short int r1;       //reserved
    short int r2;       //reserved
    int offset;         //position of image data
};
struct bmp_info{        //image header data
    int size;           //img header size
    int width;          //img width in px
    int height;         //img height
    short int plane;    //plane count
    short int bit_count;//bits per pixel
    int compression;    //compression alghoritm
    int img_size;       //image size
    int Xppm;           //x resolution
    int Yppm;           //y resolution
    int crl;            //no of colours 
    int crl_important;  //no of important colours
};
struct pixel{           //pixel, three colour values
    unsigned char B;    //blue
    unsigned char G;    //green
    unsigned char R;    //red
};

/*Function declarations*/

bool read_header(bmp_header*, char[]);                   //reads file header from bmp file
bool read_info(bmp_info&, char[]);                       //reads image header from bmp file
void  print_bmp_headers(bmp_header&, bmp_info&);        //prints header  information read from file
pixel** read_pixels(bmp_info&, bmp_header&, char[]);    //reads pixel data of image (all at once)   
bool bmp_write(bmp_header&, bmp_info&, pixel**, char[]); //writes provided headers and pixel data to new bmp file (all at once)
short*** read_masks(unsigned short&, unsigned short&, char[]);  //reads masks from a file, returns ptr to array of masks
bool apply_sobel_masks(bmp_info&, pixel**, short***, unsigned short, unsigned short);//applays sobel masks to provided array of pixels
void process_bmp();                                                                     //control function for processing bmp file
void display_help();                                                                    //displays helpful info in main menu  
bool read_write_steps(bmp_info&, bmp_header&, char[], char[]);                          //reading & writing file in small chunks
bool read_write_whole(bmp_info&, bmp_header&, char[], char[], char[], char);            //reading & writing whole file at once    
void print_img(int, int, pixel**);                                                      //debug helper


/*Function definitions*/

void print_img(int height, int width, pixel** data){
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            std::cout << static_cast<int>(data[i][j].B) << " ";
            std::cout << static_cast<int>(data[i][j].G) << " ";
            std::cout << static_cast<int>(data[i][j].R) << " ";
        }
        std::cout << std::endl;
    }
}

bool read_header(bmp_header* header, char file_name []){
    std::ifstream file;
    file.open(file_name, std::ios::binary);
    if(!file){
        std::cout << "blad otwarcia pliku\n";
        return 0;
    }
    file.read(header->type, 2);                             //reading information from header, one at a time
    file.read(reinterpret_cast<char*>(&(header->size)), 4);
    file.read(reinterpret_cast<char*>(&(header->r1)), 2);
    file.read(reinterpret_cast<char*>(&(header->r2)), 2);
    file.read(reinterpret_cast<char*>(&(header->offset)), 4);
    
    file.close();
    return 1;
}

bool read_info(bmp_info &info, char file_name []){
    std::ifstream file;
    file.open(file_name, std::ios::binary);
    if(!file){
        std::cout << "blad otwarcia pliku\n";
        return 0;
    }
    file.seekg(14, std::ios::beg);                             //skipping file header
    file.read(reinterpret_cast<char*>(&info), sizeof(info));   //size of info struct matches size of info header so I can read it all at once
    file.close();
    return 1;
}

void print_bmp_headers(bmp_header &header, bmp_info &info){
    std::cout << "Naglowek pliku: " << std::endl << std::endl;
    std::cout << "Typ: " << header.type[0] << header.type[1] << std::endl;
    std::cout << "Rozmiar pliku: " << header.size << std::endl;
    std::cout << "Pozycja danych obrazu: " << header.offset << std::endl << std::endl;
    
    std::cout << "Naglowek obrazu: " << std::endl << std::endl;
    std::cout << "Rozmiar naglowka obrazu: " << info.size << std::endl;
    std::cout << "Szerokosc obrazu (px): " << info.width << std::endl;
    std::cout << "Wysokosc obrazu (px): " << info.height << std::endl;
    std::cout << "Liczba platow: " << info.plane << std::endl;
    std::cout << "Ilosc bitow na pixel " << info.bit_count << std::endl;
    std::cout << "Rodzaj kompresjii: " << info.compression << std::endl;
    std::cout << "Rozmiar obrazu: " << info.img_size << std::endl;
    std::cout << "Rozdzielczosc pozioma: " << info.Xppm << std::endl;
    std::cout << "Rozdzielczosc pionowa: " << info.Yppm << std::endl;
    std::cout << "Liczba kolorow w palecie: " << info.crl << std::endl;
    std::cout << "Liczba warznych kolorow: " << info.crl_important << std::endl << std::endl;
}

pixel** read_pixels(bmp_info &info, bmp_header &header, char file_name []){
    std::ifstream file;
    file.open(file_name, std::ios::binary);
    if(!file){
        std::cout << "blad otwarcia pliku\n";
        return NULL;
    }
    file.seekg(header.offset, std::ios::beg);   //skipping to image data

    int row_size = (header.size - header.offset) / info.height;
    unsigned char* img = new unsigned char [row_size];          //array for row of pixels

    pixel** pixels = new pixel*[info.height];               //array for image
    for(int i = 0; i < info.height; i++)
        pixels[i] = new pixel[info.width];

    for(int i = 0; i < info.height; i++){
       file.read(reinterpret_cast<char*>(img), row_size);   //reading one row from image
       for(int j = 0, k = 0; j < info.width; j++, k+=3){    //writing that row to output array
            pixels[i][j].B = img[k];    
            pixels[i][j].G = img[k+1];
            pixels[i][j].R = img[k+2];
       }
    }

    file.close();
    delete[] img;
    img = NULL;
    return pixels ;
}

bool bmp_write(bmp_header &header, bmp_info &info, pixel** data, char file_name []){
    std::ofstream file_out;
    file_out.open(file_name, std::ios::binary);
    if(!file_out){
        std::cout << "blad otwarcia pliku\n";
        return 0;
    }

    int row_size = info.width * sizeof(pixel);                      //calculating row size from size of provided array of pixels
    if((info.width * sizeof(pixel))% 4)
        row_size = row_size + 4 - ((info.width * sizeof(pixel)) % 4);   
    unsigned char* data_row = new unsigned char[row_size];

    for(int i = 0; i < row_size; i++)                               //filling row with 0
        data_row[i] = 0;

    file_out.write(header.type, 2);                                 //writing headers to output file
    file_out.write(reinterpret_cast<char*> (&header.size), 4);
    file_out.write(reinterpret_cast<char*> (&header.r1), 2);
    file_out.write(reinterpret_cast<char*> (&header.r2), 2);
    file_out.write(reinterpret_cast<char*> (&header.offset), 4);
    file_out.write(reinterpret_cast<char*> (&info), sizeof(info));

    for(int i = 0; i < info.height; i ++){
        for(int j = 0, k = 0; j < info.width; j++, k+=3){           //writing row of provided pixels to row array
            data_row[k] = data[i][j].B;
            data_row[k + 1] = data[i][j].G;
            data_row[k + 2] = data[i][j].R;
        }
        file_out.write(reinterpret_cast<char*>(data_row), row_size);    //writing row to file
    }
    
    file_out.close();
    delete [] data_row;
    data_row = NULL;
    return 1;
}

void process_bmp(){
    bmp_header header;
    bmp_header* head_ptr = &header;
    bmp_info info;
    char input_file [20];
    char output_file [20];
    char masks_file [20];
    char mask_choice;
    char aaa;
    int mem_mode;

    clear
    std::cout << "Prosze podac nazwe pliku wejsciowego\n";
    std::cin >> input_file;
    std::cout << "Prosze podac nazwe dla pliku wyjsciowego\n";
    std::cin >> output_file;
    clear

    do{
        std::cout << "1 Wczytywanie calego obrazu na raz\n";
        std::cout << "2 Wczytywanie obrazu fragmentami\n";
        std::cout << "Prosze wybrac tryb pracy\n";
        std::cin >> mem_mode;
        clear
    }
    while(mem_mode != 1 && mem_mode != 2);

    do{
        std::cout << "Czy chcesz podac plik zawierajacy niestandardowe maski t/n\n";
        std::cin >> mask_choice;
        clear
    }while(mask_choice != 't' && mask_choice != 'n');

    if(mask_choice == 't'){
        std::cout << "Prosze podac nazwe pliku z maskami\n";
        std::cin >> masks_file;
        clear
    }

    read_header(head_ptr,input_file);
    read_info(info,input_file);
    print_bmp_headers(header, info);

    if(mem_mode == 1){
        read_write_whole(info, header, input_file, output_file, masks_file, mask_choice);
    }
    else{
        read_write_steps(info, header, input_file, output_file);
    }

    std::cout << "Gotowe, wcisnij dowolny klawisz i enter aby kontynuowac\n";
    std::cin >> aaa;
}

bool apply_sobel_masks(bmp_info &info, pixel** in_img, short*** mask = NULL, unsigned short no_masks = 8, unsigned short mask_size = 3){

    short mask_0[8][3][3] ={
        {
            { 1, 2, 1},
            { 0, 0, 0},
            {-1, -2, -1}
        },
        {
            { 2, 1, 0},
            { 1, 0, -1},
            { 0, -1, -2}
        },
        {
            { 1, 0, -1},
            { 2, 0, -2},
            { 1, 0, -1}
        },
        {
            { 0, -1, -2},
            { 1, 0, -1},
            { 2, 1, 0}
        },
        {
            {-1, -2, -1},
            { 0, 0, 0},
            { 1, 2, 1}
        },
        {
            {-2, -1, 0},
            {-1, 0, 1},
            { 0, 1, 2}
        },
        {
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        },
        {
             { 0, 1, 2},
            {-1, 0, 1},
            {-2, -1, 0}
        }
    };

    int sum_G = 0;
    int sum_B = 0;
    int sum_R = 0;
    int max = 255;
    unsigned short offset = (mask_size-1)/2;

    pixel** out_img = new pixel*[info.height];  //temporary array for output image
    for(int i = 0; i < info.height; i++)
        out_img[i] = new pixel[info.width];

    for(int i=0; i<info.height; i++){           //initializing array with 0
        for(int j=0; j<info.width; j++){
            out_img[i][j].G = 0;
            out_img[i][j].B = 0;
            out_img[i][j].R = 0;
        }
    }

    for(int i = offset; i < info.height - offset; i++){     //applying sobel masks to input array
        for(int j = offset; j < info.width - offset; j++){
            for(int8_t d = 0; d < no_masks; d++){
                sum_G = 0;
                sum_B = 0;
                sum_R = 0;
                for(int8_t a = -(offset); a < mask_size - offset; a++){
                    for(int8_t b = -(offset); b < mask_size - offset; b++){     
                        if(mask){                                                       //if custom mask pointer is valid use custom masks
                            sum_G = sum_G + in_img[i+a][j+b].G * mask[d][a+offset][b+offset];
                            sum_B = sum_B + in_img[i+a][j+b].B * mask[d][a+offset][b+offset];
                            sum_R = sum_R + in_img[i+a][j+b].R * mask[d][a+offset][b+offset];
                        }
                        else{                                                                   //if it is null use built in masks
                            sum_G = sum_G + in_img[i+a][j+b].G * mask_0[d][a+offset][b+offset];
                            sum_B = sum_B + in_img[i+a][j+b].B * mask_0[d][a+offset][b+offset];
                            sum_R = sum_R + in_img[i+a][j+b].R * mask_0[d][a+offset][b+offset];
                        }
                    }
                }
                if(sum_G > max) 
                    sum_G = max;
                if(sum_G < 0) 
                    sum_G = 0;
                if(sum_G > out_img[i][j].G)
                    out_img[i][j].G = sum_G;

                if(sum_B > max) 
                    sum_B = max;
                if(sum_B < 0) 
                    sum_B = 0;
                if(sum_B > out_img[i][j].B)
                    out_img[i][j].B = sum_B;

                if(sum_R > max) 
                    sum_R = max;
                if(sum_R < 0) 
                    sum_R = 0;
                if(sum_R > out_img[i][j].R)
                    out_img[i][j].R = sum_R;
            }
        }
    }

    for(int i=0; i<info.height; i++){           //assining output image array to input array
        for(int j=0; j<info.width; j++){
            in_img[i][j].G = out_img[i][j].G;
            in_img[i][j].B = out_img[i][j].B;
            in_img[i][j].R = out_img[i][j].R;
        }
    }   

    for(int i = 0; i < info.height; i++){       //deleting temporary output array
        delete[] out_img[i];
        out_img[i] = NULL;
    }
    delete[] out_img;
    out_img = NULL;

    if(mask){                                   //if custom masks provided deleting array of masks
        for(int i = 0; i < no_masks; i++){      
            for(int j = 0; j < mask_size; j++){
                delete[] mask[i][j];
                mask[i][j] = NULL; 
            }
            delete[] mask[i];
            mask[i] = NULL;
        }
        delete[] mask;
        mask = NULL;
    }
    return 1;
}

short*** read_masks(unsigned short &no_masks, unsigned short &mask_size, char file_name[20]){
    std::ifstream file;
    file.open(file_name);
    if(!file){
        std::cout << "blad otwarcia pliku\n";
        return NULL;
    }
    file >> no_masks >> mask_size;

    short*** masks = new short**[no_masks];     //making 3D array for masks
    for(int i = 0; i < no_masks; i++){
        masks[i] = new short*[mask_size];
        for(int j = 0; j < mask_size; j++){
            masks[i][j] = new short[mask_size];
        }
    }
    for(int i = 0; i < no_masks; i++){
        for(int j = 0; j < mask_size; j++){
            for(int k = 0; k < mask_size; k++){
                file >> masks[i][j][k];
            }
        }
    }
   return masks;
}

bool read_write_steps(bmp_info &info, bmp_header &header, char file_name [], char out_file_name []){
    std::ifstream file;
    file.open(file_name, std::ios::binary);
    if(!file){
        std::cout << "blad otwarcia pliku wejsciowego\n";
        return 0;
    }

    std::ofstream file_out;
    file_out.open(out_file_name, std::ios::binary);
    if(!file_out){
        std::cout << "blad otwarcia pliku wyjsciowego\n";
        return 0;
    }

    bmp_info dummy_info;        //dummy struct for sobel function
    dummy_info.height = 3;      //height is set to 3 because only 3 rows are processed at a time
    dummy_info.width = info.width;

    int row_size = (header.size - header.offset) / info.height;
    unsigned char* row = new unsigned char [row_size];              //array for reading and writing pixels to file rowe by row

    pixel** pixels = new pixel*[3];
    for(int i = 0; i < 3; i++)
        pixels[i] = new pixel[info.width];                          //array for read pixels size 3 x width of image

    file_out.write(header.type, 2);                                 //writing headers to output file
    file_out.write(reinterpret_cast<char*> (&header.size), 4);
    file_out.write(reinterpret_cast<char*> (&header.r1), 2);
    file_out.write(reinterpret_cast<char*> (&header.r2), 2);
    file_out.write(reinterpret_cast<char*> (&header.offset), 4);
    file_out.write(reinterpret_cast<char*> (&info), sizeof(info));
    
    file.seekg(header.offset, std::ios::beg);                           //skipping to pixels data in input image

    for(int l = 0; l < info.height - 2; l++){
        for(uint8_t i = 0; i < 3; i++){                                 //reading chunks of image 
            file.read(reinterpret_cast<char*>(row), row_size);
            for(int j = 0, k = 0; j < info.width; j++, k+=3){
                pixels[i][j].B = row[k];
                pixels[i][j].G = row[k+1];
                pixels[i][j].R = row[k+2];
            }
        }
        file.seekg(-2*row_size, std::ios::cur);                         //moving back two rows 
        apply_sobel_masks(dummy_info, pixels);                          //applying sobel masks to read portion of image

        for(int a = 0; a < row_size; a++)                               //preparing row array for writing to output image
            row[a] = 0;
        
        if(l == 0){                                                     //upon first iteration write first row of pixels (not processed by alghoritm)
            for(int a = 0, b = 0; a < info.width; a++, b+=3){
            row[b] = pixels[0][a].B;
            row[b + 1] = pixels[0][a].G;
            row[b + 2] = pixels[0][a].R;
            }
            file_out.write(reinterpret_cast<char*>(row), row_size);
        }

        for(int a = 0, b = 0; a < info.width; a++, b+=3){               //writing middle row of processed array to out imaga
            row[b] = pixels[1][a].B;        
            row[b + 1] = pixels[1][a].G;
            row[b + 2] = pixels[1][a].R;
        }

        file_out.write(reinterpret_cast<char*>(row), row_size);

        if(l == info.height - 3){                                       //upon last iteration write last row of pixels (also not processed)
            for(int a = 0, b = 0; a < info.width; a++, b+=3){
            row[b] = pixels[2][a].B;
            row[b + 1] = pixels[2][a].G;
            row[b + 2] = pixels[2][a].R;
            }
            file_out.write(reinterpret_cast<char*>(row), row_size);
        }
    }

    file.close();
    file_out.close();

    for(uint8_t i = 0; i < 3; i++){
        delete[] pixels[i];
        pixels[i] = NULL;
    }
    delete[] pixels;
    pixels = NULL;

    delete[] row;
    row = NULL;
    return 1;
}

bool read_write_whole(bmp_info &info, bmp_header &header, char input_file[], char output_file[], char masks_file[], char mask_choice){
    pixel** read_pix;                       // read pixels array
    short*** masks_ptr;                     // read custom masks array
    unsigned short no_masks, mask_size;

    read_pix = read_pixels(info, header, input_file);                     //outputs pointer to read array 

    if(mask_choice == 't'){                                                 //if custom masks provided read them
        masks_ptr = read_masks(no_masks, mask_size, masks_file);
        apply_sobel_masks(info, read_pix, masks_ptr, no_masks, mask_size);
    }
    else{                                                                      //if not use built in masks
        apply_sobel_masks(info, read_pix);
    }
    bmp_write(header, info, read_pix, output_file);
        
    for(int i = 0; i < info.height; i++){
        delete[] read_pix[i];
        read_pix[i] = NULL;
    }
    delete[] read_pix;
    read_pix = NULL;

    return 1;
}

void display_help(){
    char input;
    do{
        clear
        std::cout << "Program przyjmuje maski z pliku tekstowego o rozszerzeniu .txt\n";
        std::cout << "Struktora pliku przedstawia sie w nastepojacy sposob\n\n";
        std::cout << "Pierwszy znak to liczba oznaczająca ilosc masek w pliku\n";
        std::cout << "W nastepnej linii wielkosc maski (musi byc liczba parzysta)\n";
        std::cout << "Dane liczbowe masek, kolejne liczby oddzielone spacjami, kazdy wiersz w nowej linii\n\n";
        std::cout << "Aby wyjsc wcisnij q\n";
        std::cin >> input;
    }while(input != 'q');
}

int main(){
    
    char input;
    clear
    do{
        std::cout << "Program przeprowadza detekcje krawedzi operatorem sobela\n";
        std::cout << "1 Wybierz nazwe pliku\n";
        std::cout << "2 Wyswietl pomoc\n";
        std::cout << "3 Wyjscie\n";
        std::cout << "Prosze wybrac odpowiednia opcje\n";
        
        std::cin >> input;
        switch (input){
        case '1':
            process_bmp();
            break;
        case '2':
            display_help();
            break;
        default:
            break;
        }
        clear
    } while (input != '3');

    return 0;
}