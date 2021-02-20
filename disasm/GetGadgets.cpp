#include<bits/stdc++.h>
#include<sys/mman.h>
#include<unistd.h>
#include<fcntl.h>
#include<elf.h>

// To use the capstone framework.
#include<capstone/capstone.h>

// This has routines to collect all gadgets
#include "Gadgets.h"


int main(int argc, char **argv) {

    if(argc != 3) {
        std::cerr<<"$ "<<argv[0]<<" <BinaryName> <ConfigurableN>"<<std::endl;
        return -1;
    }

    // 1. Open the Binary
    // 2. Map it onto the memory
    // 3. Find the Entry Address 

    char *BinaryName = argv[1];
    unsigned long SizeofBinary = 0;
    unsigned long EntryAddress, TextAddress;
    unsigned char *start;
    FILE *fp;
    int fd;
    std::string TextSizeStr;
    unsigned long int TextSize;
    unsigned long int ConfN = atoi(argv[2]);

    // Finding size of the binary. 
    // 1. Using fopen and fclose() - reading the while file once - slow. TODO: 2. Should devise a faster method. 

    fp = fopen(BinaryName, "rb");
    if(fp == NULL) {
        std::cerr<<"Error: Unable to open the file"<<std::endl;
        return -1;
    }

    while(fgetc(fp) != EOF)
        SizeofBinary++;

    fclose(fp);

    // Done finding size of the binary //

    // Find Entry Address and Size of .text section // 
    
    fd = open(BinaryName, O_RDONLY, S_IRUSR);
    if(fd == -1) {
        std::cerr<<"Error: Unable to open file while finding entry address"<<std::endl;
        return -1;
    }

    start = (unsigned char *)mmap(NULL,
                                SizeofBinary,
                                PROT_READ, 
                                MAP_PRIVATE, 
                                fd,
                                0);
    
    if(start == MAP_FAILED) {
        std::cerr<<"Error: Unable the map the ELF header onto memory"<<std::endl;
        return -1;
    }


    Elf64_Ehdr *elf_hdr = (Elf64_Ehdr *)start;
    EntryAddress = (unsigned long int)elf_hdr->e_entry;
    
    if((char)elf_hdr->e_type == ET_EXEC)
        TextAddress = EntryAddress - 0x400000;
    
    else if((char)elf_hdr->e_type == ET_DYN)
        TextAddress = EntryAddress;
    
    /*
    munmap(start, sizeof(Elf64_Ehdr));
    */
    close(fd);


    // Done finding the Entry Address //

    // Find the size of .text section

    //1. Finding the size of .text segment. 
    //2. Using the size command, popen() function. 
    //3. Parsing the output and getting size. 
    //4. This is not the right way to be done. 
     
    //TODO: Properly parse the ELF file and get the size. 
     
    //5. This is just a stop-gap arragement. 
     
    
    char command[50];
    sprintf(command, "size %s", BinaryName);

    fp = popen(command, "r");
    if(fp == NULL) {
        std::cerr<<"Error: Unable to execute the command: "<<command<<std::endl;
        return -1;
    }

    char ch = fgetc(fp);
    while(ch == ' ' || isalpha(ch) || ch == '\n' || ch == '\t') {
        ch = fgetc(fp);
    }

    TextSizeStr += ch;
    while(ch != ' ') {
        ch = fgetc(fp);
        TextSizeStr += ch;
    }

    TextSize = std::stoul(TextSizeStr);
    fclose(fp);
    // Done finding size of .text segment //


    std::cout<<"Binary: "<<BinaryName<<std::endl;
    std::cout<<"Entry Address: 0x"<<std::hex<<EntryAddress<<std::endl<<std::dec;
    std::cout<<"Size of .text section: "<<TextSize<<" bytes"<<std::endl;

    // Get all gadgets!

    unsigned char *inst = (unsigned char *)(start + TextAddress);
    if(GetAllGadgets(inst, TextSize, EntryAddress, ConfN) == -1) {
        std::cerr<<"Error: GetGadgets() routine failed for some reason"<<std::endl;
        return -1;
    }

    return 0;
}