/*
Luiji's Tiny BASIC Compiler
Copyright (C) 2012 Entertaining Software, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#ifdef __GNUC__
# define PRINTF(f, va) __attribute__ ((__format__ (__printf__, f, va)))
#else
# define PRINTF(f, va) /* unsupported */
#endif

#if defined (__STDC__) || defined (_AIX) \
    || (defined (__mips) && defined (_SYSTYPE_SVR4)) \
    || defined (WIN32) || defined (__cplusplus)
# define PARAMS(protos) protos
#else
# define PARAMS(protos) (/* unsupported */)
#endif

#define MAX_WHILE_NESTING 256
enum
{
  T_PRINT,
  T_IF,
  T_GOTO,
  T_INPUT,
  T_LET,
  T_GOSUB,
  T_RETURN,
  T_REM,
  T_RUN,
  T_ASM,
  T_LABEL,
  T_GOTOL,
  T_GOSUBL,
  T_PTR,
  T_WHILE,
  T_ENDW,
  T_END
};

enum
{
  O_EQUAL,
  O_NOT_EQUAL,
  O_LESS,
  O_MORE,
  O_LESS_OR_EQUAL,
  O_MORE_OR_EQUAL
};

static char        look          = 0;
static int         line          = 1;
static int			whilelines 	  = 0;
static int 		currentwhile  = MAX_WHILE_NESTING;
static int         use_print_i   = 0;
static int         use_print_s   = 0;
static int         use_print_t   = 0;
static int         use_print_n   = 0;
static int         use_input_i   = 0;
static int         vars_used[26] = { 0 };
static int			while_stack[MAX_WHILE_NESTING+1] = { 0 };
static char **     strings       = NULL;
static int         string_count  = 0;
static const char *program_name  = "<?>";
static const char *input_name    = NULL;
static const char *output_name   = NULL;

static void  shutdown      PARAMS ((void));
static void *xmalloc       PARAMS ((size_t size));
static void *xrealloc      PARAMS ((void *original, size_t size));
static void  xfree         PARAMS ((void *pointer));
static void  parse_opts    PARAMS ((int argc, char **argv));
static void  print_help    PARAMS ((void));
static void  print_version PARAMS ((void));
static void  begin         PARAMS ((void));
static void  finish        PARAMS ((void));
static void  error         PARAMS ((const char *format, ...)) PRINTF (1, 2);
static void  eat_blanks    PARAMS ((void));
static void  match         PARAMS ((int what));
static void  get_char      PARAMS ((void));
static char  get_name      PARAMS ((void));
static int   get_num       PARAMS ((void));
static int   get_keyword   PARAMS ((void));
static int   do_line       PARAMS ((void));
static void  do_statement  PARAMS ((void));
static void  do_print      PARAMS ((void));
static void  do_print_item PARAMS ((void));
static void  do_if         PARAMS ((void));
static void  do_goto       PARAMS ((void));
static void  do_input      PARAMS ((void));
static void  do_let        PARAMS ((void));
static void  do_gosub      PARAMS ((void));
static void  do_return     PARAMS ((void));
static void  do_rem        PARAMS ((void));
static void  do_end        PARAMS ((void));
static void  do_expression PARAMS ((void));
static void  do_term       PARAMS ((void));
static void  do_factor     PARAMS ((void));
static void  do_string     PARAMS ((void));
static void  do_asm		PARAMS ((void));
static void  do_label		PARAMS ((void));
static void  do_gotol		PARAMS ((void));
static void  do_gosubl 	PARAMS ((void));
static void  do_ptr		PARAMS ((void));
static void  do_while		PARAMS ((void));
static void  do_endw		PARAMS ((void));
char* 		  return_str	PARAMS ((void));
char*        return_next_tok(void);
int
main (argc, argv)
     int argc;
     char **argv;
{
  atexit (shutdown);

  parse_opts (argc, argv);

  begin ();
  while (do_line ());
  finish ();

  return EXIT_SUCCESS;
}

static void
shutdown ()
{
  int i;

  for (i = 0; i < string_count; ++i)
    xfree (strings[i]);

  xfree (strings);
}

static void *
xmalloc (size)
     size_t size;
{
  void *data = malloc (size);
  if (!data)
    {
      fprintf (stderr, "%s: failed to allocate memory\n", program_name);
      exit (EXIT_FAILURE);
    }
  return data;
}

static void *
xrealloc (original, size)
     void *original;
     size_t size;
{
  void *data = realloc (original, size);
  if (!data)
    {
      fprintf (stderr, "%s: failed to allocate memory\n", program_name);
      exit (EXIT_FAILURE);
    }
  return data;
}

static void
xfree (pointer)
     void *pointer;
{
  if (pointer)
    free (pointer);
}

static void
parse_opts (argc, argv)
     int argc;
     char **argv;
{
  extern char *optarg;
  extern int optind;
  int getopt PARAMS ((int argc, char *const *argv, const char *optstring));

  int was_error = 0, opt;

  program_name = strrchr (argv[0], '/');
  if (program_name)
    ++program_name;
  else
    program_name = argv[0];

  while ((opt = getopt (argc, argv, "hVo:")) != -1)
    switch (opt)
      {
      case 'h':
	print_help ();
	exit (EXIT_SUCCESS);
      case 'V':
	print_version ();
	exit (EXIT_SUCCESS);
      case 'o':
	output_name = optarg;
	break;
      case '?':
	was_error = 1;
	break;
      default:
	assert (0);
	break;
      }

  if (was_error)
    exit (EXIT_FAILURE);

  if (optind < argc)
    input_name = argv[optind];

  for (++optind; optind < argc; ++optind)
    fprintf (stderr, "%s: ignoring argument '%s'\n", program_name,
	     argv[optind]);

  if (input_name)
    {
      if (!freopen (input_name, "r", stdin))
	{
	  fprintf (stderr, "%s: failed to open for reading\n", input_name);
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      input_name = "<stdin>";
    }

  if (output_name)
    {
      if (!freopen (output_name, "w", stdout))
	{
	  fprintf (stderr, "%s: failed to open for writing\n", output_name);
	  exit (EXIT_FAILURE);
	}
    }
  else
    {
      output_name = "<stdout>";
    }
}

static void
print_help ()
{
  printf ("Usage: %s [-o OUTPUT] [INPUT]\n", program_name);
  puts ("");
  puts ("Compiles Tiny BASIC programs into Netwide Assembler programs.");
  puts ("");
  puts ("Options:");
  puts ("  -o  set output file name");
  puts ("  -h  print this help message and exit");
  puts ("  -V  print this version message adn exit");
  puts ("");
  puts ("Input and output defaults to stdin and stdout.");
  puts ("");
  puts ("Report bugs to: luiji@users.sourceforge.net");
  puts ("tbc home page: <https://github.com/Luiji/TinyBASIC>");
}

static void
print_version ()
{
  puts ("Luiji's Tiny BASIC Compiler 0.0.0");
  puts ("Copyright (C) 2012 Entertaining Software, Inc.");
  puts ("License GPLv3+: GNU GPL version 3 or later \
<http://gnu.org/licenses/gpl.html>");
  puts ("This is free software: you are free to change and redistribute it.");
  puts ("There is NO WARRANTY, to the extent permitted by law.");
  puts ("Modified by Benderx2 to run on R3X Systems.\n");
}

static void
begin ()
{
  puts ("; Generated by Luiji's Tiny BASIC Compiler");

  puts ("");
  puts ("include 'libR3X/libR3X.pkg'");
  puts (".text {");
  puts ("function main\n");
  puts ("");

  get_char ();
}

static void
finish ()
{
  int i;

  puts ("endfunction main\n");
  puts ("\t; exit to operating system");
  puts ("\t; program falls through to here when there is no explicit end");
  puts ("_exit:");
  puts ("\tConsole.WaitKey");
  puts ("\tSystem.Quit 0");
  puts ("");

  if (use_print_i)
    {
      puts ("\t; print an integer to the terminal");
      puts ("\tprint_i:");
      puts ("\tpushr R1");
      puts ("\tsyscall SYSCALL_PUTI");
      puts ("\tpopr R1");
      puts ("\tret");
    }
  else
    {
      puts ("; print_i excluded");
    }
  puts ("");

  if (use_print_s)
    {
      puts ("\t; print string to terminal");
      puts ("\tprint_s:");
      puts ("\tpushr R1");
      puts ("\tsyscall SYSCALL_PUTS");
      puts ("\tpopr R1");
      puts ("\tret");
    }
  else
    {
      puts ("; print_s excluded");
    }
  puts ("");

  if (use_print_t)
    {
      puts ("\t; print a tab to the terminal");
      puts ("\tprint_t:");
      puts ("\tpush 0x09");
      puts ("\tsyscall SYSCALL_PUTCH");
      puts ("\tpop");
      puts ("\tret");
    }
  else
    {
      puts ("; print_t excluded");
    }
  puts ("");

  if (use_print_n)
    {
      puts ("\t; print a newline to the terminal");
      puts ("\tprint_n:");
	  puts ("\tpush 0x0A");
	  puts ("\tsyscall SYSCALL_PUTCH");
	  puts ("\tpop");
	  puts ("\tret");
    }
  else
    {
      puts ("; print_n excluded");
    }
  puts ("");

  if (use_input_i)
    {
      puts ("\t; read a number from the terminal");
      puts ("\tinput_i:");
      puts ("\tsyscall SYSCALL_GETC");
      puts ("\tpush 0");
      puts ("\tcmp");
      puts ("\tpop");
      puts ("\tpopr R1");
      puts ("\tje input_i");
      puts ("\tret");
    }
  else
    {
      puts ("; input_i excluded");
    }
  puts ("\tOpen a file");
  puts ("\topen_f:");
  puts ("\tpushr R1");
  puts ("\tsyscall SYSCALL_OPENSTREAM");
  puts ("\tpopr R1");
  puts ("\tpop");
  puts ("\tret");
  
  puts ("");
  puts ("}");
  puts (".bss {");
  puts ("");

  for (i = 0; i < 26; ++i)
    if (vars_used[i])
      printf ("\tv%c: rd 1\n", 'a' + i);

  puts ("}");

  puts (".data {");
  puts ("");

  for (i = 0; i < string_count; ++i)
    printf ("\ts%i: db \"%s\", 0\n", i, strings[i]);

  puts ("}");
  puts("end");
  puts ("; Task Completed -- Assemble with FASM ");
}

static void
error (format)
     const char *format;
     /* ... */
{
  va_list args;

  fprintf (stderr, "%s:%i: error: ", input_name, line);

  va_start (args, format);
  vfprintf (stderr, format, args);
  va_end (args);

  fputs ("\n", stderr);

  exit (EXIT_FAILURE);
}

static void
eat_blanks ()
{
  while (look == ' ' || look == '\t')
    get_char ();
}

static void
match (what)
     int what;
{
  if (look == what)
    get_char ();
  else
    error ("expected '%c' got '%c'", what, look);

  eat_blanks ();
}

static void
get_char ()
{
  look = fgetc (stdin);
}

static char
get_name ()
{
  char name = tolower (look);

  if (isalpha (name))
    get_char ();
  else
    error ("expected identifier got '%c'", name);

  vars_used[name - 'a'] = 1;

  eat_blanks ();

  return name;
}

static int
get_num ()
{
  int result = 0;

  if (isdigit (look))
    for (; isdigit (look); get_char ())
      {
	result *= 10;
	result += look - '0';
      }
  else
    error ("expected integer got '%c'", look);

  eat_blanks ();

  return result;
}

static int
get_keyword ()
{
  int i;
  char token[7];
  while(look == ' ' || look == '\t' || look == '\n'){
	get_char();
  }
  if (!isalpha (look))
    error ("expected keyword got '%c'", look);

  for (i = 0; i < (int) sizeof (token) - 1 && isalpha (look); ++i)
    {
      token[i] = tolower (look);
      get_char ();
    }

  token[i] = 0;

  if (!strcmp (token, "print"))
    i = T_PRINT;
  else if (!strcmp (token, "if"))
    i = T_IF;
  else if (!strcmp (token, "goto"))
    i = T_GOTO;
  else if (!strcmp (token, "input"))
    i = T_INPUT;
  else if (!strcmp (token, "let"))
    i = T_LET;
  else if (!strcmp (token, "gosub"))
    i = T_GOSUB;
  else if (!strcmp (token, "return"))
    i = T_RETURN;
  else if (!strcmp (token, "rem"))
    i = T_REM;
  else if (!strcmp (token, "run"))
    i = T_RUN;
  else if (!strcmp (token, "end"))
    i = T_END;
  else if (!strcmp(token, "asm"))
	i = T_ASM;
  else if (!strcmp(token, "label"))
	i = T_LABEL;
  else if (!strcmp(token, "gotol"))
	i = T_GOTOL;
  else if(!strcmp(token, "gosubl"))
    i = T_GOSUBL;
  else if(!strcmp(token, "ptr"))
	i = T_PTR;
  else if(!strcmp(token, "while"))
	i = T_WHILE;
  else if(!strcmp(token, "endw"))
	i = T_ENDW;
  else
    error ("expected keyword got '%s'", token);

  eat_blanks ();

  return i;
}

static int
do_line ()
{
  while (look == '\n')
    {
      ++line;
      get_char ();
    }

  if (feof (stdin))
    return 0;

  if (tolower (look) == 'r')
    {
      get_char ();
      if (tolower (look) == 'u')
	{
	  get_char ();
	  if (tolower (look) == 'n')
	    return 0;
	  else
	    error ("expected statement or 'run'");
	}

      ungetc (look, stdin);
      look = 'r';
    }

  printf ("\n\t; debug line %i\n", line);

  do_statement ();

  if (look != '\n')
    error ("expected newline got '%c'", look);
  else
    get_char ();

  ++line;

  return 1;
}

static void
do_statement ()
{
  if (isdigit (look))
    printf ("l%i:\n", get_num ());

  switch (get_keyword ())
    {
    case T_PRINT:  do_print ();  break;
    case T_IF:     do_if ();     break;
    case T_GOTO:   do_goto ();   break;
    case T_INPUT:  do_input ();  break;
    case T_LET:    do_let ();    break;
    case T_GOSUB:  do_gosub ();  break;
    case T_RETURN: do_return (); break;
    case T_REM:    do_rem ();    break;
    case T_END:    do_end ();    break;
    case T_ASM:    do_asm(); break;
    case T_LABEL:  do_label(); break;
    case T_GOTOL:  do_gotol(); break;
    case T_GOSUBL: do_gosubl(); break;
    case T_PTR:    do_ptr(); break;
    case T_WHILE:  do_while(); break;
    case T_ENDW:   do_endw(); break;
    default:       assert (0);   break;
    }
}
static void 
do_label()
{
  char* string = return_next_tok();
  printf("l%s:\n", string);
}
char* 
return_str() {
  int i;
  size_t string_size = 0;
  char *string;

  string = xmalloc (1);
  string[0] = 0;

  for (get_char (); look != '"'; get_char ())
    {
      ++string_size;
      string = xrealloc (string, string_size + 1);
      string[string_size - 1] = look;
      string[string_size] = 0;
    }

  get_char ();
  return string;
}
char* 
return_next_tok() {
  int i;
  size_t string_size = 0;
  char *string;

  string = xmalloc (1);
  string[0] = 0;

  for (; look != ' ' && look != '\t' && look != '\n'; get_char ())
    {
      ++string_size;
      string = xrealloc (string, string_size + 1);
      string[string_size - 1] = look;
      string[string_size] = 0;
    }

  //!get_char ();
  return string;
}
static void
do_gosubl() {
	printf("\tcall l%s\n", return_next_tok());
}
static void
do_print ()
{
  use_print_n = 1;

  do_print_item ();

  for (;;)
    {
      if (look == ',')
	{
	  match (',');
	  use_print_t = 1;
	  puts ("\tcall print_t");
	}
      else if (look == ';')
	{
	  match (';');
	}
      else
	{
	  break;
	}

      do_print_item ();
    }

  puts ("\tcall print_n");
}

static void
do_print_item ()
{
  if (look == '"')
    {
      use_print_s = 1;
      do_string ();
      puts ("\tcall print_s");
    }
  else
    {
      use_print_i = 1;
      do_expression ();
      puts ("\tcall print_i");
    }
}
static void 
do_asm() {
  int i;
  size_t string_size = 0;
  char *string;

  string = xmalloc (1);
  string[0] = 0;

  for (get_char (); look != '"'; get_char ())
    {
      ++string_size;
      string = xrealloc (string, string_size + 1);
      string[string_size - 1] = look;
      string[string_size] = 0;
    }

  get_char ();
  printf("\t%s\n", string);
}
static void
do_if ()
{
  int op;

  do_expression ();
  puts ("\tloadrr R4, R1");

  if (look == '=')
    {
      match ('=');
      if (look == '<')
	{
	  match ('<');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_MORE_OR_EQUAL;
	}
      else
	{
	  op = O_EQUAL;
	}
    }
  else if (look == '<')
    {
      match ('<');
      if (look == '=')
	{
	  match ('=');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_LESS;
	}
    }
  else if (look == '>')
    {
      match ('>');
      if (look == '=')
	{
	  match ('=');
	  op = O_MORE_OR_EQUAL;
	}
      else if (look == '<')
	{
	  match ('<');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_MORE;
	}
    }
  else
    {
      error ("expected =, <>, ><, <=, =<, >= or => got '%c'", look);
    }

  do_expression ();

  fputs ("\tcmpr R4, R1\n\tj", stdout);

  /* note inversal of operators */
  if (op == O_EQUAL)
    fputs ("ne", stdout);
  else if (op == O_NOT_EQUAL)
    fputs ("e", stdout);
  else if (op == O_LESS)
    fputs ("ge", stdout);
  else if (op == O_MORE)
    fputs ("le", stdout);
  else if (op == O_LESS_OR_EQUAL)
    fputs ("g", stdout);
  else if (op == O_MORE_OR_EQUAL)
    fputs ("l", stdout);

  printf (" i%i\n", line);

  do_statement ();

  printf ("i%i:\n", line);
}
/*!
 * A = 0
 * WHILE A < 5 ---> while0 :
 * 					cmpr A, 5
 * 	A = A + 1		jge endw0
 * ENDW				mov A, A+1
 * 					jmp while0
 * 					endw0:
 * */				
static void 
do_while()
{
	int op;
  printf("while%u:\n", whilelines);
  while_stack[currentwhile] = whilelines;
  do_expression ();
  puts ("\tloadrr R4, R1");

  if (look == '=')
    {
      match ('=');
      if (look == '<')
	{
	  match ('<');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_MORE_OR_EQUAL;
	}
      else
	{
	  op = O_EQUAL;
	}
    }
  else if (look == '<')
    {
      match ('<');
      if (look == '=')
	{
	  match ('=');
	  op = O_LESS_OR_EQUAL;
	}
      else if (look == '>')
	{
	  match ('>');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_LESS;
	}
    }
  else if (look == '>')
    {
      match ('>');
      if (look == '=')
	{
	  match ('=');
	  op = O_MORE_OR_EQUAL;
	}
      else if (look == '<')
	{
	  match ('<');
	  op = O_NOT_EQUAL;
	}
      else
	{
	  op = O_MORE;
	}
    }
  else
    {
      error ("expected =, <>, ><, <=, =<, >= or => got '%c'", look);
    }

  do_expression ();

  fputs ("\tcmpr R4, R1\n\tj", stdout);

  /* note inversal of operators */
  if (op == O_EQUAL)
    fputs ("ne", stdout);
  else if (op == O_NOT_EQUAL)
    fputs ("e", stdout);
  else if (op == O_LESS)
    fputs ("ge", stdout);
  else if (op == O_MORE)
    fputs ("le", stdout);
  else if (op == O_LESS_OR_EQUAL)
    fputs ("g", stdout);
  else if (op == O_MORE_OR_EQUAL)
    fputs ("l", stdout);
  printf(" endwhile%u\n", whilelines);
  
  whilelines++;
  currentwhile--;
}
static void 
do_endw()
{
	int currentwhile_now = while_stack[currentwhile+1];
	printf("\tjmp while%u\n", currentwhile_now);
	printf("endwhile%u:\n", currentwhile_now);
	currentwhile++;
}
static void
do_goto ()
{
  printf ("\tjmp l%i\n", get_num ());
}
static void
do_gotol () 
{	
  printf("\tjmp l%s\n", return_next_tok());
}
static void
do_input ()
{
  use_input_i = 1;
  do
    printf ("\tcall input_i\n\tpushr R0\n\tpushr R1\n\tloadr R0, v%c\n\tstosd\n\tpopr R1\n\tpopr R0\n", get_name ());
  while (look == ',');
}

static void
do_let ()
{
  char name = get_name ();
  match ('=');
  do_expression ();
  printf ("\tpushr R0\n\tpushr R1\n\tloadr R0, v%c\n\tstosd\n\tpopr R1\n\tpopr R0\n", name);
}
static void 
do_ptr()
{
	char name = get_name ();
  match ('=');
  do_expression ();
  printf ("\tpushr R0\n\tpushr R1\n\tloadr R0, R1\n\tlodsdloadr R0, v%c\n\tstosd\n\tpopr R1\n\tpopr R0\n", name);
}
static void
do_gosub ()
{
  printf ("\tcall l%i\n", get_num ());
}

static void
do_return ()
{
  puts ("\tret");
}

static void
do_rem ()
{
  while (look != '\n')
    get_char ();
}

static void
do_end ()
{
  puts ("\tjmp _exit");
}

static void
do_expression ()
{
  if (look == '+' || look == '-')
    puts ("\tloadr R1, 0");
  else
    do_term ();

  while (look == '+' || look == '-')
    {
      int op = look;
      puts ("\tpushr R1");
      match (look);
      do_term ();
      puts ("\tpopr R2");
      if (op == '+')
	puts ("\tpushr R1\n\tpushr R2\n\tadd\n\tpopr R1\n\tpop\n\tpop\n");
      else if (op == '-')
	puts ("\tpushr R1\n\tpushr R2\n\tsub\n\t\n\tneg\n\tpopr R1\n\tpop\n\tpop\n");
    }
}

static void
do_term ()
{
  do_factor ();

  while (look == '&' || look == '^' || look == '|' || look == '*' || look == '/')
    {
      int op = look;
      puts ("\tpushr R1");
      match (look);
      do_factor ();
      puts ("\tpopr R2");
      puts ("\tloadr R4, 0");
      if (op == '*')
	puts ("\tpushr R1\n\tpushr R2\n\tmul\n\tpopr R1\n\tpop\n\t");
      else if (op == '/')
	puts ("\tpushr R2\n\tpushr R1\n\tdiv\n\tpopr R1\n\tpop\n\tpop");
	  else if (op == '&')
	puts ("\tpushr R2\n\tpushr R1\n\tand\n\tpopr R1\n\tpop\n\tpop");
	  else if (op == '|')
	puts ("\tpushr R2\n\tpushr R1\n\tor\n\tpopr R1\n\tpop\n\tpop");
	  else if (op == '^')
	puts ("\tpushr R2\n\tpushr R1\n\txor\n\tpopr R1\n\tpop\n\tpop");
    }
}

static void
do_factor ()
{
  if (look == '(')
    {
      match ('(');
      do_expression ();
      match (')');
    }
  else if (isalpha (look))
    {
      printf ("\tloadrm dword, R1, v%c\n", get_name ());
    }
  else
    {
      printf ("\tloadr R1, %i\n", get_num ());
    }
}

static void
do_string ()
{
  int i;
  size_t string_size = 0;
  char *string;

  string = xmalloc (1);
  string[0] = 0;

  for (get_char (); look != '"'; get_char ())
    {
      ++string_size;
      string = xrealloc (string, string_size + 1);
      string[string_size - 1] = look;
      string[string_size] = 0;
    }

  get_char ();

  for (i = 0; i < string_count; ++i)
    if (!strcmp (strings[i], string))
      {
	printf ("\tloadr R1, s%i\n", i);
	return;
      }

  ++string_count;
  strings = xrealloc (strings, string_count * sizeof (char *));
  strings[string_count - 1] = string;

  printf ("\tloadr R1, s%i\n", i);
}

/* vim:set ts=8 sts=2 sw=2 noet: */
