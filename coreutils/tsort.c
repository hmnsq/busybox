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
	अचिन्हित in_मोजणी;
	अचिन्हित out_मोजणी;
	संरचना गाठ **out;
	अक्षर name[1];
};

संरचना globals {
	संरचना गाठ **nodes;
	अचिन्हित nodes_len;
};
#define G (*(संरचना globals*)bb_common_bufsiz1)
#define INIT_G() करा { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(आकार(G) > COMMON_BUFSIZE); \
	G.nodes = रिक्त; \
	G.nodes_len = 0; \
} पर्यंत (0)

स्थिर संरचना गाठ *
get_गाठ(स्थिरांक अक्षर *name)
{
	संरचना गाठ *n;
	अचिन्हित a = 0;
	अचिन्हित b = G.nodes_len;

	/* Binary search for name */
	पर्यंत (a != b) {
		अचिन्हित m = (a + b) / 2;
		पूर्णांक cmp = म_भेद(name, G.nodes[m]->name);
		जर (cmp == 0)
			परत G.nodes[m]; /* found */
		जर (cmp < 0) {
			b = m;
		} अन्यथा {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(आकार(*n) + म_माप(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	म_नकल(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * आकार(*G.nodes));
	स_हटाओ(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * आकार(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	परत n;
}

स्थिर शून्य
add_edge(संरचना गाठ *a, संरचना गाठ *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_मोजणी);
	a->out[a->out_मोजणी++] = b;
	b->in_मोजणी++;
}

पूर्णांक tsort_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक tsort_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *ओळ;
	माप_प्रकार linesz;
	ssize_t len;
	संरचना गाठ *a;
	पूर्णांक cycles;
	अचिन्हित i;
#जर ENABLE_FEATURE_CLEAN_UP
	अचिन्हित max_len;
#endif

	INIT_G();

	जर (argv[1]) {
		जर (argv[2])
			bb_show_usage();
		जर (NOT_LONE_DASH(argv[1])) {
			बंद(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = रिक्त;
	ओळ = रिक्त;
	linesz = 0;
	पर्यंत ((len = getline(&ओळ, &linesz, मानक_निवेश)) != -1) {
		अक्षर *s = ओळ;
		पर्यंत (*(s = skip_whitespace(s)) != '\0') {
			संरचना गाठ *b;
			अक्षर *word;

			word = s;
			s = skip_non_whitespace(s);
			जर (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_गाठ(word);
			जर (!a) {
				a = b;
			} अन्यथा {
				जर (a != b)
					add_edge(a, b);
				a = रिक्त;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	जर (a)
		bb_simple_error_संदेश_and_die("odd input");
	मुक्त(ओळ);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#जर ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#endif
	पर्यंत (G.nodes_len) {
		संरचना गाठ *n;

		/* Search for first node with no incoming edges */
		साठी (i = 0; i < G.nodes_len; i++) {
			जर (!G.nodes[i]->in_मोजणी)
				थांबा;
		}
		जर (i == G.nodes_len) {
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
#जर ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#endif

		/* And remove its outgoing edges */
		साठी (i = 0; i < n->out_मोजणी; i++)
			n->out[i]->in_मोजणी--;

		माला_दो(n->name);
	}
#जर ENABLE_FEATURE_CLEAN_UP
	साठी (i = 0; i < max_len; i++) {
		मुक्त(G.nodes[i]->out);
		मुक्त(G.nodes[i]);
	}
	मुक्त(G.nodes);
#endif

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
