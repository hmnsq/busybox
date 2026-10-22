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

#অন্তর্ভাব "libbb.h"
#অন্তর্ভাব "common_bufsiz.h"

কাঠামো গ্রন্থি {
	unsigned in_গুনতি;
	unsigned out_গুনতি;
	কাঠামো গ্রন্থি **out;
	অক্ষর name[1];
};

কাঠামো globals {
	কাঠামো গ্রন্থি **nodes;
	unsigned nodes_len;
};
#define G (*(কাঠামো globals*)bb_common_bufsiz1)
#define INIT_G() করো { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(আকার(G) > COMMON_BUFSIZE); \
	G.nodes = ফাঁকা; \
	G.nodes_len = 0; \
} যখন (0)

স্থির কাঠামো গ্রন্থি *
get_গ্রন্থি(ধ্রুবক অক্ষর *name)
{
	কাঠামো গ্রন্থি *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	যখন (a != b) {
		unsigned m = (a + b) / 2;
		পূর্ণসংখ্যা cmp = strcmp(name, G.nodes[m]->name);
		যদি (cmp == 0)
			ফেরত G.nodes[m]; /* found */
		যদি (cmp < 0) {
			b = m;
		} অন্যথা {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(আকার(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * আকার(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * আকার(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ফেরত n;
}

স্থির শূন্য
add_edge(কাঠামো গ্রন্থি *a, কাঠামো গ্রন্থি *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_গুনতি);
	a->out[a->out_গুনতি++] = b;
	b->in_গুনতি++;
}

পূর্ণসংখ্যা tsort_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা tsort_main(পূর্ণসংখ্যা argc UNUSED_PARAM, অক্ষর **argv)
{
	অক্ষর *সারি;
	আকার_t linesz;
	ssize_t len;
	কাঠামো গ্রন্থি *a;
	পূর্ণসংখ্যা cycles;
	unsigned i;
#যদি ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	যদি (argv[1]) {
		যদি (argv[2])
			bb_show_usage();
		যদি (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ফাঁকা;
	সারি = ফাঁকা;
	linesz = 0;
	যখন ((len = getline(&সারি, &linesz, stdin)) != -1) {
		অক্ষর *s = সারি;
		যখন (*(s = skip_whitespace(s)) != '\0') {
			কাঠামো গ্রন্থি *b;
			অক্ষর *word;

			word = s;
			s = skip_non_whitespace(s);
			যদি (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_গ্রন্থি(word);
			যদি (!a) {
				a = b;
			} অন্যথা {
				যদি (a != b)
					add_edge(a, b);
				a = ফাঁকা;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	যদি (a)
		bb_simple_error_বার্তা_and_die("odd input");
	মুক্ত(সারি);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#যদি ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	যখন (G.nodes_len) {
		কাঠামো গ্রন্থি *n;

		/* Search for first node with no incoming edges */
		জন্য (i = 0; i < G.nodes_len; i++) {
			যদি (!G.nodes[i]->in_গুনতি)
				বিরতি;
		}
		যদি (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_বার্তা("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#যদি ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		জন্য (i = 0; i < n->out_গুনতি; i++)
			n->out[i]->in_গুনতি--;

		puts(n->name);
	}
#যদি ENABLE_FEATURE_CLEAN_UP
	জন্য (i = 0; i < max_len; i++) {
		মুক্ত(G.nodes[i]->out);
		মুক্ত(G.nodes[i]);
	}
	মুক্ত(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
