#include "fasterxml.h"

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef MAX
#define MAX(_a_,_b_) (_a_>_b_?_a_:_b_)
#endif

char __FASTERXML_VERSION[] = "1.0.0" ;

#define FASTERXML_TOKEN_EOF		-1
#define FASTERXML_TOKEN_LAB		1	/* < or <? */
#define FASTERXML_TOKEN_SLASH		3	/* / or ? */
#define FASTERXML_TOKEN_RAB		4	/* > */
#define FASTERXML_TOKEN_RHAB		5	/* ?> */
#define FASTERXML_TOKEN_TEXT		6
#define FASTERXML_TOKEN_PROPNAME	11
#define FASTERXML_TOKEN_PROPVALUE	12

#define TOKENPROPERTY(_base_,_begin_,_len_,_eof_ret_)			\
	do								\
	{								\
		if( (_base_) == NULL )					\
		{							\
			return _eof_ret_;				\
		}							\
		while(1)						\
		{							\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( ! strchr( " \t\r\n" , *(_base_) ) )	\
					break;				\
			}						\
			if( *(_base_) == '\0' )				\
			{						\
				return _eof_ret_;			\
			}						\
			else if( (_base_)[0] == '<' && (_base_)[1] == '!' && (_base_)[2] == '-' && (_base_)[3] == '-' )	\
			{												\
				for( (_base_)+=4 ; *(_base_) ; (_base_)++ )						\
				{											\
					if( (_base_)[0] == '-' && (_base_)[1] == '-' && (_base_)[2] == '>' )		\
						break;									\
				}											\
				if( *(_base_) == '\0' )									\
				{											\
					return _eof_ret_;								\
				}											\
				(_base_)+=3;										\
				continue;										\
			}												\
			break;						\
		}							\
		(_begin_) = (_base_) ;					\
		if(	( (_base_)[0] == '>' )				\
			|| ( (_base_)[0] == '/' && (_base_)[1] == '>' )	\
			|| ( (_base_)[0] == '?' && (_base_)[1] == '>' ) )	\
		{							\
			return _eof_ret_;				\
		}							\
		else if( (_base_)[0] == '\"' )				\
		{							\
			(_base_)++;					\
			(_begin_) = (_base_) ;				\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( *(_base_) == '\"' )			\
					break;				\
			}						\
			(_len_) = (_base_) - (_begin_) ;		\
			(_base_)++;					\
			break;						\
		}							\
		else if( (_base_)[0] == '=' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			break;						\
		}							\
		else							\
		{							\
			(_begin_) = (_base_) ;				\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( strchr( " \t\r\n=?/>" , *(_base_) ) )	\
					break;				\
			}						\
			(_len_) = (_base_) - (_begin_) ;		\
			break;						\
		}							\
	}								\
	while(0);							\

int TravelXmlPropertiesBuffer( char *properties , int properties_len , char *xpath , int xpath_len , int xpath_size , funcCallbackOnXmlProperty *pfuncCallbackOnXmlProperty , void *p )
{
	char		*begin = NULL ;
	int		len ;
	
	char		*propname = NULL ;
	int		propname_len ;
	char		*propvalue = NULL ;
	int		propvalue_len ;
	
	int		xpath_newlen = 0 ;
	
	int		nret = 0 ;
	
	while(1)
	{
		TOKENPROPERTY( properties , begin , len , 0 )
		propname = begin ;
		propname_len = len ;
		
		TOKENPROPERTY( properties , begin , len , FASTERXML_ERROR_XML_INVALID )
		if( ! ( begin[0] == '=' && len == 1 ) )
			return FASTERXML_ERROR_XML_INVALID;
		
		TOKENPROPERTY( properties , begin , len , FASTERXML_ERROR_XML_INVALID )
		propvalue = begin ;
		propvalue_len = len ;
		
		if( pfuncCallbackOnXmlProperty )
		{
			if( xpath )
			{
				if( xpath_len + 1 + propname_len < xpath_size-1 - 1 )
				{
					sprintf( xpath + xpath_len , ".%.*s" , (int)propname_len , propname );
					xpath_newlen = xpath_len + 1 + propname_len ;
				}
				else if( xpath_len + 1 + 1 <= xpath_size-1 )
				{
					sprintf( xpath + xpath_len , ".*" );
					xpath_newlen = xpath_len + 1 + 1 ;
				}
				else
				{
					xpath_newlen = xpath_len ;
				}
			}
			
			nret = (*pfuncCallbackOnXmlProperty)( xpath , xpath_newlen , xpath_size , propname , propname_len , propvalue , propvalue_len , p ) ;
			if( nret > 0 )
				break;
			else if( nret < 0 )
				return nret;
		}
	}
	
	return 0;
}

#define TOKENCONTENT(_base_,_begin_,_len_,_tag_)			\
	do								\
	{								\
		(_begin_) = (_base_) ;					\
		for( ; *(_base_) ; (_base_)++ )				\
		{							\
			if( (unsigned char)*(_base_) > 127 )		\
				continue;				\
			if( *(_base_) == '<' )				\
				break;					\
		}							\
		if( *(_base_) == '\0' )					\
		{							\
			return FASTERXML_ERROR_XML_INVALID;		\
		}							\
		(_len_) = (_base_) - (_begin_) ;			\
		(_tag_) = FASTERXML_TOKEN_TEXT ;				\
	}								\
	while(0);							\

#define TOKENXML(_base_,_begin_,_len_,_tag_,_eof_ret_)			\
	do								\
	{								\
		if( (_base_) == NULL )					\
		{							\
			return _eof_ret_;				\
		}							\
		(_tag_) = 0 ;						\
		while(1)						\
		{							\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( ! strchr( " \t\r\n" , *(_base_) ) )	\
					break;				\
			}						\
			if( *(_base_) == '\0' )				\
			{						\
				return _eof_ret_;			\
			}						\
			if( (_base_)[0] == '<' && (_base_)[1] == '!' && (_base_)[2] == '-' && (_base_)[3] == '-' )	\
			{						\
				for( (_base_)+=4 ; *(_base_) ; (_base_)++ )	\
				{					\
					if( (_base_)[0] == '-' && (_base_)[1] == '-' && (_base_)[2] == '>' )	\
						break;			\
				}					\
				if( *(_base_) == '\0' )			\
				{					\
					return _eof_ret_;		\
				}					\
				(_base_)+=3;				\
				continue;				\
			}						\
			break;						\
		}							\
		if( (_tag_) )						\
			break;						\
		if( (_base_)[0] == '<' )				\
		{							\
			if( (_base_)[1] == '?' )			\
			{						\
				(_begin_) = (_base_) ;			\
				(_len_) = 2 ;				\
				(_base_)+=2;				\
				(_tag_) = FASTERXML_TOKEN_LAB ;		\
			}						\
			else						\
			{						\
				(_begin_) = (_base_) ;			\
				(_len_) = 1 ;				\
				(_base_)++;				\
				(_tag_) = FASTERXML_TOKEN_LAB ;		\
			}						\
			break;						\
		}							\
		if( (_base_)[0] == '/' || (_base_)[0] == '?' )		\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERXML_TOKEN_SLASH ;			\
			break;						\
		}							\
		if( (_base_)[0] == '>' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERXML_TOKEN_RAB ;			\
			break;						\
		}							\
		(_begin_) = (_base_) ;					\
		for( ++(_base_) ; *(_base_) ; (_base_)++ )		\
		{							\
			if( (unsigned char)*(_base_) > 127 )		\
				continue;				\
			if( strchr( " \t\r\n</?>" , *(_base_) ) )	\
			{						\
				(_len_) = (_base_) - (_begin_) ;	\
				(_tag_) = FASTERXML_TOKEN_TEXT ;		\
				break;					\
			}						\
		}							\
		if( *(_base_) == '\0' )					\
		{							\
			return _eof_ret_;				\
		}							\
	}								\
	while(0);							\

static int _TravelXmlBuffer( register char **xml_ptr , char *xpath , int xpath_len , int xpath_size
	, funcCallbackOnXmlNode *pfuncCallbackOnXmlNode
	, funcCallbackOnXmlNode *pfuncCallbackOnEnterXmlNode
	, funcCallbackOnXmlNode *pfuncCallbackOnLeaveXmlNode
	, funcCallbackOnXmlNode *pfuncCallbackOnXmlLeaf
	, void *p , char *preread_nodename , int preread_nodename_len )
{
	char		*begin ;
	int		len ;
	signed char	tag ;
	
	int		close_flag ;
	
	char		*nodename = NULL ;
	int		nodename_len ;
	char		*properties ;
	int		properties_len ;
	char		*content = NULL ;
	int		content_len ;
	
	int		xpath_newlen = 0 ;
	
	int		nret = 0 ;
	
	while(1)
	{
		if( preread_nodename )
		{
			nodename = preread_nodename ;
			nodename_len = preread_nodename_len ;
			preread_nodename = NULL ;
			goto _SKIP2;
		}
		
		TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_INFO_END_OF_BUFFER)
		if( tag != FASTERXML_TOKEN_LAB )
			return FASTERXML_ERROR_XML_INVALID;
		
		TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
		if( tag == FASTERXML_TOKEN_SLASH )
			break;
		
		nodename = begin ;
		nodename_len = len ;
		
_SKIP2 :
		properties = (*xml_ptr) ;
		
		if( xpath )
		{
			if( xpath_len + 1 + nodename_len < xpath_size-1 - 1 )
			{
				sprintf( xpath + xpath_len , "/%.*s" , (int)nodename_len , nodename );
				xpath_newlen = xpath_len + 1 + nodename_len ;
			}
			else if( xpath_len + 1 + 1 <= xpath_size-1 )
			{
				sprintf( xpath + xpath_len , "/*" );
				xpath_newlen = xpath_len + 1 + 1 ;
			}
			else
			{
				xpath_newlen = xpath_len ;
			}
		}
		
		close_flag = 0 ;
		while(1)
		{
			TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
			if( tag == FASTERXML_TOKEN_SLASH || tag == FASTERXML_TOKEN_RHAB )
				close_flag = 1 ;
			else if( tag == FASTERXML_TOKEN_RAB )
				break;
		}
		
		properties_len = begin - properties ;
		
		if( close_flag )
		{
			if( pfuncCallbackOnXmlNode )
			{
				nret = (*pfuncCallbackOnXmlNode)( FASTERXML_NODE_BRANCH , xpath , xpath_newlen , xpath_size , nodename , nodename_len , properties , properties_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			continue;
		}
		
		TOKENCONTENT(*xml_ptr,begin,len,tag)
		content = begin ;
		content_len = len ;
		
		TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
		if( tag != FASTERXML_TOKEN_LAB )
			return FASTERXML_ERROR_XML_INVALID;
		
		TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
		if( tag == FASTERXML_TOKEN_SLASH )
		{
			TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
			if( STRNCMP( begin , != , nodename , MAX(len,nodename_len) ) )
				return FASTERXML_ERROR_XML_INVALID;
			
			TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
			if( tag != FASTERXML_TOKEN_RAB )
				return FASTERXML_ERROR_XML_INVALID;
			
			if( pfuncCallbackOnXmlLeaf )
			{
				nret = (*pfuncCallbackOnXmlLeaf)( FASTERXML_NODE_LEAF , xpath , xpath_newlen , xpath_size , nodename , nodename_len , properties , properties_len , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else
		{
			if( pfuncCallbackOnEnterXmlNode )
			{
				nret = (*pfuncCallbackOnEnterXmlNode)( FASTERXML_NODE_ENTER | FASTERXML_NODE_BRANCH , xpath , xpath_newlen , xpath_size , nodename , nodename_len , properties , properties_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			nret = _TravelXmlBuffer( xml_ptr , xpath , xpath_newlen , xpath_size
						, pfuncCallbackOnXmlNode
						, pfuncCallbackOnEnterXmlNode
						, pfuncCallbackOnLeaveXmlNode
						, pfuncCallbackOnXmlLeaf
						, p , begin , len ) ;
			if( nret )
				return nret;
			
			TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
			if( STRNCMP( begin , != , nodename , MAX(len,nodename_len) ) )
				return FASTERXML_ERROR_XML_INVALID;
			
			if( xpath )
			{
				if( xpath_len + 1 + nodename_len < xpath_size-1 - 1 )
				{
					sprintf( xpath + xpath_len , "/%.*s" , (int)nodename_len , nodename );
					xpath_newlen = xpath_len + 1 + nodename_len ;
				}
				else if( xpath_len + 1 + 1 <= xpath_size-1 )
				{
					sprintf( xpath + xpath_len , "/*" );
					xpath_newlen = xpath_len + 1 + 1 ;
				}
				else
				{
					xpath_newlen = xpath_len ;
				}
			}
			
			if( pfuncCallbackOnLeaveXmlNode )
			{
				nret = (*pfuncCallbackOnLeaveXmlNode)( FASTERXML_NODE_LEAVE | FASTERXML_NODE_BRANCH , xpath , xpath_newlen , xpath_size , nodename , nodename_len , NULL , 0 , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			TOKENXML(*xml_ptr,begin,len,tag,FASTERXML_ERROR_XML_INVALID)
			if( tag != FASTERXML_TOKEN_RAB )
				return FASTERXML_ERROR_XML_INVALID;
		}
	}
	
	return 0;
}

int TravelXmlBuffer( char *xml_buffer , char *xpath , int xpath_size
		    , funcCallbackOnXmlNode *pfuncCallbackOnXmlNode
		    , void *p )
{
	char		*xml_ptr = xml_buffer ;
	
	return _TravelXmlBuffer( & xml_ptr , xpath , 0 , xpath_size
				, pfuncCallbackOnXmlNode
				, pfuncCallbackOnXmlNode
				, pfuncCallbackOnXmlNode
				, pfuncCallbackOnXmlNode
				, p , NULL , 0 );
}

int TravelXmlBuffer4( char *xml_buffer , char *xpath , int xpath_size
		    , funcCallbackOnXmlNode *pfuncCallbackOnXmlNode
		    , funcCallbackOnXmlNode *pfuncCallbackOnEnterXmlNode
		    , funcCallbackOnXmlNode *pfuncCallbackOnLeaveXmlNode
		    , funcCallbackOnXmlNode *pfuncCallbackOnXmlLeaf
		    , void *p )
{
	char		*xml_ptr = xml_buffer ;
	
	return _TravelXmlBuffer( & xml_ptr , xpath , 0 , xpath_size
				, pfuncCallbackOnXmlNode
				, pfuncCallbackOnEnterXmlNode
				, pfuncCallbackOnLeaveXmlNode
				, pfuncCallbackOnXmlLeaf
				, p , NULL , 0 );
}

