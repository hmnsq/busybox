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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

ساختار گره {
	unsigned in_شمار;
	unsigned out_شمار;
	ساختار گره **out;
	نویسه name[1];
};

ساختار globals {
	ساختار گره **nodes;
	unsigned nodes_len;
};
#define G (*(ساختار globals*)bb_common_bufsiz1)
#define INIT_G() انجامبده { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(اندازه(G) > COMMON_BUFSIZE); \
	G.nodes = پوچ; \
	G.nodes_len = 0; \
} وقتی (0)

ایستا ساختار گره *
get_گره(ثابت نویسه *name)
{
	ساختار گره *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	وقتی (a != b) {
		unsigned m = (a + b) / 2;
		صحیح cmp = strcmp(name, G.nodes[m]->name);
		اگر (cmp == 0)
			بازگشت G.nodes[m]; /* found */
		اگر (cmp < 0) {
			b = m;
		} وگرنه {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(اندازه(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * اندازه(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * اندازه(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	بازگشت n;
}

ایستا تهی
add_edge(ساختار گره *a, ساختار گره *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_شمار);
	a->out[a->out_شمار++] = b;
	b->in_شمار++;
}

صحیح tsort_main(صحیح argc, نویسه **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح tsort_main(صحیح argc UNUSED_PARAM, نویسه **argv)
{
	نویسه *سطر;
	size_t linesz;
	ssize_t len;
	ساختار گره *a;
	صحیح cycles;
	unsigned i;
#اگر ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	اگر (argv[1]) {
		اگر (argv[2])
			bb_show_usage();
		اگر (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = پوچ;
	سطر = پوچ;
	linesz = 0;
	وقتی ((len = getline(&سطر, &linesz, stdin)) != -1) {
		نویسه *s = سطر;
		وقتی (*(s = skip_whitespace(s)) != '\0') {
			ساختار گره *b;
			نویسه *word;

			word = s;
			s = skip_non_whitespace(s);
			اگر (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_گره(word);
			اگر (!a) {
				a = b;
			} وگرنه {
				اگر (a != b)
					add_edge(a, b);
				a = پوچ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	اگر (a)
		bb_simple_error_پیام_and_die("odd input");
	آزاد(سطر);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#اگر ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	وقتی (G.nodes_len) {
		ساختار گره *n;

		/* Search for first node with no incoming edges */
		برای (i = 0; i < G.nodes_len; i++) {
			اگر (!G.nodes[i]->in_شمار)
				بشکن;
		}
		اگر (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_پیام("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#اگر ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		برای (i = 0; i < n->out_شمار; i++)
			n->out[i]->in_شمار--;

		puts(n->name);
	}
#اگر ENABLE_FEATURE_CLEAN_UP
	برای (i = 0; i < max_len; i++) {
		آزاد(G.nodes[i]->out);
		آزاد(G.nodes[i]);
	}
	آزاد(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
