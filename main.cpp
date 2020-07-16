#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef DEBUG

#define DebugPrint( ... ) printf( "%s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__ ); printf( __VA_ARGS__ );

#else /* if !defined DEBUG */

#define DebugPrint( ... )

#endif /* DEBUG */


#define Fatal( ... ) \
{ \
    fprintf( stderr, "%s:%s:%d: Fatal error with message:\n", __FILE__, __FUNCTION__, __LINE__ ); \
    fprintf( stderr, __VA_ARGS__ ); \
    exit( -1 ); \
}

#define AllocCheck( ptr, elem_type, elems ) \
{ \
    elem_type* __temp_ptr = (elem_type*) calloc( elems, sizeof( elem_type ) ); \
    if ( NULL == __temp_ptr ) { Fatal( "Bad allocation for %s *%s.\n", #elem_type, #ptr ); } \
    ptr = __temp_ptr; \
}

#define FreeCheck( ptr ) \
{ \
    if ( NULL != ptr ) { free( ptr ); } \
}



#define LINE_LENGTH 1024



typedef std::string TypeName;
typedef std::string ParamName;

typedef enum
{
    PARAM_BOOL,
    PARAM_INT,
    PARAM_DOUBLE,
    PARAM_STRING,
    PARAM_STRINGLIST,
    PARAM_OTHER
} ParamType;

typedef class _Param
{
private:
    TypeName type_name;
    ParamName name;
    ParamName short_name;
    std::string default_value;
    ParamType type;

public:
    _Param() : type_name( "" ), name( "" ), short_name( "" ), default_value( "" ), type( PARAM_OTHER ) {}

    void AddParamToArgsStruct( FILE* hpp_file_ptr );
    void AddParamInitToArgsHandler( FILE* cpp_file_ptr );
    void AddParamHandlerToArgsHandler( FILE* cpp_file_ptr );

    void SetTypeName( TypeName new_type_name );
    void SetName( ParamName new_name ) { name = new_name; }
    void SetShortName( ParamName new_name ) { short_name = new_name; }
    void SetDefaultValue( std::string new_val ) { default_value = new_val; }
} Param;

void Param::SetTypeName( TypeName new_type_name )
{
    const char *new_type_name_str = new_type_name.c_str();

    if ( 0 == strcmp( new_type_name_str, "bool" ) )
    {
        type_name = "bool";
        type = PARAM_BOOL;
    }

    if ( 0 == strcmp( new_type_name_str, "int" ) )
    {
        type_name = "int64_t";
        type = PARAM_INT;
    }

    if ( 0 == strcmp( new_type_name_str, "float" ) )
    {
        type_name = "double";
        type = PARAM_DOUBLE;
    }

    if ( 0 == strcmp( new_type_name_str, "string" ) )
    {
        type_name = "std::string";
        type = PARAM_STRING;
    }

    if ( 0 == strcmp( new_type_name_str, "list" ) )
    {
        type_name = "std::list<std::string>";
        type = PARAM_STRINGLIST;
    }
}

void Param::AddParamInitToArgsHandler( FILE* cpp_file_ptr )
{
    if ( NULL == cpp_file_ptr )
    {
        Fatal( "No .cpp file!\n" );
    }

    if ( type != PARAM_STRINGLIST && type != PARAM_STRING )
    {
        fprintf( cpp_file_ptr, "    args.%s = %s;\n", name.c_str(), default_value.c_str() );
    }
}

void Param::AddParamToArgsStruct( FILE* hpp_file_ptr )
{
    if ( NULL == hpp_file_ptr )
    {
        Fatal( "No .h file!\n" );
    }

    fprintf( hpp_file_ptr, "    %s %s;\n", type_name.c_str(), name.c_str() );
}

void Param::AddParamHandlerToArgsHandler( FILE* cpp_file_ptr )
{
    if ( NULL == cpp_file_ptr )
    {
        Fatal( "No .c file!\n" );
    }

    switch( type )
    {
        case PARAM_BOOL:
            if ( !name.empty() )
            {
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( argv[i], \"-%s\" ) ) { args.%s = !(%s); }\n",
                         name.c_str(), name.c_str(), default_value.c_str() );
            }
            if ( !short_name.empty() != 0 && !name.empty() )
            {
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( argv[i], \"-%s\" ) ) { args.%s = !(%s); }\n",
                         short_name.c_str(), name.c_str(), default_value.c_str() );
            }
            break;

        case PARAM_INT:
            if ( !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"=\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            if ( 1 > sscanf( part_string, \"%%lld\", &(args.%s) ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"Bad int value \\\"%%s\\\" for param \\\"-%s=\\\"!\\n\", part_string );\n", name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            if ( !short_name.empty() != 0 && !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"=\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            if ( 1 > sscanf( part_string, \"%%lld\", &(args.%s) ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"Bad int value \\\"%%s\\\" for param \\\"-%s=\\\"!\\n\", part_string );\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            break;

        case PARAM_DOUBLE:
            if ( !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"=\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            if ( 1 > sscanf( part_string, \"%%lg\", &(args.%s) ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"Bad double value \\\"%%s\\\" for param \\\"-%s=\\\"!\\n\", part_string );\n", name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            if ( !short_name.empty() != 0 && !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"=\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            if ( 1 > sscanf( part_string, \"%%lg\", &(args.%s) ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"Bad double value \\\"%%s\\\" for param \\\"-%s=\\\"!\\n\", part_string );\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            break;

        case PARAM_STRING:
            if ( !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            args.%s = part_string;\n", name.c_str() );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            if ( !short_name.empty() != 0 && !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \"\" );\n" );
                fprintf( cpp_file_ptr, "            if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                args_Fatal( \"No value for param \\\"-%s=\\\"!\\n\" );\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "            args.%s = part_string;\n", name.c_str() );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            break;

        case PARAM_STRINGLIST:
            if ( !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \",\" );\n" );
                fprintf( cpp_file_ptr, "            while ( NULL != part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                std::string list_elem = part_string;\n" );
                fprintf( cpp_file_ptr, "                args.%s.push_back( list_elem );\n", name.c_str() );
                fprintf( cpp_file_ptr, "                part_string = strtok( NULL, \",\" );\n" );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            if ( !short_name.empty() != 0 && !name.empty() )
            {
                fprintf( cpp_file_ptr, "        part_string = strtok( argv[i], \"=\" );\n" );
                fprintf( cpp_file_ptr, "        if ( NULL == part_string )\n" );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            args_Fatal( \"Bad param name!\\n\" );\n" );
                fprintf( cpp_file_ptr, "        }\n" );
                fprintf( cpp_file_ptr, "        if ( 0 == strcmp( part_string, \"-%s\" ) )\n", short_name.c_str() );
                fprintf( cpp_file_ptr, "        {\n" );
                fprintf( cpp_file_ptr, "            part_string = strtok( NULL, \",\" );\n" );
                fprintf( cpp_file_ptr, "            while ( NULL != part_string )\n" );
                fprintf( cpp_file_ptr, "            {\n" );
                fprintf( cpp_file_ptr, "                std::string list_elem = part_string;\n" );
                fprintf( cpp_file_ptr, "                args.%s.push_back( list_elem );\n", name.c_str() );
                fprintf( cpp_file_ptr, "                part_string = strtok( NULL, \",\" );\n" );
                fprintf( cpp_file_ptr, "            }\n" );
                fprintf( cpp_file_ptr, "        }\n" );
            }
            break;

        case PARAM_OTHER:
        default:
            Fatal( "Unknown param type!\n" );
    }
}

int main( int argc, char *argv[] )
{
    if ( argc < 4 )
    {
        printf( "Usage: ArgsGen <file with args> <number of args> <name for args module>\n" );
        return 0;
    }

    /* Define output. */

    char *args_name = argv[3];
    int args_name_size = strlen( args_name );

    char *c_filename;
    AllocCheck( c_filename, char, args_name_size + 5 );
    snprintf( c_filename, args_name_size + 5, "%s.cpp", args_name );
    DebugPrint( "CPP filename: %s\n", c_filename );

    char *h_filename;
    AllocCheck( h_filename, char, args_name_size + 5 );
    snprintf( h_filename, args_name_size + 5, "%s.h", args_name );
    DebugPrint( "H filename: %s\n", h_filename );

    char *h_guard_name;
    AllocCheck( h_guard_name, char, args_name_size + 5 );
    snprintf( h_guard_name, args_name_size + 5, "%s_H", args_name );
    for ( int i = 0; i < args_name_size + 4; i++ )
    {
        h_guard_name[i] = toupper( h_guard_name[i] );
    }
    DebugPrint( "Guard: %s\n", h_guard_name );

    FILE* cpp_file_ptr = fopen( c_filename, "w" );
    if ( NULL == cpp_file_ptr )
    {
        FreeCheck( c_filename );
        FreeCheck( h_filename );
        FreeCheck( h_guard_name );
        Fatal( "Can't open file %s.\n", c_filename );
    }

    FILE* hpp_file_ptr = fopen( h_filename, "w" );
    if ( NULL == hpp_file_ptr )
    {
        FreeCheck( c_filename );
        FreeCheck( h_filename );
        FreeCheck( h_guard_name );
        fclose( cpp_file_ptr );
        Fatal( "Can't open file %s.\n", h_filename );
    }

    /* Define input and find number of args. */
    char *input_filename = argv[1];
    FILE* in_file_ptr = fopen( input_filename, "r" );
    if ( NULL == in_file_ptr )
    {
        FreeCheck( c_filename );
        FreeCheck( h_filename );
        FreeCheck( h_guard_name );
        fclose( cpp_file_ptr );
        fclose( hpp_file_ptr );
        Fatal( "Can't open file %s.\n", input_filename );
    }

    /* Get args data. */
    int num_args = -1;
    if ( 1 != sscanf( argv[2], "%d", &num_args ) )
    {
        FreeCheck( c_filename );
        FreeCheck( h_filename );
        FreeCheck( h_guard_name );
        fclose( cpp_file_ptr );
        fclose( hpp_file_ptr );
        fclose( in_file_ptr );
        Fatal( "Can't get number of params from arg %s.\n", argv[2] );
    }
    Param *params;
    AllocCheck( params, Param, num_args );

    char type_name[LINE_LENGTH]     = "";
    char name[LINE_LENGTH]          = "";
    char short_name[LINE_LENGTH]    = "";
    char default_value[LINE_LENGTH] = "";

    for ( int i = 0; i < num_args; i++ )
    {
        int num_read = fscanf( in_file_ptr, "%s %s %s %s\n", type_name, name, short_name, default_value );
        if ( 4 != num_read )
        {
            FreeCheck( c_filename );
            FreeCheck( h_filename );
            FreeCheck( h_guard_name );
            fclose( cpp_file_ptr );
            fclose( hpp_file_ptr );
            fclose( in_file_ptr );
            Fatal( "Can't open file %s.\n", input_filename );
        }

        params[i].SetTypeName( type_name );
        params[i].SetName( name );
        if ( strcmp( short_name, "-" ) != 0 )
        {
            params[i].SetShortName( short_name );
        }

        params[i].SetDefaultValue( default_value );
    }

    /* Output H. */
    fprintf( hpp_file_ptr, "/* Automatically generated by ArgsGen. */\n" );
    fprintf( hpp_file_ptr, "\n" );

    fprintf( hpp_file_ptr, "#ifndef %s\n", h_guard_name );
    fprintf( hpp_file_ptr, "#define %s\n", h_guard_name );

    fprintf( hpp_file_ptr, "\n" );
    fprintf( hpp_file_ptr, "#include <list>\n" );
    fprintf( hpp_file_ptr, "#include <string>\n" );

    fprintf( hpp_file_ptr, "\n" );
    fprintf( hpp_file_ptr, "#include <stdio.h>\n" );
    fprintf( hpp_file_ptr, "#include <stdlib.h>\n" );

    fprintf( hpp_file_ptr, "\n" );
    fprintf( hpp_file_ptr, "#define args_Fatal( ... ) \\\n" );
    fprintf( hpp_file_ptr, "{ \\\n" );
    fprintf( hpp_file_ptr, "    fprintf( stderr, \"%%s:%%s:%%d: Fatal error with message:\\n\", __FILE__, __FUNCTION__, __LINE__ ); \\\n" );
    fprintf( hpp_file_ptr, "    fprintf( stderr, __VA_ARGS__ ); \\\n" );
    fprintf( hpp_file_ptr, "} \n" );

    fprintf( hpp_file_ptr, "\n" );
    fprintf( hpp_file_ptr, "typedef struct Args_t\n" );
    fprintf( hpp_file_ptr, "{\n" );

    for ( int i = 0; i < num_args; i++ )
    {
        params[i].AddParamToArgsStruct( hpp_file_ptr );
    }

    fprintf( hpp_file_ptr, "} args_Args;\n" );
    fprintf( hpp_file_ptr, "\n" );

    fprintf( hpp_file_ptr, "args_Args args_GetArgs( int argc, char *argv[] );\n" );
    fprintf( hpp_file_ptr, "\n" );

    fprintf( hpp_file_ptr, "#endif /*%s*/\n", h_guard_name );

    /* Output C. */
    fprintf( cpp_file_ptr, "#include \"%s\"\n", h_filename );
    fprintf( cpp_file_ptr, "#include <string.h>\n" );
    fprintf( cpp_file_ptr, "\n" );

    fprintf( cpp_file_ptr, "args_Args args_GetArgs( int argc, char *argv[] )\n" );
    fprintf( cpp_file_ptr, "{\n" );
    fprintf( cpp_file_ptr, "    args_Args args;\n" );
    fprintf( cpp_file_ptr, "    int i = 0;\n" );
    fprintf( cpp_file_ptr, "    char *part_string = NULL;\n" );
    fprintf( cpp_file_ptr, "\n" );

    for ( int i = 0; i < num_args; i++ )
    {
        params[i].AddParamInitToArgsHandler( cpp_file_ptr );
    }
    fprintf( cpp_file_ptr, "\n" );

    fprintf( cpp_file_ptr, "    for ( i = 0; i < argc; i++ )\n" );
    fprintf( cpp_file_ptr, "    {\n" );

    for ( int i = 0; i < num_args; i++ )
    {
        params[i].AddParamHandlerToArgsHandler( cpp_file_ptr );
    }

    fprintf( cpp_file_ptr, "    }\n" );
    fprintf( cpp_file_ptr, "\n" );
    fprintf( cpp_file_ptr, "    return args;\n" );
    fprintf( cpp_file_ptr, "}\n" );


    /* End all. */
    fclose( cpp_file_ptr );
    fclose( hpp_file_ptr );
    fclose( in_file_ptr );

    FreeCheck( c_filename );
    FreeCheck( h_filename );
    FreeCheck( h_guard_name );

    return 0;
}