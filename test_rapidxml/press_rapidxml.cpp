#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

#include "rapidxml.hpp"

static long _GetFileSize(char *filename)
{
	struct stat stat_buf;
	int ret;

	ret=stat(filename,&stat_buf);
	
	if( ret == -1 )
		return -1;
	
	return stat_buf.st_size;
}

static int ReadEntireFile( char *filename , char *mode , char *buf , long *bufsize )
{
	FILE	*fp = NULL ;
	long	filesize ;
	long	lret ;
	
	if( filename == NULL )
		return -1;
	if( ! strcmp( filename , "" ) )
		return -1;
	
	filesize = _GetFileSize( filename ) ;
	if( filesize  < 0 )
		return -2;
	
	fp = fopen( filename , mode ) ;
	if( fp == NULL )
		return -3;
	
	if( filesize <= (*bufsize) )
	{
		lret = fread( buf , sizeof(char) , filesize , fp ) ;
		if( lret < filesize )
		{
			fclose( fp );
			return -4;
		}
		
		(*bufsize) = filesize ;
		
		fclose( fp );
		
		return 0;
	}
	else
	{
		lret = fread( buf , sizeof(char) , (*bufsize) , fp ) ;
		if( lret < (*bufsize) )
		{
			fclose( fp );
			return -4;
		}
		
		fclose( fp );
		
		return 1;
	}
}

static int ReadEntireFileSafely( char *filename , char *mode , char **pbuf , long *pbufsize )
{
	long	filesize ;
	
	int	nret ;
	
	filesize = _GetFileSize( filename );
	
	(*pbuf) = (char*)malloc( filesize + 1 ) ;
	if( (*pbuf) == NULL )
		return -1;
	memset( (*pbuf) , 0x00 , filesize + 1 );
	
	nret = ReadEntireFile( filename , mode , (*pbuf) , & filesize ) ;
	if( nret )
	{
		free( (*pbuf) );
		(*pbuf) = NULL ;
		return nret;
	}
	else
	{
		if( pbufsize )
			(*pbufsize) = filesize ;
		return 0;
	}
}

int main( int argc , char *argv[] )
{
	char	*xml_buffer = NULL ;
	int	xml_len ;
	
	if( argc == 1 + 2 )
	{
		char		*xml_buffer_bak = NULL ;
		int		c , count ;
		int		nret = 0 ;
		
		nret = ReadEntireFileSafely( argv[1] , "rb" , & xml_buffer_bak , NULL ) ;
		if( nret )
		{
			printf( "ReadEntireFileSafely[%s] failed[%d]\n" , argv[1] , nret );
			return nret;
		}
		xml_buffer = strdup( xml_buffer_bak ) ;
		if( xml_buffer == NULL )
		{
			printf( "strdup failed , errno[%d]\n" , errno );
			return 1;
		}
		xml_len = strlen(xml_buffer) ;
		
		count = atol(argv[2]) ;
		if( count > 0 )
		{
			time_t  t1 , t2 ;
			
			time( & t1 );
			for( c = 0 ; c < count ; c++ )
			{
				using namespace rapidxml;
				xml_document<> doc;    // character type defaults to char
				doc.parse<0>(xml_buffer);    // 0 means default parse flags
				
				memcpy( xml_buffer , xml_buffer_bak , xml_len );
			}
			time( & t2 );
			printf( "Elapse %d seconds\n" , (int)(t2-t1) );
		}
		else
		{
			int	len ;
			
			count = -count ;
			len = 0 ;
			for( c = 0 ; c < count ; c++ )
			{
				len += strlen( xml_buffer_bak + c % 10 ) ;
			}
			printf( "len[%d]\n" , len );
		}
	}
	else
	{
		printf( "USAGE : press_fastxml xml_pathfilename press_count\n" );
		exit(7);
	}
	
	return 0;
}
