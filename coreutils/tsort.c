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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

संरचना गाठ {
	unsigned in_मोजणी;
	unsigned out_मोजणी;
	संरचना गाठ **out;
	वर्ण name[1];
};

संरचना globals {
	संरचना गाठ **nodes;
	unsigned nodes_len;
};
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() कुरु { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(परिमाण(G) > COMMON_BUFSIZE); \
	G.nodes = रिक्त; \
	G.nodes_len = 0; \
} यावत् (0)

स्थिर संरचना गाठ *
get_गाठ(ध्रुव वर्ण *name)
{
	संरचना गाठ *n;
	unsigned a = 0;
	unsigned b = G.nodes_len;

	/* Binary search for name */
	यावत् (a != b) {
		unsigned m = (a + b) / 2;
		पूर्णाङ्क cmp = strcmp(name, G.nodes[m]->name);
		यदि (cmp == 0)
			प्रत्यागम G.nodes[m]; /* found */
		यदि (cmp < 0) {
			b = m;
		} अन्यथा {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(परिमाण(*n) + strlen(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * परिमाण(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * परिमाण(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	प्रत्यागम n;
}

स्थिर शून्य
add_edge(संरचना गाठ *a, संरचना गाठ *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_मोजणी);
	a->out[a->out_मोजणी++] = b;
	b->in_मोजणी++;
}

पूर्णाङ्क tsort_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tsort_main(पूर्णाङ्क argc UNUSED_PARAM, वर्ण **argv)
{
	वर्ण *ओळ;
	size_t linesz;
	ssize_t len;
	संरचना गाठ *a;
	पूर्णाङ्क cycles;
	unsigned i;
#यदि ENABLE_FEATURE_CLEAN_UP
	unsigned max_len;
#endif

	INIT_G();

	यदि (argv[1]) {
		यदि (argv[2])
			bb_show_usage();
		यदि (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = रिक्त;
	ओळ = रिक्त;
	linesz = 0;
	यावत् ((len = getline(&ओळ, &linesz, stdin)) != -1) {
		वर्ण *s = ओळ;
		यावत् (*(s = skip_whitespace(s)) != '\0') {
			संरचना गाठ *b;
			वर्ण *word;

			word = s;
			s = skip_non_whitespace(s);
			यदि (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_गाठ(word);
			यदि (!a) {
				a = b;
			} अन्यथा {
				यदि (a != b)
					add_edge(a, b);
				a = रिक्त;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	यदि (a)
		bb_simple_error_संदेश_and_die("odd input");
	मुक्त(ओळ);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#यदि ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	यावत् (G.nodes_len) {
		संरचना गाठ *n;

		/* Search for first node with no incoming edges */
		प्रति (i = 0; i < G.nodes_len; i++) {
			यदि (!G.nodes[i]->in_मोजणी)
				भङ्ग;
		}
		यदि (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#ifndef TINY
			bb_error_संदेश("cycle at %s", G.nodes[i]->name);
#endif
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#यदि ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		प्रति (i = 0; i < n->out_मोजणी; i++)
			n->out[i]->in_मोजणी--;

		puts(n->name);
	}
#यदि ENABLE_FEATURE_CLEAN_UP
	प्रति (i = 0; i < max_len; i++) {
		मुक्त(G.nodes[i]->out);
		मुक्त(G.nodes[i]);
	}
	मुक्त(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
