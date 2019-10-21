#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BF.h"
#include "HT.h"
#include "SHT.h"

int secondary_hash_function(SHT_info header_info, Record record){

		char* key;
		char* value;
		int result = 0;
		int letter;
		int buckets = header_info.numBuckets;
		int length = header_info.attrLength;

		key = malloc(length*sizeof(char));
		strcpy(key, header_info.attrName);

		value = malloc(length);
		strcpy(value, record.name);

		//an to key einai string pairnw to a8roisma twn grammatwn tou
		if(strcmp(key, "name") == 0){
				for(int i=0; i<length; i++){
						result += value[i];
				}
				free(key);
				return result%buckets;
		}
		else if(strcmp(key, "surname") == 0){
			for(int i=0; i<length; i++){
					result += value[i];
			}
			free(key);
			return result%buckets;
		}
		else if(strcmp(key, "address") == 0){
			for(int i=0; i<length; i++){
					result += value[i];
			}
			free(key);
			return result%buckets;
		}
		else{
				free(key);
				printf("error sto hash funsction\n");
				exit(0);
		}
}

int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName){
    int fileDesc;
    int* ShashTable = NULL;
    void* block = NULL;
    char* SHtable = NULL;
    char attrType = 'c';
    int FNlength = strlen(fileName);

    SHtable = malloc(2*sizeof(char));
    strncpy(SHtable, "SH", 2);				//to anagnwristiko tou arxeiou hash table

    //dhmiourgia file
    if(BF_CreateFile(sfileName) < 0) {
      BF_PrintError("Creating file failed\n");
      return -1;
    }

    //anoigma file
    if((fileDesc = BF_OpenFile(sfileName)) < 0) {
      BF_PrintError("Opening file failed\n");
      return -1;
    }

    //Allocating info-block
    if(BF_AllocateBlock(fileDesc) < 0) {
      BF_PrintError("Allocating block failed\n");
      return -1;
    }

    //diavasma prwtou block tou file
    if(BF_ReadBlock(fileDesc, 0, &block) < 0) {
      BF_PrintError("Reading block failed\n");
      return -1;
    }

    //pernaw tis plhrofories sto prwto block
    memcpy(block, SHtable, 2*sizeof(char));
    memcpy(block + 2*sizeof(char), &FNlength, sizeof(int));//megethos fileName prwteuontos hashTable
    memcpy(block + 2*sizeof(char) + sizeof(int), fileName, FNlength*sizeof(char));//fileName proteuontos hashTable
    memcpy(block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char), &fileDesc, sizeof(int));
    memcpy(block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + sizeof(int), &attrLength, sizeof(int));
    memcpy(block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + 2*sizeof(int), attrName , attrLength*sizeof(char));
    memcpy(block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + 2*sizeof(int) + attrLength*sizeof(char), &buckets, sizeof(int));

    //grafw to prwto block
    if(BF_WriteBlock(fileDesc, 0) < 0) {
      BF_PrintError("Writing block failed\n");
      return -1;
    }

    ShashTable = (int*)malloc(buckets*sizeof(int));
    for(int i = 0 ; i < buckets ; i++)
      ShashTable[i] = -1;

    //kanw allocate kai ena deutero block
    if(BF_AllocateBlock(fileDesc) < 0) {
      BF_PrintError("Allocating block failed\n");
      return -1;
    }

    //pairnw th thesi tou deuterou block pou thelw na valw
    int blockNum = BF_GetBlockCounter(fileDesc)-1;

    //to diavazw
    if(BF_ReadBlock(fileDesc, blockNum, &block) < 0) {
      if(ShashTable != NULL)
          free(ShashTable);
      BF_PrintError("Reading block failed\n");
      return -1;
    }

    //pernaw sto block pou ekana allocate ton arxikopoihmeno pinaka
    memcpy(block, ShashTable, buckets*sizeof(int));

    //grafw to block sto arxeio
    if(BF_WriteBlock(fileDesc, blockNum) < 0) {
      BF_PrintError("Writing block failed\n");
      return -1;
    }

    if(BF_CloseFile(fileDesc) < 0) {
      BF_PrintError("Closing file failed\n");
      return -1;
    }

    free(ShashTable);
    free(SHtable);
    return 0;
}

SHT_info* SHT_OpenSecondaryIndex( char *sfileName ){
    int fileDesc;
    void* block = NULL;
    int FNlength;

    SHT_info* SInfo;

    //anoigma file
    if((fileDesc = BF_OpenFile(sfileName)) < 0) {
      BF_PrintError("Opening file failed\n");
      return NULL;
    }

    if(BF_ReadBlock(fileDesc, 0, &block) < 0) {
      BF_PrintError("Reading block failed\n");
      return NULL;
    }

    SInfo = malloc(sizeof(SHT_info));

    SInfo->fileDesc = fileDesc;
    //pernaw tis plhrofories tou block 0 sth domh
    memcpy(&FNlength, block + 2*sizeof(char), sizeof(int));
    SInfo->fileName = (char*)malloc(FNlength*sizeof(char));
    memcpy(SInfo->fileName, block + 2*sizeof(char) + sizeof(int), FNlength*sizeof(char));
    memcpy(&SInfo->attrLength, block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + sizeof(int), sizeof(int));
    SInfo->attrName = (char*)malloc(SInfo->attrLength*sizeof(char));
    memcpy(SInfo->attrName, block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + 2*sizeof(int), SInfo->attrLength*sizeof(char));
    memcpy(&SInfo->numBuckets, block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + 2*sizeof(int) + SInfo->attrLength*sizeof(char), sizeof(int));

    return SInfo;
}

int SHT_CloseSecondaryIndex( SHT_info* header_info ){
	  int fileDesc = header_info->fileDesc;

	  if (BF_CloseFile(fileDesc))
	  {
	      BF_PrintError("Closing file failed");
	      return -1;
	  }
	  free(header_info->attrName);
	  free(header_info->fileName);
	  free(header_info);

	  return 0;
}

int Secondary_Insert_in_new_Block(int fileDesc, int blockId){

			void* block = NULL;
			int block_recs;
			int next;
			int rec_block;

			if(BF_AllocateBlock(fileDesc) < 0) {
				BF_PrintError("Allocating block failed\n");
				//BF_PrintError("BFE_HEADOVERFLOW");
				return -1;
			}

			//pairnw to teleutaio block pou ekana allocate
			if((rec_block = BF_GetBlockCounter(fileDesc) - 1) < 0) {
				BF_PrintError("Get block counter failed\n");
				return -1;
			}

			//diavazw to block pou tha valw to rec
			if(BF_ReadBlock(fileDesc, rec_block, &block) < 0) {
				BF_PrintError("Reading block failed\n");
				return -1;
			}
			block_recs = 1; //8a exei ena record (auto pou 8a valw)
			next = -1;			//den exei epomeno
			memcpy(block, &block_recs, sizeof(int));
			memcpy(block + sizeof(int), &next, sizeof(int));
			memcpy(block + sizeof(int) + sizeof(int), &blockId, sizeof(int));
			//grafw to block sto arxeio
			if(BF_WriteBlock(fileDesc, rec_block) < 0) {
				BF_PrintError("Writing block failed\n");
				return -1;
			}
			return rec_block;		//epistrefw to block pou evala to record
}

int SHT_SecondaryInsertEntry( SHT_info header_info, SecondaryRecord record){

    void *block = NULL;
    int hash_result, rec_block, myblock, rec;
		int recs_per_block = 20;//BLOCK_SIZE/sizeof(int) - 8;
    //ta block twn records sthn prwth kai deuterh 8esh tous exoun autes tis metavlhtes
    int block_recs; //posa records exei to block
    int next; //poio einai to epomeno block
    int fileDesc = header_info.fileDesc;
    int buckets = header_info.numBuckets;
    int* ShashTable = malloc(buckets*sizeof(int));

    //diavazw to block me to secondary hash table
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
				BF_PrintError("Reading block3 failed\n");
		    return -1;
		}

    memcpy(ShashTable, block, buckets*sizeof(int));

    //vriskw se poia 8esh tou hash table antistoixei to record
		hash_result = secondary_hash_function(header_info, record.record);
    //an den uparxei block se auth th 8esh
		//desmeuw kainourio block to arxikopoiw kai enhmerwnw to file kai to hashTable
		if(ShashTable[hash_result] == -1){
				rec_block = Secondary_Insert_in_new_Block(fileDesc, record.blockId);
				ShashTable[hash_result] = rec_block;
		}
		else{
				rec_block = ShashTable[hash_result];

				if(BF_ReadBlock(fileDesc, rec_block, &block) < 0) {
						BF_PrintError("Reading block1 failed\n");
						return -1;
				}

				memcpy(&block_recs, block, sizeof(int));
				//printf("exw %d recs sto block %d\n", block_recs, rec_block);
				memcpy(&next, block + sizeof(int), sizeof(int));
				//elegxw an einai gemato
				int exists = 0;
				if(block_recs < recs_per_block){
						//elegxw an uparxei hdh to blockID sto block
						for(int i=0; i<block_recs; i++){
								memcpy(&rec, block + 2*sizeof(int) + i*sizeof(int), sizeof(int));
								if(rec == record.blockId){
										exists = 1;
								}
						}
						//an den einai vazw to record sthn epomenh 8esh
						if(exists == 0){
								memcpy(block + 2*sizeof(int) + block_recs*sizeof(int), &record.blockId, sizeof(int));
								block_recs++;
								memcpy(block, &block_recs, sizeof(int));

						}
						//grafw to block sto arxeio
						if(BF_WriteBlock(fileDesc, rec_block) < 0) {
								BF_PrintError("Writing block failed\n");
								return -1;
						}
				}
				//an einai gemato ftanw sto teleutaio block
				else{
						while(next != -1){//oso exei epomeno block
								if(BF_ReadBlock(fileDesc, next, &block) < 0) {//diavazw to epomeno
										BF_PrintError("Reading block2 failed\n");
										return -1;
								}
								rec_block = next;
								//krataw tis plhrofories tou
								memcpy(&block_recs, block, sizeof(int));
								memcpy(&next, block + sizeof(int), sizeof(int));
						}

						//an einai kai to teleutaio gemato ftiaxnw kainourio opws sthn prohgoumenh periptwsh
						if(block_recs == recs_per_block){
								myblock = rec_block;
								rec_block = Secondary_Insert_in_new_Block(fileDesc, record.blockId);

								if(BF_ReadBlock(fileDesc, myblock, &block) < 0) {
										BF_PrintError("Reading block failed\n");
										return -1;
								}

								memcpy(block + sizeof(int), &rec_block, sizeof(int));

								if(BF_WriteBlock(fileDesc, myblock) < 0) {
										BF_PrintError("Writing block failed\n");
										return -1;
								}
						}
						//alliws vazw to record sthn epomenh 8esh
						else{
							//elegxw an uparxei hdh to blockID sto block
								for(int i=0; i<block_recs; i++){
										memcpy(&rec, block + 2*sizeof(int) + i*sizeof(int), sizeof(int));
										if(rec == record.blockId){
												exists = 1;
										}
								}
								//an den einai vazw to record sthn epomenh 8esh
								if(exists == 0){
										memcpy(block + 2*sizeof(int) + block_recs*sizeof(int), &record.blockId, sizeof(int));
										block_recs++;
										memcpy(block, &block_recs, sizeof(int));
								}
								//grafw to block sto arxeio
								if(BF_WriteBlock(fileDesc, rec_block) < 0) {
										BF_PrintError("Writing block failed\n");
										return -1;
								}
						}
				}
		}
		//printf("%d\n", record.blockId);
		if(BF_ReadBlock(fileDesc, rec_block, &block) < 0) {
				BF_PrintError("Reading block failed\n");
				return -1;
		}
		int last;
		memcpy(&block_recs, block, sizeof(int));
		memcpy(&last, block + 2*sizeof(int) + (block_recs-1)*sizeof(int), sizeof(int));

		//printf("last rec = %d position = %d me block_recs = %d\n\n", last, block_recs-1, block_recs);

		if(BF_WriteBlock(fileDesc, rec_block) < 0) {
				BF_PrintError("Writing block failed\n");
				return -1;
		}

		//pernaw to enhmerwmeno hashTable sto block 1
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
				BF_PrintError("Reading block failed\n");
				return -1;
		}

		memcpy(block, ShashTable, buckets*sizeof(int));

		if(BF_WriteBlock(fileDesc, 1) < 0) {
				BF_PrintError("Writing block failed\n");
				return -1;
		}
		free(ShashTable);
		return 0;
}

int SHT_SecondaryGetAllEntries( SHT_info header_info_sht, HT_info header_info_ht, void *value){
		void *Hblock = NULL;
		void *Sblock = NULL;
		int hash_result, myblock, ht_block;
		int recs_per_block = 20;//BLOCK_SIZE/sizeof(int) - 8;
		//ta block twn records sthn prwth kai deuterh 8esh tous exoun autes tis metavlhtes
		int block_recs, Sblock_recs; //posa records exei to block
		int next, Snext; //poio einai to epomeno block
		int counter = 0;
		int key = 0;
		char* charkey = NULL;
		int flag = 1;
		int fileDesc = header_info_ht.fileDesc;
		int buckets = header_info_ht.numBuckets;
		int SfileDesc = header_info_sht.fileDesc;
		int length = header_info_sht.attrLength;
		Record record;
		int* ShashTable = malloc(buckets*sizeof(int));

		charkey = malloc(length*sizeof(char));
		strcpy(charkey, (char*)value);

		//diavazw to block me to secondary hash table
		if(BF_ReadBlock(SfileDesc, 1, &Sblock) < 0) {
			BF_PrintError("Reading block1 failed\n");
			return -1;
		}
		//pairnw to secondary hash table apo to block
		memcpy(ShashTable, Sblock, buckets*sizeof(int));
		/*for(int i=0; i<buckets; i++){
			printf("%d, ", ShashTable[i]);
		}
		printf("\n");*/

		//vriskw th 8esh sto secondary eurethrio
		if(strcmp(header_info_sht.attrName, "id") == 0){
				key = *(int*)value;
				hash_result = key%buckets;
		}
		else if(strcmp(header_info_sht.attrName, "name") == 0 || strcmp(header_info_sht.attrName, "surname") == 0 || strcmp(header_info_sht.attrName, "address") == 0){
				for(int i=0; i<length; i++){
						key += charkey[i];
				}
				hash_result = key%buckets;
		}
		//printf("hash res %d\n", hash_result);
		myblock = ShashTable[hash_result];
		//printf("%d\n", myblock);
		//an uparxei block me pedio kleidi tou value
		if(myblock != -1){
				while(flag){
						//8a vrw thn eggrafh prwta sto secondary
						if(BF_ReadBlock(SfileDesc, myblock, &Sblock) < 0){
								BF_PrintError("Reading block failed\n");
								return -1;
						}
						//printf("my block = %d\n", myblock);
						memcpy(&Sblock_recs, Sblock, sizeof(int));
						memcpy(&Snext, Sblock + sizeof(int), sizeof(int));
						counter++;
						//printf("%d\n", Sblock_recs);

						for(int i=0; i<Sblock_recs; i++){
								memcpy(&ht_block, Sblock + 2*sizeof(int) + i*sizeof(int), sizeof(int));

								//printf("ht block %d sth 8esh %d\n", ht_block, i);

								if(BF_ReadBlock(fileDesc, ht_block, &Hblock) < 0) {
										BF_PrintError("Reading block3 failed\n");
										return -1;
								}
								memcpy(&block_recs, Hblock, sizeof(int));
								//printf("block recs %d\n", block_recs);
								memcpy(&next, Hblock + sizeof(int), sizeof(int));
								for(int j=0; j<block_recs; j++){
										memcpy(&record, Hblock + sizeof(int) + sizeof(int) + j*sizeof(Record), sizeof(Record));
										if(strcmp(header_info_sht.attrName, "name") == 0){
												if(strcmp(record.name, charkey) == 0){
														printf("id %d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
														free(ShashTable);
														return counter;
												}
										}
										else if(strcmp(header_info_ht.attrName, "surname") == 0){
												if (strcmp(record.surname, charkey) == 0){
														printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
														free(ShashTable);
														return counter;
												}
										}
										else if(strcmp(header_info_ht.attrName, "address") == 0){
												if (strcmp(record.address, charkey) == 0){
														//printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
														free(ShashTable);
														return counter;
												}
										}
										else if (strcmp(header_info_ht.attrName, "id") == 0){
												if (record.id == *((int*)value)){
														//printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
														free(ShashTable);
														return counter;
												}
										}
								}
						}
						if(Snext == -1){
								flag = 0;
						}
						else{
								myblock = Snext;
						}
				}
		}
		free(ShashTable);
		return -1;
}
