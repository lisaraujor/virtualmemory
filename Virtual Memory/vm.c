#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

void getValue(FILE *correct,int pagenumber, int offset){

        FILE *filebckstore;
        char buffer[256];

        filebckstore = fopen("BACKING_STORE.bin", "rb");
        fseek(filebckstore, (pagenumber * 256), SEEK_SET);
        fread(buffer,sizeof(char),256,filebckstore);
        fclose(filebckstore);

        fprintf(correct,"Value: %d ", (int)buffer[offset]);
}

int toDecimal8b(int bin[8]){ // Função que transforma um número binário de 8 bits em decimal
        
        int sum = 0, pot2 = 128;

        for(int i = 0; i < 8; i++){
                sum = sum + (bin[i] * pot2);
                pot2 = pot2 / 2;
        }

        return sum;
}

int toDecimal16b(FILE *correct,int bin[16], int pagenumber, int offset){ // Função que transforma um número binário de 16 bits em decimal
        
        int sum = 0, pot2 = 32768;

        for(int i = 0; i < 16; i++){
                sum = sum + (bin[i] * pot2);
                pot2 = pot2 / 2;
        }
        fprintf(correct,"Physical address: %d ", sum);
                
        getValue(correct,pagenumber,offset);
        
        return sum;
        
}

void changePhysicalMem(FILE *correct,int physicalmem[128][256], int offset[8], int pagenumber, int framenumber){

        int bin, exp, dec = framenumber, next = framenumber, result = 0, cont = 1;
        int framebin[8], physical_address[16];

        while(next >= 2){
                bin = (next % 2);
                next = (next / 2);
                cont++; 
        }

        dec = framenumber, next = framenumber;
        int i = (cont - 1), ft = (8 - cont), k = 7;

        for(int j = 0; j < ft; j++){
                framebin[j] = 0;
        }

        framebin[ft] = 1;

        while(k >= ft){
                bin = (next % 2);
                next = (next / 2);
                framebin[k] = bin; //Transforma o index (frame) em binário.
                k--;
        }

        for(int i = 0; i < 8; i++){
                physical_address[i] = framebin[i]; // Coloca o frame em binário (8bits) nas primeiras 8 posições do end. físico 
        }
        int m = 0;
        for(int i = 8; i < 16; i++){
                physical_address[i] = offset[m]; // Coloca o frame em binário (8bits) nas últimas 8 posições do end. físico
                m++;
        }

        FILE *filebckstore;
        char buffer[256];

        filebckstore = fopen("BACKING_STORE.bin", "rb"); // Chama arquivo binário
        fseek(filebckstore, (pagenumber * 256), SEEK_SET); //Pega a linha referente ao page number.
        fread(buffer,sizeof(char),256,filebckstore); //Armazena no array 'buffer'.

        for(int i = 0; i < 256; i++){
                physicalmem[framenumber][i] = buffer[i];
        }

        fclose(filebckstore); //Fecha arquivo.
        
        int ofst = toDecimal8b(offset);
        toDecimal16b(correct,physical_address,pagenumber,ofst);
}

int fifoSubsPag(FILE *correct,int pagenumber, int pagetable[128], int physicalmem[128][256], int *fifopag, int offst[8]){

        pagetable[(*fifopag)] = pagenumber;
        
        changePhysicalMem(correct,physicalmem,offst,pagenumber,*fifopag);

        (*fifopag)++;
        if((*fifopag) == 128){
                *fifopag = 0;
        }

        return (*fifopag);        
}

void fifoTLB(FILE *correct,int pagenumber, int TLB[16][2], int *fifotlb, int offst[8], int framenumber){

        TLB[(*fifotlb)][0] = pagenumber;
        TLB[(*fifotlb)][1] = framenumber;
        fprintf(correct," TLB: %d \n", *fifotlb);
        
        (*fifotlb)++;
        if((*fifotlb) == 16){
                *fifotlb = 0;
        }        
}

void searchPhysicalAddr(FILE *correct,int physicalmem[128][256], int offset[8], int pagenumber, int framenumber){

        int bin, exp, dec = framenumber, next = framenumber, result = 0, cont = 1;
        int framebin[8], physical_address[16];

        while(next >= 2){
                bin = (next % 2);
                next = (next / 2);
                cont++; 
        }

        dec = framenumber, next = framenumber;
        int i = (cont - 1), ft = (8 - cont), k = 7;

        for(int j = 0; j < ft; j++){
                framebin[j] = 0;
        }

        framebin[ft] = 1;

        while(k >= ft){
                bin = (next % 2);
                next = (next / 2);
                framebin[k] = bin; //Transforma o index (frame) em binário.
                k--;
        }

        for(int i = 0; i < 8; i++){
                physical_address[i] = framebin[i]; // Coloca o frame em binário (8bits) nas primeiras 8 posições do end. físico 
        }
        int m = 0;
        for(int i = 8; i < 16; i++){
                physical_address[i] = offset[m]; // Coloca o frame em binário (8bits) nas últimas 8 posições do end. físico
                m++;
        }

        int ofst = toDecimal8b(offset);
        toDecimal16b(correct,physical_address,pagenumber,ofst);

}

int changePageTable(FILE *correct,int pgnumber[8], int offst[8], int pagetable[128], int physicalmem[128][256], int *fifopag, int *pagefault, int alg_subspag){
        
        int cont2 = 0, cont3 = 0, index = 0, retfifo;
        int pagenumber, offset;

        pagenumber = toDecimal8b(pgnumber); //Transforma page number binário em um número decimal.
        offset = toDecimal8b(offst); //Transforma offset binário em um número decimal.      
        
        (*pagefault)++;

        for(int i = 0; i < 128; i++){ //Verifica se Page Table está vazia.
                if(pagetable[i] == -1){
                        cont2 = 1; //Indica que Page Table ainda tem espaço livre.
                }
        }
        if(cont2 == 1){ //Tem espaço livre na Page Table
                while(cont3 == 0){
                        if(pagetable[index] == -1){ //Se tal espaço da Page Table estiver vazio, coloca o page number.
                                pagetable[index] = pagenumber;
                                changePhysicalMem(correct,physicalmem,offst,pagenumber,index);
                                cont3 = 1; //Indica que colocou o PgNumb na PgTable, e sai do while.
                                return index;
                        }
                        else{ //Se não, avança para o próximo espaço.
                                index++;
                        }  
                }
        }                       
        if(cont2 == 0){ //Indica que a Pg Table está cheia.
                if(alg_subspag == 1){
                        //printf(" CHAMA FIFO ");
                        retfifo = fifoSubsPag(correct,pagenumber,pagetable, physicalmem, fifopag, offst);
                        return retfifo;
                }
                if(alg_subspag == 2){
                        //LRU();
                }
        }
 
}

void changeTLB(FILE *correct,int pgnumber[8], int offst[8], int TLB[16][2], int framenumber, int alg_tlb, int *fifotlb){

        int cont2 = 0, cont3 = 0, index = 0, pagenumber;
        pagenumber = toDecimal8b(pgnumber);
        

        for(int i = 0; i < 16; i++){ //Verifica se TLB está vazia.
                if(TLB[i][0] == -1){
                        cont2 = 1; //Indica que TLB ainda tem espaço livre.
                }
        }
        if(cont2 == 1){ //Tem espaço livre na TLB
                while(cont3 == 0){
                        if(TLB[index][0] == -1){ //Se tal espaço da TLB estiver vazio:
                                TLB[index][0] = pagenumber; //Coloca o page number.
                                TLB[index][1] = framenumber; //Coloca o frame number.
                                fprintf(correct,"TLB: %d \n", index);
                                cont3 = 1; //Indica que colocou o PgNumb na TLB, e sai do while.
                        }
                        else{ //Se não, avança para o próximo espaço.
                                index++;
                        }
                }
        }                       
        if(cont2 == 0){ //Indica que a TLB está cheia.
                if(alg_tlb == 1){
                        fifoTLB(correct,pagenumber,TLB, fifotlb,offst,framenumber);
                }
                if(alg_tlb == 2){
                        //LRU();
                }
        } 
        
}

int verifyPageTable(int pgnumber[8], int pagetable[128]){

        int pagenumber;

        pagenumber = toDecimal8b(pgnumber); //Transforma page number binário em um número decimal.
                
        for(int i = 0; i < 128; i++){ //Procura o Page Number na Page Table.
                if(pagenumber == pagetable[i]){
                        return i; //Retorna o index que esse Page Number tem na Page Table.
                }
        }
       
        return -1;  //Se o Page Number estiver na Page Table, vai retornar o index. Se não, retorna -1.
}

int verifyTLB(int pgnumber[8], int TLB[16][2]){

        int pagenumber;

        pagenumber = toDecimal8b(pgnumber); //Transforma page number binário em um número decimal.
                
        for(int i = 0; i < 16; i++){ //Procura o Page Number na TLB.
                if(pagenumber == TLB[i][0]){
                        return i; //Indica que esse Page Number já está na TLB e retorna seu index.
                }
        }
       
        return -1; //Se o Page Number estiver na TLB, vai retornar 1.       
}

void searchAddress(FILE *correct, int pgnumber[8], int offst[8], int pagetable[128], int physicalmem[128][256], int TLB[16][2], int *tlbhit, int *fifopag, int *fifotlb, int *pagefault,  int alg_tlb, int alg_subspag){

        int onTLB, onPageTable, pagenumber, framenumber;
        pagenumber = toDecimal8b(pgnumber);

        onTLB = verifyTLB(pgnumber,TLB); //Resultado da verificação da TLB, retorna o index se o Page Number estiver ou -1 caso contrario.
        onPageTable = verifyPageTable(pgnumber,pagetable);
        
        if(onTLB == -1 && onPageTable == -1){ //Se Page Number não estiver na Page Table ou TLB.
                framenumber = changePageTable(correct,pgnumber,offst,pagetable,physicalmem,fifopag,pagefault,alg_subspag);
                changeTLB(correct,pgnumber, offst, TLB, framenumber, alg_tlb, fifotlb);
        }
        if(onTLB == -1 && onPageTable >= 0){ //Não estiver na TLB, mas estiver na Page Table.
                searchPhysicalAddr(correct,physicalmem,offst,pagenumber,onPageTable);
                changeTLB(correct,pgnumber,offst,TLB,onPageTable,alg_tlb,fifotlb);
        }
        if(onTLB >= 0 && onPageTable >= 0){ //Se estiver na TLB e na Page Table
                searchPhysicalAddr(correct,physicalmem,offst,pagenumber,onPageTable);
                fprintf(correct," TLB: %d \n", onTLB);
                (*tlbhit)++;
        }     
        
}

void pgNmbOffset(FILE *correct, int bin[16], int pagetable[128], int physicalmem[128][256], int *fifopag, int *fifotlb, int *pagefault, int TLB[16][2], int *tlbhit, int alg_tlb, int alg_subspag){
        
        int pgnumber[8], offst[8];
        int j = 8;

        //Divide o endereço binário em Page Number e Offset.
        for(int i = 0; i < 8; i++){
                offst[i] = bin[j];
                j++;
        }
        for(int i = 0; i < 8; i++){
                pgnumber[i] = bin[i];
        }

        searchAddress(correct, pgnumber,offst,pagetable,physicalmem,TLB, tlbhit,fifopag,fifotlb,pagefault,alg_tlb,alg_subspag);
}

void toBoolean16b(FILE *correct, int num, int pagetable[128], int physicalmem[128][256], int *fifopag, int *fifotlb, int *pagefault, int TLB[16][2], int *tlbhit, int alg_tlb, int alg_subspag){
        
        int bin, exp, dec = num, next = num, result = 0, cont = 1;
        int binary[16];

        //Transforma endereço virtual em um número binário.
        while(next >= 2){
                bin = (next % 2);
                next = (next / 2);
                cont++;
        }

        dec = num, next = num;
        int i = (cont - 1), ft = (16 - cont), k = 15;

        for(int j = 0; j < ft; j++){
                binary[j] = 0;
        }

        binary[ft] = 1;

        while(k >= ft){
                bin = (next % 2);
                next = (next / 2);
                binary[k] = bin;
                k--;
        }
        
        fprintf(correct,"Virtual address: %d ", num);

        pgNmbOffset(correct, binary, pagetable, physicalmem, fifopag, fifotlb, pagefault, TLB, tlbhit, alg_tlb, alg_subspag);
}

void init(FILE *fileadd, FILE *correct, int alg_tlb, int alg_subspag){
        
        int pagetable[128], physicalmemory[128][256], TLB[16][2];
        int address, endTrad = 0, fifopag = 0, fifotlb = 0, pagefault = 0, tlbhit = 0;
        float pagefaultrate, tlbhitrate;
        char c;

        for(int i = 0; i < 128; i++){
                pagetable[i] = -1; //Coloca -1 em todas as posições da Page Table.
        }
        for(int i = 0; i < 16; i++){
                for(int j = 0; j < 2; j++){
                        TLB[i][j] = -1; //Coloca -1 em todas as posições da TLB.
                }
        }

        for(int i = 0; i < 1000; i++){
                fscanf(fileadd, "%d%c", &address, &c); // Lê arquivo com endereços.
                toBoolean16b(correct, address, pagetable, physicalmemory, &fifopag, &fifotlb, &pagefault, TLB, &tlbhit,  alg_tlb, alg_subspag);
                endTrad++;              
        }

        fprintf(correct,"Number of Translated Addresses = %d\n", endTrad); //Mostra quantidade de endereços traduzidos
        fprintf(correct,"Page Faults = %d\n", pagefault); // Mostra quantidade de page fault.
        
        pagefaultrate = pagefault;
        pagefaultrate = (pagefaultrate / endTrad);
        fprintf(correct,"Page Fault Rate = %.3f\n", pagefaultrate); //Page Fault rate

        fprintf(correct,"TLB Hits = %d\n", tlbhit);

        tlbhitrate = tlbhit;
        tlbhitrate = (tlbhitrate / endTrad);
        fprintf(correct,"TLB Hit Rate = %.3f\n", tlbhitrate);

}

int main(int argc, char *argv[]){
               
        int alg_tlb, alg_subspag;
        
        FILE *fileadd;
        FILE *correct;        
        fileadd = fopen(argv[1], "r");
        correct = fopen("correct.txt", "a");

        if(fileadd == NULL){
                printf("File does not exist or cannot be opened.\n");
                return -1;
        }
        if(argc != 4){
                printf("Invalid number of arguments.\n");
                return -1;
        }
        if((strcmp(argv[2],"fifo") != 0) && (strcmp(argv[2],"lru") != 0)){
                printf("Invalid type of argument.\n");
                return -1;
        }
        if((strcmp(argv[3],"fifo") != 0) && (strcmp(argv[3],"lru") != 0)){
                printf("Invalid type of argument.\n");
                return -1;
        }
        if(strcmp(argv[2],"fifo") == 0){
                alg_subspag = 1;
        }
        if(strcmp(argv[2],"lru") == 0){
                alg_subspag = 2;
        }
        if(strcmp(argv[3],"fifo") == 0){
                alg_tlb = 1;
        }
        if(strcmp(argv[3],"lru") == 0){
                alg_tlb = 2;
        }

        init(fileadd, correct, alg_tlb, alg_subspag); //Chama a função init.    
       
        fclose(fileadd); //Fecha arquivo.
        fclose(correct);

        return 0;
}