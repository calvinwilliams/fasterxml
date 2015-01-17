#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "fasterxml.h"

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
	if( strcmp( filename , "" ) == 0 )
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

funcCallbackOnXmlProperty CallbackOnXmlProperty ;
int CallbackOnXmlProperty( char *xpath , int xpath_len , int xpath_size , char *propname , int propname_len , char *propvalue , int propvalue_len , void *p )
{
	printf( "    PROPERTY p[%s] xpath[%.*s] propname[%.*s] propvalue[%.*s]\n" , (char*)p , xpath_len , xpath , propname_len , propname , propvalue_len , propvalue );
	
	return 0;
}

funcCallbackOnXmlNode CallbackOnXmlNode ;
int CallbackOnXmlNode( int type , char *xpath , int xpath_len , int xpath_size , char *nodename , int nodename_len , char *properties , int properties_len , char *content , int content_len , void *p )
{
	int		nret = 0 ;
	
	if( type & FASTERXML_NODE_BRANCH )
	{
		if( type & FASTERXML_NODE_ENTER )
		{
			printf( "ENTER-BRANCH p[%s] xpath[%.*s] nodename[%.*s] properties[%.*s] content[%.*s]\n" , (char*)p , xpath_len , xpath , nodename_len , nodename , properties_len , properties , content_len , content );
		}
		else if( type & FASTERXML_NODE_LEAVE )
		{
			printf( "LEAVE-BRANCH p[%s] xpath[%.*s] nodename[%.*s] properties[%.*s] content[%.*s]\n" , (char*)p , xpath_len , xpath , nodename_len , nodename , properties_len , properties , content_len , content );
		}
		else
		{
			printf( "BRANCH       p[%s] xpath[%.*s] nodename[%.*s] properties[%.*s] content[%.*s]\n" , (char*)p , xpath_len , xpath , nodename_len , nodename , properties_len , properties , content_len , content );
		}
	}
	else if( type & FASTERXML_NODE_LEAF )
	{
		printf( "LEAF         p[%s] xpath[%.*s] nodename[%.*s] properties[%.*s] content[%.*s]\n" , (char*)p , xpath_len , xpath , nodename_len , nodename , properties_len , properties , content_len , content );
	}
	
	if( properties && properties[0] )
	{
		nret = TravelXmlPropertiesBuffer( properties , properties_len , xpath , xpath_len , xpath_size , & CallbackOnXmlProperty , p ) ;
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	char	xpath[ 1024 + 1 ] ;
	char	*xml_buffer = NULL ;
	char	*p = "hello world" ;
	
	AddSkipXmlTag( "meta" );
	AddSkipXmlTag( "span" );
	AddSkipXmlTag( "br" );
	AddSkipXmlTag( "p" );
	AddSkipXmlTag( "image" );
	AddSkipXmlTag( "img" );
	AddSkipXmlTag( "link" );
	AddSkipXmlTag( "input" );
	
	if( argc == 1 + 1 )
	{
		int		nret = 0 ;
		
		nret = ReadEntireFileSafely( argv[1] , "rb" , & xml_buffer , NULL ) ;
		if( nret )
		{
			printf( "ReadEntireFileSafely[%s] failed[%d]\n" , argv[1] , nret );
			return nret;
		}
		
		{
			char	*ptr = NULL ;
			while( ( ptr = strstr( xml_buffer , "<br>" ) ) )
				memset( ptr , ' ' , 4 );
			while( ( ptr = strstr( xml_buffer , "</br>" ) ) )
				memset( ptr , ' ' , 5 );
			/*
			while( ( ptr = strstr( xml_buffer , "<tbody>" ) ) )
				memset( ptr , ' ' , 7 );
			while( ( ptr = strstr( xml_buffer , "</tbody>" ) ) )
				memset( ptr , ' ' , 8 );
			while( ( ptr = strstr( xml_buffer , "{{/ each }}\n</script>\n<script id=\"collection2-template\" type=\"text/x-handlebars-template\">" ) ) )
				memcpy( ptr , "   </table>" , 11 );
			while( ( ptr = strstr( xml_buffer , "{{/ each }}\n</script>\n<script id=\"confirm-loan-template\" type=\"text/x-handlebars-template\">" ) ) )
				memcpy( ptr , "   </table>" , 11 );
			while( ( ptr = strstr( xml_buffer , " />理财管理" ) ) )
				memcpy( ptr , "  " , 2 );
			while( ( ptr = strstr( xml_buffer , " />借款管理" ) ) )
				memcpy( ptr , "  " , 2 );
			while( ( ptr = strstr( xml_buffer , " />我的人人贷" ) ) )
				memcpy( ptr , "  " , 2 );
			while( ( ptr = strstr( xml_buffer , "{{# if button }}" ) ) )
				memset( ptr , ' ' , 16 );
			while( ( ptr = strstr( xml_buffer , "{{/ if }}" ) ) )
				memset( ptr , ' ' , 9 );
			while( ( ptr = strstr( xml_buffer , "i < bduidArr.length" ) ) )
				memset( ptr + 2 , ' ' , 1 );
			while( ( ptr = strstr( xml_buffer , "i <= 1;" ) ) )
				memset( ptr + 2 , ' ' , 1 );
			*/
		}
		
		memset( xpath , 0x00 , sizeof(xpath) );
		nret = TravelXmlBuffer( xml_buffer , xpath , sizeof(xpath) , & CallbackOnXmlNode , p ) ;
		free( xml_buffer );
		if( nret )
		{
			printf( "TravelXmlTree failed[%d]\n" , nret );
			return nret;
		}
	}
	else
	{
		printf( "USAGE : test_fastxml xml_pathfilename\n" );
		exit(7);
	}
	
	CleanSkipXmlTags();
	
	return 0;
}
