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

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ಸಂರಚನಾ ಗಂಟು {
	unsigned in_ಎಣಿಕೆ;
	unsigned out_ಎಣಿಕೆ;
	ಸಂರಚನಾ ಗಂಟು **out;
	ವರ್ಣ name[1];
};

ಸಂರಚನಾ globals {
	ಸಂರಚನಾ ಗಂಟು **nodes;
	unsigned nodes_len;
};
#define G (*(ಸಂರಚನಾ globals*)bb_common_bufsiz1)
#define INIT_G() ಕುರು { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(ಪರಿಮಾಣ(G) > COMMON_BUFSIZE); \
	G.nodes = ರಿಕ್ತ; \
	G.nodes_len = 0; \
} ಯಾವತ್ (0)

ಸ್ಥಿರ ಸಂರಚನಾ ಗಂಟು *
get_ಗಂಟು(ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ ಗಂಟು *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ಯಾವತ್ (a != b) {
		unsigned m = (a + b) / 2;
		ಪೂರ್ಣಾಙ್ಕ cmp = strcmp(name, G.nodes[m]->name);
		ಯದಿ (cmp == 0)
			ಪ್ರತ್ಯಾಗಮ G.nodes[m]; /* found */
		ಯದಿ (cmp < 0) {
			b = m;
		} ಅನ್ಯಥಾ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(ಪರಿಮಾಣ(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * ಪರಿಮಾಣ(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * ಪರಿಮಾಣ(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ಪ್ರತ್ಯಾಗಮ n;
}

ಸ್ಥಿರ ಶೂನ್ಯ
add_edge(ಸಂರಚನಾ ಗಂಟು *a, ಸಂರಚನಾ ಗಂಟು *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_ಎಣಿಕೆ);
	a->out[a->out_ಎಣಿಕೆ++] = b;
	b->in_ಎಣಿಕೆ++;
}

ಪೂರ್ಣಾಙ್ಕ tsort_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಙ್ಕ tsort_main(ಪೂರ್ಣಾಙ್ಕ argc UNUSED_PARAM, ವರ್ಣ **argv)
{
	ವರ್ಣ *ಸಾಲು;
	size_t linesz;
	ssize_t len;
	ಸಂರಚನಾ ಗಂಟು *a;
	ಪೂರ್ಣಾಙ್ಕ cycles;
	unsigned i;
#ಯದಿ ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	ಯದಿ (argv[1]) {
		ಯದಿ (argv[2])
			bb_show_usage();
		ಯದಿ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ರಿಕ್ತ;
	ಸಾಲು = ರಿಕ್ತ;
	linesz = 0;
	ಯಾವತ್ ((len = getline(&ಸಾಲು, &linesz, stdin)) != -1) {
		ವರ್ಣ *s = ಸಾಲು;
		ಯಾವತ್ (*(s = skip_whitespace(s)) != '\0') {
			ಸಂರಚನಾ ಗಂಟು *b;
			ವರ್ಣ *word;

			word = s;
			s = skip_non_whitespace(s);
			ಯದಿ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ಗಂಟು(word);
			ಯದಿ (!a) {
				a = b;
			} ಅನ್ಯಥಾ {
				ಯದಿ (a != b)
					add_edge(a, b);
				a = ರಿಕ್ತ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ಯದಿ (a)
		bb_simple_error_ಸಂದೇಶ_and_die("odd input");
	ಮುಕ್ತ(ಸಾಲು);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#ಯದಿ ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ಯಾವತ್ (G.nodes_len) {
		ಸಂರಚನಾ ಗಂಟು *n;

		/* Search for first node with no incoming edges */
		ಪ್ರತಿ (i = 0; i < G.nodes_len; i++) {
			ಯದಿ (!G.nodes[i]->in_ಎಣಿಕೆ)
				ಭಙ್ಗ;
		}
		ಯದಿ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_ಸಂದೇಶ("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ಯದಿ ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		ಪ್ರತಿ (i = 0; i < n->out_ಎಣಿಕೆ; i++)
			n->out[i]->in_ಎಣಿಕೆ--;

		puts(n->name);
	}
#ಯದಿ ENABLE_FEATURE_CLEAN_UP
	ಪ್ರತಿ (i = 0; i < max_len; i++) {
		ಮುಕ್ತ(G.nodes[i]->out);
		ಮುಕ್ತ(G.nodes[i]);
	}
	ಮುಕ್ತ(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
