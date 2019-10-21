#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "BF.h"
#include "HT.h"

int hash_function(HT_info header_info, Record record){

		char* key;
		int first_letter;
		int buckets = header_info.numBuckets;
		int length = header_info.attrLength;

		key = malloc(length*sizeof(char));
		strcpy(key, header_info.attrName);

		if(strcmp(key, "id") == 0){
				//an to key einai int tote dinw san apotelesma hash to upoloipo tou me ta sunolika buckets
				free(key);
				return record.id%buckets;
		}
		//an to key einai string pairnw ton int apo to to prwto gramma kai epistrefw to upoloipo me to numBuckets
		else if(strcmp(key, "name") == 0){
				free(key);
				first_letter = record.name[0] - '0';
				return first_letter%buckets;
		}
		else if(strcmp(key, "surname") == 0){
				free(key);
				first_letter = record.surname[0] - '0';
				return first_letter%buckets;
		}
		else if(strcmp(key, "address") == 0){
				free(key);
				first_letter = record.address[0] - '0';
				return first_letter%buckets;
		}
		else{
				free(key);
				printf("error sto hash funsction\n");
				exit(0);
		}
}

int HT_CreateIndex( char *fileName, char attrType, char* attrName, int attrLength, int buckets) {
		int fileDesc;
		int* hashTable = NULL;
		void* block = NULL;
		char* Hashtable = NULL;
		int recs_per_block = BLOCK_SIZE/sizeof(Record);//to -1 einai gia to xwro pou theloume se kathe block na krata posa records exei

		Hashtable = malloc(2*sizeof(char));
		strncpy(Hashtable, "HT", 2);				//to anagnwristiko tou arxeiou hash table

		//dhmiourgia file
		if(BF_CreateFile(fileName) < 0) {
			BF_PrintError("Creating file failed\n");
			return -1;
		}

		//anoigma file
		if((fileDesc = BF_OpenFile(fileName)) < 0) {
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
		memcpy(block, Hashtable, 2*sizeof(char));
		memcpy(block + 2*sizeof(char), &fileDesc, sizeof(int));
		memcpy(block + 2*sizeof(char) + sizeof(int), &attrType, sizeof(char));
		memcpy(block + 2*sizeof(char) + sizeof(int) + sizeof(char), &attrLength, sizeof(int));
		memcpy(block + 2*sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int), attrName , attrLength*sizeof(char));
		memcpy(block + 2*sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int) + attrLength*sizeof(char), &buckets, sizeof(int));

	  //grafw to prwto block
		if(BF_WriteBlock(fileDesc, 0) < 0) {
			BF_PrintError("Writing block failed\n");
	    return -1;
		}

		hashTable = (int*)malloc(buckets*sizeof(int));
		for(int i = 0 ; i < buckets ; i++)
			hashTable[i] = -1;

		//kanw allocate kai ena deutero block
		if(BF_AllocateBlock(fileDesc) < 0) {
			BF_PrintError("Allocating block failed\n");
	    return -1;
		}

		//pairnw th thesi tou deuterou block pou thelw na valw
		int blockNum = BF_GetBlockCounter(fileDesc)-1;

		//to diavazw
		if(BF_ReadBlock(fileDesc, blockNum, &block) < 0) {
			if(hashTable != NULL)
					free(hashTable);
			BF_PrintError("Reading block failed\n");
	    return -1;
		}

		//pernaw sto block pou ekana allocate ton arxikopoihmeno ppinaka
		memcpy(block, hashTable, buckets*sizeof(int));

		//grafw to block sto arxeio
		if(BF_WriteBlock(fileDesc, blockNum) < 0) {
			BF_PrintError("Writing block failed\n");
	    return -1;
		}

		if(BF_CloseFile(fileDesc) < 0) {
			BF_PrintError("Closing file failed\n");
			return -1;
		}

		free(hashTable);
		free(Hashtable);
}

HT_info* HT_OpenIndex(char *fileName){
		int fileDesc;
		void* block = NULL;

		HT_info* Info;

		//anoigma file
		if((fileDesc = BF_OpenFile(fileName)) < 0) {
			BF_PrintError("Opening file failed\n");
			return NULL;
		}

		if(BF_ReadBlock(fileDesc, 0, &block) < 0) {
			BF_PrintError("Reading block failed\n");
	    return NULL;
		}

		Info = malloc(sizeof(HT_info));

		Info->fileDesc = fileDesc;
		//pernaw tis plhrofories tou block 0 sth domh
		memcpy(&Info->attrType, block + 2*sizeof(char) + sizeof(int), sizeof(char));
		memcpy(&Info->attrLength, block + 2*sizeof(char) + sizeof(int) + sizeof(char), sizeof(int));
		Info->attrName = (char*)malloc(Info->attrLength*sizeof(char));
		memcpy(Info->attrName, block + 2*sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int), Info->attrLength*sizeof(char));
		memcpy(&Info->numBuckets, block + 2*sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int) + Info->attrLength*sizeof(char), sizeof(int));

		return Info;
}

int HT_CloseIndex(HT_info* header_info){
		int fileDesc = header_info->fileDesc;

		if (BF_CloseFile(fileDesc))
		{
				BF_PrintError("Closing file failed");
				return -1;
		}

		free(header_info->attrName);
		free(header_info);

		return 0;
}

int Insert_in_new_Block(int fileDesc, Record record){

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
			memcpy(block + sizeof(int) + sizeof(int), &record, sizeof(Record));

			//grafw to block sto arxeio
			if(BF_WriteBlock(fileDesc, rec_block) < 0) {
				BF_PrintError("Writing block failed\n");
				return -1;
			}
			return rec_block;		//epistrefw to block pou evala to record
}

int HT_InsertEntry(HT_info header_info, Record record){

		void *block = NULL;
		int hash_result, rec_block, myblock;
		int recs_per_block = BLOCK_SIZE/sizeof(Record);
		//ta block twn records sthn prwth kai deuterh 8esh tous exoun autes tis metavlhtes
		int block_recs; //posa records exei to block
		int next; //poio einai to epomeno block
		int fileDesc = header_info.fileDesc;
		int buckets = header_info.numBuckets;
		int* hashTable = malloc(buckets*sizeof(int));

		//diavazw to block me to hash table
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
			BF_PrintError("Reading block failed\n");
	    return -1;
		}

		//pairnw to hash table apo to block
		memcpy(hashTable, block, buckets*sizeof(int));

		//vriskw se poia 8esh tou hash table antistoixei to record
		hash_result = hash_function(header_info, record);

		//an den uparxei block se auth th 8esh
		//desmeuw kainourio block to arxikopoiw kai enhmerwnw to file kai to hashTable
		if(hashTable[hash_result] == -1){
				rec_block = Insert_in_new_Block(fileDesc, record);
				hashTable[hash_result] = rec_block;
		}
		//alliws exw block gia na valw to record
		else{
				//pairnw to block apo to hash table
				rec_block = hashTable[hash_result];
				if(BF_ReadBlock(fileDesc, rec_block, &block) < 0) {
					BF_PrintError("Reading block failed\n");
					return -1;
				}

				//pairnw tis plhrofories gia ta recs kai to next
				memcpy(&block_recs, block, sizeof(int));
				memcpy(&next, block + sizeof(int), sizeof(int));
				//elegxw an einai gemato
				if(block_recs < recs_per_block){
						//an den einai vazw to record sthn epomenh 8esh
						memcpy(block + 2*sizeof(int) + block_recs*sizeof(Record), &record, sizeof(Record));
						block_recs++;
						memcpy(block, &block_recs, sizeof(int));
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
										BF_PrintError("Reading block failed\n");
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
								rec_block = Insert_in_new_Block(fileDesc, record);

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
								memcpy(block + 2*sizeof(int) + block_recs*sizeof(Record), &record, sizeof(Record));
								block_recs++;
								memcpy(block, &block_recs, sizeof(int));
								//grafw to block sto arxeio
								if(BF_WriteBlock(fileDesc, rec_block) < 0) {
										BF_PrintError("Writing block failed\n");
										return -1;
								}
						}
				}
		}

		//penraw to enhmerwmeno hashTable sto block 1
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
				BF_PrintError("Reading block failed\n");
				return -1;
		}

		memcpy(block, hashTable, buckets*sizeof(int));

		if(BF_WriteBlock(fileDesc, 1) < 0) {
				BF_PrintError("Writing block failed\n");
				return -1;
		}
		free(hashTable);
		return rec_block;
}

void delete_from_block(int myblock, Record record, HT_info header_info, int position){
		void* block = NULL;
		int fileDesc = header_info.fileDesc;
		int block_recs;
		int i = position;

		if(BF_ReadBlock(fileDesc, myblock, &block) < 0) {
				BF_PrintError("Reading block failed\n");
				exit(0);
		}

		memcpy(&block_recs, block, sizeof(int));

		while(i != block_recs){
				memcpy(block + sizeof(int) + sizeof(int) + i*sizeof(Record), block + sizeof(int) + sizeof(int) + (i+1)*sizeof(Record), sizeof(Record));
				i++;
		}
		block_recs--;

		memcpy(block, &block_recs, sizeof(int));

		if(BF_WriteBlock(fileDesc, myblock) < 0) {
				BF_PrintError("Writing block failed\n");
				exit(0);
		}
		return;
}

int HT_DeleteEntry(	HT_info header_info, void* value){
		//printf("Delete entry\n");
		//printf("Get All Entries\n");
		void *block = NULL;
		int hash_result, myblock;
		int recs_per_block = BLOCK_SIZE/sizeof(Record);
		//ta block twn records sthn prwth kai deuterh 8esh tous exoun autes tis metavlhtes
		int block_recs; //posa records exei to block
		int next; //poio einai to epomeno block
		int counter = 0;
		int key = 1;
		char* charkey = NULL;
		int flag = 1;
		int fileDesc = header_info.fileDesc;
		int buckets = header_info.numBuckets;
		int length = header_info.attrLength;
		Record record;
		int* hashTable = malloc(buckets*sizeof(int));

		//diavazw to block me to hash table
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
			BF_PrintError("Reading block failed\n");
			return -1;
		}

		//pairnw to hash table apo to block
		memcpy(hashTable, block, buckets*sizeof(int));

		if(strcmp(header_info.attrName, "id") == 0){
				key = *(int*)value;
				hash_result = key%buckets;
		}
		else if(strcmp(header_info.attrName, "name") == 0){
				charkey = malloc(length*sizeof(char));
				strcpy(charkey, (char*)value);
				key = charkey[0] - '0';
				hash_result = key%buckets;
				free(charkey);
		}

		myblock = hashTable[hash_result];

		if(myblock!=-1)
		{
				while(flag)
				{
						if(BF_ReadBlock(fileDesc, myblock, &block) < 0) {
								BF_PrintError("Reading block failed\n");
								return -1;
						}

						memcpy(&block_recs, block, sizeof(int));
						memcpy(&next, block + sizeof(int), sizeof(int));

						counter++;

						for(int i=0; i<block_recs; i++){
								memcpy(&record, block + sizeof(int) + sizeof(int) + i*sizeof(Record), sizeof(Record));
								//printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
								if(strcmp(header_info.attrName,"name")==0)
								{
										if (strcmp(record.name, (char*)value) == 0){
												delete_from_block(myblock, record, header_info, i);
												free(hashTable);
												return 0;
										}
								}
								else if(strcmp(header_info.attrName, "surname") == 0)
								{
										if (strcmp(record.surname, (char*)value) == 0){
												delete_from_block(myblock, record, header_info, i);
												free(hashTable);
												return 0;
										}
								}
								else if(strcmp(header_info.attrName, "address") == 0)
								{
										if (strcmp(record.address, (char*)value) == 0){
												delete_from_block(myblock, record, header_info, i);
												free(hashTable);
												return 0;
										}
								}
								else if (strcmp(header_info.attrName, "id") == 0)
								{
										if (record.id==*((int*)value)){
												//printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
												delete_from_block(myblock, record, header_info, i);
												free(hashTable);
												return 0;
										}
								}
						}

						if(next == -1){
								flag = 0;
						}
						else{
								myblock = next;
						}
				}
		}
		free(hashTable);
		return -1;
}

int HT_GetAllEntries(HT_info header_info, void *value){
		//printf("Get All Entries\n");
		void *block = NULL;
		int hash_result, myblock;
		int recs_per_block = BLOCK_SIZE/sizeof(Record);
		//ta block twn records sthn prwth kai deuterh 8esh tous exoun autes tis metavlhtes
		int block_recs; //posa records exei to block
		int next; //poio einai to epomeno block
		int counter = 0;
		int key = 1;
		char* charkey = NULL;
		int flag = 1;
		int fileDesc = header_info.fileDesc;
		int buckets = header_info.numBuckets;
		int length = header_info.attrLength;
		Record record;
		int* hashTable = malloc(buckets*sizeof(int));

		//diavazw to block me to hash table
		if(BF_ReadBlock(fileDesc, 1, &block) < 0) {
			BF_PrintError("Reading block failed\n");
			return -1;
		}

		//pairnw to hash table apo to block
		memcpy(hashTable, block, buckets*sizeof(int));

		if(strcmp(header_info.attrName, "id") == 0){
				key = *(int*)value;
				hash_result = key%buckets;
		}
		else if(strcmp(header_info.attrName, "name") == 0 || strcmp(header_info.attrName, "surname") == 0 || strcmp(header_info.attrName, "address") == 0){
				for(int i=0; i<length; i++){
						key += charkey[i];
				}
				hash_result = key%buckets;
		}

		//printf("hash %d\n", hash_result);
		//hash_result = hash_function(header_info, value);

		myblock = hashTable[hash_result];
		//printf("my block %d\n", myblock);
		//an uparxei block me pedio kleidi tou value
		if(myblock!=-1){
				while(flag){
						if(BF_ReadBlock(fileDesc, myblock, &block) < 0){
								BF_PrintError("Reading block failed\n");
								return -1;
						}

						memcpy(&block_recs, block, sizeof(int));
						memcpy(&next, block + sizeof(int), sizeof(int));

						counter++;

						for(int i=0; i<block_recs; i++){
								memcpy(&record, block + sizeof(int) + sizeof(int) + i*sizeof(Record), sizeof(Record));
								//printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
								if(strcmp(header_info.attrName, "name") == 0){
										if(strcmp(record.name, charkey) == 0){
												printf("id %d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
												free(hashTable);
												return counter;
										}
								}
								else if(strcmp(header_info.attrName, "surname") == 0){
										if (strcmp(record.surname, charkey) == 0){
												printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
												free(hashTable);
												return counter;
										}
								}
								else if(strcmp(header_info.attrName, "address") == 0){
										if (strcmp(record.address, charkey) == 0){
												printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
												free(hashTable);
												return counter;
										}
								}
								else if (strcmp(header_info.attrName, "id") == 0){
										if (record.id == *((int*)value)){
												printf("%d, \"%s\", \"%s\", \"%s\"\n",record.id,record.name,record.surname,record.address);
												free(hashTable);
												return counter;
										}
								}
						}

						if(next == -1){
								flag = 0;
						}
						else{
								myblock = next;
						}
				}
		}
		else{
				printf("NO matches\n");
		}
		free(hashTable);
		return -1;
}

int HashStatistics( char* filename ){
		//printf("Statistics\n");
		void* block = NULL;
		int fileDesc;
		int buckets;
		int length, FNlength;
		int recs_per_block;
		int block_recs, next, myblock;
		int min, max, med_rec, med_res, overflow, buckets_with_overflow, med_overflow;
		int all_buckets = 2;
		char* anagnwristiko = malloc(2*sizeof(char));
		int* hashTable = NULL;

		if((fileDesc = BF_OpenFile(filename)) < 0) {
				BF_PrintError("Opening file failed\n");
				return -1;
		}

		if(BF_ReadBlock(fileDesc, 0, &block) < 0){
				BF_PrintError("Reading block failed\n");
				return -1;
		}

		strcpy(anagnwristiko, block);

		if(strcmp(anagnwristiko, "HT") == 0){
				memcpy(&length, block + 2*sizeof(char) + sizeof(int) + sizeof(char), sizeof(int));
				memcpy(&buckets, block + 2*sizeof(char) + sizeof(int) + sizeof(char) + sizeof(int) + length*sizeof(char), sizeof(int));
				recs_per_block = BLOCK_SIZE/sizeof(Record);
				hashTable = malloc(buckets*sizeof(int));
		}
		else{
				memcpy(&FNlength, block + 2*sizeof(char), sizeof(int));//megethos fileName prwteuontos hashTable
				memcpy(&length, block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + sizeof(int), sizeof(int));
				memcpy(&buckets, block + 2*sizeof(char) + sizeof(int) + FNlength*sizeof(char) + 2*sizeof(int) + length*sizeof(char), sizeof(int));
				recs_per_block = 20;//BLOCK_SIZE/sizeof(int) - 8;
				hashTable = malloc(buckets*sizeof(int));
		}

		buckets_with_overflow = 0;
		med_overflow = 0;

		if(BF_ReadBlock(fileDesc, 1, &block) < 0){
				BF_PrintError("Reading block failed\n");
				return -1;
		}

		memcpy(hashTable, block, buckets*sizeof(int));

		for(int i=0; i<buckets; i++){

				myblock = hashTable[i];
				//printf("my block = %d\n", myblock);
				overflow = 0;
				med_rec = 0;
				min = recs_per_block;
				max = 0;

				if(myblock != -1){
						all_buckets++;

						if(BF_ReadBlock(fileDesc, myblock, &block) < 0){
								BF_PrintError("Reading block failed\n");
								return -1;
						}

						memcpy(&block_recs, block, sizeof(int));
					//	printf("%d\n", block_recs);
						memcpy(&next, block + sizeof(int), sizeof(int));

						if(block_recs < min)
								min = block_recs;

						if(block_recs > max)
								max = block_recs;

						med_rec += block_recs;

						if(myblock!=hashTable[i])
								overflow++;

						while(next != -1){

								myblock = next;

								if(BF_ReadBlock(fileDesc, myblock, &block) < 0){
										BF_PrintError("Reading block failed\n");
										return -1;
								}
								memcpy(&block_recs, block, sizeof(int));
						//		printf("%d\n", block_recs);
								memcpy(&next, block + sizeof(int), sizeof(int));

								if(block_recs < min)
										min = block_recs;

								if(block_recs > max)
										max = block_recs;

								med_rec += block_recs;

								if(myblock!=hashTable[i])
										overflow++;

								all_buckets++;
						}
						if(overflow!=0){
								buckets_with_overflow++;
						}
						med_res = med_rec / (overflow + 1);
						med_overflow += overflow;
				}
				else{
						min = 0;
						max = 0;
						med_res = 0;
				}
				//Print results
				printf("HashTable[%d] exei min records %d, max records %d, med records %d kai overflow %d blocks\n", i, min, max, med_res, overflow);
		}
		printf("sto arxeio %s exw sunolika %d block, %d buckets exoun overflow blocks kai o mesos ari8mos blocks tou backet einai %d\n", filename, all_buckets, buckets_with_overflow, med_overflow/buckets);

		return -1;
}
