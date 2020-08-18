//@author: Patrick Canny, Andrew Growney, Liam Ormiston
//@brief: A cache simulation in C for EECS645
//  Created by Gary J. Minden on 9/24/15.
//  Copyright 2015 Gary J. Minden. All rights reserved.
//
//  Updates:
//
//      B40922 -- Added a loop with random indices
//
//
//	Description:
//			This program simulates a multi-way direct mapped cache.
//

#include <stdio.h>
#include <time.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>

//Cache Block Struct
typedef struct cacheBlock
{
	uint32_t valid;
	uint32_t tag;
	uint32_t line;

} cacheBlock;
//global round robin state array to keep track of the RRstate for every set
// initialize values of RRstate array to all 0's
int RRstate [64] = {0};

//
//	Definitions of the cache
//
//	The cache size exponent and capacity in bytes
//
#define  CacheSize_Exp  15
#define  CacheSize_Nbr  ( 1 << CacheSize_Exp )

//
//	Address size exponent
//
#define  AddressSize_Exp  32

//
//	The cache associativity
//
#define  CacheAssociativity_Exp 0
#define  CacheAssociativity ( 3 << CacheAssociativity_Exp )

//
//	The cache block size exponent, capacity in bytes, and mask
//
#define  BlockSize_Exp  6
#define  BlockSize_Nbr  ( 1 << BlockSize_Exp )
#define  BlockSize_Mask  ( BlockSize_Nbr - 1 )

//
//	The number of lines in the cache. A line can contain multiple blocks
//
#define  Lines_Exp   ( (CacheSize_Exp) - (CacheAssociativity_Exp + BlockSize_Exp) )
#define  Lines_Nbr   ( 1 << Lines_Exp )
#define  Lines_Mask   ( Lines_Nbr - 1 )

//
//	Tag size exponent and mask
//
#define  Tag_Exp    ( AddressSize_Exp - BlockSize_Exp - Lines_Exp )
#define  Tag_Nbr    ( 1 << Tag_Exp )
#define  Tag_Mask   ( Tag_Nbr - 1 )

//Global Cache variable
cacheBlock cache[Lines_Nbr][CacheAssociativity];

//Globals for tracking hits and misses
int hits = 0;
int misses = 0;

//=============================================================================
// FUNCTION DECLARATIONS
//
//	Function to report defined values.
//
void ReportParameters (const char* theFilename ) {

	printf( "Filename: %s\n", theFilename );

	printf( "Cache Parameters: CacheSize_Exp: %08X; CacheSize_Nbr: %08X\n",
	CacheSize_Exp, CacheSize_Nbr );

	printf( "Address size: AddressSize_Exp: %08X\n", AddressSize_Exp );

	printf( "Cache Associativity: %08X\n", CacheAssociativity );

	printf( "Block Parameters: BlockSize_Exp: %08X; BlockSize_Nbr: %08X; BlockSize_Mask: %08X\n",
	BlockSize_Exp, BlockSize_Nbr, BlockSize_Mask );

	printf( "Line Parameters: Lines_Exp: %08X; Lines_Nbr: %08X; Lines_Mask: %08X\n",
	Lines_Exp, Lines_Nbr, Lines_Mask );

	printf( "Tag Parameters: Tag_Exp: %08X; Tag_Nbr: %08X; Tag_Mask: %08X\n",
	Tag_Exp, Tag_Nbr, Tag_Mask );

}

bool RoundRobin(cacheBlock *line, uint32_t j, int RRstate[]) {
	for(int i = 0; i < 8; i++) {
		// if valid = 1 means that a value exists at that index
		if (cache[i][j].valid == 1 && cache[i][j].tag == line->tag) {
			// value trying to insert already exists
			return true;
		}
		else {
			// found an empty line
			cache[i][j] = *line;
			cache[i][j].valid = 1;
			return false;
		}
	}
	// Reached the end of the set. Need to round robin replace.
	// finds where the new line should go
	int new_line = RRstate[j]%8;
	// puts the new line into the cache where the RRstate said it belongs
	cache[new_line][j] = *line;
	// increments RRstate to know where to insert a new line next time
	RRstate[j]++;
	return false;
}
//@pre: Cache is not initialized
//@post: initialize cache sequentially
//@return: none
//@brief: This fucntion builds out the global cache object that we will use
void buildCache(){
	for(size_t i = 0; i < Lines_Nbr; i++) {
		for (size_t j = 0; j < CacheAssociativity; j++) {
			cache[i][j].valid = 0;
			cache[i][j].tag = 0;
			cache[i][j].line = 0;
		}
	}
}

//@pre: a 32-Bit Address
//@post: no change to main cache
//@return: Extracted Line From Address
int ParseLineFromAddress(uint32_t MyAddress){
	return (MyAddress >> BlockSize_Exp) & Lines_Mask;
}

//@pre: 32-Bit Address
//@post: no change to main cache
//@return: Extracted Tag From address
int ParseTagFromAddress(uint32_t MyAddress){
	return ((MyAddress >> (BlockSize_Exp + Lines_Exp)) & Tag_Mask);
}

//@pre:
//@post:
cacheBlock* BuildCacheBlockFromTagAndLine(int tag, int line){
	cacheBlock* newBlock = malloc(sizeof(cacheBlock));
	newBlock->line = line;
	newBlock->tag = tag;
	newBlock->valid = 1;
	return newBlock;
}

//=============================================================================
//
//	Main function
//
int main (int argc, const char * argv[]) {
	if (argc < 2) {
		printf("Too Few Arguments\nUSAGE: ./cacheSim <Desired Binary Input File>\n");
		return 0;
	}
	else{
		//
		//	Local variables
		//
		float hitRatio = 0.0;
		FILE *myBinaryFile;
		const char* file_name = argv[1];

		//
		//	Allocate a Cache Sim
		//
		buildCache();

		//
		//	Report cache parameters
		//
		ReportParameters(file_name);

		//
		//	Open address trace file, reset counters, and process Accesses_Max addresses
		//
		//Need to Set Up variables Here for Searching the Cache

		int blockIndex = 0;
		int found_Value = 0;
		uint32_t cache_Line = 0;
		uint32_t cache_Tag = 0;
		int iter = 0;
		cacheBlock* testerBlock = malloc(sizeof(cacheBlock));
		//This is going to be your buffer Growney
		uint32_t GrowneyAddress = 0;

		//
		// Growney's Variables
		//
		int limit = 0;

		unsigned char buffer[4];
		uint32_t instruction;
		myBinaryFile = fopen(file_name,"rb");

		while (!feof(myBinaryFile)) {
			fread(&buffer,sizeof(buffer),1,myBinaryFile);
			memcpy(&instruction,buffer,4);

			for(int k=0;k <4;k++){
				buffer[k]=0;
			}
			limit++;

			GrowneyAddress = instruction;

			cache_Line = ParseLineFromAddress(GrowneyAddress);
			cache_Tag = ParseTagFromAddress(GrowneyAddress);
			testerBlock = BuildCacheBlockFromTagAndLine(cache_Tag, cache_Line);

			// NOTE: testerBlock = cacheBlock; line of file we just read
			//			 cache_Line = cacheBlock; our 2D array cache
			//			 RRstate = array; The global RRstate array;
			if(RoundRobin(testerBlock, cache_Line, RRstate)) {
				hits++;
			}
			else {
				misses++;
			}
		}

		//
		//	Report cache performance
		//
		hitRatio = ((float)(hits)/((float)(hits+misses))*100.00);
		printf("\nTotal Addresses processed: %d", limit);
		printf("\nHits: %d", hits);
		printf("\nMisses: %d", misses);
		printf("\nHit Ratio: %f\n", hitRatio);

		fclose(myBinaryFile);
		//
		//	Return 1 for success
		//
		return( 1 );
	}
}
