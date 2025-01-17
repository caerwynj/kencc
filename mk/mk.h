#include	<lib9.h>
#include	<bio.h>
#include	<regexp.h>

#undef assert
#define	assert	mkassert
extern Biobuf bout;

typedef struct Bufblock
{
	struct Bufblock *next;
	char 		*start;
	char 		*end;
	char 		*current;
} Bufblock;

typedef struct Word
{
	char 		*s;
	struct Word 	*next;
} Word;

typedef struct Envy
{
	char 		*name;
	Word 		*values;
} Envy;

extern Envy *envy;

typedef struct Rule
{
	char 		*target;	/* one target */
	Word 		*tail;		/* constituents of targets */
	char 		*recipe;	/* do it ! */
	short 		attr;		/* attributes */
	short 		line;		/* source line */
	char 		*file;		/* source file */
	Word 		*alltargets;	/* all the targets */
	int 		rule;		/* rule number */
	Reprog		*pat;		/* reg exp goo */
	char		*prog;		/* to use in out of date */
	struct Rule	*chain;		/* hashed per target */
	struct Rule	*next;
} Rule;

extern Rule *rules, *metarules, *patrule;

/*	Rule.attr	*/
#define		META		0x0001
#define		UNUSED		0x0002
#define		UPD		0x0004
#define		QUIET		0x0008
#define		VIR		0x0010
#define		REGEXP		0x0020
#define		NOREC		0x0040
#define		DEL		0x0080
#define		NOVIRT		0x0100

#define		NREGEXP		10

typedef struct Arc
{
	short		flag;
	struct Node	*n;
	Rule		*r;
	char		*stem;
	char		*prog;
	char		*match[NREGEXP];
	struct Arc	*next;
} Arc;

	/* Arc.flag */
#define		TOGO		1

typedef struct Node
{
	char		*name;
	long		time;
	unsigned short	flags;
	Arc		*prereqs;
	struct Node	*next;		/* list for a rule */
} Node;

	/* Node.flags */
#define		VIRTUAL		0x0001
#define		CYCLE		0x0002
#define		READY		0x0004
#define		CANPRETEND	0x0008
#define		PRETENDING	0x0010
#define		NOTMADE		0x0020
#define		BEINGMADE	0x0040
#define		MADE		0x0080
#define		MADESET(n,m)	n->flags = (n->flags&~(NOTMADE|BEINGMADE|MADE))|(m)
#define		PROBABLE	0x0100
#define		VACUOUS		0x0200
#define		NORECIPE	0x0400
#define		DELETE		0x0800
#define		NOMINUSE	0x1000

typedef struct Job
{
	Rule		*r;	/* master rule for job */
	Node		*n;	/* list of node targets */
	char		*stem;
	char		**match;
	Word		*p;	/* prerequistes */
	Word		*np;	/* new prerequistes */
	Word		*t;	/* targets */
	Word		*at;	/* all targets */
	int		nproc;	/* slot number */
	struct Job	*next;
} Job;
extern Job *jobs;

typedef struct Symtab
{
	short		space;
	char		*name;
	void		*value;
	struct Symtab	*next;
} Symtab;

enum {
	S_VAR,		/* variable -> value */
	S_TARGET,	/* target -> rule */
	S_TIME,		/* file -> time */
	S_PID,		/* pid -> products */
	S_NODE,		/* target name -> node */
	S_AGG,		/* aggregate -> time */
	S_BITCH,	/* bitched about aggregate not there */
	S_NOEXPORT,	/* var -> noexport */
	S_OVERRIDE,	/* can't override */
	S_OUTOFDATE,	/* n1\377n2 -> 2(outofdate) or 1(not outofdate) */
	S_MAKEFILE,	/* target -> node */
	S_MAKEVAR,	/* dumpable mk variable */
	S_EXPORTED,	/* var -> current exported value */
	S_WESET,	/* variable; we set in the mkfile */
	S_INTERNAL	/* an internal mk variable (e.g., stem, target) */
};

extern	int	debug;
extern	int	nflag, tflag, iflag, kflag, aflag, mflag;
extern	int	mkinline;
extern	char	*infile;
extern	int	nreps;
extern	char	*explain;

#define	SYNERR(l)	(fprint(2, "mk: %s:%d: syntax error; ", infile, ((l)>=0)?(l):mkinline))
#define	RERR(r)		(fprint(2, "mk: %s:%d: rule error; ", (r)->file, (r)->line))
#define	NAMEBLOCK	1000
#define	BIGBLOCK	20000

#define	SEP(c)		(((c)==' ')||((c)=='\t')||((c)=='\n'))
#define WORDCHR(r)	((r) > ' ' && !utfrune("!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", (r)))

#define	DEBUG(x)	(debug&(x))
#define		D_PARSE		0x01
#define		D_GRAPH		0x02
#define		D_EXEC		0x04

#define	LSEEK(f,o,p)	seek(f,o,p)

#define	PERCENT(ch)	(((ch) == '%') || ((ch) == '&'))

/* pad: Shell below allows to change the shell used by mk at runtime.
 * Thus, mk of kencc can be used to compile kencc itself and fork-plan9,
 * which have different requirements. I did that in mk-in-ocaml first.
 */
typedef struct Shell {
    char* shell;
    char* shellname;
    char *shflags;

    int IWS;
    char* termchars;

    char* (*charin)(char *cp, char *pat);
    char* (*expandquote)(char *s, Rune r, Bufblock *b);
    int (*escapetoken)(Biobuf *bp, Bufblock *buf, int preserve, int esc);
    char* (*copyq)(char *s, Rune q, Bufblock *buf);
} Shell;

extern Shell sh;
extern Shell rc;

extern Shell *shell;

#include	"fns.h"
