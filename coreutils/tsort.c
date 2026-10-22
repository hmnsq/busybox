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

#ଅନ୍ତର୍ଭାଵ "libbb.h"
#ଅନ୍ତର୍ଭାଵ "common_bufsiz.h"

ସଂରଚନା ଗଣ୍ଠି {
	unsigned in_ଗଣନା;
	unsigned out_ଗଣନା;
	ସଂରଚନା ଗଣ୍ଠି **out;
	ଅକ୍ଷର name[1];
};

ସଂରଚନା globals {
	ସଂରଚନା ଗଣ୍ଠି **nodes;
	unsigned nodes_len;
};
#define G (*(ସଂରଚନା globals*)bb_common_bufsiz1)
#define INIT_G() କର { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(ଆକାର(G) > COMMON_BUFSIZE); \
	G.nodes = ଖାଲି; \
	G.nodes_len = 0; \
} ଯେତେବେଳେ (0)

ସ୍ଥିର ସଂରଚନା ଗଣ୍ଠି *
get_ଗଣ୍ଠି(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା ଗଣ୍ଠି *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ଯେତେବେଳେ (a != b) {
		unsigned m = (a + b) / 2;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmp = strcmp(name, G.nodes[m]->name);
		ଯଦି (cmp == 0)
			ଫେରନ୍ତୁ G.nodes[m]; /* found */
		ଯଦି (cmp < 0) {
			b = m;
		} ଅନ୍ୟଥା {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(ଆକାର(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * ଆକାର(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * ଆକାର(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ଫେରନ୍ତୁ n;
}

ସ୍ଥିର ଶୂନ୍ୟ
add_edge(ସଂରଚନା ଗଣ୍ଠି *a, ସଂରଚନା ଗଣ୍ଠି *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_ଗଣନା);
	a->out[a->out_ଗଣନା++] = b;
	b->in_ଗଣନା++;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tsort_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା tsort_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc UNUSED_PARAM, ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *ଧାଡ଼ି;
	size_t linesz;
	ssize_t len;
	ସଂରଚନା ଗଣ୍ଠି *a;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cycles;
	unsigned i;
#ଯଦି ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	ଯଦି (argv[1]) {
		ଯଦି (argv[2])
			bb_show_usage();
		ଯଦି (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ଖାଲି;
	ଧାଡ଼ି = ଖାଲି;
	linesz = 0;
	ଯେତେବେଳେ ((len = getline(&ଧାଡ଼ି, &linesz, stdin)) != -1) {
		ଅକ୍ଷର *s = ଧାଡ଼ି;
		ଯେତେବେଳେ (*(s = skip_whitespace(s)) != '\0') {
			ସଂରଚନା ଗଣ୍ଠି *b;
			ଅକ୍ଷର *word;

			word = s;
			s = skip_non_whitespace(s);
			ଯଦି (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ଗଣ୍ଠି(word);
			ଯଦି (!a) {
				a = b;
			} ଅନ୍ୟଥା {
				ଯଦି (a != b)
					add_edge(a, b);
				a = ଖାଲି;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ଯଦି (a)
		bb_simple_error_ସନ୍ଦେଶ_and_die("odd input");
	ମୁକ୍ତ(ଧାଡ଼ି);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#ଯଦି ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ଯେତେବେଳେ (G.nodes_len) {
		ସଂରଚନା ଗଣ୍ଠି *n;

		/* Search for first node with no incoming edges */
		ପାଇଁ (i = 0; i < G.nodes_len; i++) {
			ଯଦି (!G.nodes[i]->in_ଗଣନା)
				ଭାଙ୍ଗ;
		}
		ଯଦି (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_ସନ୍ଦେଶ("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ଯଦି ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		ପାଇଁ (i = 0; i < n->out_ଗଣନା; i++)
			n->out[i]->in_ଗଣନା--;

		puts(n->name);
	}
#ଯଦି ENABLE_FEATURE_CLEAN_UP
	ପାଇଁ (i = 0; i < max_len; i++) {
		ମୁକ୍ତ(G.nodes[i]->out);
		ମୁକ୍ତ(G.nodes[i]);
	}
	ମୁକ୍ତ(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
