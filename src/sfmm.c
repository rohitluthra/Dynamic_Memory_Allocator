/**
* All functions you make for the assignment must be implemented in this file.
* Do not submit your assignment with a main function in this file.
* If you submit with a main function in this file, you will get a zero.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "debug.h"
#include "sfmm.h"
#include "functions.h"

#define BLOCK_SIZE_MASK_16 0xfffffffffffffffc
#define BLOCK_SIZE_MASK_16_D 0xfffffffffffffffd
#define BLOCK_SIZE_MASK_16_E 0xfffffffffffffffe
#define ALLOCATING 0x3
#define ALLOCATING_1 1
#define ALLOCATING_2 2

#define HEADERPLUSFOOTER_8 8
#define HEADERPLUSFOOTER 16
#define HEADERPLUSFOOTER_32 32
#define BASE_SIZE_4049 4049
#define BASE_SIZE_4096 4096

// int getindex(int block_size);
// void coalesce(void *voidptr);
// int calculate(int incoming, size_t size);
// int getPositionForSF (int incoming);

int calculate (int incoming, size_t size)
{
    int b_sizeee = incoming;
    int headPlusFoot = size + HEADERPLUSFOOTER;
    int remain = headPlusFoot%HEADERPLUSFOOTER;
    int quot = headPlusFoot/HEADERPLUSFOOTER;
    if(headPlusFoot<HEADERPLUSFOOTER_32)
        b_sizeee=HEADERPLUSFOOTER_32;
    else
    {
        if(remain!=0)
            b_sizeee = quot*HEADERPLUSFOOTER + HEADERPLUSFOOTER;
        else
            b_sizeee = headPlusFoot;
    }
    return b_sizeee;
}

void *sf_malloc(size_t size) {

    if(size<=0)
        return NULL;

    int b_sizeee = 0;
    b_sizeee = calculate(b_sizeee, size);

    int index= getPositionForSF(b_sizeee);
    bool mem_init = (sf_mem_end()==sf_mem_start());

    if(mem_init)
    {
        int count =0;
        while(count!=NUM_FREE_LISTS )
        {
            sf_free_list_heads[count].body.links.prev = &sf_free_list_heads[count];
            sf_free_list_heads[count].body.links.next = &sf_free_list_heads[count];
            count++;
        }

        void *pointerToMemGrow = sf_mem_grow();
        if(!pointerToMemGrow)
        {
            sf_errno = ENOMEM;
            return NULL;
        }

// Prologue Starts.
        struct sf_prologue mainPro;
        sf_prologue *mainProloguePointer = (sf_prologue *)sf_mem_start();
        mainPro.header = 32;
        mainPro.header = mainPro.header +3;
        mainPro.footer = mainPro.header ^ sf_magic();
        *mainProloguePointer = mainPro;
        mainProloguePointer+=1;

//Header Starts.
        void *mainPrologueFooterPointer = (void *)mainProloguePointer;
        mainPrologueFooterPointer = mainPrologueFooterPointer - HEADERPLUSFOOTER_8;

        sf_block newBlock;
        newBlock.prev_footer = mainPro.footer;
        newBlock.header= BASE_SIZE_4049;
        sf_block *temp_3 = &sf_free_list_heads[NUM_FREE_LISTS-2];
        newBlock.body.links.next = temp_3;
        newBlock.body.links.prev = temp_3;

        sf_block *newBlockPointer;
        newBlockPointer = (sf_block *) mainPrologueFooterPointer;
        *newBlockPointer = newBlock;
        index = NUM_FREE_LISTS-2;

        sf_free_list_heads[index].body.links.next = (sf_block *)newBlockPointer;
        sf_free_list_heads[index].body.links.prev = (sf_block *)newBlockPointer;
       // }

//Epilogue Starts.

        sf_epilogue *mainEpiloguePointer = (sf_epilogue *)sf_mem_end();
        mainEpiloguePointer = mainEpiloguePointer - 1;
        struct sf_epilogue mainEpi;
        mainEpi.header = 2;
        *mainEpiloguePointer = mainEpi;

//Footer for header above starts.

        sf_footer *footerForHeaderAbovePointer = (sf_footer *)mainEpiloguePointer;
        footerForHeaderAbovePointer = (sf_footer *)mainEpiloguePointer-1 ;
        sf_footer footerForHeaderAbove = ((*newBlockPointer).header) ^ sf_magic();
        *footerForHeaderAbovePointer = footerForHeaderAbove;
    }

    index = getPositionForSF(b_sizeee);

    vaapis_bc:
    while( &sf_free_list_heads[index] == sf_free_list_heads[index].body.links.next )
    {
        if(index<NUM_FREE_LISTS)
            index+=1;
    }

    if(index<NUM_FREE_LISTS)
    {
        sf_block *blockAtIndex = sf_free_list_heads[index].body.links.next;

        while( blockAtIndex != &sf_free_list_heads[index] &&((*blockAtIndex).header & BLOCK_SIZE_MASK_16) < b_sizeee )
        {
            blockAtIndex = (*blockAtIndex).body.links.next;
        }
        if(blockAtIndex == &sf_free_list_heads[index])
        {
            index+=1;
            goto vaapis_bc;
        }

        else if(((*blockAtIndex).header & BLOCK_SIZE_MASK_16) >= b_sizeee)
        {
            if ( HEADERPLUSFOOTER_32 <= ((*blockAtIndex).header & BLOCK_SIZE_MASK_16) - b_sizeee )
            {
                sf_block new_block_Split;
                new_block_Split.prev_footer = (*blockAtIndex).prev_footer;
                new_block_Split.header = b_sizeee + 3;

                sf_block split_block_free;
                split_block_free.prev_footer = new_block_Split.header ^ sf_magic();
                split_block_free.header = ((*blockAtIndex).header & BLOCK_SIZE_MASK_16) - b_sizeee + 1;

                // = ((blockAtIndex)->header & BLOCK_SIZE_MASK_16) - b_sizeee;

                ((((blockAtIndex)->body.links.next))->body.links.next) = (blockAtIndex)->body.links.next;
                ((((blockAtIndex)->body.links.next))->body.links.prev) = (blockAtIndex)->body.links.prev;

                index = getPositionForSF(((blockAtIndex)->header & BLOCK_SIZE_MASK_16) - b_sizeee);

                void *its_temp_void_var = (void *)blockAtIndex;
                its_temp_void_var = its_temp_void_var + b_sizeee;

                sf_block *split_block_free_Pointer;
                split_block_free_Pointer = (sf_block *)its_temp_void_var;
                *split_block_free_Pointer = split_block_free;

                its_temp_void_var  = its_temp_void_var - b_sizeee;

                sf_block *new_Block_Split_Pointer;
                new_Block_Split_Pointer = (sf_block *)its_temp_void_var;
                *new_Block_Split_Pointer = new_block_Split;
                its_temp_void_var = b_sizeee + its_temp_void_var;

                its_temp_void_var+= (((*split_block_free_Pointer).header & BLOCK_SIZE_MASK_16 ));


                if(its_temp_void_var!= (sf_mem_end() - HEADERPLUSFOOTER))
                    (*((sf_block*)its_temp_void_var)).prev_footer=(*split_block_free_Pointer).header^sf_magic();

                else
                    *((sf_footer*)its_temp_void_var)=(*split_block_free_Pointer).header^sf_magic();


                sf_block *current_index_next = sf_free_list_heads[index].body.links.next;
                sf_block *current_index = &sf_free_list_heads[index];

                (*split_block_free_Pointer).body.links.next = current_index_next;
                (*split_block_free_Pointer).body.links.prev = current_index;

                (*(sf_free_list_heads[index].body.links.next)).body.links.prev=split_block_free_Pointer;

                sf_free_list_heads[index].body.links.next=split_block_free_Pointer;
                return (*new_Block_Split_Pointer).body.payload;
            }
            else
            {
                (*blockAtIndex).header=((*blockAtIndex).header & BLOCK_SIZE_MASK_16_D)+2;
                void* its_temp_void_var=(void*)blockAtIndex;

                its_temp_void_var+=((*blockAtIndex).header & BLOCK_SIZE_MASK_16);

                if((sf_mem_end() - HEADERPLUSFOOTER)==its_temp_void_var){
                    *((sf_footer*)its_temp_void_var)=(*blockAtIndex).header^sf_magic();
                    its_temp_void_var = HEADERPLUSFOOTER_8 + its_temp_void_var;
                    (*((sf_epilogue*)its_temp_void_var)).header=((*((sf_epilogue*)its_temp_void_var)).header & BLOCK_SIZE_MASK_16) + 3;
                    its_temp_void_var = its_temp_void_var - HEADERPLUSFOOTER_8;
                }
                else{
                    (*((sf_block*)its_temp_void_var)).prev_footer=(*blockAtIndex).header^sf_magic();
                    (*((sf_block*)its_temp_void_var)).header=((*((sf_block*)its_temp_void_var)).header & BLOCK_SIZE_MASK_16_E) + 1;
                    its_temp_void_var = ((*((sf_block*)its_temp_void_var)).header & BLOCK_SIZE_MASK_16) + its_temp_void_var;

                    if((sf_mem_end() - HEADERPLUSFOOTER)==its_temp_void_var){
                        *((sf_footer*)its_temp_void_var)=(*(((sf_block*)its_temp_void_var))).header^sf_magic();
                        its_temp_void_var = HEADERPLUSFOOTER_8 + its_temp_void_var;
                        (*((sf_epilogue*)its_temp_void_var)).header=((*((sf_epilogue*)its_temp_void_var)).header & BLOCK_SIZE_MASK_16) + 3;
                        its_temp_void_var= its_temp_void_var - HEADERPLUSFOOTER_8;
                    }
                    else
                        (*((sf_block*)its_temp_void_var)).prev_footer=(*((sf_block*)its_temp_void_var)).header^sf_magic();

                }

                sf_block *current_index_next = (*blockAtIndex).body.links.prev;

                ((*(current_index_next)).body.links.next)=(*blockAtIndex).body.links.next;
                ((*(current_index_next)).body.links.prev)=(*blockAtIndex).body.links.prev;

                return (*blockAtIndex).body.payload;

            }
        }
    }

    else {
        void* voidpointer=sf_mem_end();

        voidpointer = voidpointer - HEADERPLUSFOOTER;

        sf_block empty_block;

        empty_block.prev_footer=*((sf_footer*)voidpointer);
        if(!sf_mem_grow()){
            sf_errno = ENOMEM;
            return NULL;
        }
        if((((*((sf_footer*)voidpointer))^sf_magic()) & THIS_BLOCK_ALLOCATED)!=2)
            empty_block.header=BASE_SIZE_4096 ;

        else
            empty_block.header=BASE_SIZE_4096 +1;

        sf_epilogue* epi_copy=(sf_epilogue*)voidpointer;
        epi_copy=(sf_epilogue*)sf_mem_end();
        epi_copy=  epi_copy -1 ;

        sf_epilogue newepi;
        newepi.header=2;

        *epi_copy=newepi;
        epi_copy = epi_copy - 1;

        *((sf_footer*)epi_copy)=empty_block.header^sf_magic();
        *((sf_block*)voidpointer)=empty_block;

        coalesce((sf_block*)voidpointer);

        index = getPositionForSF(b_sizeee);

        goto vaapis_bc;

    }
    return NULL;
}

void sf_free(void *pp){

    void *incomingPointer = pp;
    incomingPointer = incomingPointer -HEADERPLUSFOOTER;

    void *memEnd = sf_mem_end();
    memEnd = memEnd -HEADERPLUSFOOTER;

    (*(((sf_block*)incomingPointer))).header = ((*(((sf_block*)incomingPointer))).header) & BLOCK_SIZE_MASK_16_D;
    int headerSize = ((*(((sf_block*)incomingPointer))).header) & BLOCK_SIZE_MASK_16;
    incomingPointer =  headerSize + incomingPointer;

    if(memEnd != incomingPointer)
    {
        (*((sf_block *)incomingPointer)).prev_footer = (* (((sf_block*)incomingPointer))).header ^ sf_magic();
        (*((sf_block *)incomingPointer)).header = BLOCK_SIZE_MASK_16_E & (*((sf_block *)incomingPointer)).header ;
    }
    else{
        (*((sf_footer*)incomingPointer)) = (*(((sf_block*)incomingPointer))).header ^ sf_magic();
        incomingPointer = HEADERPLUSFOOTER_8 + incomingPointer;

        sf_epilogue* epiiiii = (sf_epilogue*)incomingPointer;
        (*epiiiii).header = (*(((sf_block*)incomingPointer))).header & BLOCK_SIZE_MASK_16_E;
        incomingPointer = incomingPointer+ HEADERPLUSFOOTER_8;
    }

    coalesce(incomingPointer - headerSize);

    return;
}

void coalesce(void* voidptr){

    sf_block* current_block=(sf_block*)voidptr;
    int current_block_header_size = (*current_block).header & BLOCK_SIZE_MASK_16;
    int is_it_a_footer=0;
    int previous_is_free =0;
    int next_is_free = 0;
    int prev_allocated_block_bit = ((*current_block).header & PREV_BLOCK_ALLOCATED);

    if(!prev_allocated_block_bit)
        previous_is_free=1;

    voidptr = voidptr + current_block_header_size;

    if(voidptr!=(sf_mem_end() -16)){
        if(!(((*((sf_block*)voidptr)).header) & THIS_BLOCK_ALLOCATED))
            next_is_free=1;
    }
    else{
        is_it_a_footer=1;
    }

    voidptr = voidptr - current_block_header_size;

    if(!previous_is_free &&  !next_is_free ){

        if (!is_it_a_footer)
        {
            sf_block* temp1 = (sf_block *)voidptr;
            (*temp1).header=((*temp1).header & BLOCK_SIZE_MASK_16_E);
            voidptr += ((*temp1).header & BLOCK_SIZE_MASK_16);

            (*((sf_block*)voidptr)).prev_footer=(*temp1).header^sf_magic();

            int index= getPositionForSF(current_block_header_size);

            sf_block *saving_block = sf_free_list_heads[index].body.links.next;

            (current_block)->body.links.next = saving_block;
            ((saving_block))->body.links.prev = current_block;

            sf_free_list_heads[index].body.links.next = current_block;
            (current_block)->body.links.prev=&sf_free_list_heads[index];


        }
        else
        {
            int findingindex = current_block_header_size;

            int index = getPositionForSF(findingindex);

            (*current_block).body.links.next = sf_free_list_heads[index].body.links.next;

            sf_block *save_block = sf_free_list_heads[index].body.links.next;
            sf_block *save_block_temp = &sf_free_list_heads[index];

            (*(save_block)).body.links.prev = current_block;
            save_block = current_block;

            (*current_block).body.links.prev=save_block_temp;
        }
    }
    if( previous_is_free && !next_is_free )
    {
        int previous_block_header_size = ((*current_block).prev_footer^sf_magic()) & BLOCK_SIZE_MASK_16;

        voidptr= voidptr - previous_block_header_size;

        sf_block *prevvvvvvv = (sf_block*)voidptr;

        (*((*((sf_block*)voidptr)).body.links.prev)).body.links.next = (*((sf_block*)voidptr)).body.links.next;
        (*((*((sf_block*)voidptr)).body.links.next)).body.links.prev = (*((sf_block*)voidptr)).body.links.prev;
        (*((sf_block*)voidptr)).header += current_block_header_size;

        voidptr =  ((*((sf_block*)voidptr)).header & BLOCK_SIZE_MASK_16) + voidptr;

        if(!is_it_a_footer)
            (*((sf_block*)voidptr)).prev_footer=(prevvvvvvv)->header^sf_magic();

        else
            (*((sf_footer*)voidptr))=(*prevvvvvvv).header^sf_magic();

        voidptr = voidptr -  ((prevvvvvvv)->header & BLOCK_SIZE_MASK_16);

        int index= getPositionForSF((prevvvvvvv)->header & BLOCK_SIZE_MASK_16);

        sf_block *current_index_next = sf_free_list_heads[index].body.links.next;
        sf_block *current_index = &sf_free_list_heads[index];

        (prevvvvvvv)->body.links.prev = current_index;
        (prevvvvvvv)->body.links.next = current_index_next;


        ((current_index_next))->body.links.prev = prevvvvvvv;
        sf_free_list_heads[index].body.links.next = prevvvvvvv;
    }

    if(next_is_free && previous_is_free==0){
        voidptr=  current_block_header_size + voidptr;

        int block_next_to_currentsize=(*((sf_block*)voidptr)).header & BLOCK_SIZE_MASK_16;

        (*((*((sf_block*)voidptr)).body.links.prev)).body.links.next=(*((sf_block*)voidptr)).body.links.next;
        (*((*((sf_block*)voidptr)).body.links.next)).body.links.prev=(*((sf_block*)voidptr)).body.links.prev;

        (*current_block).header+=block_next_to_currentsize;

        voidptr=  block_next_to_currentsize + voidptr;

        if(voidptr!=(sf_mem_end() -16))
            (((sf_block*)voidptr))->prev_footer=(*current_block).header^sf_magic();

        else
            (*((sf_footer*)voidptr))=(current_block)->header^sf_magic();

        int index=getPositionForSF(  ((current_block)->header & BLOCK_SIZE_MASK_16));

        sf_block *current_index_next = sf_free_list_heads[index].body.links.next;
        sf_block *current_index = &sf_free_list_heads[index];

        (current_block)->body.links.next=current_index_next;
        (*current_block).body.links.prev=current_index;

        (sf_free_list_heads[index].body.links.next)->body.links.prev = current_block;
        sf_free_list_heads[index].body.links.next = current_block;
    }

    if(next_is_free && previous_is_free){

        int previous_block_header_size = BLOCK_SIZE_MASK_16 & ((*current_block).prev_footer^sf_magic()) ;

        voidptr= voidptr - previous_block_header_size;

        sf_block* previous_block_header_size_pointer=(sf_block*)voidptr;

        (*((*((sf_block*)voidptr)).body.links.prev)).body.links.next=(*((sf_block*)voidptr)).body.links.next;
        (*((*((sf_block*)voidptr)).body.links.next)).body.links.prev=(*((sf_block*)voidptr)).body.links.prev;

        (*((sf_block*)voidptr)).header+=current_block_header_size;

        voidptr= voidptr + (BLOCK_SIZE_MASK_16 &  (*((sf_block*)voidptr)).header) ;

        (*((sf_block*)voidptr)).prev_footer=(*previous_block_header_size_pointer).header^sf_magic();



        int index=getPositionForSF((*previous_block_header_size_pointer).header & BLOCK_SIZE_MASK_16);

        int block_next_to_currentsize = BLOCK_SIZE_MASK_16 & (*((sf_block*)voidptr)).header ;

        (*((*((sf_block*)voidptr)).body.links.prev)).body.links.next=(*((sf_block*)voidptr)).body.links.next;
        (*((*((sf_block*)voidptr)).body.links.next)).body.links.prev=(*((sf_block*)voidptr)).body.links.prev;

        (*previous_block_header_size_pointer).header = block_next_to_currentsize + (*previous_block_header_size_pointer).header;

        voidptr = voidptr + block_next_to_currentsize;

        if(voidptr!=(sf_mem_end() -16))
            (*(((sf_block*)voidptr))).prev_footer=(*previous_block_header_size_pointer).header^sf_magic();

        else
            (*((sf_footer*)voidptr))=(previous_block_header_size_pointer)->header^sf_magic();

        int b_sizeee=  (previous_block_header_size_pointer)->header & BLOCK_SIZE_MASK_16;

        index=getPositionForSF(b_sizeee);

        sf_block *current_index_next = sf_free_list_heads[index].body.links.next;
        sf_block *current_index = &sf_free_list_heads[index];

        (*previous_block_header_size_pointer).body.links.next=current_index_next;
        (*previous_block_header_size_pointer).body.links.prev=current_index;

        ((sf_free_list_heads[index].body.links.next))->body.links.prev = previous_block_header_size_pointer;
        sf_free_list_heads[index].body.links.next = previous_block_header_size_pointer;

        return;
    }
}


void *sf_realloc(void *pp, size_t rsize) {


    if(pp==NULL){
        abort();
    }
    pp-=HEADERPLUSFOOTER;
    sf_block *incoming_block_to_realloc = (sf_block*)pp;

    if(pp < ( sf_mem_start() + HEADERPLUSFOOTER_32) || pp > (sf_mem_end()-HEADERPLUSFOOTER_8))
    {
        sf_errno = EINVAL;
        return NULL;
    }

    if(!(((*incoming_block_to_realloc).header & THIS_BLOCK_ALLOCATED )))
    {
        sf_errno = EINVAL;
        return NULL;
    }

    if(((*incoming_block_to_realloc).header & BLOCK_SIZE_MASK_16) < HEADERPLUSFOOTER_32)
    {
        sf_errno = EINVAL;
        return NULL;
    }

    if(!((*incoming_block_to_realloc).header & PREV_BLOCK_ALLOCATED )){
        pp = pp -(((*incoming_block_to_realloc).prev_footer ^ sf_magic()) & BLOCK_SIZE_MASK_16);
        if(!(((*incoming_block_to_realloc).header & THIS_BLOCK_ALLOCATED))){
            sf_errno = EINVAL;
            return NULL;
        }
        pp = (((*incoming_block_to_realloc).prev_footer^sf_magic()) & BLOCK_SIZE_MASK_16) + pp;
    }

    size_t header=(*((sf_block*)pp)).header;

    pp = (header & BLOCK_SIZE_MASK_16) + pp;

    if(pp!=(sf_mem_end() - HEADERPLUSFOOTER )){
        if((*((sf_block*)pp)).prev_footer!=(header^sf_magic())){
           sf_errno = EINVAL;
           return NULL;
       }
   }
   else{
    if( (header^sf_magic()) != (*((sf_footer*)pp))  ){
        sf_errno = EINVAL;
        return NULL;
    }
}
pp = pp - (header & BLOCK_SIZE_MASK_16);

sf_block* above_block_pointer=(sf_block*)pp;

int b_sizeee = 0;
b_sizeee = calculate(b_sizeee, rsize);

if(!rsize){
    pp+=HEADERPLUSFOOTER;
    sf_free(pp);
    return NULL;
}
else if(b_sizeee > ((((*above_block_pointer)).header & BLOCK_SIZE_MASK_16))){
    void* ptr=sf_malloc(rsize);
    if(ptr==NULL){
        return NULL;
    }
    pp = HEADERPLUSFOOTER + pp;

    memcpy(ptr,pp,((*above_block_pointer).header & BLOCK_SIZE_MASK_16)-HEADERPLUSFOOTER);

    sf_free(pp);
    return ptr;
}
else if(b_sizeee <= ((*((sf_block*)pp)).header & BLOCK_SIZE_MASK_16))
{
    if(((*((sf_block*)pp)).header & BLOCK_SIZE_MASK_16)-b_sizeee<HEADERPLUSFOOTER_32){
        return (*((sf_block*)pp)).body.payload;
    }
    else{

        sf_block dummy_block;
        dummy_block.prev_footer = (*above_block_pointer).prev_footer;
        dummy_block.header = b_sizeee + ((*above_block_pointer).header & ALLOCATING);

        sf_block dummy_block_2;
        dummy_block_2.prev_footer=dummy_block.header^sf_magic();
        dummy_block_2.header=((*above_block_pointer).header & BLOCK_SIZE_MASK_16)-b_sizeee+1;

        sf_block *dummy_block_pointer;
        void* dummy_block_void=(void*)above_block_pointer;
        dummy_block_pointer=(sf_block*)dummy_block_void;
        dummy_block_void = dummy_block_void + b_sizeee;
        sf_block *dummy_block_2_pointer;
        dummy_block_2_pointer=(sf_block *)dummy_block_void;
        *(dummy_block_2_pointer)=dummy_block_2;

        *dummy_block_pointer = dummy_block;
        dummy_block_void = (((*(dummy_block_2_pointer)).header & BLOCK_SIZE_MASK_16)) + dummy_block_void;

        if(dummy_block_void!=(sf_mem_end() - HEADERPLUSFOOTER) )
            (*((sf_block*)dummy_block_void)).prev_footer=(*(dummy_block_2_pointer)).header^sf_magic();

        else
            (*((sf_footer*)dummy_block_void))=(*(dummy_block_2_pointer)).header^sf_magic();

        coalesce((dummy_block_2_pointer));

        return dummy_block_pointer->body.payload;
    }
}
return NULL;
}


int getPositionForSF (int incoming ){
    int index=0;
    int b_sizeee = 0;
    b_sizeee = incoming;

    if(incoming <0)
    {
        return -1;
    }

    if(b_sizeee>32*HEADERPLUSFOOTER_32 && b_sizeee <=  64*HEADERPLUSFOOTER_32){
        index=6;
        return index;
    }
    if(b_sizeee>HEADERPLUSFOOTER_32    && b_sizeee <=  2*HEADERPLUSFOOTER_32)
    {
        index=1;
        return index;
    }
    if(b_sizeee <=  8*HEADERPLUSFOOTER_32 && b_sizeee>4*HEADERPLUSFOOTER_32 ){
        index=3;
        return index;
    }
    if(b_sizeee <=  32*HEADERPLUSFOOTER_32 && b_sizeee>16*HEADERPLUSFOOTER_32 ){
        index=5;
        return index;
    }
    if(b_sizeee <=  4*HEADERPLUSFOOTER_32 && b_sizeee>2*HEADERPLUSFOOTER_32  ){
        index=2;
        return index;
    }

    if(b_sizeee>8*HEADERPLUSFOOTER_32  && b_sizeee <=  16*HEADERPLUSFOOTER_32){
        index=4;
        return index;
    }
    if(b_sizeee>128*HEADERPLUSFOOTER_32)
    {
        index=8;
        return index;
    }

    if(b_sizeee <=  128*HEADERPLUSFOOTER_32 && b_sizeee>64*HEADERPLUSFOOTER_32 ){
        index=7;
        return index;
    }

    return index;
}




