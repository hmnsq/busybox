/* vi: set sw=4 ts=4: */
/*
 * tsort implementation for busybox
 *
 * public domain -- David Leonard, 2022
 */
//config:config TSORT
//config:	bool "tsort (2.6 kb)"
//config:	default y
//config:	help
//config:	tsort performs a topological sort.

//applet:IF_TSORT(APPLET(tsort, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TSORT) += tsort.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/tsort.html */

//usage:#define tsort_trivial_usage
//usage:       "[FILE]"
//usage:#define tsort_full_usage "\n\n"
//usage:       "Topological sort"
//usage:#define tsort_example_usage
//usage:       "$ echo -e \"a b\\nb c\" | tsort\n"
//usage:       "a\n"
//usage:       "b\n"
//usage:       "c\n"

#അന്തര്ഭാവ "libbb.h"
#അന്തര്ഭാവ "common_bufsiz.h"

ഘടന കണ്ണി {
	unsigned in_എണ്ണം;
	unsigned out_എണ്ണം;
	ഘടന കണ്ണി **out;
	അക്ഷരം name[1];
};

ഘടന globals {
	ഘടന കണ്ണി **nodes;
	unsigned nodes_len;
};
#define G (*(ഘടന globals*)bb_common_bufsiz1)
#define INIT_G() ചെയ്യുക { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(വലുപ്പം(G) > COMMON_BUFSIZE); \
	G.nodes = ശൂന്യ; \
	G.nodes_len = 0; \
} ആകുമ്പോൾ (0)

സ്ഥിതം ഘടന കണ്ണി *
get_കണ്ണി(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന കണ്ണി *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ആകുമ്പോൾ (a != b) {
		unsigned m = (a + b) / 2;
		പൂർണ്ണസംഖ്യ cmp = strcmp(name, G.nodes[m]->name);
		എങ്കിൽ (cmp == 0)
			തിരികെ G.nodes[m]; /* found */
		എങ്കിൽ (cmp < 0) {
			b = m;
		} അല്ലെങ്കിൽ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(വലുപ്പം(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * വലുപ്പം(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * വലുപ്പം(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	തിരികെ n;
}

സ്ഥിതം ശൂന്യം
add_edge(ഘടന കണ്ണി *a, ഘടന കണ്ണി *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_എണ്ണം);
	a->out[a->out_എണ്ണം++] = b;
	b->in_എണ്ണം++;
}

പൂർണ്ണസംഖ്യ tsort_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ tsort_main(പൂർണ്ണസംഖ്യ argc UNUSED_PARAM, അക്ഷരം **argv)
{
	അക്ഷരം *വരി;
	size_t linesz;
	ssize_t len;
	ഘടന കണ്ണി *a;
	പൂർണ്ണസംഖ്യ cycles;
	unsigned i;
#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	എങ്കിൽ (argv[1]) {
		എങ്കിൽ (argv[2])
			bb_show_usage();
		എങ്കിൽ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ശൂന്യ;
	വരി = ശൂന്യ;
	linesz = 0;
	ആകുമ്പോൾ ((len = getline(&വരി, &linesz, stdin)) != -1) {
		അക്ഷരം *s = വരി;
		ആകുമ്പോൾ (*(s = skip_whitespace(s)) != '\0') {
			ഘടന കണ്ണി *b;
			അക്ഷരം *word;

			word = s;
			s = skip_non_whitespace(s);
			എങ്കിൽ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_കണ്ണി(word);
			എങ്കിൽ (!a) {
				a = b;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ (a != b)
					add_edge(a, b);
				a = ശൂന്യ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	എങ്കിൽ (a)
		bb_simple_error_സന്ദേശം_and_die("odd input");
	സ്വതന്ത്രം(വരി);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ആകുമ്പോൾ (G.nodes_len) {
		ഘടന കണ്ണി *n;

		/* Search for first node with no incoming edges */
		വേണ്ടി (i = 0; i < G.nodes_len; i++) {
			എങ്കിൽ (!G.nodes[i]->in_എണ്ണം)
				നിർത്തുക;
		}
		എങ്കിൽ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_സന്ദേശം("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		വേണ്ടി (i = 0; i < n->out_എണ്ണം; i++)
			n->out[i]->in_എണ്ണം--;

		puts(n->name);
	}
#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
	വേണ്ടി (i = 0; i < max_len; i++) {
		സ്വതന്ത്രം(G.nodes[i]->out);
		സ്വതന്ത്രം(G.nodes[i]);
	}
	സ്വതന്ത്രം(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
