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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

بنية node {
	unsigned in_count;
	unsigned out_count;
	بنية node **out;
	حرف name[1];
};

بنية globals {
	بنية node **nodes;
	unsigned nodes_len;
};
#define G (*(بنية globals*)bb_common_bufsiz1)
#define INIT_G() ܥܒܕ { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = عدم; \
	G.nodes_len = 0; \
} ܟܡܐ (0)

ساكن بنية node *
get_node(ثابت حرف *name)
{
	بنية node *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	ܟܡܐ (a != b) {
		unsigned m = (a + b) / 2;
		صحيح cmp = strcmp(name, G.nodes[m]->name);
		ܐܢ (cmp == 0)
			ܦܢܐ G.nodes[m]; /* found */
		ܐܢ (cmp < 0) {
			b = m;
		} ܐܚܪܢܐ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(حجم(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * حجم(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * حجم(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ܦܢܐ n;
}

ساكن فراغ
add_edge(بنية node *a, بنية node *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_count);
	a->out[a->out_count++] = b;
	b->in_count++;
}

صحيح tsort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tsort_main(صحيح argc UNUSED_PARAM, حرف **argv)
{
	حرف *line;
	size_t linesz;
	ssize_t len;
	بنية node *a;
	صحيح cycles;
	unsigned i;
#ܐܢ ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	ܐܢ (argv[1]) {
		ܐܢ (argv[2])
			bb_show_usage();
		ܐܢ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = عدم;
	line = عدم;
	linesz = 0;
	ܟܡܐ ((len = getline(&line, &linesz, stdin)) != -1) {
		حرف *s = line;
		ܟܡܐ (*(s = skip_whitespace(s)) != '\0') {
			بنية node *b;
			حرف *word;

			word = s;
			s = skip_non_whitespace(s);
			ܐܢ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_node(word);
			ܐܢ (!a) {
				a = b;
			} ܐܚܪܢܐ {
				ܐܢ (a != b)
					add_edge(a, b);
				a = عدم;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ܐܢ (a)
		bb_simple_error_msg_and_die("odd input");
	حرر(line);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#ܐܢ ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	ܟܡܐ (G.nodes_len) {
		بنية node *n;

		/* Search for first node with no incoming edges */
		لكل (i = 0; i < G.nodes_len; i++) {
			ܐܢ (!G.nodes[i]->in_count)
				اكسر;
		}
		ܐܢ (i == G.nodes_len) {
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
#ܐܢ ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		لكل (i = 0; i < n->out_count; i++)
			n->out[i]->in_count--;

		puts(n->name);
	}
#ܐܢ ENABLE_FEATURE_CLEAN_UP
	لكل (i = 0; i < max_len; i++) {
		حرر(G.nodes[i]->out);
		حرر(G.nodes[i]);
	}
	حرر(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
