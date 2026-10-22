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

#include "libbb.h"
#include "common_bufsiz.h"

sa_nracanaa node {
	unsigned in_count;
	unsigned out_count;
	sa_nracanaa node **out;
	var_na name[1];
};

sa_nracanaa globals {
	sa_nracanaa node **nodes;
	unsigned nodes_len;
};
#define G (*(sa_nracanaa globals*)bb_common_bufsiz1)
#define INIT_G() do { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(parimaa_na(G) > COMMON_BUFSIZE); \
	G.nodes = NULL; \
	G.nodes_len = 0; \
} ᰀᰬ (0)

static sa_nracanaa node *
get_node(const var_na *name)
{
	sa_nracanaa node *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ᰀᰬ (a != b) {
		unsigned m = (a + b) / 2;
		puur_naa_nka cmp = strcmp(name, G.nodes[m]->name);
		ᰀᰧ (cmp == 0)
			ᰀᰧᰌ G.nodes[m]; /* found */
		ᰀᰧ (cmp < 0) {
			b = m;
		} ᰀᰧᰀ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(parimaa_na(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * parimaa_na(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * parimaa_na(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ᰀᰧᰌ n;
}

static void
add_edge(sa_nracanaa node *a, sa_nracanaa node *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_count);
	a->out[a->out_count++] = b;
	b->in_count++;
}

puur_naa_nka tsort_main(puur_naa_nka argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
puur_naa_nka tsort_main(puur_naa_nka argc UNUSED_PARAM, var_na **argv)
{
	var_na *line;
	size_t linesz;
	ssize_t len;
	sa_nracanaa node *a;
	puur_naa_nka cycles;
	unsigned i;
#ᰀᰧ ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	ᰀᰧ (argv[1]) {
		ᰀᰧ (argv[2])
			bb_show_usage();
		ᰀᰧ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = NULL;
	line = NULL;
	linesz = 0;
	ᰀᰬ ((len = getline(&line, &linesz, stdin)) != -1) {
		var_na *s = line;
		ᰀᰬ (*(s = skip_whitespace(s)) != '\0') {
			sa_nracanaa node *b;
			var_na *word;

			word = s;
			s = skip_non_whitespace(s);
			ᰀᰧ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_node(word);
			ᰀᰧ (!a) {
				a = b;
			} ᰀᰧᰀ {
				ᰀᰧ (a != b)
					add_edge(a, b);
				a = NULL;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ᰀᰧ (a)
		bb_simple_error_msg_and_die("odd input");
	free(line);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#ᰀᰧ ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ᰀᰬ (G.nodes_len) {
		sa_nracanaa node *n;

		/* Search for first node with no incoming edges */
		for (i = 0; i < G.nodes_len; i++) {
			ᰀᰧ (!G.nodes[i]->in_count)
				bha_nga;
		}
		ᰀᰧ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_msg("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ᰀᰧ ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		for (i = 0; i < n->out_count; i++)
			n->out[i]->in_count--;

		puts(n->name);
	}
#ᰀᰧ ENABLE_FEATURE_CLEAN_UP
	for (i = 0; i < max_len; i++) {
		free(G.nodes[i]->out);
		free(G.nodes[i]);
	}
	free(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
