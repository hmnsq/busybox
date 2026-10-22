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

#અન્તર્ભાવ "libbb.h"
#અન્તર્ભાવ "common_bufsiz.h"

સંરચના ગાંઠ {
	unsigned in_ગણતરી;
	unsigned out_ગણતરી;
	સંરચના ગાંઠ **out;
	અક્ષર name[1];
};

સંરચના globals {
	સંરચના ગાંઠ **nodes;
	unsigned nodes_len;
};
#define G (*(સંરચના globals*)bb_common_bufsiz1)
#define INIT_G() કરો { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(કદ(G) > COMMON_BUFSIZE); \
	G.nodes = ખાલી; \
	G.nodes_len = 0; \
} જ્યારે (0)

સ્થિર સંરચના ગાંઠ *
get_ગાંઠ(અચલ અક્ષર *name)
{
	સંરચના ગાંઠ *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	જ્યારે (a != b) {
		unsigned m = (a + b) / 2;
		પૂર્ણાંક cmp = strcmp(name, G.nodes[m]->name);
		જો (cmp == 0)
			પાછા G.nodes[m]; /* found */
		જો (cmp < 0) {
			b = m;
		} નહીંતર {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(કદ(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * કદ(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * કદ(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	પાછા n;
}

સ્થિર શૂન્ય
add_edge(સંરચના ગાંઠ *a, સંરચના ગાંઠ *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_ગણતરી);
	a->out[a->out_ગણતરી++] = b;
	b->in_ગણતરી++;
}

પૂર્ણાંક tsort_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક tsort_main(પૂર્ણાંક argc UNUSED_PARAM, અક્ષર **argv)
{
	અક્ષર *લીટી;
	size_t linesz;
	ssize_t len;
	સંરચના ગાંઠ *a;
	પૂર્ણાંક cycles;
	unsigned i;
#જો ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	જો (argv[1]) {
		જો (argv[2])
			bb_show_usage();
		જો (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ખાલી;
	લીટી = ખાલી;
	linesz = 0;
	જ્યારે ((len = getline(&લીટી, &linesz, stdin)) != -1) {
		અક્ષર *s = લીટી;
		જ્યારે (*(s = skip_whitespace(s)) != '\0') {
			સંરચના ગાંઠ *b;
			અક્ષર *word;

			word = s;
			s = skip_non_whitespace(s);
			જો (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ગાંઠ(word);
			જો (!a) {
				a = b;
			} નહીંતર {
				જો (a != b)
					add_edge(a, b);
				a = ખાલી;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	જો (a)
		bb_simple_error_સંદેશ_and_die("odd input");
	મુક્ત(લીટી);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#જો ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	જ્યારે (G.nodes_len) {
		સંરચના ગાંઠ *n;

		/* Search for first node with no incoming edges */
		માટે (i = 0; i < G.nodes_len; i++) {
			જો (!G.nodes[i]->in_ગણતરી)
				તોડો;
		}
		જો (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_સંદેશ("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#જો ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		માટે (i = 0; i < n->out_ગણતરી; i++)
			n->out[i]->in_ગણતરી--;

		puts(n->name);
	}
#જો ENABLE_FEATURE_CLEAN_UP
	માટે (i = 0; i < max_len; i++) {
		મુક્ત(G.nodes[i]->out);
		મુક્ત(G.nodes[i]);
	}
	મુક્ત(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
