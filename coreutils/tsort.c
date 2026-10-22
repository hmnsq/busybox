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
} ໃນຂະນະທີ່ (0)

static sa_nracanaa node *
get_node(const var_na *name)
{
	sa_nracanaa node *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ໃນຂະນະທີ່ (a != b) {
		unsigned m = (a + b) / 2;
		ຈຳນວນເຕັມ cmp = strcmp(name, G.nodes[m]->name);
		ຖ້າ (cmp == 0)
			ສົ່ງຄືນ G.nodes[m]; /* found */
		ຖ້າ (cmp < 0) {
			b = m;
		} ບໍ່ດັ່ງນັ້ນ {
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
	ສົ່ງຄືນ n;
}

static void
add_edge(sa_nracanaa node *a, sa_nracanaa node *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_count);
	a->out[a->out_count++] = b;
	b->in_count++;
}

ຈຳນວນເຕັມ tsort_main(ຈຳນວນເຕັມ argc, var_na **argv) MAIN_EXTERNALLY_VISIBLE;
ຈຳນວນເຕັມ tsort_main(ຈຳນວນເຕັມ argc UNUSED_PARAM, var_na **argv)
{
	var_na *line;
	size_t linesz;
	ssize_t len;
	sa_nracanaa node *a;
	ຈຳນວນເຕັມ cycles;
	unsigned i;
#ຖ້າ ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	ຖ້າ (argv[1]) {
		ຖ້າ (argv[2])
			bb_show_usage();
		ຖ້າ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = NULL;
	line = NULL;
	linesz = 0;
	ໃນຂະນະທີ່ ((len = getline(&line, &linesz, stdin)) != -1) {
		var_na *s = line;
		ໃນຂະນະທີ່ (*(s = skip_whitespace(s)) != '\0') {
			sa_nracanaa node *b;
			var_na *word;

			word = s;
			s = skip_non_whitespace(s);
			ຖ້າ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_node(word);
			ຖ້າ (!a) {
				a = b;
			} ບໍ່ດັ່ງນັ້ນ {
				ຖ້າ (a != b)
					add_edge(a, b);
				a = NULL;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ຖ້າ (a)
		bb_simple_error_msg_and_die("odd input");
	free(line);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#ຖ້າ ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ໃນຂະນະທີ່ (G.nodes_len) {
		sa_nracanaa node *n;

		/* Search for first node with no incoming edges */
		ສຳລັບ (i = 0; i < G.nodes_len; i++) {
			ຖ້າ (!G.nodes[i]->in_count)
				bha_nga;
		}
		ຖ້າ (i == G.nodes_len) {
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
#ຖ້າ ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		ສຳລັບ (i = 0; i < n->out_count; i++)
			n->out[i]->in_count--;

		puts(n->name);
	}
#ຖ້າ ENABLE_FEATURE_CLEAN_UP
	ສຳລັບ (i = 0; i < max_len; i++) {
		free(G.nodes[i]->out);
		free(G.nodes[i]);
	}
	free(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
