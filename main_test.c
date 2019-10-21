#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BF.h"
#include "HT.h"
#include "SHT.h"

/*h sunarthsh auth pairnei ena arxeio kai diavazei grammh grammh ka8e record
  kai to pernaei sta arxeia katakermatismou*/
void InsertRecords(HT_info *hi, SHT_info *shi, FILE *inputFile, int flag) {
    char *line = NULL;
    size_t len = 0;
    int blockId;
    Record record; //= malloc(sizeof(Record));
    SecondaryRecord Srecord;
    ssize_t my_read;
    my_read = getline(&line, &len, inputFile);
    //printf("%s\n", line);

    while (!feof(inputFile)) {
        if(my_read != -1){
            char *tok;
            //record = {0,"name_0","surname_0","address_0"}
            while (tok != NULL && my_read != -1) {

                tok = strtok(line, "{");
                //printf("1) %s\n", tok);

                tok = strtok(tok, ",");
                //printf("1) %s\n", tok);
                record.id = atoi(tok);

                tok = strtok(NULL, "\"");
                //printf("2) %s\n", tok);
                strncpy(record.name, tok, sizeof(record.name));

                tok = strtok(NULL, "\",");
                //printf("3) %s\n", tok);
                strncpy(record.surname, tok, sizeof(record.surname));

                tok = strtok(NULL, "\",");
                //printf("4) %s\n", tok);
                strncpy(record.address, tok, sizeof(record.address));

                blockId = HT_InsertEntry(*hi, record);
                if(flag == 1){
                    Srecord.blockId = blockId;
                    Srecord.record = record;
                    SHT_SecondaryInsertEntry(*shi, Srecord);
                }
                my_read = getline(&line, &len, inputFile);
            }
        }
    }
    rewind(inputFile);
    free(line);
}

int main(char argc,char** argv){
    /*
    Init the BF layer.
    */
    BF_Init();
    /*
    Index parameters.
    */
    char* fileName="primary.index";
    char attrType='i';
    char* attrName="id";
    int attrLength=4;
    int buckets=10;
    int flag = 0;
    char* sfileName="secondary.index";
    char sAttrType='c';
    char* sAttrName="name";
    int sAttrLength=15;
    int sBuckets=10;
    HT_info* hi;

    FILE* inputFile = fopen(argv[1], "r"); ////read file
    if (inputFile == NULL)    {
        printf("Error");
    }

    //to euros twn eggrafwn pou anazhtw me thn GetAllEntries
    int test_start = atoi(argv[2]);
    int test_stop = atoi(argv[3]);

    //dhmiourgw to hash arxeio
    if(HT_CreateIndex(fileName,attrType,attrName,attrLength,buckets)<0){
        printf("error in HT_CreateIndex()\n");
        return -1;
    }
    else{
        printf("Created a hash Index\n");
    }

    //anoigw to hash arxeio kai epistrefw th domh HT_Info
    hi=HT_OpenIndex(fileName);
  	if(hi!=NULL && hi->attrType==attrType && strcmp(hi->attrName,attrName)==0){
  		printf("Opened the Hash index\n");
  	}
  	else{
  		printf("error in HT_OpenIndex()\n");
      return -1;
  	}

    //dhmiourgw secondary
  	if (SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,fileName)<0){
  		printf("error in SHT_CreateSecondaryIndex()\n");
  		return -1;
  	}
    else{
        printf("Created a secondary hash Index\n");
    }

    //anoigw secondary kai epistrefw th domh SHT_Info
  	SHT_info* shi=SHT_OpenSecondaryIndex(sfileName);
  	if(shi!=NULL){
  		printf("Opened Secondary\n");
  	}
  	else{
  		printf("error in SHT_OpenSecondaryIndex()\n");
  	}

    //eisagw ta records apo to arxeio
    InsertRecords(hi, shi, inputFile, flag);
    printf("inserted records in ht\n");

    int c = 0;
    for (int i=test_start; i<test_stop+1; i++){
        Record test_record;
        test_record.id=i;
        sprintf(test_record.name,"name_%d",i);
        sprintf(test_record.surname,"surname_%d",i);
        sprintf(test_record.address,"address_%d",i);
        int err=HT_GetAllEntries(*hi,(void*)&test_record.id);
        if (err<0){
             c+=1;
        }
    }
    if(c>0){
      printf("error in HT_GetAllEntries()\n");
    }
    else{
      printf("HT_GetAllEntries() SUCCESS\n");
    }

    //eisagw ta records xrhsimopoiontas kai secondary eurethrio
    flag = 1;
    InsertRecords(hi, shi, inputFile, flag);
    printf("inserted records in secondary\n");

    c = 0;
    for (int i=test_start; i<test_stop+1; i++){
        Record test_record;
        test_record.id=i;
        sprintf(test_record.name,"name_%d",i);
        sprintf(test_record.surname,"surname_%d",i);
        sprintf(test_record.address,"address_%d",i);
        int err=SHT_SecondaryGetAllEntries(*shi,*hi,(void*)test_record.name);
        if (err<0){
             c+=1;
        }
    }
    if(c>0){
      printf("error in SHT_SecondaryGetAllEntries()\n");
    }
    else{
      printf("SHT_SecondaryGetAllEntries() SUCCESS\n");
    }

    int htCloseError=HT_CloseIndex(hi);

    int shtCloseError=SHT_CloseSecondaryIndex(shi);

    if (htCloseError==0 && shtCloseError==0){
  		printf("HT_CloseIndex() and SHT_CloseSecondaryIndex() SUCCESS\n");
  	}
  	else{
  		printf("HT_CloseIndex() and SHT_CloseSecondaryIndex() FAIL\n");
  	}

    printf("\nStatistics:HT\n");
  	HashStatistics(fileName);
  	printf("\nStatistics:SHT\n");
  	HashStatistics(sfileName);

}
