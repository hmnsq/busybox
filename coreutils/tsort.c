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

#அந்தர்பாவ "libbb.h"
#அந்தர்பாவ "common_bufsiz.h"

கட்டமைப்பு முனை {
	unsigned in_மொத்தம்;
	unsigned out_மொத்தம்;
	கட்டமைப்பு முனை **out;
	எழுத்து name[1];
};

கட்டமைப்பு globals {
	கட்டமைப்பு முனை **nodes;
	unsigned nodes_len;
};
#define G (*(கட்டமைப்பு globals*)bb_common_bufsiz1)
#define INIT_G() செய் { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(அளவு(G) > COMMON_BUFSIZE); \
	G.nodes = வெற்று; \
	G.nodes_len = 0; \
} போது (0)

நிலையான கட்டமைப்பு முனை *
get_முனை(மாறிலி எழுத்து *name)
{
	கட்டமைப்பு முனை *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	போது (a != b) {
		unsigned m = (a + b) / 2;
		முழு cmp = strcmp(name, G.nodes[m]->name);
		எனில் (cmp == 0)
			திரும்பு G.nodes[m]; /* found */
		எனில் (cmp < 0) {
			b = m;
		} இல்லையெனில் {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(அளவு(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * அளவு(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * அளவு(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	திரும்பு n;
}

நிலையான வெறுமை
add_edge(கட்டமைப்பு முனை *a, கட்டமைப்பு முனை *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_மொத்தம்);
	a->out[a->out_மொத்தம்++] = b;
	b->in_மொத்தம்++;
}

முழு tsort_main(முழு argc, எழுத்து **argv) MAIN_EXTERNALLY_VISIBLE;
முழு tsort_main(முழு argc UNUSED_PARAM, எழுத்து **argv)
{
	எழுத்து *வரி;
	size_t linesz;
	ssize_t len;
	கட்டமைப்பு முனை *a;
	முழு cycles;
	unsigned i;
#எனில் ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	எனில் (argv[1]) {
		எனில் (argv[2])
			bb_show_usage();
		எனில் (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = வெற்று;
	வரி = வெற்று;
	linesz = 0;
	போது ((len = getline(&வரி, &linesz, stdin)) != -1) {
		எழுத்து *s = வரி;
		போது (*(s = skip_whitespace(s)) != '\0') {
			கட்டமைப்பு முனை *b;
			எழுத்து *word;

			word = s;
			s = skip_non_whitespace(s);
			எனில் (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_முனை(word);
			எனில் (!a) {
				a = b;
			} இல்லையெனில் {
				எனில் (a != b)
					add_edge(a, b);
				a = வெற்று;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	எனில் (a)
		bb_simple_error_செய்தி_and_die("odd input");
	விடுவி(வரி);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#எனில் ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	போது (G.nodes_len) {
		கட்டமைப்பு முனை *n;

		/* Search for first node with no incoming edges */
		க்கு (i = 0; i < G.nodes_len; i++) {
			எனில் (!G.nodes[i]->in_மொத்தம்)
				நிறுத்து;
		}
		எனில் (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_செய்தி("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#எனில் ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		க்கு (i = 0; i < n->out_மொத்தம்; i++)
			n->out[i]->in_மொத்தம்--;

		puts(n->name);
	}
#எனில் ENABLE_FEATURE_CLEAN_UP
	க்கு (i = 0; i < max_len; i++) {
		விடுவி(G.nodes[i]->out);
		விடுவி(G.nodes[i]);
	}
	விடுவி(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
