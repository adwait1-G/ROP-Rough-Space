#include"Gadgets.h"

enum ret{

    ret_near = 1,
    ret_far = 2, 
    ret_near_pop = 3,
    ret_far_pop = 4,
};

class Gadget {
    unsigned char ret_type;
    
};

std::vector<std::vector<cs_insn> > gadgets;


//========================================GetGadgets - Get all ret gadgets===================================//

int GetGadgets(unsigned char *retptr, unsigned long RetAddress, unsigned long N) {

    unsigned char *ptr = retptr;
    unsigned long Address = RetAddress;
    csh handle;
    cs_insn *insn; 
    unsigned long InstCount;

    if(cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        std::cerr<<"Error: Unable to open capstone handle"<<std::endl;
        return -1;
    }

    unsigned long count = 1;
    while(count <= N) {

        std::vector<cs_insn> gadget;
        
        InstCount = cs_disasm(handle, ptr, count, Address, 0, &insn);
        if(InstCount == 0) {
            //std::cerr<<"Error: Unable to disassemble - Inside GetGadgets()"<<std::endl;
            ptr--;
            Address--;
            //return -1;
            continue;
        }

        if(strcmp(insn[InstCount-1].mnemonic, "ret")== 0) {
            std::cout<<std::endl;
            // std::cout << "INTERESTING!" << std::endl;
            for(unsigned long j = 0; j < InstCount; j++) {
                printf("0x%lx: %s %s ", insn[j].address, insn[j].mnemonic, insn[j].op_str);
                gadget.push_back(insn[j]);
                printf("\n");
            }
        }
        if(gadget.size() > 0) {
            gadgets.push_back(gadget);
        }
        
        
            
        count++;
        ptr--;
        Address--;
    }

    cs_close(&handle);

    return 0;
}

//=================================GetAllGadgets: Driver function====================================//

int GetAllGadgets(unsigned char *inst, unsigned long TextSize, unsigned long EntryAddress, unsigned long N) {

    //Disassembly of the text section will go into a file named disass.dump. 
    std::cout<<"Disassembly of .text section is dumped in disass.dump"<<std::endl;
    FILE *fdisass;
    fdisass = fopen("disass.dump", "wb");
    if(fdisass == NULL) {
        std::cerr<<"Error: Unable to create file disass.dump"<<std::endl;
        std::cerr<<"Exiting..."<<std::endl;
        return -1;
    }
    

    //Capstone related variables. 
    csh handle;
    cs_insn *insn;
    unsigned long InstCount;

    //Initialize the handle
    if(cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        std::cerr<<"Error: Unable to open cs handle"<<std::endl;
        return -1;
    }
    
    InstCount = cs_disasm(handle,
                        inst,
                        TextSize,
                        EntryAddress,
                        0,
                        &insn);
    
    if(InstCount == 0) {
        std::cerr<<"Error: Unable to disassemble the .text section"<<std::endl;
        return -1;
    }

    for(unsigned long i = 0; i < InstCount; i++) {
        fprintf(fdisass, "0x%lx: %s %s %s\n", insn[i].address, insn[i].bytes, insn[i].mnemonic, insn[i].op_str);
    }

    fclose(fdisass);

    // Disassembly of .text section is done at this point.


    //The following code will aim to dump all ret gadgets present in the text section. 
    std::cout<<"All Gadgets will be dumped in gadgets.rop"<<std::endl;

    FILE *fretgg;
    fretgg = fopen("gadgets.rop", "wb");
    if(fretgg == NULL) {
        std::cerr<<"Error: Unable to create file gadgets.rop"<<std::endl;
        std::cerr<<"Exiting..."<<std::endl;
        return -1;
    }

    unsigned char *ptr1 = inst, *ptr2 = inst;
    unsigned long MovingAddress = EntryAddress, count = 0;
    unsigned char buffer[N];
    memset(buffer, '\0', sizeof(buffer));
/*
    for(unsigned long i = 0; i < TextSize; i++) {
        
        if(*ptr1 == 0xc3 || *ptr1 == 0xc2 || *ptr1 == 0xcb || *ptr1 == 0xca) {
            printf("count = %lu, Moving Address = 0x%lx\n", count, MovingAddress);
            count++;
        }
        ptr1++;
        MovingAddress++;
    }
*/

     for(unsigned long i = 0; i < TextSize; i++) {
         unsigned long OldAddress = MovingAddress;
        if(*ptr2 == 0xc3 || *ptr2 == 0xcb || *ptr2 == 0xc2 || *ptr2 == 0xca) {

            //printf("\nCount = %lu, Address: 0x%lx, rettype = %x\n", count, MovingAddress, *ptr1);
            GetGadgets(ptr2, MovingAddress, N);
            count++;
        }

         ptr2++;
         MovingAddress++;
     }



    cs_close(&handle);
}



