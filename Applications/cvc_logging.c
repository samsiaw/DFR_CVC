/*
 * cvc_logging.c
 *
 *  Created on: Dec 7, 2018
 *      Author: f002bc7
 */

/* Includes ------------------------------------------------------------------*/
#include "cvc_logging.h"
#include "cvc_serial.h"
#include "stm32f7xx_nucleo_144.h"

/* Constants -----------------------------------------------------------------*/

/* The SD card is mounted in the root of the file system. */
#define mainHAS_SDCARD					1
#define mainSD_CARD_DISK_NAME			"/"
#define mainSD_CARD_TESTING_DIRECTORY	"/fattest"

/* The number of bytes read/written to the example files at a time. */
#define fsRAM_BUFFER_SIZE 				200

/* The number of bytes written to the file that uses f_putc() and f_getc(). */
#define fsPUTC_FILE_SIZE				100


/* Private variables ---------------------------------------------------------*/
/* The SD card disk. */
static FF_Disk_t *pxDisk = NULL;

/* Names of directories that are created. */
static const char *pcDirectory1 = "SUB1", *pcDirectory2 = "SUB2", *pcFullPath = "/SUB1/SUB2";


/* Private Function Prototypes ------------------------------------------------*/
static void prvCreateExampleFiles( void );
static void prvCreateDemoFilesUsing_ff_fwrite( const char *pcMountPath );
static void prvVerifyDemoFileUsing_ff_fread( void );
static void prvCreateDemoFileUsing_ff_fputc( const char *pcMountPath );
static void prvVerifyDemoFileUsing_ff_fgetc( const char *pcMountPath );


/* Public Functions ---------------------------------------------------------*/

void demoLoggingTask(void * parameters)
{

	const TickType_t xSDCardInsertDelay = pdMS_TO_TICKS( 1000UL );		/* Delay period for failed disk initialization */

	while( ( pxDisk = FF_SDDiskInit( mainSD_CARD_DISK_NAME ) ) == NULL )
	{
		vTaskDelay( xSDCardInsertDelay );
	}
	BSP_LED_Init(LED_BLUE);
	BSP_LED_Toggle(LED_BLUE);
	//console_write("SD card detected, creating example files.\n");
	prvCreateExampleFiles();
	//console_write("Example files created.\n");

	BSP_LED_Init(LED_GREEN);
	while(1)	{
		vTaskDelay(xSDCardInsertDelay);
		BSP_LED_Toggle(LED_GREEN);
	}
}

int lUDPLoggingPrintf( const char *apFmt, ... )
{
	return 1;
}

/* Private Functions ---------------------------------------------------------*/
/*
 * Creates files on the SD card disk.  The files can then be viewed via the FTP
 * server and the command line interface.
 */
static void prvCreateExampleFiles( void )
{
	//console_write("Mount SD-card\n");
	if( pxDisk != NULL )
	{
		/* Remove the base directory again, ready for another boot. */
		ff_deltree( mainSD_CARD_TESTING_DIRECTORY );

		/* Make sure that the testing directory exists. */
		ff_mkdir( mainSD_CARD_TESTING_DIRECTORY );

		/* Create and verify a few example files using both line based and character
		based reads and writes. */
		prvCreateDemoFilesUsing_ff_fwrite( mainSD_CARD_TESTING_DIRECTORY );
		prvVerifyDemoFileUsing_ff_fread();
		prvCreateDemoFileUsing_ff_fputc( mainSD_CARD_TESTING_DIRECTORY );
		prvVerifyDemoFileUsing_ff_fgetc( mainSD_CARD_TESTING_DIRECTORY );
	}
}

/*-----------------------------------------------------------*/
/*
 * Create a set of example files in the root directory of the volume using
 * ff_fwrite().
 */
static void prvCreateDemoFilesUsing_ff_fwrite( const char *pcMountPath )
{
BaseType_t xFileNumber, xWriteNumber;
const BaseType_t xMaxFiles = 5;
int32_t lItemsWritten;
int32_t lResult;
FF_FILE *pxFile;
char *pcRAMBuffer, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBuffer = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBuffer );
	configASSERT( pcFileName );

	/* Ensure in the root of the mount being used. */
	lResult = ff_chdir( pcMountPath );
	configASSERT( lResult >= 0 );

	/* Create xMaxFiles files.  Each created file will be
	( xFileNumber * fsRAM_BUFFER_SIZE ) bytes in length, and filled
	with a different repeating character. */
	for( xFileNumber = 1; xFileNumber <= xMaxFiles; xFileNumber++ )
	{
		/* Generate a file name. */
		snprintf( pcFileName, ffconfigMAX_FILENAME, "root%03d.txt", ( int ) xFileNumber );

		/* Obtain the current working directory and print out the file name and
		the	directory into which the file is being written. */
		ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
		//FF_PRINTF( "Creating file %s in %s\n", pcFileName, pcRAMBuffer );

		/* Open the file, creating the file if it does not already exist. */
		pxFile = ff_fopen( pcFileName, "w" );
		configASSERT( pxFile );

		/* Fill the RAM buffer with data that will be written to the file.  This
		is just a repeating ascii character that indicates the file number. */
		memset( pcRAMBuffer, ( int ) ( '0' + xFileNumber ), fsRAM_BUFFER_SIZE );

		/* Write the RAM buffer to the opened file a number of times.  The
		number of times the RAM buffer is written to the file depends on the
		file number, so the length of each created file will be different. */
		for( xWriteNumber = 0; xWriteNumber < xFileNumber; xWriteNumber++ )
		{
			lItemsWritten = ff_fwrite( pcRAMBuffer, fsRAM_BUFFER_SIZE, 1, pxFile );
			configASSERT( lItemsWritten == 1 );
		}

		/* Close the file so another file can be created. */
		ff_fclose( pxFile );
	}

	vPortFree( pcRAMBuffer );
	vPortFree( pcFileName );
}

/*-----------------------------------------------------------*/
/*
 * Use ff_fread() to read back and verify the files that were created by
 * prvCreateDemoFilesUsing_ff_fwrite().
 */
static void prvVerifyDemoFileUsing_ff_fread( void )
{
BaseType_t xFileNumber, xReadNumber;
const BaseType_t xMaxFiles = 5;
size_t xItemsRead, xChar;
FF_FILE *pxFile;
char *pcRAMBuffer, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBuffer = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBuffer );
	configASSERT( pcFileName );

	/* Read back the files that were created by
	prvCreateDemoFilesUsing_ff_fwrite(). */
	for( xFileNumber = 1; xFileNumber <= xMaxFiles; xFileNumber++ )
	{
		/* Generate the file name. */
		snprintf( pcFileName, ffconfigMAX_FILENAME, "root%03d.txt", ( int ) xFileNumber );

		/* Obtain the current working directory and print out the file name and
		the	directory from which the file is being read. */
		ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
		//FF_PRINTF( "Reading file %s from %s\n", pcFileName, pcRAMBuffer );

		/* Open the file for reading. */
		pxFile = ff_fopen( pcFileName, "r" );
		configASSERT( pxFile );

		/* Read the file into the RAM buffer, checking the file contents are as
		expected.  The size of the file depends on the file number. */
		for( xReadNumber = 0; xReadNumber < xFileNumber; xReadNumber++ )
		{
			/* Start with the RAM buffer clear. */
			memset( pcRAMBuffer, 0x00, fsRAM_BUFFER_SIZE );

			xItemsRead = ff_fread( pcRAMBuffer, fsRAM_BUFFER_SIZE, 1, pxFile );
			configASSERT( xItemsRead == 1 );

			/* Check the RAM buffer is filled with the expected data.  Each
			file contains a different repeating ascii character that indicates
			the number of the file. */
			for( xChar = 0; xChar < fsRAM_BUFFER_SIZE; xChar++ )
			{
				configASSERT( pcRAMBuffer[ xChar ] == ( '0' + ( char ) xFileNumber ) );
			}
		}

		/* Close the file. */
		ff_fclose( pxFile );
	}

	vPortFree( pcRAMBuffer );
	vPortFree( pcFileName );

	/*_RB_ also test what happens when attempting to read using too large item
	sizes, etc. */
}

/*-----------------------------------------------------------*/
/*
 * Create an example file in a sub-directory using f_putc().
 */
static void prvCreateDemoFileUsing_ff_fputc( const char *pcMountPath )
{
int32_t iReturn, iByte, iReturned;
FF_FILE *pxFile;
char *pcRAMBuffer, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBuffer = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBuffer );
	configASSERT( pcFileName );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "In directory %s\n", pcRAMBuffer );

	/* Create a sub directory. */
	iReturn = ff_mkdir( pcDirectory1 );
	configASSERT( iReturn == pdFREERTOS_ERRNO_NONE );

	/* Move into the created sub-directory. */
	iReturn = ff_chdir( pcDirectory1 );
	configASSERT( iReturn == pdFREERTOS_ERRNO_NONE );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "In directory %s\n", pcRAMBuffer );

	/* Create a subdirectory in the new directory. */
	iReturn = ff_mkdir( pcDirectory2 );
	configASSERT( iReturn == pdFREERTOS_ERRNO_NONE );

	/* Move into the directory just created - now two directories down from
	the root. */
	iReturn = ff_chdir( pcDirectory2 );
	configASSERT( iReturn == pdFREERTOS_ERRNO_NONE );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "In directory %s\n", pcRAMBuffer );
	snprintf( pcFileName, ffconfigMAX_FILENAME, "%s%s", pcMountPath, pcFullPath );
	configASSERT( strcmp( pcRAMBuffer, pcFileName ) == 0 );

	/* Generate the file name. */
	snprintf( pcFileName, ffconfigMAX_FILENAME, "%s.txt", pcDirectory2 );

	/* Print out the file name and the directory into which the file is being
	written. */
	//FF_PRINTF( "Writing file %s in %s\n", pcFileName, pcRAMBuffer );

	pxFile = ff_fopen( pcFileName, "w" );
	configASSERT( pxFile );

	/* Create a file 1 byte at a time.  The file is filled with incrementing
	ascii characters starting from '0'. */
	for( iByte = 0; iByte < fsPUTC_FILE_SIZE; iByte++ )
	{
		iReturned = ff_fputc( ( ( int ) '0' + iByte ), pxFile );
		configASSERT( iReturned ==  ( ( int ) '0' + iByte ) );
	}

	/* Finished so close the file. */
	ff_fclose( pxFile );

	/* Move back to the root directory. */
	iReturned = ff_chdir( "../.." );
	configASSERT( iReturn == pdFREERTOS_ERRNO_NONE );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "Back in root directory %s\n", pcRAMBuffer );
	configASSERT( strcmp( pcRAMBuffer, pcMountPath ) == 0 );

	vPortFree( pcRAMBuffer );
	vPortFree( pcFileName );
}

/*-----------------------------------------------------------*/
/*
 * Use f_getc() to read back and verify the file that was created by
 * prvCreateDemoFileUsing_f_putc().
 */
static void prvVerifyDemoFileUsing_ff_fgetc( const char *pcMountPath )
{
int iByte, iReturned;
FF_FILE *pxFile;
char *pcRAMBuffer, *pcFileName;

	/* Allocate buffers used to hold date written to/from the disk, and the
	file names. */
	pcRAMBuffer = ( char * ) pvPortMalloc( fsRAM_BUFFER_SIZE );
	pcFileName = ( char * ) pvPortMalloc( ffconfigMAX_FILENAME );
	configASSERT( pcRAMBuffer );
	configASSERT( pcFileName );

	/* Move into the directory in which the file was created. */
	snprintf( pcFileName, ffconfigMAX_FILENAME, "%s%s", pcMountPath, pcFullPath );
	iReturned = ff_chdir( pcFileName );
	configASSERT( iReturned == pdFREERTOS_ERRNO_NONE );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "Back in directory %s\n", pcRAMBuffer );
	configASSERT( strcmp( pcRAMBuffer, pcFileName ) == 0 );

	/* pcFileName is about to be overwritten - take a copy. */
	strcpy( pcRAMBuffer, pcFileName );

	/* Generate the file name. */
	sprintf( pcFileName, "%s.txt", pcDirectory2 );

	/* Print out the file name and the directory from which the file is being
	read. */
	//FF_PRINTF( "Reading file %s in %s\n", pcFileName, pcRAMBuffer );

	/* This time the file is opened for reading. */
	pxFile = ff_fopen( pcFileName, "r" );

	/* Read the file 1 byte at a time. */
	for( iByte = 0; iByte < fsPUTC_FILE_SIZE; iByte++ )
	{
		iReturned = ff_fgetc( pxFile );
		configASSERT( iReturned ==  ( ( int ) '0' + iByte ) );
	}

	/* Should not be able to read another bytes. */
	iReturned = ff_fgetc( pxFile );
	configASSERT( iReturned ==  FF_EOF );

	/* Finished so close the file. */
	ff_fclose( pxFile );

	/* Move back to the root directory. */
	iReturned = ff_chdir( "../.." );

	/* Obtain and print out the working directory. */
	ff_getcwd( pcRAMBuffer, fsRAM_BUFFER_SIZE );
	//FF_PRINTF( "Back in root directory %s\n", pcRAMBuffer );

	vPortFree( pcRAMBuffer );
	vPortFree( pcFileName );
}
